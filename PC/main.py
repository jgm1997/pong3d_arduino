import sys

from PySide6.QtWidgets import *
from PySide6.QtCore import *
from PySide6.QtGui import *

from PongView import PongView
from Helper import Helper

class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__()

        self.setWindowTitle('Pong3D')

        # Menu bar
        self.menu = self.menuBar()
        testing = self.menu.addMenu("Testing")
        aux = QAction("MQTT Auxiliar", self)
        aux.triggered.connect(self.openHelperWindow)
        testing.addAction(aux)

        # Widget de visualización del juego
        self.pongView = PongView(parent=self)

        # Botones de cambio de vista
        self.left = QPushButton("Paleta 1")
        self.right = QPushButton("Paleta 2")

        self.left.clicked.connect(self.setViewPaddle1)
        self.right.clicked.connect(self.setViewPaddle2)

        # Etiqueta vista actual
        self.label = QLabel("Seleccionar vista")
        self.label.setAlignment(Qt.AlignHCenter|Qt.AlignVCenter)

        # Widget selección de vista
        hlayout = QHBoxLayout()
        hlayout.addWidget(self.left)
        hlayout.addWidget(self.label)
        hlayout.addWidget(self.right)
        hlayout.setContentsMargins(2, 2, 2, 2)

        viewSelector = QWidget(parent=self)
        viewSelector.setLayout(hlayout)

        # Widget central
        vlayout = QVBoxLayout()
        vlayout.addWidget(self.pongView)
        vlayout.addWidget(viewSelector)
        vlayout.setContentsMargins(2, 2, 2, 2)

        centralWidget = QWidget(parent=self)
        centralWidget.setLayout(vlayout)

        self.setCentralWidget(centralWidget)

    def setViewPaddle1(self):
        self.pongView.setViewPaddle1()

    def setViewPaddle2(self):
        self.pongView.setViewPaddle2()

    def openHelperWindow(self):
        helper = Helper()
        helper.show()

if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
