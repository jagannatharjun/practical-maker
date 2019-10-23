#include "codeeditor.hpp"
#include "consolewidget.hpp"

#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QtDebug>

QString myTab(int count) { return QString(count * 2, ' '); }

CodeEditor::CodeEditor(QWidget *parent) : QTextEdit(parent), m_clangFormat(new QProcess) {
    auto f = font();
    f.setFamily("Source Code Pro");
    f.setPointSize(14);
    setFont(f);

    setAcceptRichText(false);

    m_clangFormat->setProgram("clang-format");
}

void CodeEditor::compile() { _compile(); }

void CodeEditor::onlyRun() {
    if (m_console->isHidden())
        m_console->show();
    m_console->setPlainText("");
    m_console->start("./a.exe");
}

void CodeEditor::run() {
    connect(m_console, &ConsoleWidget::processFinished, this, &CodeEditor::_run);
    _compile();
}

void CodeEditor::format() {
    if (m_clangFormat->state() == QProcess::Running) {
        QMessageBox::warning(this, "Warning",
                             QString("%1 already running").arg(m_clangFormat->program()));
        return;
    }
    m_clangFormat->disconnect();
    auto postition = textCursor().position();
    auto formatedText = new QString();
    connect(m_clangFormat, &QProcess::readyReadStandardOutput, [this, formatedText]() {
        formatedText->append(m_clangFormat->readAllStandardOutput());
    });
    connect(m_clangFormat, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, postition, formatedText](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus == QProcess::ExitStatus::NormalExit && !formatedText->isEmpty()) {
                    this->setPlainText(*formatedText);
                    auto t = this->textCursor();
                    t.setPosition(postition);
                    this->setTextCursor(t);
                }
                delete formatedText;
            });
    m_clangFormat->start();
    if (!m_clangFormat->waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start clang-format");
        m_clangFormat->disconnect();
        delete formatedText;
        return;
    }
    m_clangFormat->write(toPlainText().toUtf8());
    m_clangFormat->closeWriteChannel();
}

void CodeEditor::_run(int compileExitCode) {
    if (compileExitCode == 0) {
        onlyRun();
    }
    disconnect(m_console, &ConsoleWidget::processFinished, this, &CodeEditor::_run);
}

CodeEditor::Language CodeEditor::lang() const { return m_lang; }

void CodeEditor::setLang(const CodeEditor::Language lang) { m_lang = lang; }

void CodeEditor::_compile() {

    m_console->show();
    auto code = toPlainText().toUtf8();

    m_console->setPlainText("Compiling....\n");
    if (m_lang == CppLang) {
        m_console->start("g++", {"-x", "c++", "--static", "-"});
        code.append("\n\n#include <stdio.h>\nstruct __CODE_EDITOR_DISABLE_IO_BUFFER { "
                    "__CODE_EDITOR_DISABLE_IO_BUFFER() {setvbuf(stdout, NULL, _IONBF, 0);} } "
                    "__CODE_EDITOR_DISABLE_IO_BUFFER_OBJ{};");
    } else {
        m_console->start("gcc", {"-x", "c", "--static", "-"});
        code.append("\n\n#include <stdio.h>\nvoid __attribute__ ((constructor)) "
                    "__CODE_EDITOR_DISABLE_IO_BUFFER() {setvbuf(stdout, NULL, _IONBF, 0);}");
    }
    m_console->childWrite(code);
    m_console->childCloseWrite();
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

    if (k == Qt::Key_Tab) {
        insertPlainText(myTab(1));
    } else if (event->matches(QKeySequence::StandardKey::ZoomIn)) {
        zoomIn();
    } else if (event->matches(QKeySequence::StandardKey::ZoomOut)) {
        zoomOut();
    } else if (!keyToEat.isEmpty() && event->text() == keyToEat) {
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
        insertPlainText(myTab(i));
        if (initialPos < initialText.size() && initialText[initialPos] == '}') {
            auto s = '\n' + myTab(i - 1);
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
    if (!m_fileName.isEmpty()) {
        save();
    }
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
