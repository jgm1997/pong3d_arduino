from PySide6.QtWidgets import *
from PySide6.QtCore    import *
from PySide6.QtGui     import *

from Transform3D   import Transform3D
from BallItem      import BallItem
from PlayfieldItem import PlayfieldItem
from PaddleItem    import PaddleItem

class PongView(QGraphicsView):

    PROYDIST = 500

    # Playfield constants
    PF_WIDTH    = 500
    PF_HEIGHT   = 300
    PF_DEPTH    = 800

    PF_COLOR    = Qt.green

    # Paddle constants
    PADDLE_WIDTH  = PF_WIDTH  / 5
    PADDLE_HEIGHT = PF_HEIGHT / 5

    PADDLE1_COLOR = Qt.red
    PADDLE2_COLOR = Qt.blue

    # Ball constants
    BALL_RADIUS = 5
    BALL_COLOR  = Qt.white

    MARGIN = 10

    def __init__(self, parent=None):
        super(PongView, self).__init__()

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setRenderHint(QPainter.Antialiasing)

        # Transform 3D
        self.t3d = Transform3D()
        self.t3d.setProyectionPlaneDistance(self.PROYDIST)

        # Pong items
        self.paddle1 = PaddleItem(self.PADDLE1_COLOR)
        self.paddle1.setColor(self.PADDLE1_COLOR)
        self.paddle1.setSize(self.PADDLE_WIDTH, self.PADDLE_HEIGHT)
        self.paddle1.setTransform3D(self.t3d)
        self.paddle1.initPaddle()
        self.paddle1.setPosition(0, 0, 0)

        self.paddle2 = PaddleItem()
        self.paddle2.setColor(self.PADDLE2_COLOR)
        self.paddle2.setSize(self.PADDLE_WIDTH, self.PADDLE_HEIGHT)
        self.paddle2.setTransform3D(self.t3d)
        self.paddle2.initPaddle()
        self.paddle2.setPosition(0, 0, self.PF_DEPTH)

        self.ball = BallItem()
        self.ball.setRadius(self.BALL_RADIUS)
        self.ball.setColor(self.BALL_COLOR)
        self.ball.setGuideRectangle(self.PF_WIDTH, self.PF_HEIGHT)
        self.ball.setTransform3D(self.t3d)
        self.ball.initBall()
        self.ball.setPosition(0, 0, self.PF_DEPTH/2)

        self.playfield = PlayfieldItem()
        self.playfield.setDimensions(self.PF_WIDTH, self.PF_HEIGHT, self.PF_DEPTH)
        self.playfield.setColor(self.PF_COLOR)
        self.playfield.setTransform3D(self.t3d)
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
        self.scene.addItem(self.ball)
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
        if event.key() == Qt.Key_Up:
            self.ball.move(0, -10, 0)
        if event.key() == Qt.Key_Down:
            self.ball.move(0, 10, 0)
        if event.key() == Qt.Key_Right:
            self.ball.move(10, 0, 0)
        if event.key() == Qt.Key_Left:
            self.ball.move(-10, 0, 0)
        if event.key() == Qt.Key_W:
            self.ball.move(0, 0, 10)
        if event.key() == Qt.Key_S:
            self.ball.move(0, 0, -10)


