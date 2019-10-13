#include "codeeditor.hpp"
#include "consolewidget.hpp"

#include <QKeyEvent>
#include <QtDebug>

CodeEditor::CodeEditor(QWidget *parent) : QTextEdit(parent) {
    auto f = font();
    f.setFamily("Source Code Pro");
    f.setPointSize(16);
    setFont(f);
}

bool CodeEditor::compile() {
    m_console->start("g++", {"-x", "c++", "-"});

    m_console->childWrite(toPlainText().toUtf8());
    m_console->childCloseWrite();

    return m_console->exitCode() == 0;
}

void CodeEditor::run() {
    if (compile()) {
        m_console->start("./a.exe");
    }
}

int identWidth(const QString &s, int pos) {
    int w = 0;
    for (int i = 0; i < pos; i++) {
        if (s[i] == '{')
            w++;
        else if (s[i] == '}')
            w--;
    }
    if (pos < s.size() && s[pos] == '}')
        w--;
    return w < 0 ? 0 : w;
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    bool accept = false;
    auto k = event->key();
    if (event->matches(QKeySequence::StandardKey::ZoomIn)) {
        zoomIn();
    } else if (event->matches(QKeySequence::StandardKey::ZoomOut)) {
        zoomOut();
    } else if (k == Qt::Key_R && event->modifiers() & Qt::CTRL) {
        run();
    } else {
        accept = true;
    }

    if (accept)
        QTextEdit::keyPressEvent(event);

    if (k == Qt::Key_Enter || k == Qt::Key_Enter) {
        int i = identWidth(toPlainText(), textCursor().position());
        QString s(i * 2, ' ');
        insertPlainText(s);
    }
}
