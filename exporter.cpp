#include "exporter.hpp"
#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <QPrinter>
#include <QString>
#include <QTextDocument>

void makeHtmlPrintableText(QString &txt) {
    txt.replace('<', "&lt;");
    txt.replace('>', "&gt;");
}
static const int textMargins = 12;  // in millimeters
static const int borderMargins = 2; // in millimeters

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
    painter->drawRect(borderRect);

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

    painter->drawText(footerRect, Qt::AlignVCenter | Qt::AlignLeft, footer);
}

static void printDocument(QPrinter &printer, QTextDocument *doc, QWidget *parentWidget,
                          const QString &footer) {
    QPainter painter(&printer);
    auto f = painter.font();
    f.setFamily("Source Code Pro");
    painter.setFont(f);
    QSizeF pageSize = printer.pageRect().size(); // page size in pixels
    // Calculate the rectangle where to lay out the text
    const double tm = mmToPixels(printer, textMargins);
    const qreal footerHeight = painter.fontMetrics().height();
    const QRectF textRect(tm, tm, pageSize.width() - 2 * tm,
                          pageSize.height() - 2 * tm - footerHeight);
    // qDebug() << "textRect=" << textRect;
    doc->setPageSize(textRect.size());

    const int pageCount = doc->pageCount();

    bool firstPage = true;
    for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
        if (!firstPage)
            printer.newPage();

        paintPage(printer, pageIndex, pageCount, &painter, doc, textRect, footerHeight, footer);
        firstPage = false;
    }
}

void exportAsPdf(QString question, QString code, QString codeOutput, QString footer) {

    QPrinter pdf;
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setOutputFormat(QPrinter::OutputFormat::PdfFormat);
    pdf.setOutputFileName(QFileDialog::getSaveFileName(nullptr, "Export As", {}, "*.pdf"));
    pdf.setFullPage(true);

    makeHtmlPrintableText(question);
    makeHtmlPrintableText(code);
    makeHtmlPrintableText(codeOutput);

    const QString html = QString(R"(<font face="Source Code Pro" size=4><b>%1</b></font>
                   <pre><font size=4 face="Source Code Pro">%2</font></pre>
                   <br><div style='background-color: grey;'><pre><font size=3 face= "Courier New"><b>%3</b></font></pre></div>)")
                             .arg(question, code, codeOutput);

    QTextDocument doc;
    doc.setPageSize(pdf.pageLayout().fullRectPoints().size());
    doc.setHtml(html);
    // doc.print(&pdf);
    printDocument(pdf, &doc, nullptr, footer);
}
