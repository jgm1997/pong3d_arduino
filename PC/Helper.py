import sys

from PySide6.QtWidgets import *
from PySide6.QtCore import *
from PySide6.QtGui import *

from MqttClient    import MqttClient

class Helper(QDialog):
    def __init__(self, parent=None):
        super(Helper, self).__init__()
        self.setWindowTitle('Auxiliar')

        # MQTT Client
        self.client = MqttClient(self)

        self.client.hostname = "tom.uib.es"
        self.client.connectToHost()

        # Buttons
        label = QLabel("Publica en uno de estos tópicos:")

        self.conBut = QPushButton("Connected")
        self.rdyBut = QPushButton("Ready")
        self.pd1But = QPushButton("Paddle 1 response")
        self.pd2But = QPushButton("Paddle 2 response")

        self.conBut.clicked.connect(self.sendConnected)
        self.rdyBut.clicked.connect(self.sendReady)
        self.pd1But.clicked.connect(lambda: self.sendResponse(False))
        self.pd2But.clicked.connect(lambda: self.sendResponse(True))

        # Layout
        layout = QVBoxLayout()
        layout.addWidget(label)
        layout.addWidget(self.conBut)
        layout.addWidget(self.rdyBut)
        layout.addWidget(self.pd1But)
        layout.addWidget(self.pd2But)

        self.setLayout(layout)
        self.setFixedSize(self.sizeHint())

    def sendConnected(self):
        self.client.m_client.publish("/pong3d/connected", b'')

    def sendReady(self):
        self.client.m_client.publish("/pong3d/ready", b'')

    def sendResponse(self, paddle1):
        topic = '/pong3d/paddle{}/response/'.format(2 if paddle1 else 1)
        self.client.m_client.publish(topic + "x",  b'\0\0\0\0')
        self.client.m_client.publish(topic + "y",  b'\0\0\0\0')
        self.client.m_client.publish(topic + "vx", b'\0\0\0\0')
        self.client.m_client.publish(topic + "vy", b'\0\0\0\0')


if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = Helper()
    window.show()

    sys.exit(app.exec())
