#ifndef CODEEDITOR_HPP
#define CODEEDITOR_HPP

#include <QPlainTextEdit>

class ConsoleWidget;
class QProcess;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    enum Language { CLang, CppLang };

    CodeEditor(QWidget *parent = nullptr);
    void setConsole(ConsoleWidget *edt) { m_console = edt; }

    Language lang() const;
    void setLang(const Language lang);

protected:
    void keyPressEvent(QKeyEvent *) override;

public slots:
    void newEditor();
    void save();
    void saveAs();
    void open();
    void compile();
    void onlyRun();
    void run();
    void format();

private slots:
    void _run(int compileExitCode);

private:
    ConsoleWidget *m_console;
    QString m_fileName;
    Language m_lang = Language::CppLang;
    QProcess *m_clangFormat;

    void _compile();
};

#endif // CODEEDITOR_HPP
