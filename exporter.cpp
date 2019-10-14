#include "exporter.hpp"
#include <QDebug>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QString>
#include <QTextDocument>

void makeHtmlPrintableText(QString &txt) {
    txt.replace('<', "&lt;");
    txt.replace('>', "&gt;");
}

#include <QHBoxLayout>
#include <QTextEdit>

void exportAsPdf(const QString &fileName, QString question, QString code, QString codeOutput) {

    QPdfWriter pdf(fileName);
    pdf.setPageSize(QPageSize(QPageSize::A4));

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
    doc.print(&pdf);

}
