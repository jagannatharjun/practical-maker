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
    return w < 0 ? 0 : w;
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    bool accept = false;
    auto k = event->key();

    static auto completekeys = {std::pair{'{', '}'}, {'(', ')'}, {'[', ']'}};

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

    for (auto keyPair : completekeys) {
        if (event->text() == keyPair.first) {
            insertPlainText(QString(keyPair.second));
            auto t = textCursor();
            t.setPosition(t.position() - 1);
            setTextCursor(t);
            break;
        }
    }

    if (k == Qt::Key_Enter || k == Qt::Key_Return) {
        const auto initialText = toPlainText();
        const auto initialPos = textCursor().position();
        int i = identWidth(initialText, initialPos);
        QString s(i * 2, ' ');
        insertPlainText(s);
        if (initialPos < initialText.size() &&
            initialText[initialPos] == '}') {
            auto s = '\n' + QString((i - 1) * 2, ' ');
            insertPlainText(s);
            auto t = textCursor();
            t.setPosition(t.position() - s.size());
            setTextCursor(t);
        }
    }
}
