#include "codeeditor.hpp"
#include "consolewidget.hpp"
#include "exporter.hpp"

#include <QApplication>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QStyleFactory>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("fusion"));
    QMainWindow m;

    QWidget c;
    m.setCentralWidget(&c);

    QVBoxLayout l;
    c.setLayout(&l);

    QLineEdit ques;
    auto f = ques.font();
    f.setFamily("Source Code Pro");
    f.setPointSize(11);
    ques.setFont(f);
    l.addWidget(&ques);

    CodeEditor edit;
    l.addWidget(&edit);

    ConsoleWidget console;
    edit.setConsole(&console);

    QLineEdit footer;
    footer.setFont(f);
    l.addWidget(&footer);

    QMenuBar menu;
    auto fileMenu = menu.addMenu("File");
    fileMenu->addAction("New", &edit, &CodeEditor::newEditor, QKeySequence::New);
    fileMenu->addAction("Open", &edit, &CodeEditor::open, QKeySequence::Open);
    fileMenu->addAction("Save", &edit, &CodeEditor::save, QKeySequence::Save);
    fileMenu->addAction("Save As", &edit, &CodeEditor::saveAs, QKeySequence::SaveAs);
    fileMenu->addAction("Export", [&]() {
        exportAsPdf(ques.text(), edit.toPlainText(), console.toPlainText(), footer.text());
    });
    fileMenu->addAction("Exit", qApp, &QApplication::closeAllWindows, QKeySequence::Quit);

    auto runMenu = menu.addMenu("Run");
    runMenu->addAction("Compile", &edit, &CodeEditor::compile, QKeySequence(Qt::CTRL + Qt::Key_B));
    runMenu->addAction("Run", &edit, &CodeEditor::onlyRun,
                       QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
    runMenu->addAction("Compile And Run", &edit, &CodeEditor::run,
                       QKeySequence(Qt::CTRL + Qt::Key_R));

    m.setMenuBar(&menu);

    m.resize(800, 448);
    m.show();

    return a.exec();
}
