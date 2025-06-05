"""
DO NOT RE-WRITE THIS!!!!!!!!!!!!!!!

IN YOUR GUI FILE JUST DO THIS:
from serial_in import parse_input_serial_data, INPUT_PACKET_SIZE

Then, in you can just call this function as if it has been copied to your file.

If you want to test stuff, you can run this file directly by clicking play. 
This will execute everything in main()
"""
import struct
import serial
import time
import random

NUM_CHANNELS = 8

RANGE_ONE_VOLT = 0
RANGE_TEN_VOLT = 1

ALARM_ACTIVE = 0
ALARM_DISABLED = 1
ALARM_LATCHED = 2

UNITS_VOLT = 0
UNITS_DEG = 1
UNITS_MS2 = 2

import struct
import serial

NUM_CHANNELS = 8

# Bitfield constants
RANGE_ONE_VOLT = 0
RANGE_TEN_VOLT = 1

ALARM_ACTIVE = 0
ALARM_DISABLED = 1
ALARM_LATCHED = 2

UNITS_VOLT = 0
UNITS_DEG = 1
UNITS_MS2 = 2

# Format:
# - float: channelReading, alarmLow, alarmHigh = 3*8 = 24 floats
# - uint8: alarmStatus, unit, inputRange = 3*8 = 24 bytes
# - uint32: rtcTime = 4 bytes
# - uint8: recordingState + opticalLinkState = 2 bytes
FORMAT = '<' + 'f'*NUM_CHANNELS*3 + 'B'*NUM_CHANNELS*3 + 'IBB'
INPUT_PACKET_SIZE = struct.calcsize(FORMAT)

def parse_input_serial_data(data):
    if len(data) != INPUT_PACKET_SIZE:
        raise ValueError(f"Expected {INPUT_PACKET_SIZE} bytes, got {len(data)}")

    unpacked = struct.unpack(FORMAT, data)
    idx = 0

    channelReading = [x for x in unpacked[idx:idx + NUM_CHANNELS]]
    idx += NUM_CHANNELS

    alarmLowThreshold = [x for x in unpacked[idx:idx + NUM_CHANNELS]]
    idx += NUM_CHANNELS

    alarmHighThreshold = [x for x in unpacked[idx:idx + NUM_CHANNELS]]
    idx += NUM_CHANNELS

    alarmStatusRaw = list(unpacked[idx:idx + NUM_CHANNELS])
    alarmStatusBase = [val & 0b00000011 for val in alarmStatusRaw]
    alarmOn = [(val & 0b00000100) != 0 for val in alarmStatusRaw]
    idx += NUM_CHANNELS

    unitRaw = list(unpacked[idx:idx + NUM_CHANNELS])
    unitBase = [val & 0b00000011 for val in unitRaw]
    currentSource = [(val >> 2) & 0x01 for val in unitRaw]
    sensorType = [(val >> 3) & 0x01 for val in unitRaw]
    idx += NUM_CHANNELS

    inputRange = list(unpacked[idx:idx + NUM_CHANNELS])
    idx += NUM_CHANNELS

    rtcTime = unpacked[idx]
    idx += 1

    recordingState = unpacked[idx]
    idx += 1

    opticalLinkState = unpacked[idx]

    result = {
        "channelReading": channelReading,
        "alarmLowThreshold": alarmLowThreshold,
        "alarmHighThreshold": alarmHighThreshold,
        "alarmMode": alarmStatusBase,
        "alarmOn": alarmOn,
        "units": unitBase,
        "currentSource": currentSource,
        "sensorType": sensorType,
        "inputRange": inputRange,
        "rtcTime": rtcTime,
        "recordingState": recordingState,
        "opticalLinkState": opticalLinkState
    }

    print(f"Serial In: {result}")

    return result


def receive_data():
    ser = serial.Serial('COM8', 115200)
    while True:
        raw_data = ser.read(INPUT_PACKET_SIZE)
        parsed = parse_input_serial_data(raw_data)
        print(parsed)


def receive_random_data():
    return {
        "channelReading": [random.randint(0, 10) for x in range(0, 8)],
        "alarmLowThreshold": [0] * 8,
        "alarmHighThreshold": [1] * 8,
        "alarmMode": [ALARM_DISABLED] * 8,
        "alarmOn": [1] * 8,
        "units": [UNITS_VOLT] * 4 + [UNITS_MS2] * 3 + [UNITS_DEG],
        "currentSource": [0] * 8,
        "sensorType": [0] * 8,
        "inputRange": [0] * 8,
        "rtcTime": int(time.time()),
        "recordingState": False,
        "opticalLinkState": True
    }


def main():
    # while True:
    #     print(receive_random_data())
    #     time.sleep(0.5)
    receive_data()

if __name__ == "__main__":
    main()