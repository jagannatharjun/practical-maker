#include "codeeditor.hpp"
#include "consolewidget.hpp"
#include "exporter.hpp"

#include <QActionGroup>
#include <QApplication>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QStyleFactory>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    qputenv("Path", qgetenv("Path") + ";" + qApp->applicationDirPath().toUtf8() + "\\mingw32\\bin;");
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
    ques.setToolTip("Question");
    ques.setPlaceholderText("Enter your Question here...");
    l.addWidget(&ques);

    CodeEditor edit;
    edit.setToolTip("Code");
    edit.setPlaceholderText("Enter your Code here...");
    l.addWidget(&edit);

    ConsoleWidget console;
    edit.setConsole(&console);

    QLineEdit footer;
    footer.setFont(f);
    footer.setToolTip("Footer");
    footer.setPlaceholderText("Enter your footer here...");
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

    auto langMenu = menu.addMenu("Language");
    QActionGroup langGroup(&m);
    langGroup.setExclusive(true);
    auto clang = langMenu->addAction("C"), cpplang = langMenu->addAction("Cpp");
    clang->setCheckable(true);
    cpplang->setCheckable(true);

    langGroup.addAction(clang);
    langGroup.addAction(cpplang);
    cpplang->setChecked(true);
    clang->setChecked(false);
    QObject::connect(clang, &QAction::triggered, [&edit]() { edit.setLang(CodeEditor::CLang); });
    QObject::connect(cpplang, &QAction::triggered,
                     [&edit]() { edit.setLang(CodeEditor::CppLang); });

    m.setMenuBar(&menu);

    m.resize(900, 500);
    m.show();

    return a.exec();
}
