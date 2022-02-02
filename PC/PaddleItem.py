from PySide6.QtWidgets import *
from PySide6.QtCore    import *
from PySide6.QtGui     import *

class PaddleItem(QGraphicsItemGroup):
    def __init__(self):
        super(PaddleItem, self).__init__()

    def setTransform3D(self, t3d):
        self.t3d = t3d

    def setColor(self, color):
        self.color1 = QColor(color)
        self.color2 = QColor(color)
        self.color2.setAlpha(60)

    def setSize(self, size_x, size_y):
        self.size_x = size_x
        self.size_y = size_y

    def setPosition(self, x, y, z=None):
        self.x = x
        self.y = y
        if z is not None:
            self.z = z
        self.t3d.setProyectionMatrix(x, y, self.z)
        self.setTransform(self.t3d)

    def setZ(self, z):
        self.z = z
        self.t3d.setProyectionMatrix(self.x, self.y, self.z)
        self.setTransform(self.t3d)

    def invertX(self):
        self.x *= -1
        self.t3d.setProyectionMatrix(self.x, self.y, self.z)
        self.setTransform(self.t3d)

    def move(self, dx, dy, dz):
        self.x += dx
        self.y += dy
        self.z += dz
        self.t3d.setProyectionMatrix(self.x, self.y, self.z)
        self.setTransform(self.t3d)

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
        self.clearPaddle()
        self.initPaddle()
        self.update()

    def clearPaddle(self):
        self.removeFromGroup(self.rectangle)
        self.removeFromGroup(self.v_line)
        self.removeFromGroup(self.h_line)

        self.scene().removeItem(self.rectangle)
        self.scene().removeItem(self.v_line)
        self.scene().removeItem(self.h_line)
