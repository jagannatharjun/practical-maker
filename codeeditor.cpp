#include "codeeditor.hpp"
#include "consolewidget.hpp"

#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QtDebug>

CodeEditor::CodeEditor(QWidget *parent) : QTextEdit(parent) {
    auto f = font();
    f.setFamily("Source Code Pro");
    f.setPointSize(14);
    setFont(f);

    setAcceptRichText(false);
}

void CodeEditor::compile() { _compile(); }

void CodeEditor::onlyRun() {
    m_console->show();
    m_console->start("./a.exe");
}

void CodeEditor::run() {
    if (_compile()) {
        onlyRun();
    }
}

bool CodeEditor::_compile() {
    m_console->show();
    m_console->setText("Compiling...");
    m_console->start("g++", {"-x", "c++", "-"});

    auto code = toPlainText().toUtf8();
    code.append("\n\n#include <stdio.h>\nstruct __CODE_EDITOR_DISABLE_IO_BUFFER { "
                "__CODE_EDITOR_DISABLE_IO_BUFFER() {setvbuf(stdout, NULL, _IONBF, 0);} } "
                "__CODE_EDITOR_DISABLE_IO_BUFFER_OBJ{};");

    m_console->childWrite(code);
    m_console->childCloseWrite();

    return m_console->exitCode() == 0;
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
    static QString keyToEat;

    if (event->matches(QKeySequence::StandardKey::ZoomIn)) {
        zoomIn();
    } else if (event->matches(QKeySequence::StandardKey::ZoomOut)) {
        zoomOut();
    } else if (!keyToEat.isEmpty() && event->text() == keyToEat) {
        accept = false;
        auto t = textCursor();
        t.setPosition(t.position() + 1);
        setTextCursor(t);
    } else {
        accept = true;
    }

    if (accept)
        QTextEdit::keyPressEvent(event);

    keyToEat.clear();
    for (auto keyPair : completekeys) {
        if (event->text() == keyPair.first) {
            insertPlainText(QString(keyPair.second));
            auto t = textCursor();
            t.setPosition(t.position() - 1);
            setTextCursor(t);
            keyToEat = keyPair.second;
            break;
        }
    }

    if (k == Qt::Key_Enter || k == Qt::Key_Return) {
        const auto initialText = toPlainText();
        const auto initialPos = textCursor().position();
        int i = identWidth(initialText, initialPos);
        QString s(i * 2, ' ');
        insertPlainText(s);
        if (initialPos < initialText.size() && initialText[initialPos] == '}') {
            auto s = '\n' + QString((i - 1) * 2, ' ');
            insertPlainText(s);
            auto t = textCursor();
            t.setPosition(t.position() - s.size());
            setTextCursor(t);
        }
    }
}

void CodeEditor::newEditor() { setPlainText({}); }

void CodeEditor::save() {
    if (m_fileName.isEmpty()) {
        saveAs();
        return;
    }
    QFile f(m_fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(
            this, "Error",
            tr("Failed to open \"%1\" for writing, error: %2").arg(m_fileName, f.errorString()));
        return;
    }
    f.write(toPlainText().toUtf8());
    f.close();
}

void CodeEditor::saveAs() {
    m_fileName = QFileDialog::getSaveFileName(this, "Save as");
    save();
}

void CodeEditor::open() {
    m_fileName = QFileDialog::getOpenFileName(this, "Open");
    if (m_fileName.isEmpty())
        return;
    QFile f(m_fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(
            this, "Error",
            tr("Failed to open \"%1\" for reading, error: %2").arg(m_fileName, f.errorString()));
        return;
    }
    setPlainText(f.readAll());
    f.close();
}
