import os.path
import sys
import traceback
from datetime import datetime
import time
from collections import deque
import numpy as np
import pyqtgraph as pg
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from PyQt5.QtGui import *
import csv
import win32api
import win32con
width = win32api.GetSystemMetrics(win32con.SM_CXSCREEN)
height = win32api.GetSystemMetrics(win32con.SM_CYSCREEN)
import serial.tools.list_ports
import serial
import math
from ui import Ui_Form
import json
from serial_in import  parse_input_serial_data ,INPUT_PACKET_SIZE
from serial_out import encode_serial_out_data

import warnings
warnings.filterwarnings('ignore', message='All-NaN slice encountered')

def rtd_temperature_pt1000(resistance_ohm, r0=1000.0, alpha=0.00385):
    """
    Converts RTD resistance to temperature in Celsius for a Pt1000 sensor.
    Based on simplified Callendar-Van Dusen: T = (R - R0) / (alpha * R0)

    Parameters:
    - resistance_ohm: Measured resistance in ohms
    - r0: Resistance at 0°C (default 1000 ohms for Pt1000)
    - alpha: Temperature coefficient (default 0.00385 for standard Pt1000)

    Returns:
    - Temperature in Celsius
    """
    temperature_c = (resistance_ohm - r0) / (alpha * r0)
    return temperature_c


def ntc_temperature_beta(resistance_ohm, r0=10000.0, beta=3380.0, t0=25.0):
    """
    Converts NTC thermistor resistance to temperature in Celsius using the Beta equation.

    Parameters:
    - resistance_ohm: Measured resistance in ohms
    - r0: Resistance at reference temperature (typically 10kΩ @ 25°C)
    - beta: Beta constant (K)
    - t0: Reference temperature in Celsius (default 25°C)

    Returns:
    - Temperature in Celsius
    """
    t0_k = t0 + 273.15  # Convert to Kelvin
    try:
        temp_k = 1 / (1 / t0_k + (1 / beta) * math.log(resistance_ohm / r0))
        temperature_c = temp_k - 273.15
        return temperature_c
    except ValueError:
        return float('nan')  # Handle log of negative or zero resistance



class USBCommThread(QThread):
    data_updated = pyqtSignal(dict)
    config_updated = pyqtSignal(dict)
    timeout_occurred = pyqtSignal()
    def __init__(self,ser):
        super().__init__()
        self.ser = ser
        self.connect_status = True
        self.last_data_time = time.time()
        self.timeout_seconds = 2

    def run(self):
        try:
            # Simulate device communication (actually needs to be connected to specific hardware)
            while self.connect_status:
                # Checking for timeout
                if time.time() - self.last_data_time > self.timeout_seconds:
                    self.close_port()
                    self.timeout_occurred.emit()
                    break
                data = self.ser.read(INPUT_PACKET_SIZE)
                if len(data) == INPUT_PACKET_SIZE:
                    # serial_data = receive_random_data()  # Simulate receiving data
                    serial_data = parse_input_serial_data(data)
                    self.data_updated.emit(serial_data)
                    self.last_data_time = time.time()

                # time.sleep(0.5)  # delete later

                time.sleep(0.1)
        except Exception as e:
            self.close_port()
            print(e)
            traceback.print_exc()

    def close_port(self):
        self.connect_status = False

def change_value(button):
    try:
        button.setText("Off")
        button.setStyleSheet("background-color:green;")
    except Exception as e:
        print(e)


