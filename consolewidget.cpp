#include "consolewidget.hpp"
#include <QDebug>
#include <QKeyEvent>

ConsoleWidget::ConsoleWidget(QWidget *parent) : QPlainTextEdit(parent) {
    auto f = font();
    f.setFamily("Source Code Pro");
    f.setPointSize(11);
    setFont(f);

    m_lastWritePos = 0;

    connect(&m_process, &QProcess::readyReadStandardOutput, [this]() {
        moveCursor(QTextCursor::End);
        insertPlainText(m_process.readAllStandardOutput());
        moveCursor(QTextCursor::End);
        m_lastWritePos = textCursor().position();
    });

    connect(&m_process, &QProcess::readyReadStandardError, [this]() {
        moveCursor(QTextCursor::End);
        insertPlainText(m_process.readAllStandardError());
        moveCursor(QTextCursor::End);
        m_lastWritePos = textCursor().position();
    });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &ConsoleWidget::_processFinised);
}

void ConsoleWidget::start(const QString &cmd, const QStringList &args) {
    m_lastWritePos = 0;
    qDebug() << cmd << args;
    m_process.start(cmd, args);
}

void ConsoleWidget::childWrite(const QByteArray &data) { m_process.write(data); }

void ConsoleWidget::childCloseWrite() { m_process.closeWriteChannel(); }

void ConsoleWidget::setWorkingDirectory(const QString &dir) { m_process.setWorkingDirectory(dir); }

void ConsoleWidget::_processFinised(int exitcode, QProcess::ExitStatus exitStatus) {
    appendPlainText("Program exited with code " + QString::number(exitcode));
    if (exitStatus != QProcess::ExitStatus::NormalExit)
        appendPlainText("\nError: " + m_process.errorString());
    emit processFinished(exitcode);
}

void ConsoleWidget::keyPressEvent(QKeyEvent *event) {
    bool accept;
    int key = event->key();
    if (key == Qt::Key_Backspace || event->key() == Qt::Key_Left) {
        accept = textCursor().position() > m_lastWritePos;
    } else if (key == Qt::Key_Up) {
        accept = false;
    } else if (event->matches(QKeySequence::StandardKey::ZoomIn)) {
        accept = false;
        zoomIn();
    } else if (event->matches(QKeySequence::StandardKey::ZoomOut)) {
        accept = false;
        zoomOut();
    } else {
        accept = textCursor().position() >= m_lastWritePos;
    }

    if (accept) {
        QPlainTextEdit::keyPressEvent(event);
    }

    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        auto text = toPlainText();
        int count = text.count() - m_lastWritePos;
        QString cmd = text.right(count);
        m_process.write(cmd.toUtf8());
        m_lastWritePos = textCursor().position();
    }
}

void ConsoleWidget::closeEvent(QCloseEvent *e) {
    m_process.kill();
    QPlainTextEdit::closeEvent(e);
}
