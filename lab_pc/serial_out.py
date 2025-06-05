"""
DO NOT RE-WRITE THIS!!!!!!!!!!!!!!

IN YOUR GUI FILE JUST DO THIS:
from serial_out import encode_serial_out_data, OUTPUT_PACKET_SIZE

Then, in you can just call this function as if it has been copied to your file.

If you want to test stuff, you can run this file directly by clicking play. 
This will execute everything in main()
"""
import struct
import serial

RANGE_ONE_VOLT = 0
RANGE_TEN_VOLT = 1

ALARM_ACTIVE = 0
ALARM_DISABLED = 1
ALARM_LATCHED = 2

UNITS_VOLT = 0
UNITS_DEG = 1
UNITS_MS2 = 2

NUM_CHANNELS = 8
FORMAT = '<' + 'f'*NUM_CHANNELS*2 + 'B'*NUM_CHANNELS*2 + 'B'*NUM_CHANNELS + 'IB'
OUTPUT_PACKET_SIZE = struct.calcsize(FORMAT)

def encode_serial_out_data(data_dict: dict) -> bytes:
    """
    This function will take a dictionary input and output the raw bytes that
    you can put straight into serial.write().
    """
    print(f"Serial Out: {data_dict}")
    # Validate input
    required_keys = [
        'alarmLowThreshold', 'alarmHighThreshold', 'alarmMode',
        'alarmOn', 'units', 'currentSource', 'sensorType',
        'inputRange', 'rtcTime', 'recordingState', 'serialState'
    ]
    for key in required_keys:
        if key not in data_dict:
            raise KeyError(f"Missing required key: {key}")
        if isinstance(data_dict[key], list) and len(data_dict[key]) != NUM_CHANNELS:
            raise ValueError(f"{key} must have {NUM_CHANNELS} elements")

    # Prepare fields
    alarmLow = [round(x, 3) for x in data_dict['alarmLowThreshold']]
    alarmHigh = [round(x, 3) for x in data_dict['alarmHighThreshold']]

    # Bit 2 in alarmStatus = alarmOn
    alarmStatus = [
        (data_dict['alarmMode'][i] & 0b00000011) |  # lower 2 bits preserved
        (0b00000100 if data_dict['alarmOn'][i] else 0)
        for i in range(NUM_CHANNELS)
    ]

    # Bit 2 = current source (0=10uA, 1=200uA)
    # Bit 3 = sensor type (0=thermistor, 1=RTD)
    unit = [
        (data_dict['units'][i] & 0b00000011) |
        ((data_dict['currentSource'][i] & 1) << 2) |
        ((data_dict['sensorType'][i] & 1) << 3)
        for i in range(NUM_CHANNELS)
    ]

    inputRange = data_dict['inputRange']
    rtcTime = data_dict['rtcTime']
    recordingState = data_dict['recordingState'] | (data_dict['serialState'] << 1)

    packed = struct.pack(
        FORMAT,
        *alarmLow,
        *alarmHigh,
        *alarmStatus,
        *unit,
        *inputRange,
        rtcTime,
        recordingState
    )

    return packed

# Convert binary data to bitstring
def bytes_to_bitstring(data):
    return ' '.join(f"{byte:08b}" for byte in data)

def main():
    data_dict = {
        "alarmLowThreshold": [0.1, 0.4, 0.3, 0.4, 0.5, 0.6, 0.7, 0.1],
        "alarmHighThreshold": [-5.764] * 8,
        "alarmMode": [0, 1, 2, 0, 1, 2, 0, 1],   # ALARM_* values
        "alarmOn": [True, False, True, False, True, False, True, False],
        "units": [0, 0, 0, 0, 2, 2, 2, 1],          # UNITS_* values
        "currentSource": [0, 1, 0, 1, 0, 1, 0, 1],     # 0 = 10uA, 1 = 200uA
        "sensorType": [0, 1, 0, 1, 0, 1, 0, 1],        # 0 = thermistor, 1 = RTD
        "inputRange": [0, 1, 0, 1, 0, 1, 1, 1],        # RANGE_* values
        "rtcTime": 1000000,
        "recordingState": False,
        "serialState": True
    }
    raw_bytes = encode_serial_out_data(data_dict=data_dict)
    print(bytes_to_bitstring(raw_bytes))

    with serial.Serial('COM8', 115200, timeout=10000) as ser:
        ser.write(raw_bytes)


if __name__ == "__main__":
    main()
