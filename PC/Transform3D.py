from PySide6.QtWidgets import *
from PySide6.QtCore    import *
from PySide6.QtGui     import *

class Transform3D(QTransform):
    def setProyectionPlaneDistance(self, distance):
        self.proy_dist = distance

    def getProyectionScaleFactor(self, z):
        return self.proy_dist / (self.proy_dist + z)

    def setProyectionMatrix(self, x, y, z):
        s = self.getProyectionScaleFactor(z)
        x_s = x * s
        y_s = y * s
        self.setMatrix(s, 0, 0, 0, s, 0, x_s, y_s, 1)

    def getProyectedItem(self, item):
        return self.map(item)

#    def getProyectedItem(self, item, x, y, z):
#        s = self.getProyectionScaleFactor(z)
#        x_s = x * s
#        y_s = y * s
#        transform = QTransform(s, 0, 0, s, x_s, y_s)
#        return transform.map(item)
