from PySide6.QtWidgets import *
from PySide6.QtCore    import *
from PySide6.QtGui     import *

class BallItem(QGraphicsItemGroup):
    def __init__(self, parent=None):
        super(BallItem, self).__init__()
        self.useGuideRectangle = False
        self.x = 0
        self.y = 0
        self.z = 0

    def setRadius(self, r):
        self.r = r

    def setColor(self, color):
        self.color = QColor(color)

    def setTransform3D(self, t3d):
        self.t3d = t3d

    def setPosition(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

        self.t3d.setProyectionMatrix(x, y, z)
        self.ellipse.setTransform(self.t3d)

        self.t3d.setProyectionMatrix(0, 0, z)
        self.guideRect.setTransform(self.t3d)

    def invertX(self):
        self.x *= -1
        self.t3d.setProyectionMatrix(self.x, self.y, self.z)
        self.ellipse.setTransform(self.t3d)

    def invertZ(self, zplane=0):
        self.z -= zplane
        self.z *= -1
        self.z += zplane

        self.t3d.setProyectionMatrix(self.x, self.y, self.z)
        self.ellipse.setTransform(self.t3d)

        self.t3d.setProyectionMatrix(0, 0, self.z)
        self.guideRect.setTransform(self.t3d)

    def getZ(self):
        return self.z

    def move(self, dx, dy, dz):
        self.x += dx
        self.y += dy
        self.z += dz

        self.t3d.setProyectionMatrix(self.x, self.y, self.z)
        self.ellipse.setTransform(self.t3d)

        self.t3d.setProyectionMatrix(0, 0, self.z)
        self.guideRect.setTransform(self.t3d)

    def setGuideRectangle(self, width, height):
        self.useGuideRectangle = True
        self.gr_width = width
        self.gr_height = height

    def unsetGuideRectangle(self):
        self.useGuideRectangle = False

    def initBall(self):
        self.ellipse = QGraphicsEllipseItem(self.x - self.r, self.y - self.r, self.r * 2, self.r * 2)
        self.ellipse.setBrush(self.color)
        self.addToGroup(self.ellipse)

        # Playfield guide rectangle
        if(self.useGuideRectangle):
            half_x = -self.gr_width / 2
            half_y = -self.gr_height / 2
            self.guideRect = QGraphicsRectItem(half_x, half_y, self.gr_width, self.gr_height)
            print(half_x, half_y, self.gr_width, self.gr_height)
            self.guideRect.setPen(self.color)
            self.addToGroup(self.guideRect)

    def updateBall(self):
        pass

    def clearBall(self):
        pass
