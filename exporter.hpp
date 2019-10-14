#ifndef EXPORTER_HPP
#define EXPORTER_HPP

class QString;

void exportAsPdf(const QString &fileName, QString question, QString code,
                 QString codeOutput);

#endif // EXPORTER_HPP
