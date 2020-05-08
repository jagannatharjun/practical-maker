import sys
import re
import os
import tempfile
import subprocess
import shlex
from PySide2.QtCore import QProcess, Qt, Signal, Slot, QObject
from PySide2.QtWidgets import QApplication, QPlainTextEdit
from PySide2.QtGui import QTextCursor


class Args:
    def __init__(self, dir, args):
        self.dir = dir
        self.args = args


class Form(QPlainTextEdit):
    processFinished = Signal()

    def __init__(self, fileName, parent=None):
        super(Form, self).__init__(parent)
        self._fileName = fileName

        self.setStyleSheet("font-family: Source Code Pro; font-size: 16px; ")

        self._process = QProcess()
        self._process.readyReadStandardOutput.connect(self._processStdOut)
        self._process.readyReadStandardError.connect(self._processStdErr)
        self._process.finished.connect(self._processFinished)

    def run(self, args):
        self._append("> " + shlex.join(args.args) + "\n")
        self._lastPos = 0
        self._process.setWorkingDirectory(args.dir)
        self._process.setProgram(args.args[0])
        self._process.setArguments(args.args[1:])
        self._process.start()

    @Slot()
    def putTextToFile(self):
        open(self._fileName, "w").write(self.toPlainText())

    def keyPressEvent(self, event):
        k = event.key()
        if not self.textCursor().position() < self._lastPos:
            super().keyPressEvent(event)
        if k == Qt.Key_Return or k == Qt.Key_Enter:
            self._process.write(bytes(self.toPlainText()[self._lastPos :], "utf-8"))
            self._lastPos = self.textCursor().position()

    def _processStdOut(self):
        self._append(str(self._process.readAllStandardOutput(), "utf-8"))

    def _processStdErr(self):
        self._append(str(self._process.readAllStandardError(), "utf-8"))

    def _append(self, output):
        self.moveCursor(QTextCursor.End)
        self.insertPlainText(output)
        self.moveCursor(QTextCursor.End)
        self._lastPos = self.textCursor().position()

    def _processFinished(self, exitCode):
        self._append("\nProcess Finished with exit code: " + str(exitCode) + "\n")
        self.processFinished.emit()


class ArgRunner(QObject):
    run = Signal(list)
    finished = Signal()
    args: list()

    def start(self):
        self._pos = -1
        self.nextArg()

    @Slot()
    def nextArg(self):
        self._pos = self._pos + 1
        if self._pos < len(self.args):
            self.run.emit(self.args[self._pos])
        elif self._pos == len(self.args):
            self.finished.emit()


def compile(dir, fileName):
    fileContent = open(fileName, "r").read()
    classSearchRes = re.search(
        r"class\s*([_a-zA-Z][_a-zA-Z0-9]{0,30})((.|\n)*)main\s*\(\s*String", fileContent
    )
    if classSearchRes is None:
        raise ("Can't find classname")
    argStrs = re.findall(r"\/\/ args -(.*)", fileContent)
    args = [shlex.split(x) for x in argStrs]

    className = classSearchRes.group(1)
    classFileName = os.path.join(dir, className + ".java")
    open(classFileName, "w").write(fileContent)
    subprocess.run(["javac", classFileName])
    if not args:
        return [Args(dir, ["java", className])]
    return [Args(dir, ["java", className] + x) for x in args]


if __name__ == "__main__":
    # Create the Qt Application
    os.environ["PATH"] = "C:\\Program Files\\Java\\jdk-14\\bin;" + os.environ["PATH"]
    tempd = tempfile.mkdtemp()
    argRunner = ArgRunner()
    fileName = sys.argv[1]
    argRunner.args = compile(tempd, fileName)

    app = QApplication(sys.argv)
    # Create and show the form
    form = Form(os.path.splitext(fileName)[0] + ".output")
    form.resize(800, 400)
    argRunner.run.connect(form.run)
    form.processFinished.connect(argRunner.nextArg)
    argRunner.finished.connect(form.putTextToFile)
    argRunner.start()
    form.show()
    # Run the main Qt loop
    sys.exit(app.exec_())
