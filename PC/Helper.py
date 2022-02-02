import sys

from PySide6.QtWidgets import *
from PySide6.QtCore import *
from PySide6.QtGui import *

from MqttClient    import MqttClient

class Helper(QDialog):
    paddleControlSignal = Signal(bool, bool)

    IDS = ['test1', 'test2']
    def __init__(self, parent=None):
        super(Helper, self).__init__()
        self.setWindowTitle('Auxiliar')

        # MQTT Client
        self.client = MqttClient(self)
        self.client.stateChanged.connect(self.on_stateChanged)
        self.client.messageSignal.connect(self.on_messageSignal)

        self.client.hostname = "tom.uib.es"
        self.client.connectToHost()

        # Buttons
        label = QLabel("Publica en uno de estos tópicos:")

        self.conBut = QPushButton("Connected")
        self.rdyBut = QPushButton("Ready")
        self.pd1RespBut = QPushButton("Paddle 1 response")
        self.pd2RespBut = QPushButton("Paddle 2 response")
        self.client_id = 0

        label2 = QLabel("Asignación de ID al jugador:")
        self.assignSwitch = False
        self.assignButton = QPushButton(str(self.assignSwitch))
        self.id = 0

        label3 = QLabel("Controlar paletas:")
        label4 = QLabel("Paleta 1")
        self.p1ControlSwitch = False
        self.p1ControlButton = QPushButton(str(self.p1ControlSwitch))

        label5 = QLabel("Paleta 2")
        self.p2ControlSwitch = False
        self.p2ControlButton = QPushButton(str(self.p2ControlSwitch))

        # Connect buttons
        self.conBut.clicked.connect(self.sendConnected)
        self.rdyBut.clicked.connect(self.sendReady)
        self.pd1RespBut.clicked.connect(lambda: self.sendResponse(False))
        self.pd2RespBut.clicked.connect(lambda: self.sendResponse(True))
        self.pd2RespBut.clicked.connect(lambda: self.sendResponse(True))

        self.assignButton.clicked.connect(self.switchAssign)
        self.p1ControlButton.clicked.connect(self.switchP1Control)
        self.p2ControlButton.clicked.connect(self.switchP2Control)

        # Layout
        vlayout1 = QVBoxLayout()
        vlayout1.addWidget(label)
        vlayout1.addWidget(self.conBut)
        vlayout1.addWidget(self.rdyBut)
        vlayout1.addWidget(self.pd1RespBut)
        vlayout1.addWidget(self.pd2RespBut)
        vl1frame = QFrame()
        vl1frame.setFrameStyle(QFrame.Box | QFrame.Raised)
        vl1frame.setLayout(vlayout1)

        vlayout2 = QVBoxLayout()
        vlayout2.addWidget(label2)
        vlayout2.addWidget(self.assignButton)
        vl2frame = QFrame()
        vl2frame.setFrameStyle(QFrame.Box | QFrame.Raised)
        vl2frame.setLayout(vlayout2)

        glayout1 = QGridLayout()
        glayout1.addWidget(label3,0,0,1,2)
        glayout1.addWidget(label4,1,0)
        glayout1.addWidget(self.p1ControlButton,1,1)
        glayout1.addWidget(label5,2,0)
        glayout1.addWidget(self.p2ControlButton,2,1)
        gl1frame = QFrame()
        gl1frame.setFrameStyle(QFrame.Box | QFrame.Raised)
        gl1frame.setLayout(glayout1)

        glayout2 = QGridLayout()
        glayout2.addWidget(vl1frame, 0,0,2,1)
        glayout2.addWidget(vl2frame, 0,1)
        glayout2.addWidget(gl1frame, 1,1)

        self.setLayout(glayout2)
        self.setFixedSize(self.sizeHint())

    def sendConnected(self):
        self.client.m_client.publish("/pong3d/connected", Helper.IDS[self.client_id % 2])
        self.client_id += 1

    def sendReady(self):
        self.client.m_client.publish("/pong3d/ready", Helper.IDS[self.client_id % 2])
        self.client_id += 1

    def sendResponse(self, paddle1):
        topic = '/pong3d/paddle{}/response/'.format(2 if paddle1 else 1)
        self.client.m_client.publish(topic + "x",  b'\0\0\0\0')
        self.client.m_client.publish(topic + "y",  b'\0\0\0\0')
        self.client.m_client.publish(topic + "vx", b'\0\0\0\0')
        self.client.m_client.publish(topic + "vy", b'\0\0\0\0')

    def switchAssign(self):
        self.assignSwitch = not self.assignSwitch
        self.assignButton.setText(str(self.assignSwitch))

    def switchP1Control(self):
        self.p1ControlSwitch = not self.p1ControlSwitch
        self.p1ControlButton.setText(str(self.p1ControlSwitch))
        self.paddleControlSignal.emit(True, self.p1ControlSwitch)

    def switchP2Control(self):
        self.p2ControlSwitch = not self.p2ControlSwitch
        self.p2ControlButton.setText(str(self.p2ControlSwitch))
        self.paddleControlSignal.emit(False, self.p2ControlSwitch)

    @Slot(int)
    def on_stateChanged(self, state):
        if state == MqttClient.Connected:
            self.client.subscribe("/pong3d/connected")

    @Slot(str, bytes)
    def on_messageSignal(self, topic, msg):
        if self.assignSwitch:
            char = '1' if self.id % 2 == 0 else '2'
            self.client.m_client.publish('/pong3d/player_id',  bytes(char,'utf-8'))
            self.id += 1


if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = Helper()
    window.show()

    sys.exit(app.exec())
