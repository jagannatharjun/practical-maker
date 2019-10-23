#include "exporter.hpp"
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>
#include <memory>
#include <vector>

static const int textMargins = 12;  // in millimeters
static const int borderMargins = 2; // in millimeters

struct Practical {
    std::unique_ptr<QTextDocument> practical;
    QString footer;
    Practical(std::unique_ptr<QTextDocument> &&p, QString &&f)
        : practical(std::move(p)), footer(f) {}
};

static std::vector<Practical> practicals;

static double mmToPixels(QPrinter &printer, int mm) {
    return mm * 0.039370147 * printer.resolution();
}

static void paintPage(QPrinter &printer, int pageNumber, int pageCount, QPainter *painter,
                      QTextDocument *doc, const QRectF &textRect, qreal footerHeight,
                      const QString &footer) {
    // qDebug() << "Printing page" << pageNumber;
    const QSizeF pageSize = printer.paperRect().size();
    // qDebug() << "pageSize=" << pageSize;

    const double bm = mmToPixels(printer, borderMargins);
    const QRectF borderRect(bm, bm, pageSize.width() - 2 * bm, pageSize.height() - 2 * bm);
    // painter->drawRect(borderRect);

    painter->save();
    // textPageRect is the rectangle in the coordinate system of the QTextDocument, in pixels,
    // and starting at (0,0) for the first page. Second page is at y=doc->pageSize().height().
    const QRectF textPageRect(0, pageNumber * doc->pageSize().height(), doc->pageSize().width(),
                              doc->pageSize().height());
    // Clip the drawing so that the text of the other pages doesn't appear in the margins
    painter->setClipRect(textRect);
    // Translate so that 0,0 is now the page corner
    painter->translate(0, -textPageRect.top());
    // Translate so that 0,0 is the text rect corner
    painter->translate(textRect.left(), textRect.top());
    doc->drawContents(painter);
    painter->restore();

    // Footer: page number or "end"
    QRectF footerRect = textRect;
    footerRect.setTop(textRect.bottom());
    footerRect.setHeight(footerHeight);

    painter->drawText(footerRect, Qt::AlignBottom | Qt::AlignLeft, footer);
}

static void printPractical(QPrinter &printer, std::vector<Practical> &docs, QWidget *parentWidget) {
    QPainter painter(&printer);
    auto f = painter.font();
    f.setFamily("Source Code Pro");
    painter.setFont(f);
    QSizeF pageSize = printer.pageRect().size(); // page size in pixels
    // Calculate the rectangle where to lay out the text
    const double tm = mmToPixels(printer, textMargins);
    const qreal footerHeight = painter.fontMetrics().height() * 2;
    const QRectF textRect(tm, tm, pageSize.width() - 2 * tm,
                          pageSize.height() - 2 * tm - footerHeight);
    // qDebug() << "textRect=" << textRect;

    bool firstPage = true;
    for (auto &doc : docs) {
        doc.practical->setPageSize(textRect.size());

        const int pageCount = doc.practical->pageCount();
        for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
            if (!firstPage)
                printer.newPage();

            paintPage(printer, pageIndex, pageCount, &painter, doc.practical.get(), textRect,
                      footerHeight, doc.footer);
            firstPage = false;
        }
    }
}

QPrinter *getPdf(QString outputFileName, bool previous) {
    static QPrinter pdf;
    if (!previous || pdf.outputFileName().isEmpty()) {
        pdf.setPageSize(QPageSize(QPageSize::A4));
        pdf.setOutputFormat(QPrinter::OutputFormat::PdfFormat);
        pdf.setFontEmbeddingEnabled(true);
        if (QFile::exists(outputFileName) && !QFile::remove(outputFileName)) {
            QMessageBox::critical(nullptr, "Error", "Failed to remove existing pdf");
            return nullptr;
        }
        pdf.setOutputFileName(outputFileName);
    }
    return &pdf;
}

#include <QTextEdit>

void exportAsPdf(QString outputFileName, QString question, QString code, QString codeOutput,
                 QString footer, bool previous) {

    QPrinter *pdf = getPdf(outputFileName, previous);
    if (!pdf)
        return;

    if (!previous)
        practicals.clear();

    practicals.emplace_back(std::make_unique<QTextDocument>(), std::move(footer));
    QTextCursor *textCursor(new QTextCursor(practicals.back().practical.get()));

    QTextFrameFormat frameFormat;
    QTextCharFormat format;
    QFont font("Source Code Pro", 14);

    auto questionFrame = textCursor->insertFrame(frameFormat);
    auto questionCursor = new QTextCursor(questionFrame);

    font.setBold(true);
    format.setFont(font);
    questionCursor->insertText(question + "\n", format);

    auto codeFrame = textCursor->insertFrame(frameFormat);
    auto codeCursor = new QTextCursor(codeFrame);

    font.setBold(false);
    font.setPointSize(12);
    format.setFont(font);
    codeCursor->insertText(code + "\n", format);

    QPixmap codeBg(1024, 1024);
    {
        codeBg.fill(QColor(Qt::gray).darker());
        QPainter p;
        p.begin(&codeBg);
        p.fillRect(QRect(0, 0, 6, codeBg.height()), QBrush(Qt::black));
        p.end();
        frameFormat.setPadding(12);
    }

    frameFormat.setBackground(QBrush(codeBg));
    auto codeOutputFrame = textCursor->insertFrame(frameFormat);
    auto codeOutputCursor = new QTextCursor(codeOutputFrame);
    format.setForeground(QBrush(Qt::white));
    font.setBold(true);
    format.setFont(font);
    codeOutputCursor->insertText(codeOutput, format);

    //    QTextEdit *edt = new QTextEdit;
    //    edt->setDocument(practicals.back().practical.get());
    //    edt->show();

    printPractical(*pdf, practicals, nullptr);
}

void deletePreviousMerge() {
    QPrinter *pdf = getPdf({}, true);
    if (practicals.size())
        practicals.pop_back();
    printPractical(*pdf, practicals, nullptr);
}
