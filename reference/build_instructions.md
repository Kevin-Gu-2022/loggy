# Build Instructions
## PC Software
To run the PC software, run the following command in a terminal:
```
gui.bat
```
This will install all the relevant dependencies and launch the program. Ensure a version of Python is present on the host.

## Embedded System
The code for the Control Unit and Sensor Units are built using the STM32CubeIDE's build tool. 
1. Start a new project in STM32CubeIDE with the workspace set in the `embedded` directory.
2. Import the two projects independently by selecting 'New STM32 Project from Existing STM32CubeMX Configuration File (.ioc)'.
3. Once imported, the program can simply be built by clicking Build button.
4. Using the JLink Programmer, flash the binary on the chip. Make sure to set the Debugger Probe in Run Configurations to SEGGER J-LINK. Flash `loggy_v2.elf` onto the Control Unit (the chip on the board with the LCD) and `Sensor_Unit.elf` onto the Sensor Unit.
