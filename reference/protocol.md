# PC to Control Unit Serial Protocol
The serial protocol involves a simple struct that contains all state information exchanged between the PC software, and the control and sensor units.

## #Defines
```
/* Define's for the different units */
#define VOLT 0
#define DEG 1
#define MS2 2

/* Channel Names */
#define CH1 0
#define CH2 1
#define CH3 2
#define CH4 3
#define CH5 4
#define CH6 5
#define CH7 6
#define CH8 7

#define ONE_VOLT 0
#define TEN_VOLT 1

#define ACTIVE 0
#define DISABLED 1
#define LATCHED 2

#define SENSOR_THERM 0
#define SENSOR_RTD 1

#define CURRENT_10UA 0
#define CURRENT_200UA 1

#define ALARM_ON 1
#define ALARM_OFF 0
```

## STM32 Chip to PC
```
typedef struct SerialData {
	float channelReading[NUM_CHANNELS];
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmStatus[NUM_CHANNELS];  // 3rd bit is if alarm is on or off
	uint8_t unit[NUM_CHANNELS];  // 3rd bit = current source, 4th bit = thermistor/RTD
	uint8_t inputRange[NUM_CHANNELS];
	uint32_t rtcTime;  // Total seconds since start of Unix Epoch
	uint8_t recordingState;
	uint8_t opticalLinkState;
} SerialData_t;
```
###  Alarm Status Example
```
0000 0010: Latched Mode, alarm off
0000 0110: Latched Mode, alarm on
0000 0100: Active Mode, alarm on
0000 0000: Active Mode, alarm off
0000 0001: Disabled mode, alarm off
```

### Unit
```
0000 0000: Volts; 10uA; thermistor
0000 1000: Volts; 10uA; RTD
0000 0100: Volts; 200uA; thermistor
0000 1100: Volts; 200uA; RTD

0000 0001: degrees; 10uA; thermistor
0000 1001: degrees; 10uA; RTD
0000 0101: degrees; 200uA; thermistor
0000 1101: degrees; 200uA; RTD

0000 0010: m/s; 10uA; thermistor
0000 1010: m/s; 10uA; RTD
0000 0110: m/s; 200uA; thermistor
0000 1110: m/s; 200uA; RTD
```

All the members above are structured so that they align with the 32 bit system used in the STM32 chips. Floats are 4 bytes.


## PC to STM32 Chip
```
typedef struct SerialData {
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmStatus[NUM_CHANNELS];  // 3rd bit is if alarm is on or off
	uint8_t unit[NUM_CHANNELS];  // 3rd bit = current source, 4th bit = thermistor/RTD
	uint8_t inputRange[NUM_CHANNELS];
	uint32_t rtcTime;  // Total seconds since start of Unix Epoch
	uint8_t recordingState;  // Second bit is serial comms state: 0 = Disconnect; 1 = Connected
} SerialData_t;
```
###  Alarm Status Example
```
0000 0010: Latched Mode, alarm off
0000 0110: Latched Mode, alarm on
0000 0100: Active Mode, alarm on
0000 0000: Active Mode, alarm off
0000 0001: Disabled mode, alarm off
```

### Unit
```
0000 0000: Volts; 10uA; thermistor
0000 1000: Volts; 10uA; RTD
0000 0100: Volts; 200uA; thermistor
0000 1100: Volts; 200uA; RTD

0000 0001: degrees; 10uA; thermistor
0000 1001: degrees; 10uA; RTD
0000 0101: degrees; 200uA; thermistor
0000 1101: degrees; 200uA; RTD

0000 0010: m/s; 10uA; thermistor
0000 1010: m/s; 10uA; RTD
0000 0110: m/s; 200uA; thermistor
0000 1110: m/s; 200uA; RTD
```

# Optical Link Protocol
- The two MCUs will take turns sending information.
- The sensor unit will act as the master, sending information every 500 ms
- The control unit will receive the sensor unit's UART transmission from an 
interrupt, and place it in a ring buffer, then set a flag
- Upon reaching the `process_optical_input()` function in super-loop, it will check
for the flag. If set, it will send the current state across.
- The control unit will never send information out of the blue
- If nothing received for more than 1s, change `opticalLinkState` to `0`

NOTE: When sending UART on TX, the associated RX **must** be turned off

## Control Unit to Sensor Unit
```
typedef struct {
	float alarmLowThreshold[NUM_CHANNELS];
	float alarmHighThreshold[NUM_CHANNELS];
	uint8_t alarmMode[NUM_CHANNELS];  // Alarm mode 3rd bit will be whether alarm is on/off
	uint8_t inputRange[NUM_CHANNELS];
} OpticalState_t;
```
### Alarm Mode Example
`0000 0110`: Alarm on and Latching mode\
`0000 0010`: Alarm off and Latching mode 

## Sensor Unit to Control Unit
```
typedef struct {
	float channelReading[NUM_CHANNELS];
} OpticalState_t;
```