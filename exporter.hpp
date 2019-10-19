#ifndef EXPORTER_HPP
#define EXPORTER_HPP

#include <QString>

void exportAsPdf(QString outputFileName, QString question, QString code, QString codeOutput,
                 QString footer, bool previous);
void deletePreviousMerge();

#endif // EXPORTER_HPP
