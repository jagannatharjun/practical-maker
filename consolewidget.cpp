#include "consolewidget.hpp"
#include <QDebug>
#include <QKeyEvent>
ConsoleWidget::ConsoleWidget(QWidget *parent) : QTextEdit(parent) {
    auto f = font();
    f.setFamily("Source Code Pro");
    f.setPointSize(11);
    setFont(f);

    m_lastWritePos = 0;

    connect(&m_process, &QProcess::readyReadStandardOutput, [this]() {
        append(m_process.readAllStandardOutput());
        m_lastWritePos = textCursor().position();
    });

    connect(&m_process, &QProcess::readyReadStandardError, [this]() {
        append(m_process.readAllStandardError());
        m_lastWritePos = textCursor().position();
    });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &ConsoleWidget::_processFinised);

    connect(&m_process, &QProcess::errorOccurred, [this](QProcess::ProcessError) {
        append("Encountered Error - " + m_process.errorString());
        append("Tried executing - " + m_process.program());
    });
}

void ConsoleWidget::start(const QString &cmd, const QStringList &args) {
    m_lastWritePos = 0;
    m_process.start(cmd, args);
}

void ConsoleWidget::childWrite(const QByteArray &data) { m_process.write(data); }

void ConsoleWidget::childCloseWrite() { m_process.closeWriteChannel(); }

void ConsoleWidget::_processFinised(int exitcode, QProcess::ExitStatus exitStatus) {
    append("Program exited with code " + QString::number(exitcode));
    if (exitStatus != QProcess::ExitStatus::NormalExit)
        append("\nError: " + m_process.errorString());
    emit processFinished(exitcode);
}

void ConsoleWidget::keyPressEvent(QKeyEvent *event) {
    bool accept;
    int key = event->key();
    if (key == Qt::Key_Backspace || event->key() == Qt::Key_Left) {
        accept = textCursor().position() > m_lastWritePos;
    } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        accept = false;
        int count = toPlainText().count() - m_lastWritePos;
        QString cmd = toPlainText().right(count) + "\n";
        qDebug() << m_process.write(cmd.toUtf8());
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
        QTextEdit::keyPressEvent(event);
    }
}

void ConsoleWidget::closeEvent(QCloseEvent *e) {
    m_process.kill();
    QTextEdit::closeEvent(e);
}
