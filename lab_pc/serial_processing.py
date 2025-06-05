import serial
import struct
import time
import random

# Define constants
NUM_CHANNELS = 8
SERIAL_PORT = 'COM8'
BAUD_RATE = 115200

RANGE_ONE_VOLT = 0
RANGE_TEN_VOLT = 1

ALARM_ACTIVE = 0
ALARM_DISABLED = 1
ALARM_LATCHED = 2

UNITS_VOLT = 0
UNITS_DEG = 1
UNITS_MS2 = 2

def receive_reading() -> dict:
    """
    Receives reading from COM port directly using the pyserial library.
    """
    # Define struct format: 8 floats + 8 floats + 8 floats + 8 uint8 + 8 uint8 + 8 uint8 + 1 uint32 + 2 uint8
    struct_format = "<" + "f" * NUM_CHANNELS * 3 + "B" * NUM_CHANNELS * 3 + "I" + "B" * 2
    struct_size = struct.calcsize(struct_format)

    serial_data = {}
    
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=10000) as ser:
        print("Waiting for data...")
        data = ser.read(struct_size)
        
        if len(data) == struct_size:
            unpacked_data = struct.unpack(struct_format, data)
            
            serial_data = {
                "channel_reading": unpacked_data[0:NUM_CHANNELS],
                "alarm_low_threshold": unpacked_data[NUM_CHANNELS:NUM_CHANNELS*2],
                "alarm_high_threshold": unpacked_data[NUM_CHANNELS*2:NUM_CHANNELS*3],
                "alarm_status": unpacked_data[NUM_CHANNELS*3:NUM_CHANNELS*4],
                "unit": unpacked_data[NUM_CHANNELS*4:NUM_CHANNELS*5],
                "input_range": unpacked_data[NUM_CHANNELS*5:NUM_CHANNELS*6],
                "rtc_time": unpacked_data[NUM_CHANNELS*6],
                "recording_state": bool(unpacked_data[NUM_CHANNELS*6+1]),
                "optical_link_state": bool(unpacked_data[NUM_CHANNELS*6+2])
            }
            
            print("======= Received Data =======")
            for key, value in serial_data.items():
                print(f"{key}: {value}")

            print("========= Complete ==========\n")
        else:
            print("Error: Incomplete data received")
    
    return serial_data


def serialise_data(serial_data: dict) -> bytes:
    """
    Use this function to convert a dictionary into serial data.

    serial_data: {
        "channel_reading": float[NUM_CHANNELS]
        "alarm_low_threshold": float[NUM_CHANNELS]
        "alarm_high_threshold": float[NUM_CHANNELS]
        "alarm_status": uint8_t[NUM_CHANNELS]
        "unit": uint8_t[NUM_CHANNELS]
        "input_range": uint8_t[NUM_CHANNELS]
        "rtc_time": uint32_t
        "recording_state": bool
        "optical_link_state": bool
    }

    """
    # Define the struct format string
    struct_format = "<" + "f" * NUM_CHANNELS * 3 + "B" * NUM_CHANNELS * 3 + "I" + "B" * 2
    # Prepare the data for struct packing
    packed_data = (
        tuple(serial_data["channel_reading"]) +
        tuple(serial_data["alarm_low_threshold"]) +
        tuple(serial_data["alarm_high_threshold"]) +
        tuple(serial_data["alarm_status"]) +
        tuple(serial_data["unit"]) +
        tuple(serial_data["input_range"]) +
        (serial_data["rtc_time"], serial_data["recording_state"], serial_data["optical_link_state"])
    )
    
    # Pack the data into a binary format
    serialised = struct.pack(struct_format, *packed_data)
    
    return serialised

# Convert binary data to bitstring
def bytes_to_bitstring(data):
    return ' '.join(f"{byte:08b}" for byte in data)

if __name__ == "__main__":
    while True:
        receive_reading()


        # random_data = [1.23, 4.27, 8.35, 2.86, 1.65, 4.98, 2.45, 6.23]
        # random_int = [1, 0, 1, 0, 0, 1, 1, 0]

        # serial_data = {
        #     "channel_reading": [round(random.uniform(0, 10), 3) for _ in range(NUM_CHANNELS)],
        #     "alarm_low_threshold": [round(random.uniform(0, 10), 3) for _ in range(NUM_CHANNELS)],
        #     "alarm_high_threshold": [round(random.uniform(0, 10), 3) for _ in range(NUM_CHANNELS)],
        #     "alarm_status": [random.randint(0, 1) for _ in range(NUM_CHANNELS)],
        #     "unit": [random.randint(0, 2) for _ in range(NUM_CHANNELS)],
        #     "input_range": [random.randint(0, 1) for _ in range(NUM_CHANNELS)],
        #     "rtc_time": int(time.time()),
        #     "recording_state": False,
        #     "optical_link_state": True
        #     }

        # data = serialise_data(serial_data)
        # print(f"Data Sent: {serial_data}")
        # # print(bytes_to_bitstring(data))

        # with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=10000) as ser:
        #     ser.write(data)

        # time.sleep(1)