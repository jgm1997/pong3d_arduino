# Source code modified: https://stackoverflow.com/a/52639291

from PySide2.QtCore    import *
from PySide2.QtWidgets import *
import paho.mqtt.client as mqtt

class MqttClient(QObject):
    Disconnected = 0
    Connecting = 1
    Connected = 2

    MQTT_3_1 = mqtt.MQTTv31
    MQTT_3_1_1 = mqtt.MQTTv311

    connected = Signal()
    disconnected = Signal()

    stateChanged = Signal(int)
    hostnameChanged = Signal(str)
    portChanged = Signal(int)
    keepAliveChanged = Signal(int)
    cleanSessionChanged = Signal(bool)
    protocolVersionChanged = Signal(int)

    messageSignal = Signal(str, str)

    def __init__(self, parent=None):
        super(MqttClient, self).__init__(parent)

        self.m_hostname = ""
        self.m_port = 1883
        self.m_keepAlive = 60
        self.m_cleanSession = True
        self.m_protocolVersion = MqttClient.MQTT_3_1

        self.m_state = MqttClient.Disconnected

        self.m_client =  mqtt.Client(clean_session=self.m_cleanSession,
            protocol=self.protocolVersion)

        self.m_client.on_connect = self.on_connect
        self.m_client.on_message = self.on_message
        self.m_client.on_disconnect = self.on_disconnect


    @Property(int, notify=stateChanged)
    def state(self):
        return self.m_state

    @state.setter
    def state(self, state):
        if self.m_state == state: return
        self.m_state = state
        self.stateChanged.emit(state)

    @Property(str, notify=hostnameChanged)
    def hostname(self):
        return self.m_hostname

    @hostname.setter
    def hostname(self, hostname):
        if self.m_hostname == hostname: return
        self.m_hostname = hostname
        self.hostnameChanged.emit(hostname)

    @Property(int, notify=portChanged)
    def port(self):
        return self.m_port

    @port.setter
    def port(self, port):
        if self.m_port == port: return
        self.m_port = port
        self.portChanged.emit(port)

    @Property(int, notify=keepAliveChanged)
    def keepAlive(self):
        return self.m_keepAlive

    @keepAlive.setter
    def keepAlive(self, keepAlive):
        if self.m_keepAlive == keepAlive: return
        self.m_keepAlive = keepAlive
        self.keepAliveChanged.emit(keepAlive)

    @Property(bool, notify=cleanSessionChanged)
    def cleanSession(self):
        return self.m_cleanSession

    @cleanSession.setter
    def cleanSession(self, cleanSession):
        if self.m_cleanSession == cleanSession: return
        self.m_cleanSession = cleanSession
        self.cleanSessionChanged.emit(cleanSession)

    @Property(int, notify=protocolVersionChanged)
    def protocolVersion(self):
        return self.m_protocolVersion

    @protocolVersion.setter
    def protocolVersion(self, protocolVersion):
        if self.m_protocolVersion == protocolVersion: return
        if protocolVersion in (MqttClient.MQTT_3_1, MQTT_3_1_1):
            self.m_protocolVersion = protocolVersion
            self.protocolVersionChanged.emit(protocolVersion)

    #################################################################
    @Slot()
    def connectToHost(self):
        if self.m_hostname:
            self.m_client.connect(self.m_hostname,
                port=self.port,
                keepalive=self.keepAlive)

            self.state = MqttClient.Connecting
            self.m_client.loop_start()

    @Slot()
    def disconnectFromHost(self):
        self.m_client.disconnect()

    def subscribe(self, path):
        if self.state == MqttClient.Connected:
            self.m_client.subscribe(path)

    #################################################################
    # callbacks
    def on_message(self, mqttc, obj, msg):
        mstr = msg.payload.decode("ascii")
        # print("on_message", mstr, obj, mqttc)
        self.messageSignal.emit(msg.topic, mstr)

    def on_connect(self, *args):
        # print("on_connect", args)
        self.state = MqttClient.Connected
        self.connected.emit()

    def on_disconnect(self, *args):
        # print("on_disconnect", args)
        self.state = MqttClient.Disconnected
        self.disconnected.emit()


class Widget(QWidget):
    def __init__(self, parent=None):
        super(Widget, self).__init__(parent)

        label_x = QLabel('x:', parent=self)
        label_y = QLabel('y:', parent=self)
        label_z = QLabel('z:', parent=self)
        self.lcd_number_x = QLCDNumber()
        self.lcd_number_y = QLCDNumber()
        self.lcd_number_z = QLCDNumber()

        lay = QGridLayout(self)
        lay.addWidget(label_x, 0, 0)
        lay.addWidget(self.lcd_number_x, 0, 1)
        lay.addWidget(label_y, 1, 0)
        lay.addWidget(self.lcd_number_y, 1, 1)
        lay.addWidget(label_z, 2, 0)
        lay.addWidget(self.lcd_number_z, 2, 1)

        self.client = MqttClient(self)
        self.client.stateChanged.connect(self.on_stateChanged)
        self.client.messageSignal.connect(self.on_messageSignal)

        self.client.hostname = "tom.uib.es"
        self.client.connectToHost()

    @Slot(int)
    def on_stateChanged(self, state):
        if state == MqttClient.Connected:
            self.client.subscribe("/pong3d/ball/x")
            self.client.subscribe("/pong3d/ball/y")
            self.client.subscribe("/pong3d/ball/z")

    @Slot(str, str)
    def on_messageSignal(self, topic, msg):
        try:
            val = float(msg)
            subt = topic.split('/')[-1]
            if   subt == 'x':
                self.lcd_number_x.display(val)
            elif subt == 'y':
                self.lcd_number_y.display(val)
            elif subt == 'z':
                self.lcd_number_z.display(val)
        except ValueError:
            print('error: Value sent at "{}" is not a number'.format(topic))


if __name__ == '__main__':
    import sys

    print("Puedes modificar los valores publicando en los siguientes topics:")
    print("- /pong3d/x")
    print("- /pong3d/y")
    print("- /pong3d/z")

    app = QApplication(sys.argv)
    w = Widget()
    w.show()
    sys.exit(app.exec_())
