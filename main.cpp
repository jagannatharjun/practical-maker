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

    QFrame exportWidget;
    exportWidget.setMaximumHeight(40);

    QHBoxLayout el;
    exportWidget.setLayout(&el);

    QLineEdit ques;
    el.addWidget(&ques);
    el.setSpacing(0);
    el.setMargin(0);
    auto f = ques.font();
    f.setFamily("Source Code Pro");
    f.setPointSize(11);
    ques.setFont(f);
    l.addWidget(&exportWidget, 1);

    CodeEditor edit;
    l.addWidget(&edit);

    ConsoleWidget console;
    edit.setConsole(&console);

    QMenuBar menu;
    auto fileMenu = menu.addMenu("File");
    fileMenu->addAction("New", &edit, &CodeEditor::newEditor, QKeySequence::New);
    fileMenu->addAction("Open", &edit, &CodeEditor::open, QKeySequence::Open);
    fileMenu->addAction("Save", &edit, &CodeEditor::save, QKeySequence::Save);
    fileMenu->addAction("Save As", &edit, &CodeEditor::saveAs, QKeySequence::SaveAs);
    fileMenu->addAction("Export", [&]() {
        exportAsPdf("E:\\file.pdf", ques.text(), edit.toPlainText(), console.toPlainText());
    });
    fileMenu->addAction("Exit", qApp, &QApplication::closeAllWindows, QKeySequence::Quit);

    m.setMenuBar(&menu);

    m.resize(800, 448);
    m.show();

    return a.exec();
}
