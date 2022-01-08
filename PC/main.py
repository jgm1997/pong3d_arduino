import sys

from PySide6.QtWidgets import *
from PySide6.QtCore import *
from PySide6.QtGui import *

from PongView import PongView

class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__()

        self.setWindowTitle('Pong3D')
        self.pongView = PongView(parent=self)
        self.setCentralWidget(self.pongView)



if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
