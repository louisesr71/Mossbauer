from PyQt5 import QtCore, QtWidgets, QtSerialPort, QtGui
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg

class Graph(PlotWidget):
    def __init__(self, size, title, x_label='Time', y_label='Counts'):
        PlotWidget.__init__(self)
        
        self.setTitle(title, size='10pt')
        self.setMouseEnabled(x=False)
        self.setXRange(0, size, padding=0.02)
        self.setLabel('bottom', x_label)
        self.setLabel('left', y_label)
        
        self.bins = list(range(size))
        self.values = [0] * size
        
        self.data = self.plot(self.bins, self.values)
        
        
       

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)
        
        self.setWindowTitle("Mossbauer GUI")
        
        # Set up serial connection.
        self.serial = QtSerialPort.QSerialPort('COM3', baudRate=QtSerialPort.QSerialPort.Baud115200, readyRead=self.receive)
        self.serial.open(QtCore.QIODevice.ReadWrite)
        
        # Layout.
        lay_window = QtWidgets.QHBoxLayout()
        
        lay_graphs = QtWidgets.QVBoxLayout()
        lay_buttons = QtWidgets.QVBoxLayout()
        
        # Set up graphs.
        self.pha = Graph(4096, 'Pulse Amplitude', x_label='Amplitude')
        self.mcs = Graph(1024, 'Multi-Channel Scalar', x_label='Energy')
        lay_graphs.addWidget(self.pha)
        lay_graphs.addWidget(self.mcs)
        
        # Setup buttons.
        self.connect_btn = QtWidgets.QPushButton(text="Connect", checkable=True, toggled=self.connect_toggled)
        self.start_btn = QtWidgets.QPushButton(text="Start", checkable=True, toggled=self.start_toggled)
        self.reset_btn = QtWidgets.QPushButton(text="Reset", pressed=self.reset_pressed)
        
        self.freq_lbl = QtWidgets.QLabel("Piezo Frequency (Hz):")
        self.freq_txt = QtWidgets.QLineEdit()
        self.freq_txt.setMaxLength(3)
        self.freq_txt.setText("10")
        self.ser_lbl = QtWidgets.QLabel("Serial Interval (s):     ")
        self.ser_txt = QtWidgets.QLineEdit()
        self.ser_txt.setMaxLength(5)
        self.ser_txt.setText("2")
        self.update_btn = QtWidgets.QPushButton(text="Update", pressed=self.update_pressed)
        
        self.save_txt = QtWidgets.QLineEdit()
        self.save_btn = QtWidgets.QPushButton(text="Save")
        
        lay_start_reset = QtWidgets.QHBoxLayout()
        lay_freq = QtWidgets.QHBoxLayout()
        lay_ser = QtWidgets.QHBoxLayout()
        lay_save = QtWidgets.QHBoxLayout()
        
        lay_buttons.addWidget(self.connect_btn)
        
        lay_start_reset.addWidget(self.start_btn)
        lay_start_reset.addWidget(self.reset_btn)
        lay_buttons.addLayout(lay_start_reset)
        
        lay_buttons.addSpacing(50)
        
        lay_freq.addWidget(self.freq_lbl)
        lay_freq.addWidget(self.freq_txt)
        lay_buttons.addLayout(lay_freq)
        
        lay_ser.addWidget(self.ser_lbl)
        lay_ser.addWidget(self.ser_txt)
        lay_buttons.addLayout(lay_ser)
        
        lay_buttons.addWidget(self.update_btn)
        
        lay_buttons.addSpacing(50)
        
        lay_save.addWidget(self.save_txt)
        lay_save.addWidget(self.save_btn)
        lay_buttons.addLayout(lay_save)
        
        lay_buttons.setAlignment(QtCore.Qt.AlignTop)
        
        lay_window.addLayout(lay_buttons, 1)
        lay_window.addLayout(lay_graphs, 4)
        

        
        widget = QtWidgets.QWidget()
        widget.setLayout(lay_window)
        self.setCentralWidget(widget)
        
        
        
    def receive(self):
        while self.serial.canReadLine():
            text = self.serial.readLine().data().decode()
            text = text.rstrip('\r\n')
            values = [int(i) for i in text.split(',')]
            if len(values) == 5120:
                self.pha.data.setData(y=values[0:4096])
                self.mcs.data.setData(y=values[4096:5120])
            else:
                print("Data array is wrong size")
                print(len(values))
                
    def connect_toggled(self, checked):
        self.connect_btn.setText('Disconnect' if checked else 'Connect')
    
    def start_toggled(self, checked):
        self.start_btn.setText('Stop' if checked else 'Start')
    
    def reset_pressed(self):
        # Reset graphs and clear Teensy arrays.
        self.pha.data.setData(y=([0]*4096))
        self.mcs.data.setData(y=([0]*1024))
        self.serial.write('r'.encode())
        
    def update_pressed(self):
        # Send new values for frequency and serialInterval to Teensy.
        self.serial.write(('u' + self.freq_txt.text() + ',' + self.ser_txt.text()).encode())  

if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication([])
    w = MainWindow()
    w.show()
    sys.exit(app.exec_())