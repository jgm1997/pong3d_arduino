from PySide6.QtWidgets import *
from PySide6.QtCore    import *
from PySide6.QtGui     import *

class PlayfieldItem(QGraphicsItemGroup):
    # Division lines
    X_DIVISIONS = 3
    Y_DIVISIONS = 2
    Z_DIVISIONS = 4

    def __init__(self):
        super(PlayfieldItem, self).__init__()

    def setDimensions(self, width, height, depth):
        self.width  = width
        self.height = height
        self.depth  = depth

    def setTransform3D(self, t3d):
        self.t3d = t3d

    def setColor(self, color):
        self.color1 = QColor(color)
        self.color2 = QColor(color)
        self.color2.setAlpha(60)

    def initPlayfield(self):
        half_width = self.width / 2
        half_height = self.height / 2
        half_depth = self.depth / 2

        self.t3d.setProyectionMatrix(0, 0, self.depth)

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
            p2 = self.t3d.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

            # Bottom
            p1 = QPointF(x0 + i*xdiv_size, -y0)
            p2 = self.t3d.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

        # Y axis divisions
        for i in range(1, self.Y_DIVISIONS):
            # Left
            p1 = QPointF(x0, y0 + i*ydiv_size)
            p2 = self.t3d.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

            # Right
            p1 = QPointF(-x0, y0 + i*ydiv_size)
            p2 = self.t3d.getProyectedItem(p1)
            line = QGraphicsLineItem(QLineF(p1, p2), parent=self)
            line.setPen(pen)
            self.addToGroup(line)
            self.lines.append(line)

        # Square divisions
        for i in range(self.Z_DIVISIONS + 1):
            rectangle = QGraphicsRectItem(x0, y0, self.width, self.height)
            rectangle.setPen(pen)
            scale = self.t3d.getProyectionScaleFactor(i*zdiv_size)
            rectangle.setScale(scale)
            self.addToGroup(rectangle)
            self.rects.append(rectangle)

    def updatePlayfield(self):
        self.clearPlayfield()
        self.initPlayfield()
        self.update()

    def clearPlayfield(self):
        for item in self.lines + self.rects:
            self.removeFromGroup(item)
            self.scene().removeItem(item)

        self.lines = []
        self.rects = []

