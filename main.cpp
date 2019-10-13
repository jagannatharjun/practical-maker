#include "codeeditor.hpp"
#include "consolewidget.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QMainWindow m;

    QWidget c;
    m.setCentralWidget(&c);

    QVBoxLayout l;
    c.setLayout(&l);

    CodeEditor edit;
    l.addWidget(&edit, 7);

    ConsoleWidget console;
    l.addWidget(&console, 3);
    edit.setConsole(&console);

    m.resize(800,448);
    m.show();

    edit.setPlainText(
        "#include <iostream>\nint main() { std::cout << \"Hello World!\"; }");

    return a.exec();
}
