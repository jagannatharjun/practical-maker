#ifndef CONSOLEWIDGET_HPP
#define CONSOLEWIDGET_HPP

#include <QProcess>
#include <QTextCursor>
#include <QTextEdit>

class ConsoleWidget : public QTextEdit {
    Q_OBJECT
public:
    explicit ConsoleWidget(QWidget *parent = nullptr);
    void start(const QString &cmd, const QStringList &args = {});

    void childWrite(const QByteArray &data);
    void childCloseWrite();

signals:
    void processFinished(int exitcode);

private slots:
    void _processFinised(int exitcode, QProcess::ExitStatus);

private:
    int m_lastWritePos;
    QProcess m_process;

    void keyPressEvent(QKeyEvent *) override;
    void closeEvent(QCloseEvent *) override;
};

#endif // CONSOLEWIDGET_HPP
