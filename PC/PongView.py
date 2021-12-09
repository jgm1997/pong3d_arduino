from PySide6.QtWidgets import *
from PySide6.QtCore    import *
from PySide6.QtGui     import *

class Transform3D:
    def setProyectionPlaneDistance(self, distance):
        self.proy_dist = distance

    def getProyectionScaleFactor(self, z):
        return self.proy_dist / (self.proy_dist + z)

    def setProyectionMatrix(self, x, y, z):
        s = self.getProyectionScaleFactor(z)
        x_s = x * s
        y_s = y * s
        self.transform = QTransform(s, 0, 0, s, x_s, y_s)

    def getProyectedItem(self, item):
        return self.transform.map(item)

#    def getProyectedItem(self, item, x, y, z):
#        s = self.getProyectionScaleFactor(z)
#        x_s = x * s
#        y_s = y * s
#        transform = QTransform(s, 0, 0, s, x_s, y_s)
#        return transform.map(item)

class PaddleItem(QGraphicsItemGroup):
    def __init__(self, size_x, size_y, color, parent=None):
        super(PaddleItem, self).__init__()
        self.setSize(size_x, size_y)
        self.setColor(color)

    def setColor(self, color):
        self.color1 = QColor(color)
        self.color2 = QColor(color)
        self.color2.setAlpha(60)

    def setSize(self, size_x, size_y):
        self.size_x = size_x
        self.size_y = size_y

    def initPaddle(self):
        half_x = self.size_x / 2
        half_y = self.size_y / 2

        pen   = QPen  (self.color1)
        brush = QBrush(self.color2)

        self.rectangle = QGraphicsRectItem(-half_x, -half_y, self.size_x, self.size_y, parent=self)
        self.rectangle.setPen(pen)
        self.rectangle.setBrush(brush)
        self.addToGroup(self.rectangle)

        self.v_line = QGraphicsLineItem(0, -half_y, 0, half_y, parent=self)
        self.v_line.setPen(pen)
        self.addToGroup(self.v_line)

        self.h_line = QGraphicsLineItem(-half_x, 0, half_x, 0, parent=self)
        self.h_line.setPen(pen)
        self.addToGroup(self.h_line)

    def updatePaddle(self):
        self.rectangle.setPen(pen)
        self.rectangle.setBrush(brush)
        self.v_line.setPen(pen)
        self.h_line.setPen(pen)

    def clearPaddle(self):
        self.removeFromGroup(self.rectangle)
        self.removeFromGroup(self.v_line)
        self.removeFromGroup(self.h_line)

class BallItem(QGraphicsEllipseItem):
    def __init__(self, parent=None):
        super(BallItem, self).__init__()

class PlayfieldItem(QGraphicsItemGroup):
    # Division lines
    X_DIVISIONS = 3
    Y_DIVISIONS = 2
    Z_DIVISIONS = 4

    def __init__(self, width, height, depth, proy_dist, color, parent=None):
        super(PlayfieldItem, self).__init__()
        self.setDimensions(width, height, depth)
        self.setProyectionPlaneDistance(proy_dist)
        self.setColor(color)

    def setDimensions(self, width, height, depth):
        self.width  = width
        self.height = height
        self.depth  = depth

    def setProyectionPlaneDistance(self, distance):
        self.proy_dist = distance

    def setColor(self, color):
        self.color1 = QColor(color)
        self.color2 = QColor(color)
        self.color2.setAlpha(60)

    def initPlayfield(self):
        half_width = self.width / 2
        half_height = self.height / 2
        half_depth = self.depth / 2

        # 3D proyection
        transform = Transform3D()
        transform.setProyectionPlaneDistance(self.proy_dist)
        transform.setProyectionMatrix(0, 0, self.depth)

        self.lines = []
        self.rects = []

        xdiv_size = self.width  / self.X_DIVISIONS
        ydiv_size = self.height / self.Y_DIVISIONS
        zdiv_size = self.depth  / self.Z_DIVISIONS

        pen   = QPen  (self.color1)
        brush = QBrush(self.color2)

        # X axis divisions
        x0 = -half_width
        y0 = -half_height
        for i in range(self.X_DIVISIONS + 1):
            # Top
            p1 = QPointF(x0 + i*xdiv_size, y0)
            p2 = transform.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

            # Bottom
            p1 = QPointF(x0 + i*xdiv_size, -y0)
            p2 = transform.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

        # Y axis divisions
        for i in range(1, self.Y_DIVISIONS):
            # Left
            p1 = QPointF(x0, y0 + i*ydiv_size)
            p2 = transform.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

            # Right
            p1 = QPointF(-x0, y0 + i*ydiv_size)
            p2 = transform.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

        # Square divisions
        for i in range(self.Z_DIVISIONS + 1):
            rectangle = QGraphicsRectItem(x0, y0, self.width, self.height)
            rectangle.setPen(pen)
            scale = transform.getProyectionScaleFactor(i*zdiv_size)
            rectangle.setScale(scale)
            self.addToGroup(rectangle)
            self.rects.append(rectangle)

    def updatePlayfield(self):
        pass

    def clearPlayfield(self):
        pass

class PongView(QGraphicsView):

    # Playfield constants
    PF_WIDTH    = 500
    PF_HEIGHT   = 300
    PF_DEPTH    = 800
    PF_PROYDIST = 300

    # Paddle constants
    PADDLE_WIDTH  = PF_WIDTH  / 5
    PADDLE_HEIGHT = PF_HEIGHT / 5

    PADDLE1_COLOR = Qt.red
    PADDLE2_COLOR = Qt.blue

    MARGIN = 10

    def __init__(self, parent=None):
        super(PongView, self).__init__()

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setRenderHint(QPainter.Antialiasing)

        # Pong items
        self.paddle1 = PaddleItem(self.PADDLE_WIDTH, self.PADDLE_HEIGHT, self.PADDLE1_COLOR)
        self.paddle1.initPaddle()

        self.paddle2 = PaddleItem(self.PADDLE_WIDTH, self.PADDLE_HEIGHT, self.PADDLE2_COLOR)
        self.paddle2.initPaddle()
        self.paddle2.setScale(PongView.getProyectionScaleFactor(PongView.PF_DEPTH))

        self.ball = BallItem()

        self.playfield = PlayfieldItem(self.PF_WIDTH, self.PF_HEIGHT, self.PF_DEPTH, self.PF_PROYDIST, Qt.green, parent=self)
        self.playfield.initPlayfield()

        self._initScreen()

    def _initScreen(self):
        self.scene = QGraphicsScene(parent=self)

        width  = self.PF_WIDTH  + 2 * self.MARGIN
        height = self.PF_HEIGHT + 2 * self.MARGIN

        self.scene.setSceneRect(-width/2, -height/2, width, height)
        self.scene.setBackgroundBrush(QBrush(Qt.black))

        self.scene.addItem(self.playfield)
        self.scene.addItem(self.paddle2)
        self.scene.addItem(self.paddle1)

        self.resetTransform()

        self.setScene(self.scene)

    def _updateScreen(self):
        pass

    def _clearScreen(self):
        pass

    def resizeEvent(self, event):
        self.fitInView(self.scene.sceneRect(), Qt.KeepAspectRatio)

    def getProyectionScaleFactor(depth):
        return PongView.PF_PROYDIST / (PongView.PF_PROYDIST + depth)

    def keyPressEvent(self, event):
        if   event.key() == Qt.Key_Up:
            pass
        elif event.key() == Qt.Key_Down:
            pass
        elif event.key() == Qt.Key_Left:
            pass
        elif event.key() == Qt.Key_Right:
            pass
