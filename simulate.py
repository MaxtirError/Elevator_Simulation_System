from os import X_OK
from PySide2 import QtGui, QtWidgets, QtCore
import sys
import cv2
Floor_Pos_x = [1200, 300]
Floor_Pos_y = [700, 550, 400, 250, 100]
Person_Pos_x = 1000
ELEVATOR_HEIGHT = 150
ELEVATOR_LENGTH = 150
PERSON_LENGTH = 50
PERSON_HEIGHT = 100
class MainUi(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.Floor = [1, 1]
        self.init_ui()
        self.clock = 0
        file = open("simulation.txt")
        self.event = []
        for i in range(10001):
            self.event.append([])
        self.personlabel = []
        self.waitqueue = []
        self.inside = [[], []]
        self.updown = []
        for i in range(5):
            self.waitqueue.append([])
        while 1:
            line = file.readline()
            if not line:
                break
            x = list(map(int, line.split()))
            self.event[x[0]].append(x[1:])
            if x[1] == 0 and x[2] == 1:
                self.personlabel.append(QtWidgets.QLabel(self.main_widget))
                self.updown.append(0)
        file.close()

    def init_ui(self):
        self.setWindowTitle("elevator_system")
        self.resize(2048, 960)
        self.main_widget = QtWidgets.QWidget()
        self.elevatorlabel = [QtWidgets.QLabel(self.main_widget), QtWidgets.QLabel(self.main_widget)]
        self.setElevator(0, 0)
        self.setElevator(0, 1)
        self.insidelabel = [QtWidgets.QLabel(self.main_widget), QtWidgets.QLabel(self.main_widget)]
        self.insidelabel[0].setText("INSIDE0:\nNULL")
        self.insidelabel[0].setGeometry(1400, 0, 200, 1000)
        self.insidelabel[0].setStyleSheet('''
                       font-size: 12pt;
                       color: black;
                       ''')
        self.insidelabel[1].setText("INSIDE1:\nNULL")
        self.insidelabel[1].setGeometry(100, 0, 200, 1000)
        self.insidelabel[1].setStyleSheet('''
                       font-size: 12pt;
                       color: black;
                       ''')
        
        self.anim = [QtCore.QPropertyAnimation(x, b'geometry') for x in self.elevatorlabel]
        self.setCentralWidget(self.main_widget)
        self.timer = QtCore.QTimer(self)
        self.timer.start(10)
        self.timer.timeout.connect(self.Activate)
    
    def get_png_with_text(self, imgsrc, text):
        
        src = cv2.imread(imgsrc)
        cv2.putText(src, text, (20, 20), cv2.FONT_HERSHEY_COMPLEX, 0.7, (0, 0, 0), 2)
        # BGR => RGB 文件格式
        shrink = cv2.cvtColor(src, cv2.COLOR_BGR2RGB)
        # cv 图片转换成 qt图片
        qt_img = QtGui.QImage(shrink.data, # 数据源
                                shrink.shape[1],  # 宽度
                                shrink.shape[0],	# 高度
                                shrink.shape[1] * 3, # 行字节数
                                QtGui.QImage.Format_RGB888)

        png = QtGui.QPixmap.fromImage(qt_img)
        #self.png.load("person.png")
        return png
    
    def setElevator(self, state, id):
        self.elevatorlabel[id].setGeometry(Floor_Pos_x[id], Floor_Pos_y[self.Floor[id]], ELEVATOR_LENGTH, ELEVATOR_HEIGHT)
        if state == 0:
            self.elevatorlabel[id].setPixmap(self.get_png_with_text('elevator.png','elevator'))
        else:
            self.elevatorlabel[id].setPixmap(self.get_png_with_text('elevator_open.png','elevator'))
        self.elevatorlabel[id].setScaledContents(True)
    
    def ElevatorMove(self, St, Ed, Tm, id):
        print(str(Tm) + ' ' + str(id))
        self.anim[id].stop()
        self.anim[id].setDuration(Tm)
        self.anim[id].setStartValue(QtCore.QRect(Floor_Pos_x[id], Floor_Pos_y[St], ELEVATOR_LENGTH, ELEVATOR_HEIGHT))
        self.anim[id].setEndValue(QtCore.QRect(Floor_Pos_x[id], Floor_Pos_y[Ed], ELEVATOR_LENGTH, ELEVATOR_HEIGHT))
        self.anim[id].start()
        self.Floor[id] = Ed

    def reshowFloor(self, Floor):
        for i, x in enumerate(self.waitqueue[Floor]):
            if self.updown[x]:
                text = str(x + 1) + "down"
            else:
                text = str(x + 1) + "up"
            self.personlabel[x].setGeometry(Person_Pos_x - (PERSON_LENGTH + 10) * (i + 1), Floor_Pos_y[Floor], PERSON_LENGTH, PERSON_HEIGHT)
            self.personlabel[x].setPixmap(self.get_png_with_text('person.png',text))
            self.personlabel[x].setScaledContents(True)


    def showperson(self, Floor, id, updown):
        self.waitqueue[Floor].append(id - 1)
        self.updown[id - 1] = updown
        print(self.waitqueue)
        self.reshowFloor(Floor)
    
    def delperson(self, Floor, id):
        self.waitqueue[Floor].remove(id - 1)
        print(self.waitqueue)
        self.personlabel[id - 1].setPixmap("")
        self.reshowFloor(Floor)
            
    def persongoinside(self, Floor, id, eid):
        self.inside[eid].append(id)
        text = "INSIDE" + str(eid) + "\n:"
        for x in self.inside[eid]:
            text += str(x) + '\n'
        self.insidelabel[eid].setText(text)
        self.delperson(Floor, id)

    def persongooutside(self, Floor, id, eid):
        self.inside[eid].remove(id)
        text = "INSIDE" + str(eid) + "\n:"
        if len(self.inside[eid]) == 0:
            text += "NULL"
        else:
            for x in self.inside[eid]:
                text += str(x) + '\n'
        self.insidelabel[eid].setText(text)

    def Activate(self):
        self.clock = self.clock + 1
        if self.clock > 10000:
            exit()
        for op in self.event[self.clock]:
            print(self.clock)
            if op[0] == 1 and (op[1] == 7 or op[1] == 8):
                assert(len(op) == 6)
                self.ElevatorMove(op[2], op[3], op[4] * 10, op[5])
            if op[0] == 1 and op[1] == 3:
                print("OPEN")
                self.setElevator(1, op[2])
            if op[0] == 1 and op[1] == 5:
                print("CLOSE")
                self.setElevator(0, op[2])
            if op[0] == 0 and op[1] == 1:
                self.showperson(op[2], op[3], op[4])
            if op[0] == 0 and op[1] == 4:
                self.delperson(op[2], op[3])
            if op[0] == 0 and op[1] == 5:
                self.persongoinside(op[2], op[3], op[4])
            if op[0] == 0 and op[1] == 6:
                self.persongooutside(op[2], op[3], op[4])
            if op[0] == 1 and op[1] == 2:
                self.ElevatorMove(self.Floor[op[3]], 1, op[2] * 10, op[3])


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    gui = MainUi()
    gui.show()
    sys.exit(app.exec_())