class MainWindow(QWidget, Ui_Form):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.recv = None
        self.count = None
        self.setupUi(self)
        self.retranslateUi(self)
        self.all_dic = {"ch1l": self.doubleSpinBox.value(), "ch1u": self.doubleSpinBox_2.value(),"ch1state":"Disabled","ch1label":self.comboBox_13,"ch1status":False,"ir1v":self.comboBox_3,
                       "ch2l": self.doubleSpinBox_3.value(), "ch2u": self.doubleSpinBox_4.value(),"ch2state":"Disabled","ch2label":self.comboBox_14,"ch2status":False,"ir2v":self.comboBox_5,
                       "ch3l": self.doubleSpinBox_5.value(), "ch3u": self.doubleSpinBox_6.value(),"ch3state":"Disabled","ch3label":self.comboBox_15,"ch3status":False,"ir3v":self.comboBox_7,
                       "ch4l": self.doubleSpinBox_7.value(), "ch4u": self.doubleSpinBox_8.value(),"ch4state":"Disabled","ch4label":self.comboBox_16,"ch4status":False,"ir4v":self.comboBox_9,
                       "ch5l": self.doubleSpinBox_9.value(), "ch5u": self.doubleSpinBox_10.value(),"ch5state":"Disabled","ch5label":self.comboBox_17,"ch5status":False,
                       "ch6l": self.doubleSpinBox_11.value(), "ch6u": self.doubleSpinBox_12.value(),"ch6state":"Disabled","ch6label":self.comboBox_18,"ch6status":False,
                       "ch7l": self.doubleSpinBox_13.value(), "ch7u": self.doubleSpinBox_14.value(),"ch7state":"Disabled","ch7label":self.comboBox_25,"ch7status":False,
                       "ch8l": self.doubleSpinBox_19.value(), "ch8u": self.doubleSpinBox_20.value(),"ch8state":"Disabled","ch8label":self.comboBox_26,"ch8status":False,
                        "ch1units": self.comboBox_12, "ch2units": self.comboBox_56,"ch3units": self.comboBox_58,"ch4units": self.comboBox_60,
                        "ch1currentSource_widget": self.comboBox_11, "ch2currentSource_widget": self.comboBox_55,"ch3currentSource_widget": self.comboBox_57,"ch4currentSource_widget": self.comboBox_59}
        self.alarm_status_button = [self.pushButton, self.pushButton_2, self.pushButton_3, self.pushButton_4,
                                    self.pushButton_5, self.pushButton_8, self.pushButton_7, self.pushButton_6]
        self.all_dic[f"ch1label"].currentIndexChanged.connect(lambda : change_value(self.alarm_status_button[0]))
        self.all_dic[f"ch2label"].currentIndexChanged.connect(lambda: change_value(self.alarm_status_button[1]))
        self.all_dic[f"ch3label"].currentIndexChanged.connect(lambda: change_value(self.alarm_status_button[2]))
        self.all_dic[f"ch4label"].currentIndexChanged.connect(lambda: change_value(self.alarm_status_button[3]))
        self.all_dic[f"ch5label"].currentIndexChanged.connect(lambda: change_value(self.alarm_status_button[4]))
        self.all_dic[f"ch6label"].currentIndexChanged.connect(lambda: change_value(self.alarm_status_button[5]))
        self.all_dic[f"ch7label"].currentIndexChanged.connect(lambda: change_value(self.alarm_status_button[6]))
        self.all_dic[f"ch8label"].currentIndexChanged.connect(lambda: change_value(self.alarm_status_button[7]))
        for i in range(8):
            self.all_dic[f"ch{i+1}label"].setCurrentIndex(1)
            if i < 4:
                self.all_dic[f"ch{i + 1}units"].setCurrentIndex(0)
                self.all_dic[f"ch{i + 1}uv"] = self.all_dic[f"ch{i + 1}units"].currentIndex()
                self.all_dic[f"ch{i+1}currentSource"] = self.all_dic[f"ch{i + 1}currentSource_widget"].currentIndex()
                self.all_dic[f"ch{i + 1}range"] = self.all_dic[f"ir{i + 1}v"].currentIndex()
                if self.all_dic[f"ch{i + 1}units"].currentIndex() != 2:
                    self.all_dic[f"ch{i+1}sensorType"] = self.all_dic[f"ch{i + 1}units"].currentIndex()
                else:
                    self.all_dic[f"ch{i + 1}sensorType"] = 0
        self.limit_dic = {"ch1l": self.doubleSpinBox, "ch1u": self.doubleSpinBox_2,
                     "ch2l": self.doubleSpinBox_3, "ch2u": self.doubleSpinBox_4,
                     "ch3l": self.doubleSpinBox_5, "ch3u": self.doubleSpinBox_6,
                     "ch4l": self.doubleSpinBox_7, "ch4u": self.doubleSpinBox_8,
                     "ch5l": self.doubleSpinBox_9, "ch5u": self.doubleSpinBox_10,
                     "ch6l": self.doubleSpinBox_11, "ch6u": self.doubleSpinBox_12,
                     "ch7l": self.doubleSpinBox_13, "ch7u": self.doubleSpinBox_14,
                     "ch8l": self.doubleSpinBox_19, "ch8u": self.doubleSpinBox_20}
        self.temp_com_list = [[self.comboBox_11, self.comboBox_12],[self.comboBox_55, self.comboBox_56],[self.comboBox_57, self.comboBox_58],[self.comboBox_59, self.comboBox_60]]
        self.range_com_list = [self.comboBox_3,self.comboBox_5,self.comboBox_7,self.comboBox_9]
        self.pushButton_38.clicked.connect(self.input_range_set)
        for i in self.alarm_status_button:
            i.setStyleSheet("background-color:green;")
        self.time_set()
        self.sync_rtc_time_status = False
        self.plots = []
        self.curves_list = []
        self.curves = []
        self.TimeTimer = QTimer()
        self.TimeTimer.timeout.connect(self.time_set)
        self.TimeTimer.start(1000)
        self.plot_timer = QTimer()
        self.plot_timer.timeout.connect(self.update_replay_plot)
        self.PortTimer = QTimer()
        self.PortTimer.timeout.connect(self.refresh_serial_ports)
        self.existing_ports = set()
        self.PortTimer.start(1000)
        self.connect_state = False
        self.recording = False
        self.csv_file = None
        self.csv_writer = None
        self.first_timestamp = None
        self.start_time = None
        self.points_visible = 1000
        self.replay_path = None
        self.optical_link_state = False
        self.first_connect_sync = False
        self.recording_state = False
        self.replay_status = False
        self.sync_data = None
        # Load and convert time
        self.replay_data = {
            'timestamps': [],  # Store relative time (seconds)
            'channels': [[] for _ in range(8)]
        }
        self.ser = serial.Serial()
        self.ui_set()
        self.init_plot()
        self.time_buffer = deque(maxlen=self.points_visible)
        self.data_buffers = {i: deque(maxlen=self.points_visible) for i in range(8)}
        self.data_tem_buffers = {i: deque(maxlen=self.points_visible) for i in range(8)}
        self.load_setting()
        self.write_setting()
        print(self.all_dic)

    def input_range_set(self):
        try:
            for i in range(4):
                self.all_dic[f"ch{i+1}range"] = self.all_dic[f"ir{i+1}v"].currentIndex()
                if self.all_dic[f"ch{i+1}range"] == 0:
                    if self.all_dic[f"ch{i+1}l"] < -1:
                        self.all_dic[f"ch{i+1}l"] = -1
                        self.limit_dic[f"ch{i+1}l"].setValue(self.all_dic[f"ch{i+1}l"])
                    if self.all_dic[f"ch{i+1}u"] > 1:
                        self.all_dic[f"ch{i + 1}u"] = -1
                        self.limit_dic[f"ch{i + 1}u"].setValue(self.all_dic[f"ch{i + 1}u"])
            self.write_setting()
            self.write_data()
        except Exception as e:
            print(e)
            traceback.print_exc()

    def refresh_serial_ports(self):
        try:
            current_ports = set(p.device for p in serial.tools.list_ports.comports())

            new_ports = current_ports - self.existing_ports
            if new_ports:
                for port in new_ports:
                    self.comboBox.addItem(port)
                    self.existing_ports.add(port)
            removed_ports = self.existing_ports - current_ports
            if removed_ports:
                for port in removed_ports:
                    if port == self.ser.port and self.ser.is_open:
                        self.connect_state = False
                        self.first_connect_sync = False
                        self.optical_link_state = False
                        self.recv.close_port()
                        self.toolButton_2.setText("Connect")
                        self.label_2.setText("Disconnect")
                        self.label_2.setStyleSheet("color:black;")
                        self.label_4.setText("Optical Link State：DisConnect")
                        self.label_4.setStyleSheet("color:black;")
                        self.write_data(self.connect_state)
                    index = self.comboBox.findText(port)
                    if index != -1:
                        self.comboBox.removeItem(index)
                    self.existing_ports.remove(port)
        except Exception as e:
            print(f"Error refreshing serial ports: {e}")

    def write_setting(self):
        try:
            all_dic = {"ch1l":self.doubleSpinBox.value(),"ch1u":self.doubleSpinBox_2.value(),
                       "ch2l": self.doubleSpinBox_3.value(), "ch2u": self.doubleSpinBox_4.value(),
                       "ch3l": self.doubleSpinBox_5.value(), "ch3u": self.doubleSpinBox_6.value(),
                       "ch4l": self.doubleSpinBox_7.value(), "ch4u": self.doubleSpinBox_8.value(),
                       "ch5l": self.doubleSpinBox_9.value(), "ch5u": self.doubleSpinBox_10.value(),
                       "ch6l": self.doubleSpinBox_11.value(), "ch6u": self.doubleSpinBox_12.value(),
                       "ch7l": self.doubleSpinBox_13.value(), "ch7u": self.doubleSpinBox_14.value(),
                       "ch8l": self.doubleSpinBox_19.value(), "ch8u": self.doubleSpinBox_20.value(),
                       "ich1": self.comboBox_3.currentText(), "tch1": self.comboBox_11.currentText(),
                       "ich2": self.comboBox_5.currentText(), "tch2": self.comboBox_55.currentText(),
                       "ich3": self.comboBox_7.currentText(), "tch3": self.comboBox_57.currentText(),
                       "ich4": self.comboBox_9.currentText(), "tch4": self.comboBox_59.currentText(),
                        "tch11": self.comboBox_12.currentText(),"tch21": self.comboBox_56.currentText(),
                       "tch31": self.comboBox_58.currentText(),"tch41": self.comboBox_60.currentText(),
                       "yaxis":self.doubleSpinBox2.value(),"points_visible":self.spinBox.value(),
                       }
            with open("all_data.json","w",encoding="utf-8")as f:
                json.dump(all_dic, f, ensure_ascii=False, indent=4)
        except Exception as e:
            print(e)
            traceback.print_exc()

    def init_plot(self):
        try:
            self.plots = []
            self.curves_list = []
            layout = QVBoxLayout()
            colors_rgb = [
                (255, 0, 0),  # red
                (0, 255, 0),  # green
                (0, 0, 255),  # blue
                (255, 255, 0),  # yellow
                (255, 0, 255),  # pink
                (0, 255, 255),  # Cyan
                (255, 165, 0),  # Orange
                (128, 0, 128)  # Violet
            ]
            for i in range(3):
                plot = pg.PlotWidget(title=["Voltage channels","Acceleration channels","Temperature channel"][i])
                units_list = ['V', 'm/s²', '°C']
                plot.setLabel('left', 'Value', units=units_list[i])
                plot.setLabel('bottom', 'Time', units='s')
                plot.addLegend()
                plot.showGrid(x=True, y=True)
                plot.setXRange(0, 10)
                plot.setYRange(0, 10)
                # Initialize 8 curves
                self.curves = []
                if i == 0:
                    for j in range(4):
                        curve = plot.plot(
                            name=f"CH{j + 1}",
                            pen=colors_rgb[j],
                            symbol='x',
                            symbolSize=5,
                            symbolBrush=colors_rgb[j]
                        )
                        self.curves.append(curve)
                elif i == 1:
                    for j in range(3):
                        curve = plot.plot(
                            name=f"CH{j + 5}",
                            pen=colors_rgb[j+4],
                            symbol='x',
                            symbolSize=5,
                            symbolBrush=colors_rgb[j+4]
                        )
                        self.curves.append(curve)
                elif i == 2:
                    curve = plot.plot(
                        name=f"CH8",
                        pen=colors_rgb[-1],
                        symbol='x',
                        symbolSize=5,
                        symbolBrush=colors_rgb[-1]
                    )
                    self.curves.append(curve)
                    for j in range(4):
                        curve = plot.plot(
                            name=f"CH{j+1}",
                            pen=colors_rgb[j],
                            symbol='x',
                            symbolSize=5,
                            symbolBrush=colors_rgb[j]
                        )
                        self.curves.append(curve)
                layout.addWidget(plot)
                self.plots.append(plot)
                self.curves_list.append(self.curves)
                # Mouseover Events
                plot.scene().sigMouseMoved.connect(
                    lambda pos, plot_ = plot: self.show_hover_data(pos, plot_))
            self.widget.setLayout(layout)
        except Exception as e:
            print(e)
            traceback.print_exc()

    def clear_plots(self):
        try:
            for buf in self.data_buffers.values():
                buf.clear()
            for buf in self.data_tem_buffers.values():
                buf.clear()
            for i in self.curves_list:
                for curves in i:
                    curves.setData([], [])
            self.start_time = time.time()
        except Exception as e:
            print(e)
            traceback.print_exc()

    def show_hover_data(self, pos, plot):
        try:
            # Get the time corresponding to the mouse position
            vb = plot.plotItem.vb
            mouse_point = vb.mapSceneToView(pos)
            x_val = mouse_point.x()
            if self.replay_status:
                time_buffer = self.replay_data["timestamps"]
                # If the data is empty, return directly
                if len(time_buffer) == 0:
                    self.label_3.setText("Date Not Ready")
                    return
                time_diff = np.abs(time_buffer - x_val)
                closest_idx = np.argmin(time_diff)  # Find the nearest index
                closest_timestamp = time_buffer[closest_idx]
                data_buffers = self.replay_data["channels"]
                closest_channel_data = [channel[closest_idx] for channel in data_buffers]
                # Constructing display text
                text = f"Time: {closest_timestamp:.2f}s "
                for i, data_point in enumerate(closest_channel_data):
                    unit = self.channel_units[i]
                    if i < 4:
                        if unit == "deg C":
                            text += f"CH{i + 1}: {data_point:.3f}℃ "
                        else:
                            text += f"CH{i + 1}: {data_point:.3f}V "
                    elif i in [4, 5, 6]:
                        text += f"CH{i + 1}: {data_point:.3f}m/s² "
                    elif i == 7:
                        text += f"CH{i + 1}: {data_point:.3f}℃ "
            else:
                # If the data is empty, return directly
                if not self.time_buffer:
                    self.label_3.setText("Date Not Ready")
                    return

                # Convert the time buffer to a numpy array to quickly find the nearest time point
                time_array = np.array(self.time_buffer)
                time_diff = np.abs(time_array - x_val)
                closest_idx = np.argmin(time_diff)  # Find the nearest index
                closest_time = time_array[closest_idx]

                # Check if the time is within a reasonable range (avoid marginal errors)
                max_time_diff = 0.5 * (time_array[-1] - time_array[0]) / len(time_array)  # Dynamic Error Threshold
                if abs(closest_time - x_val) > max_time_diff:
                    self.label_3.setText(f"Time exceeds the range.: {x_val:.2f}s")
                    return

                # Constructing display text
                text = f"Time: {closest_time:.2f}s "
                for i in range(8):
                    # Get the data of the i-th channel
                    ch_data = list(self.data_buffers[i])
                    if closest_idx < len(ch_data):
                        value = ch_data[closest_idx][1]
                        if i < 4:
                            if ch_data[closest_idx][2] == 1:
                                value1 = self.data_tem_buffers[i][closest_idx][1]
                                text += f"CH{i + 1}: {value1:.3f}℃ "
                            else:
                                text += f"CH{i + 1}: {value:.3f}V "
                        elif i in [4,5,6]:
                            text += f"CH{i + 1}: {value:.3f}m/s² "
                        elif i == 7:
                            text += f"CH{i + 1}: {value:.3f}℃ "
                    else:
                        text += f"CH{i + 1}: ---"  # Display placeholder when insufficient data

            self.label_3.setText(text)
        except Exception as e:
            print(e)
            traceback.print_exc()

    def ui_set(self):
        try:
            font = QFont()
            font.setFamily("Arial")
            font.setBold(False)
            font.setPointSize(int(width / 170))
            for widget in self.findChildren(QLabel):
                widget.setFont(font)
            for widget in self.findChildren(QDoubleSpinBox):
                widget.setFont(font)
            for widget in self.findChildren(QComboBox):
                widget.setFont(font)
            font.setBold(True)
            font.setPointSize(int(width / 135))
            self.label.setFont(font)
            self.label_7.setFont(font)
            self.label_2.setFont(font)
            font.setPointSize(int(width / 160))
            font.setBold(False)
            for widget in self.findChildren(QToolButton):
                widget.setFont(font)
            for widget in self.findChildren(QPushButton):
                widget.setFont(font)
            self.pushButton.clicked.connect(lambda :self.set_low_and_upp(0,self.pushButton,self.all_dic["ch1label"]))
            self.pushButton_2.clicked.connect(lambda: self.set_low_and_upp(1, self.pushButton_2, self.all_dic["ch2label"]))
            self.pushButton_3.clicked.connect(lambda: self.set_low_and_upp(2, self.pushButton_3, self.all_dic["ch3label"]))
            self.pushButton_4.clicked.connect(lambda: self.set_low_and_upp(3, self.pushButton_4, self.all_dic["ch4label"]))
            self.pushButton_5.clicked.connect(lambda: self.set_low_and_upp(4, self.pushButton_5, self.all_dic["ch5label"]))
            self.pushButton_8.clicked.connect(lambda: self.set_low_and_upp(5, self.pushButton_8, self.all_dic["ch6label"]))
            self.pushButton_7.clicked.connect(lambda: self.set_low_and_upp(6, self.pushButton_7, self.all_dic["ch7label"]))
            self.pushButton_6.clicked.connect(lambda: self.set_low_and_upp(7, self.pushButton_6, self.all_dic["ch8label"]))
            self.doubleSpinBox.valueChanged.connect(lambda :self.set_input_limit(self.doubleSpinBox,"ch1l",1))
            self.doubleSpinBox_2.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_2,"ch1u",1))
            self.doubleSpinBox_3.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_3,"ch2l",2))
            self.doubleSpinBox_4.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_4,"ch2u",2))
            self.doubleSpinBox_5.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_5,"ch3l",3))
            self.doubleSpinBox_6.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_6,"ch3u",3))
            self.doubleSpinBox_7.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_7,"ch4l",4))
            self.doubleSpinBox_8.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_8,"ch4u",4))
            self.doubleSpinBox_9.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_9,"ch5l",5))
            self.doubleSpinBox_10.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_10,"ch5u",5))
            self.doubleSpinBox_11.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_11,"ch6l",6))
            self.doubleSpinBox_12.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_12,"ch6u",6))
            self.doubleSpinBox_13.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_13,"ch7l",7))
            self.doubleSpinBox_14.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_14,"ch7u",7))
            self.doubleSpinBox_19.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_19,"ch8l",8))
            self.doubleSpinBox_20.valueChanged.connect(lambda: self.set_input_limit(self.doubleSpinBox_20,"ch8u",8))
            self.toolButton.clicked.connect(self.sync_rtc_time)
            self.pushButton_13.clicked.connect(self.set_temp)
            self.pushButton_14.clicked.connect(self.set_limit)
            self.toolButton_3.setStyleSheet("color:green")
            self.label_5.setStyleSheet("color:red")
            self.toolButton_2.clicked.connect(self.port_connect)
            self.toolButton_3.clicked.connect(self.toggle_recording)
            self.toolButton_7.clicked.connect(self.real_time)
            self.toolButton_5.clicked.connect(self.replay)
            self.toolButton_6.clicked.connect(self.clear_plots)
            self.toolButton_4.clicked.connect(self.change_plot)
            self.pushButton_20.clicked.connect(self.change_yaxis)
            self.pushButton_21.clicked.connect(self.change_max)
        except Exception as e:
            print(e)
            traceback.print_exc()

    def real_time(self):
        try:
            self.replay_status = False
            self.plot_timer.stop()
            self.toolButton_5.setText("Replay")
        except Exception as e:
            print(e)
            traceback.print_exc()


    def sync_rtc_time(self):
        try:
            self.sync_rtc_time_status = True
            self.write_setting()
            self.write_data()
        except Exception as e:
            print(e)

    # def set_input_limit(self, double, value, key):
    #     try:
    #         if "l" == value[-1]:
    #             upp_key = f"ch{key}u"
    #             lower = self.limit_dic[value].value()
    #             upper = self.limit_dic[upp_key].value()
    #         else:
    #             low_key = f"ch{key}l"
    #             lower = self.limit_dic[low_key].value()
    #             upper = self.limit_dic[value].value()
    #         if lower <= upper:
    #             if key in [1,2,3,4]:
    #                 if self.all_dic[f"ch{key}uv"] == 1: #Deg
    #                     V = 1 if self.all_dic[f"ch{key}range"] == 0 else 10
    #                     I = 0.00001 if self.all_dic[f"ch{key}currentSource"] == 0 else 0.0002
    #                     if self.all_dic[f"ch{key}sensorType"] == 0:
    #                         if "u" == value[-1]:    #upper
    #                             tem = rtd_temperature_pt1000(V/I)
    #                             if upper > tem:
    #                                 if self.all_dic[value] > tem:
    #                                     double.setValue(tem)
    #                                     if lower > tem:
    #                                         low_key = f"ch{key}l"
    #                                         self.limit_dic[low_key].setValue(tem)
    #                                 else:
    #                                     double.setValue(self.all_dic[value])
    #                     elif self.all_dic[f"ch{key}sensorType"] == 1:
    #                         if "u" == value[-1]:
    #                             tem = ntc_temperature_beta(V / I)
    #                             if upper > tem:
    #                                 if self.all_dic[value] > tem:
    #                                     double.setValue(tem)
    #                                     if lower > tem:
    #                                         low_key = f"ch{key}l"
    #                                         self.limit_dic[low_key].setValue(tem)
    #                                 else:
    #                                     double.setValue(self.all_dic[value])
    #                 else:
    #                     if self.all_dic[f"ch{key}range"] == 0:
    #                         if "l" == value[-1]:
    #                             if lower >= 1 or lower < -1:
    #                                 double.setValue(self.all_dic[value])
    #                         else:
    #                             if upper > 1 or upper <= -1:
    #                                 double.setValue(self.all_dic[value])
    #                     else:
    #                         if "l" == value[-1]:
    #                             if lower >= 10 or lower < -10:
    #                                 double.setValue(self.all_dic[value])
    #                         else:
    #                             if upper > 10 or upper <= -10:
    #                                 double.setValue(self.all_dic[value])
    #         else:
    #             double.setValue(self.all_dic[value])
    #     except Exception as e:
    #         print(e)
    #         traceback.print_exc()

    def set_input_limit(self, double, value, key):
        try:
            if "l" == value[-1]:
                upp_key = f"ch{key}u"
                lower = self.limit_dic[value].value()
                upper = self.limit_dic[upp_key].value()
            else:
                low_key = f"ch{key}l"
                lower = self.limit_dic[low_key].value()
                upper = self.limit_dic[value].value()
            if lower <= upper:
                if key in [1,2,3,4]:
                    if self.all_dic[f"ch{key}uv"] == 1: #Deg
                        pass
                    else:
                        if self.all_dic[f"ch{key}range"] == 0:
                            if "l" == value[-1]:
                                if lower >= 1 or lower < -1:
                                    double.setValue(self.all_dic[value])
                            else:
                                if upper > 1 or upper <= -1:
                                    double.setValue(self.all_dic[value])
                        else:
                            if "l" == value[-1]:
                                if lower >= 10 or lower < -10:
                                    double.setValue(self.all_dic[value])
                            else:
                                if upper > 10 or upper <= -10:
                                    double.setValue(self.all_dic[value])
            else:
                double.setValue(self.all_dic[value])
        except Exception as e:
            print(e)
            traceback.print_exc()

    def set_limit(self):
        try:
            for i in range(1, 9):
                low_key = f"ch{i}l"
                upp_key = f"ch{i}u"
                alarm_mode = f"ch{i}label"
                self.all_dic[low_key] = self.limit_dic[low_key].value()
                self.all_dic[upp_key] = self.limit_dic[upp_key].value()
                self.all_dic[f"ch{i}state"] = self.all_dic[alarm_mode].currentText()
                print(i,self.all_dic[f"ch{i}state"])
            self.write_setting()
            self.write_data()
        except Exception as e:
            print(e)
            traceback.print_exc()

    def set_temp(self):
        try:
            for i in range(1,5):
                self.all_dic[f"ch{i}currentSource"] = self.all_dic[f"ch{i}currentSource_widget"].currentIndex()
                if self.all_dic[f"ch{i}units"].currentIndex() == 2:
                    self.all_dic[f"ch{i}sensorType"] = 0
                    self.all_dic[f"ch{i}uv"] = 0
                else:
                    self.all_dic[f"ch{i}sensorType"] = self.all_dic[f"ch{i}units"].currentIndex()
                    self.all_dic[f"ch{i}uv"] = 1
            self.write_setting()
            self.write_data()
        except Exception as e:
            print(e)
            traceback.print_exc()

    def set_low_and_upp(self,ch,btn,label):
        try:
            #Disabled  Live Latching
            if btn.text() == "On" and label.currentIndex() == 2:
                self.all_dic[f"ch{ch+1}status"] = False
                btn.setText("Off")
                btn.setStyleSheet("background-color:green;")
        except Exception as e:
            print(e)
            traceback.print_exc()

    def change_max(self):
        try:
            value = self.spinBox.value()
            self.points_visible = value
            """Update maxlen of all data buffers (based on the latest self.points_visible)"""
            for channel in self.data_buffers:
                # Keep the current data, recreate the deque and update maxlen
                old_data = list(self.data_buffers[channel])
                self.data_buffers[channel] = deque(old_data, maxlen=self.points_visible)
            if self.replay_path:
                self._load_replay_data(self.replay_path)
                self._setup_replay_plots()
            self.write_setting()
        except Exception as e:
            print(e)
            traceback.print_exc()

    def show_all_plots(self):
        try:
            for plot in self.plots:
                plot.setVisible(True)
            self.widget.layout().setStretch(0, 1)
            self.widget.layout().setStretch(1, 1)
            self.widget.layout().setStretch(2, 1)
        except Exception as e:
            print(e)
            traceback.print_exc()
            traceback.print_exc()

    def change_plot(self):
        try:
            index = self.comboBox_2.currentIndex()
            # Hide all charts
            if index == 3:
                self.show_all_plots()
            else:
                for i, plot in enumerate(self.plots):
                    plot.setVisible(i == index)
        except Exception as e:
            print(e)
            traceback.print_exc()
            traceback.print_exc()

    def disable_all(self):
        try:
            for i in self.tabWidget.findChildren(QDoubleSpinBox):
                i.setEnabled(False)
            for i in self.tabWidget.findChildren(QComboBox):
                i.setEnabled(False)
            for i in self.tabWidget.findChildren(QPushButton):
                i.setEnabled(False)
        except Exception as e:
            print(e)
            traceback.print_exc()

    def enable_all(self):
        try:
            for i in self.tabWidget.findChildren(QPushButton):
                i.setEnabled(True)
            for i in self.tabWidget.findChildren(QDoubleSpinBox):
                i.setEnabled(True)
            for i in self.tabWidget.findChildren(QComboBox):
                i.setEnabled(True)
        except Exception as e:
            print(e)
            traceback.print_exc()

    def load_setting(self):
        try:
            if os.path.exists("all_data.json"):
                with open("all_data.json", "r", encoding="utf-8") as file:
                    data = json.load(file)
                # Set the value of the control
                self.doubleSpinBox.setValue(data.get("ch1l", 0.0))
                self.doubleSpinBox_2.setValue(data.get("ch1u", 0.0))
                self.doubleSpinBox_3.setValue(data.get("ch2l", 0.0))
                self.doubleSpinBox_4.setValue(data.get("ch2u", 0.0))
                self.doubleSpinBox_5.setValue(data.get("ch3l", 0.0))
                self.doubleSpinBox_6.setValue(data.get("ch3u", 0.0))
                self.doubleSpinBox_7.setValue(data.get("ch4l", 0.0))
                self.doubleSpinBox_8.setValue(data.get("ch4u", 0.0))
                self.doubleSpinBox_9.setValue(data.get("ch5l", 0.0))
                self.doubleSpinBox_10.setValue(data.get("ch5u", 0.0))
                self.doubleSpinBox_11.setValue(data.get("ch6l", 0.0))
                self.doubleSpinBox_12.setValue(data.get("ch6u", 0.0))
                self.doubleSpinBox_13.setValue(data.get("ch7l", 0.0))
                self.doubleSpinBox_14.setValue(data.get("ch7u", 0.0))
                self.doubleSpinBox_19.setValue(data.get("ch8l", 0.0))
                self.doubleSpinBox_20.setValue(data.get("ch8u", 0.0))
                self.doubleSpinBox2.setValue(data.get("yaxis", 0.0))
                self.spinBox.setValue(data.get("points_visible", 0))

                self.comboBox_3.setCurrentText(data.get("ich1", ""))
                self.comboBox_11.setCurrentText(data.get("tch1", ""))
                self.comboBox_5.setCurrentText(data.get("ich2", ""))
                self.comboBox_55.setCurrentText(data.get("tch2", ""))
                self.comboBox_7.setCurrentText(data.get("ich3", ""))
                self.comboBox_57.setCurrentText(data.get("tch3", ""))
                self.comboBox_9.setCurrentText(data.get("ich4", ""))
                self.comboBox_59.setCurrentText(data.get("tch4", ""))
                self.comboBox_12.setCurrentText(data.get("tch11", ""))
                self.comboBox_56.setCurrentText(data.get("tch21", ""))
                self.comboBox_58.setCurrentText(data.get("tch31", ""))
                self.comboBox_60.setCurrentText(data.get("tch41", ""))
                value = data.get("yaxis", 0.0)
                for i in self.plots:
                    i.setYRange(0, value)

        except Exception as e:
            print(e)
            traceback.print_exc()

    def change_yaxis(self):
        try:
            value = self.doubleSpinBox2.value()
            print(value)
            for i in self.plots:
                i.setYRange(0,value)
            self.write_setting()
        except Exception as e:
            print(e)
            traceback.print_exc()

    def replay(self):
        if self.toolButton_5.text() == "Replay":
            file_path, _ = QFileDialog.getOpenFileName(
                self,
                "Select CSV file",
                "",
                "CSV File (*.csv)" )
            self.replay_path = file_path
            if self.replay_path:
                self.show_all_plots()
                self._load_replay_data(self.replay_path)
                self._setup_replay_plots()
                self.toolButton_5.setText("Stop")
        else:
            self.plot_timer.stop()
            self.toolButton_5.setText("Replay")

    def _load_replay_data(self, filename):
        """Load CSV playback data and convert time to numerical value"""
        with open(filename, 'r') as f:
            reader = csv.reader(f)
            headers = next(reader)

            # Analysis channel unit
            self.channel_units = []
            for header in headers[1:]:
                unit = header.split('(')[-1].rstrip(')').strip()
                self.channel_units.append(unit)

            # Load and convert time
            self.replay_data = {
                'timestamps': [],  # Store relative time (seconds)
                'channels': [[] for _ in range(8)]
            }
            first_timestamp = None  # Base time

            for row in reader:
                # # Parse a string into a datetime object
                try:
                    dt = datetime.strptime(row[0], "%Y-%m-%d_%H-%M-%S.%f")  # Format with microseconds
                except ValueError:
                    dt = datetime.strptime(row[0], "%Y-%m-%d_%H-%M-%S")  # There is no format for microseconds

                # Calculate relative time (seconds)
                if first_timestamp is None:
                    first_timestamp = dt
                rel_time = (dt - first_timestamp).total_seconds()

                self.replay_data['timestamps'].append(rel_time)
                for i in range(8):
                    self.replay_data['channels'][i].append(float(row[i + 1]))

        # Convert to numpy array
        if len(self.replay_data['timestamps']) > self.points_visible:
            self.replay_data['timestamps'] = self.replay_data['timestamps'][-self.points_visible:]
            for i in range(8):
                self.replay_data['channels'][i] = self.replay_data['channels'][i][-self.points_visible:]
            floored_first = math.floor(self.replay_data["timestamps"][0])
            celled_last = math.ceil(self.replay_data["timestamps"][-1])
            for i in self.plots:
                i.setXRange(floored_first, celled_last)
        else:
            self.replay_data['timestamps'] = np.array(self.replay_data['timestamps'])
            for i in range(8):
                self.replay_data['channels'][i] = np.array(self.replay_data['channels'][i])

    def _setup_replay_plots(self):
        try:
            """Configuring drawing in replay mode"""
            # Clear existing curves
            for i in self.curves_list:
                for curves in i:
                    curves.setData([], [])
            self.y_data = [[] for _ in range(8)]
            total_points = len(self.replay_data['timestamps'])
            self.replay_start_idx = max(0, total_points - self.points_visible)
            # Initialize the current plot index
            self.current_plot_idx = 0  # Start from the first data point

            # Extracting time values and channel data
            x = self.replay_data['timestamps'][self.replay_start_idx:]
            self.x_data = x  # Store the full x data for later use

            # Traverse all channels and decide which graph to draw in based on the channel number
            for ch in range(8):
                unit = self.channel_units[ch]
                y = self.replay_data['channels'][ch][self.replay_start_idx:]
                self.y_data[ch] = y  # Store the full y data for later use

                # Voltage (CH1-CH4)
                if ch < 4:
                    if unit == "V":
                        # Unit is Volt, shown in the first figure
                        self.curves_list[0][ch].setVisible(True)
                        self.curves_list[2][ch + 1].setVisible(False)
                        self.curves_list[0][ch].setData([x[0]], [y[0]])  # Initially plot the first data point
                    elif unit == "deg C":
                        # Unit is Temperature, shown in the third figure
                        self.curves_list[0][ch].setVisible(False)
                        self.curves_list[2][ch + 1].setVisible(True)
                        self.curves_list[2][ch + 1].setData([x[0]], [y[0]])  # Initially plot the first data point
                    else:
                        # In other cases, such as errors, hide all
                        self.curves_list[0][ch].setVisible(False)
                        self.curves_list[2][ch + 1].setVisible(False)
                # Acceleration (CH5-CH7)
                elif ch < 7:
                    self.curves_list[1][ch - 4].setData([x[0]], [y[0]])  # Initially plot the first data point
                # Temperature (CH8)
                else:
                    self.curves_list[2][0].setData([x[0]], [y[0]])  # Initially plot the first data point

            # Start the timer to update the plot
            self.replay_status = True
            self.plot_timer.start(500)  # Update every 500ms
        except Exception as e:
            print(e)

    def update_replay_plot(self):
        try:
            if self.current_plot_idx < len(self.x_data) - 1:
                self.current_plot_idx += 1  # Move to the next data point

                # Update all channels
                for ch in range(8):
                    unit = self.channel_units[ch]
                    x = self.x_data[:self.current_plot_idx + 1]
                    y = self.y_data[ch][:self.current_plot_idx + 1]

                    # Voltage (CH1-CH4)
                    if ch < 4:
                        if unit == "V":
                            self.curves_list[0][ch].setData(x, y)
                        elif unit == "deg C":
                            self.curves_list[2][ch + 1].setData(x, y)
                    # Acceleration (CH5-CH7)
                    elif ch < 7:
                        self.curves_list[1][ch - 4].setData(x, y)
                    # Temperature (CH8)
                    else:
                        self.curves_list[2][0].setData(x, y)
                for i in self.plots:
                    i.enableAutoRange(axis=pg.ViewBox.YAxis)
            else:
                # Stop the timer when all data points are plotted
                self.plot_timer.stop()
                self.replay_status = False
        except Exception as e:
            print(e)
            traceback.print_exc()

    def port_connect(self):
        try:
            if self.comboBox.currentText():
                if self.toolButton_2.text() == "Connect":
                    com = self.comboBox.currentText()
                    self.ser.port = com  #Serial port selection box
                    self.ser.baudrate = 115200  # Baud rate input box
                    self.ser.bytesize = int(8)  # Data bit input box
                    self.ser.stopbits = int(1)  # Stop position input box
                    self.ser.parity = serial.PARITY_NONE  # Check digit input box
                    self.ser.timeout = 2
                    self.ser.open()
                    if self.ser.is_open:  # Open the serial port and press, disable the open button and enable the close button
                        self.time_buffer = deque(maxlen=self.points_visible)
                        self.data_buffers = {i: deque(maxlen=self.points_visible) for i in range(8)}
                        self.data_tem_buffers = {i: deque(maxlen=self.points_visible) for i in range(8)}
                        self.start_time = time.time()
                        self.toolButton_2.setText("Disconnect")
                        self.connect_state = True
                        self.optical_link_state = False
                        self.first_connect_sync = True
                        self.label_2.setText("Connect")
                        self.label_2.setStyleSheet("color:green;")
                        self.recv = USBCommThread(self.ser)
                        self.recv.data_updated.connect(self.update_plots)
                        self.recv.timeout_occurred.connect(self.com_port_timeout)
                        self.recv.start()
                        self.show_all_plots()
                else:
                    self.write_data(False)
                    self.ser.close()
                    # self.clear_plots()
                    self.connect_state = False
                    self.first_connect_sync = False
                    self.optical_link_state = False
                    self.recv.close_port()
                    self.toolButton_2.setText("Connect")
                    self.label_2.setText("Disconnect")
                    self.label_2.setStyleSheet("color:black;")
                    self.label_4.setText("Optical Link State：DisConnect")
                    self.label_4.setStyleSheet("color:black;")
            else:
                QMessageBox.warning(self,"Error","No device selected")
        except Exception as e:
            print(e)
            traceback.print_exc()

    def com_port_timeout(self):
        try:
            self.write_data(False)
            if self.ser.is_open:
                self.ser.close()
            # self.clear_plots()
            self.connect_state = False
            self.first_connect_sync = False
            self.optical_link_state = False
            self.toolButton_2.setText("Connect")
            self.label_2.setText("Disconnect")
            self.label_2.setStyleSheet("color:black;")
            self.label_4.setText("Optical Link State：DisConnect")
            self.label_4.setStyleSheet("color:black;")
        except Exception as e:
            print(e)


    def sync(self, data):
        try:
            if self.sync_data is not None:
                for i in range(8):
                    if data["alarmLowThreshold"][i] != self.sync_data["alarmLowThreshold"][i]:
                        self.all_dic[f"ch{i + 1}l"] = data["alarmLowThreshold"][i]
                        self.limit_dic[f"ch{i + 1}l"].setValue(data["alarmLowThreshold"][i])
                    if data["alarmHighThreshold"][i] != self.sync_data["alarmHighThreshold"][i]:
                        self.all_dic[f"ch{i + 1}u"] = data["alarmHighThreshold"][i]
                        self.limit_dic[f"ch{i + 1}u"].setValue(data["alarmHighThreshold"][i])
                    if data["alarmMode"][i] != self.sync_data["alarmMode"][i]:
                        temp = (data["alarmMode"][i])
                        if temp == 2:
                            self.all_dic[f"ch{i + 1}state"] = "Latching"
                        elif temp == 1:
                            self.all_dic[f"ch{i + 1}state"] = "Disabled"
                        else:
                            self.all_dic[f"ch{i + 1}state"] = "Live"
                        self.all_dic[f"ch{i + 1}label"].setCurrentIndex(temp)
                    if data["alarmOn"][i] != self.sync_data["alarmOn"][i]:
                        self.all_dic[f"ch{i + 1}status"] = data["alarmOn"][i]
                        if self.all_dic[f"ch{i + 1}status"]:
                            self.alarm_status_button[i].setText("On")
                            self.alarm_status_button[i].setStyleSheet("background-color:red;")
                        else:
                            self.alarm_status_button[i].setText("Off")
                            self.alarm_status_button[i].setStyleSheet("background-color:green;")
                    if data["units"][i] != self.sync_data["units"][i]:
                        if i < 4:
                            self.all_dic[f"ch{i + 1}uv"] = data["units"][i]
                            self.all_dic[f"ch{i + 1}units"].setCurrentIndex(self.all_dic[f"ch{i + 1}uv"])
                    if data["currentSource"][i] != self.sync_data["currentSource"][i]:
                        if i < len(self.temp_com_list):
                            self.temp_com_list[i][0].setCurrentIndex(data["currentSource"][i])
                            self.all_dic[f"ch{i+1}currentSource"] = self.all_dic[f"ch{i+1}currentSource_widget"].currentIndex()
                    if data["sensorType"][i] != self.sync_data["sensorType"][i]:
                        if i < len(self.temp_com_list):
                            self.temp_com_list[i][1].setCurrentIndex(data["sensorType"][i])
                            self.all_dic[f"ch{i+1}sensorType"] = data["sensorType"][i]
                    if data["inputRange"][i] != self.sync_data["inputRange"][i]:
                        if i < len(self.range_com_list):
                            self.range_com_list[i].setCurrentIndex(data["inputRange"][i])
                            self.all_dic[f"ch{i + 1}range"] = data["inputRange"][i]
                            if data["inputRange"][i] ==0:
                                self.limit_dic[f"ch{i + 1}l"].setSingleStep(0.05)
                                self.limit_dic[f"ch{i + 1}u"].setSingleStep(0.05)
                            else:
                                self.limit_dic[f"ch{i + 1}l"].setSingleStep(0.5)
                                self.limit_dic[f"ch{i + 1}u"].setSingleStep(0.5)

                self.sync_data = data
            else:
                for i in range(8):
                    self.all_dic[f"ch{i + 1}l"] = data["alarmLowThreshold"][i]
                    self.limit_dic[f"ch{i + 1}l"].setValue(data["alarmLowThreshold"][i])
                    self.all_dic[f"ch{i + 1}u"] = data["alarmHighThreshold"][i]
                    self.limit_dic[f"ch{i + 1}u"].setValue(data["alarmHighThreshold"][i])
                    temp = data["alarmMode"][i]
                    if temp == 2:
                        self.all_dic[f"ch{i + 1}state"] = "Latching"
                    elif temp == 1:
                        self.all_dic[f"ch{i + 1}state"] = "Disabled"
                    else:
                        self.all_dic[f"ch{i + 1}state"] = "Live"
                    self.all_dic[f"ch{i + 1}label"].setCurrentIndex(temp)
                    self.all_dic[f"ch{i + 1}status"] = data["alarmOn"][i]
                    if self.all_dic[f"ch{i + 1}status"]:
                        self.alarm_status_button[i].setText("On")
                        self.alarm_status_button[i].setStyleSheet("background-color:red;")
                    else:
                        self.alarm_status_button[i].setText("Off")
                        self.alarm_status_button[i].setStyleSheet("background-color:green;")
                    if i < 4:
                        self.all_dic[f"ch{i + 1}uv"] = data["units"][i]
                        self.all_dic[f"ch{i+1}units"].setCurrentIndex(self.all_dic[f"ch{i + 1}uv"])
                    if i < len(self.temp_com_list):
                        self.temp_com_list[i][0].setCurrentIndex(data["currentSource"][i])
                    if i < len(self.temp_com_list):
                        self.temp_com_list[i][1].setCurrentIndex(data["sensorType"][i])
                    if i < len(self.range_com_list):
                        self.range_com_list[i].setCurrentIndex(data["inputRange"][i])
                        self.all_dic[f"ch{i + 1}range"] = data["inputRange"][i]
                self.sync_data = data
        except Exception as e:
            print(e)
            traceback.print_exc()

    def get_alarm_status(self):
        try:
            alarm_on = []
            alarm_mode = []
            for i in range(8):
                # chXstatus is True/False, convert to 1/0
                alarm_on.append(True if self.all_dic[f"ch{i + 1}status"] else False)
                # Determine the alarm mode
                if self.all_dic[f"ch{i + 1}state"] == "Disabled":
                    temp = 1
                elif self.all_dic[f"ch{i + 1}state"] == "Latching":
                    temp = 2
                else:
                    temp = 0
                alarm_mode.append(temp)

            return [alarm_on, alarm_mode]  # The first one is the on/off switch, the second one is the alarm mode
        except Exception as e:
            print(e)
            traceback.print_exc()

    def get_temp(self):
        try:
            alist = []
            alist1 = []
            for i in range(4):
                alist.append(self.all_dic[f"ch{i+1}currentSource"])
                alist1.append(self.all_dic[f"ch{i+1}sensorType"])
            for i in range(4):
                alist.append(0)
                alist1.append(0)
            print([alist, alist1])
            return [alist,alist1]
        except Exception as e:
            print(e)
            traceback.print_exc()

    def get_units(self):
        try:
            alist = [0,0,0,0,2,2,2,1]
            for i in range(4):
                alist[i] = self.all_dic[f"ch{i+1}uv"]
            return alist
        except Exception as e:
            print(e)
            traceback.print_exc()

    def get_range_value(self):
        try:
            alist = []
            for i in range(4):
                alist.append(self.all_dic[f"ch{i+1}range"])
            for i in range(4):
                alist.append(0)
            return alist
        except Exception as e:
            print(e)
            traceback.print_exc()

    def write_data(self,serial_state=True):
        try:
            if self.ser.is_open and self.sync_data:
                low_limit = [self.all_dic[f"ch{i+1}l"] for i in range(8)]
                upp_limit = [self.all_dic[f"ch{i+1}u"] for i in range(8)]
                alarm_status = self.get_alarm_status()
                tem = self.get_temp()
                units = self.get_units()
                input_range = self.get_range_value()
                if self.sync_rtc_time_status:
                    rtc_time = int(time.time()) + 36000
                    self.sync_rtc_time_status = False
                else:
                    rtc_time = 0
                data_dict = {
                    "alarmLowThreshold": low_limit,
                    "alarmHighThreshold": upp_limit,
                    "alarmMode": alarm_status[1],  # ALARM_* values
                    "alarmOn": alarm_status[0],
                    "units": units,  # UNITS_* values
                    "currentSource": tem[0],  # 0 = 10uA, 1 = 200uA
                    "sensorType": tem[1],  # 0 = thermistor, 1 = RTD
                    "inputRange": input_range,  # RANGE_* values
                    "rtcTime": rtc_time,
                    "recordingState": self.recording_state,
                    "serialState": serial_state
                }
                if self.sync_data != data_dict:
                    self.sync_data = data_dict
                data = encode_serial_out_data(data_dict)
                self.ser.write(data)
        except Exception as e:
            print(e)
            traceback.print_exc()

    def toggle_recording(self):
        try:
            if self.connect_state:
                self.recording = not self.recording
                self.recording_state = self.recording
                self.label_5.setText(f"Recording State： {'in progress' if self.recording else 'Stop'}")
                if self.recording:
                    self.toolButton_3.setStyleSheet("color:red")
                    self.label_5.setStyleSheet("color:green")
                    self.first_timestamp = None
                    self.disable_all()
                else:
                    self.toolButton_3.setStyleSheet("color:green")
                    self.label_5.setStyleSheet("color:red")
                    self.csv_file = None
                    self.csv_writer = None
                    self.enable_all()
                self.write_setting()
                self.write_data()
            else:
                QMessageBox.warning(self,"Warning","Device DisConnect")
        except Exception as e:
            print(e)
            traceback.print_exc()

    def time_set(self):
        try:
            self.label.setText(f"System Time:{str(datetime.now())[:-7]}")
        except Exception as e:
            print(e)
            traceback.print_exc()

    def update_plots(self, data):
        try:
            self.replay_path = None
            timestamp = time.time()

            # Initialize start_time
            if not hasattr(self, 'start_time') or self.start_time is None:
                self.start_time = timestamp

            # Calculating relative time
            t = timestamp - self.start_time
            self.time_buffer.append(t)
            self.sync(data)
            if self.first_connect_sync:
                self.write_data()
                self.first_connect_sync = False
            for i in range(8):
                if i < 4:
                    if data['units'][i] == 0:
                        self.data_buffers[i].append((t, data['channelReading'][i],data['units'][i]))
                        self.data_tem_buffers[i].append((t, np.nan,data['units'][i]))
                    elif data['units'][i] == 1:
                        self.data_tem_buffers[i].append((t, data['channelReading'][i],data['units'][i]))
                        self.data_buffers[i].append((t, np.nan,data['units'][i]))
                else:
                    self.data_buffers[i].append((t, data['channelReading'][i],data['units'][i]))
                    # self.data_tem_buffers[i].append((t, None,data['units'][i]))
            if not self.replay_status:
                celled_last = math.ceil(t)
                if celled_last > 10:
                    for plot in self.plots:
                        plot.setXRange(0, celled_last)
                for ch in range(8):
                    x = [d[0] for d in self.data_buffers[ch]]
                    y = [d[1] for d in self.data_buffers[ch]]
                    # draw on the correct subimage
                    if ch < 4:
                        unit = data['units'][ch]
                        if unit == 0:  # Volt
                            self.curves_list[0][ch].setData(x, y)
                            x1 = [d[0] for d in self.data_tem_buffers[ch]]
                            y1 = [d[1] for d in self.data_tem_buffers[ch]]
                            self.curves_list[2][ch + 1].setData(x1, y1)
                        elif unit == 1:  # Temperature
                            self.curves_list[0][ch].setData(x, y)
                            x1 = [d[0] for d in self.data_tem_buffers[ch]]
                            y1 = [d[1] for d in self.data_tem_buffers[ch]]
                            self.curves_list[2][ch + 1].setData(x1, y1)
                        else:
                            pass  # unit=2 (m/s²) will not appear in CH1-4
                    elif ch < 7:
                        # ch5-ch7 (Acceleration channels)
                        self.curves_list[1][ch - 4].setData(x, y)
                    else:
                        # ch8 (Temperature channel)
                        self.curves_list[2][0].setData(x, y)
                for i in self.plots:
                    i.enableAutoRange(axis=pg.ViewBox.YAxis)
            # Update recording status
            recording_state = data.get("recordingState", False)
            if recording_state != self.recording_state:
                self.recording_state = recording_state
                if recording_state:
                    self.label_5.setText(f"Recording State：in progress")
                    self.label_5.setStyleSheet("color:green;")
                    self.disable_all()
                else:
                    self.label_5.setText(f"Recording State：Stop")
                    self.label_5.setStyleSheet("color:red;")
                    self.enable_all()

            # Convert a timestamp to a datetime object
            try:
                rtc_time = data['rtcTime'] - 36000
                if rtc_time < 0:
                    print(rtc_time)
                else:
                    dt_object = datetime.fromtimestamp(rtc_time)
                    formatted_time = dt_object.strftime("%Y-%m-%d %H:%M:%S")
                    self.label_7.setText(f"RTC Time: {formatted_time}")
            except (ValueError, OSError) as e:
                print(f"Error converting RTC time: {e}")

            if data["opticalLinkState"] and not self.optical_link_state:
                self.optical_link_state = True
                self.label_4.setText("Optical Link State：Connect")
                self.label_4.setStyleSheet("color:green;")
            else:
                if self.optical_link_state:
                    self.optical_link_state = False
                    self.label_4.setText("Optical Link State：DisConnect")
                    self.label_4.setStyleSheet("color:black;")

        except Exception as e:
            print("Plot update error:", e)
            traceback.print_exc()

    def closeEvent(self, event, **kwargs):
        try:
            if self.ser.is_open:
                self.write_data(False)
        except Exception as e:
            print(e)


    def _write_to_csv(self, data):
        try:
            """Writing data to a CSV file"""
            # Initialize the file when recording for the first time
            if self.csv_file is None:
                self._init_csv_file(data)

            # Format timestamp (accurate to milliseconds)
            timestamp_str = datetime.fromtimestamp(data['rtcTime']).strftime("%Y-%m-%d_%H-%M-%S.%f")[:-3]

            # Constructing Data Rows
            row = [timestamp_str] + [round(val, 6) for val in data['channelReading']]
            self.csv_writer.writerow(row)
            self.csv_file.flush()  # Ensure real-time write
        except IOError as e:
            self.toggle_recording()
            print(f"Failed to write file: {str(e)}")
        except Exception as e:
            self.toggle_recording()
            raise RuntimeError(f"An unknown error occurred during recording: {str(e)}")

    def _init_csv_file(self, data):
        """Initialize CSV file and header information"""
        # Generate file name (without milliseconds)
        self.first_timestamp = data['rtcTime']
        dt_obj = datetime.fromtimestamp(self.first_timestamp)
        filename = dt_obj.strftime("%Y-%m-%d_%H-%M-%S") + ".csv"

        # Create a file and write a header
        self.csv_file = open(filename, 'w', newline='')
        self.csv_writer = csv.writer(self.csv_file)
        hand_list = []
        for i in range(8):
            if data["units"][i] == 0:
                hand_list.append(f"CH{i + 1}(V)")
            elif data["units"][i] == 1:
                hand_list.append(f"CH{i + 1}(deg C)")
            elif data["units"][i] == 2:
                hand_list.append(f"CH{i + 1}(m/s^2)")
        headers = ["Timestamp"] + hand_list
        self.csv_writer.writerow(headers)


if __name__ == '__main__':
    # Initialize the QApplication with command-line arguments
    app = QApplication(sys.argv)
    # Create an instance of the MainWindow class
    window = MainWindow()
    # Show the main window
    window.show()
    # Enter the main event loop and exit when done
    sys.exit(app.exec_())