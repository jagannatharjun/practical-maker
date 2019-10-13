#ifndef CODEEDITOR_HPP
#define CODEEDITOR_HPP

#include <QTextEdit>

class ConsoleWidget;

class CodeEditor : public QTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = nullptr);
    void setConsole(ConsoleWidget *edt) { m_console = edt; }

    bool compile();
    void run();

protected:
    void keyPressEvent(QKeyEvent *) override;

private:
    ConsoleWidget *m_console;
};

#endif // CODEEDITOR_HPP
