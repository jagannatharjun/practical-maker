#include "consolewidget.hpp"
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

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                append("Program exited with code " + QString::number(exitCode));
                if (exitStatus != QProcess::ExitStatus::NormalExit)
                    append("\nError: " + m_process.errorString());
            });
}

void ConsoleWidget::start(const QString &cmd, const QStringList &args) {
    setPlainText("");
    m_lastWritePos = 0;
    m_process.start(cmd, args);
}

void ConsoleWidget::childWrite(const QByteArray &data) { m_process.write(data); }

void ConsoleWidget::childCloseWrite() { m_process.closeWriteChannel(); }
#include <QGuiApplication>
int ConsoleWidget::exitCode() {
    while (!m_process.waitForFinished(100)) {
        qGuiApp->processEvents();
    }
    return m_process.exitCode();
}
#include <QDebug>
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

    qDebug() << accept;
}

void ConsoleWidget::closeEvent(QCloseEvent *) { m_process.kill(); }
