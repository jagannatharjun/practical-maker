#ifndef PRACTICALFILE_HPP
#define PRACTICALFILE_HPP

#include <QObject>

class PracticalFile : public QObject
{
    Q_OBJECT
public:
    enum Language { CLang, CppLang };

    explicit PracticalFile(QObject *parent = nullptr);

signals:

public slots:

private:
    Language m_language;
};

#endif // PRACTICALFILE_HPP
