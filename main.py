# Это python часть проекта намотки катушек
import serial
from time import sleep
from time import gmtime, strftime
import sys
from PyQt5 import QtWidgets, uic
from PyQt5.QtCore import QObject, QThread, QEventLoop, pyqtSignal

StepsIn = 44
CurPos = 0
CurTurns = 0
CoilCount = 0
CoilStart = 0
CoilLength = 0
TurnToDo = 0
lastMsg = ''


class Arduino(serial.Serial):
    def init(self, port, baud):
        self.port = port
        self.baudrate = baud
        self.parity = serial.PARITY_NONE
        self.bytesize = 8
        if self.isOpen():
            self.close()
        self.open()
        self.flushInput()
        self.flushOutput()

    def send(self, msg):
        self.write(msg.encode('utf-8'))
        print(strftime("%Y-%m-%d %H:%M:%S", gmtime())+' Computer -> Arduino ' + msg.replace("\n", ""))
        sleep(0.1)


duino = Arduino('COM6', 19200)


def shift(n):
    global CurPos
    global lastMsg
    for i in range(abs(n)):
        dir = int(n / abs(n))
        duino.send('s;' + str(dir) + '\n')
        timeout = 0
        while lastMsg != 'R':
            timeout += 1
            sleep(0.005)
            lastmsg()
            if timeout > 1000:
                return ()
        lastMsg = ''
        CurPos += dir


def turn(n):
    global CurTurns
    global TurnToDo
    global form
    global lastMsg
    TurnToDo += n
    for i in range(abs(n)):
        dir = int(n / abs(n))
        duino.send('t;' + str(dir) +';'+str(int(form.checkBox.isChecked())) + '\n')
        timeout = 0
        while lastMsg != 'R':

            timeout += 1
            sleep(0.05)
            print(timeout)
            lastmsg()
            if timeout > 100:
                return ()
        lastMsg = ''
        CurTurns += dir
        TurnToDo -= dir


def d_led_on():
    duino.send('DebugLED;1\n')


def d_led_off():
    duino.send('DebugLED;0\n')


def cur_turns_add(n):
    global CurTurns
    CurTurns += n


def lastmsg():
    global lastMsg
    if duino.inWaiting() > 0:
        lastMsg = duino.readline().decode('utf-8').replace("\r\n", "")
        print(strftime("%Y-%m-%d %H:%M:%S", gmtime())+' Arduino -> Computer ' + lastMsg)
        duino.flushInput()
        duino.flushOutput()


def resetT():
    global CurTurns
    global TurnToDo
    CurTurns = 0
    TurnToDo = 0

def resetS():
    global CurPos
    CurPos = 0


def getpos():
    global CurPos
    duino.flushOutput()
    duino.flushInput()
    duino.send("PosGet\n")
    timeout = 0
    while duino.inWaiting() < 1:
        timeout += 1
        sleep(0.1)
        if timeout > 20:
            CurPos = 0
            return ()
    lastmsg()
    CurPos = int(lastMsg)


def makecoil(begin, end, count):
    global TurnToDo
    global CoilCount
    CoilCount = count
    TurnsinCycle = form.spinBox_7.value()
    ShiftsinCycle = form.spinBox_8.value()
    dir = 1
    if count == 0 or begin >= end:
        return ()
    TurnToDo = count
    i=0
    while i<count:

        if CurPos in range(begin,end+1):
            if (count-i)>=TurnsinCycle:
                TurnToDo -= TurnsinCycle
                turn(TurnsinCycle)
                i+=TurnsinCycle
            else:
                TurnToDo -= (count-i)
                turn(count-i)
                i += (count-i)
            form.lcdNumber_3.display(int((CoilCount - TurnToDo) / (CoilCount -0.01) * 100))
        if CurPos >= end:
            dir = -1
        if CurPos <= begin:
            dir = 1
        for j in range(ShiftsinCycle):
            shift(dir)

    pass

def setmode(n):
    duino.send('setMode;'+str(int(n))+'\n')

class MyThread(QThread):

    def start(self, function_name, *args):
        self.func = function_name
        self.args = args

        super().start()

    def run(self):
        self.func(*self.args)
        self.exit(0)

    def stop(self):
        global lastMsg
        global TurnToDo
        TurnToDo = 0
        lastmsg()
        lastMsg = ''
        self.exit(0)


class uiupd(QThread):
    def __init__(self, a, thread):
        super(uiupd, self).__init__()
        self.a = a
        self.thread = thread
    def run(self):
        while True:
            self.a.lcdNumber.display(CurTurns)
            self.a.lcdNumber_2.display(TurnToDo)
            self.a.lcdNumber_5.display(CurPos)
            self.a.lcdNumber_6.display(CurPos / StepsIn)
            self.a.verticalSlider.setSliderPosition(int(CurPos))
            sleep(0.1)


class Ui(QtWidgets.QMainWindow):

    def __init__(self):
        super(Ui, self).__init__()
        uic.loadUi('Mainwindow/window.ui', self)

        self.debugthread = MyThread()
        self.mythread = MyThread()
        self.updater = uiupd(self,self.mythread)
        self.updater.start()
        self.show()
        # DEBUG FunCS
        self.dLedON.clicked.connect(lambda: self.debugthread.start(d_led_on))  # Debug LedOn
        self.dLedOFF.clicked.connect(lambda: self.debugthread.start(d_led_off))  # Debug LedOff
        # ENDDEBUG

        self.pushButton.clicked.connect(lambda: self.mythread.start(turn, 1))  # Turn ++
        self.pushButton_2.clicked.connect(lambda: self.mythread.start(turn, -1))  # Turn --
        self.pushButton_3.clicked.connect(resetT)  # Turn Reset
        self.pushButton_4.clicked.connect(lambda: self.mythread.start(turn, self.spinBox.value()))  # Turn by number
        self.pushButton_5.clicked.connect(lambda: cur_turns_add(self.spinBox.value()))  # Turn add number
        self.pushButton_6.clicked.connect(lambda: self.spinBox_5.setValue(CurPos))
        self.pushButton_7.clicked.connect(lambda: self.mythread.start(duino.send,self.lineEdit.text()+'\n'))
        self.pushButton_11.clicked.connect(resetS)
        self.pushButton_12.clicked.connect(lambda: self.mythread.start(shift, -1))  # Shift ++
        self.pushButton_13.clicked.connect(lambda: self.mythread.start(shift, 1))  # Shift ++
        self.pushButton_14.clicked.connect(lambda: self.mythread.start(shift, self.spinBox_3.value()))
        self.pushButton_15.clicked.connect(lambda: self.mythread.start(getpos))
        self.pushButton_16.clicked.connect(lambda: self.mythread.start(makecoil,self.spinBox_5.value(),self.spinBox_6.value(),self.spinBox_4.value()))
        self.pushButton_17.clicked.connect(self.mythread.terminate)


def main():
    global form
    app = QtWidgets.QApplication(sys.argv)
    form = Ui()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
