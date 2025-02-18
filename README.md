# FluidScreen
Wireless controller for FluidNC

![e19302e4588211ce1077c6e39101f643c2d2b93d](https://github.com/user-attachments/assets/e4b53310-34f3-468c-906b-707aa155567e)
![20250101_100233](https://github.com/user-attachments/assets/ceba9de2-fea9-43f2-af1a-83d94e07b5c3)

## Overview
This is a wireless controller (except for power) for a 3 axis CNC machine running FluidNC.  It connects to FluidNC via a WebSocket connection.  This uses an ESP32 and is written in C++ using PlatformIO.

# Features
- General - Hold, Resume, Unlock, Reset
- Jogging - In XY, X, Y, or Z using the physical joystick.  The further the joystick is from the center, the faster the feedrate.  The value does have a curve to make it easier to perform slow jogging with small movements.
- Home - All, X, Y, or Z
- Macro - Special macro format using GCode parameters and expression functionality of FluidNC.  Supports up to 6 custom defined macros with a description of each displayed on the screen.
- Configuration - Display a captive portal webpage for updating configuration.
  - Wifi/FluidNC - Set Wifi and FluidNC settings.
  - Theming - This is a bit extra but the individual theme and status colors can be updated.
- Hold Messages - Displays the last message on the screen.  When a hold occurs, the last message is displayed with the Resume option activated.

# Parts

## Hardware
> The 3D printed case was made for these specific components. If alternates are used, I am not certain if things will fit properly.
- 1x ESP32 Devkit - https://www.amazon.com/Espressif-ESP32-DevKitC-32E-Development-Board/dp/B09MQJWQN2
- 1x Joystick - https://www.amazon.com/gp/product/B0CPFDKWZF
- 1x Rotary Encoder - https://www.amazon.com/gp/product/B06XQTHDRR
- 1x Display - https://www.amazon.com/gp/product/B073R7BH1B
- 1x 10uF Capacitor - This allows for flashing the firmware without needing to press the Boot button on the ESP32. Otherwise, the ESP32 would need to be removed to access the Boot button.

## Miscellaneous
- ~16x M3x6mm Screws
- Female to Female Dupont Wires
- Dupont Connector Kit - This is optional. It is helpful to have a 9 or 10 plug connector for combining all the wires for the screen into a single plug. https://www.amazon.com/gp/product/B08X6C7PZM
- USB Micro Cable - For power and flashing firmware.  Ensure it is a data and power cable, not just power only.
- Solder Seal Heat Shrink Butt Connectors - This is optional and was used for joining wires to share 3.3v and ground. Used the red 18-22 awg from this kit: https://www.amazon.com/dp/B083LZS69N
  - Heat Gun - https://www.amazon.com/dp/B08VFY8THD

## 3D Print
> These were printed in PLA with 3 walls and 50% infill with a 0.4mm nozzle at 0.2mm layer height.
- 1x Case Top - Print with flat part on build plate. No supports necessary.
- 1x Case Bottom - Print with flat part on build plate. Supports required for screw holes on bottom.
- 1x ESP32 Holder - Print with flat part on build plate.  Supports required for screw holes on bottom.
- 1x Encoder Knob - Print with flat part on build plate.  Supports may be required for indent.
- 4x Washer - This is optional. A few M3 washers can be used instead. This is necessary for attaching the screen to the case top because otherwise the M3x6mm screws are too long.  Alternatively, M3x4mm screws could be used.

# Firmware

## Pre-Requisites
1. Visual Studio Code (https://code.visualstudio.com/) with "PlatformIO IDE" extension installed.
2. Silicon Labs CP210x USB to UART Bridge Driver (https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads)
3. Download or checkout this repository.

## Installation
> Note: If this project has not been assembled yet or the ESP32 is standalone, the "Boot" button on the ESP32 will need to be held for a couple seconds when it starts to upload the filesystem image and when it flashes the firmware.

1. Open the Source folder in Visual Studio Code.
2. Connect the ESP32 to the computer with a USB cable.
3. Click the PlatformIO icon on the left.
4. Under "Project Tasks", select "esp32dev" -> "Platform", select the "Build Filesystem Image" task to create the filesystem image.
5. Under "Platform", select the "Upload Filesystem Image" task to upload.
6. Under "Project Tasks", select "esp32dev" -> "General", select the "Build" task to build the project.
7. Under "General", select the "Upload" task to upload the firmware.

# Assembly

![20250101_095737](https://github.com/user-attachments/assets/16095dd1-a825-4bd2-91f9-86dcda2af6db)
![20250101_095801](https://github.com/user-attachments/assets/fe11f1cb-850f-4a74-8883-0aa20d032d7b)
![20250101_095815](https://github.com/user-attachments/assets/a4b8f138-79a8-41b5-92fd-9487cd27314d)
![20250101_095844](https://github.com/user-attachments/assets/42b02fbd-64da-4fcd-9222-29a433d9fc76)
![20250101_100716](https://github.com/user-attachments/assets/d4dc9358-170d-4d4b-9df1-078462c60cb9)

## Pin Mappings

### Display
> Note: Touch Pins are not used.

| Pin       | ESP32 Pin |
| --------- | --------- |
| VCC       | 3V3       |
| GND       | GND       |
| CS        | 5         |
| RESET     | 17        |
| DC        | 16        |
| SDI(MOSI) | 23        |
| SCK       | 18        |
| LED       | 22        |
| SDI(MISO) | 19        |

### Joystick
> Note: SW pin is not used.

| Pin | ESP32 Pin |
| --- | --------- |
| GND | GND       |
| +5V | 3V3       |
| VRX | 32        |
| VRY | 33        |

### Rotary Encoder
| Pin | ESP32 Pin |
| --- | --------- |
| GND | GND       |
| +   | 3V3       |
| SW  | 27        |
| DT  | 25        |
| CLK | 26        |

### Capacitor
| Pin                  | ESP32 Pin |
| -------------------- | --------- |
| Short Leg with White | GND       |
| Long Leg             | EN        |

## Steps
> With the pin mappings and pictures, it can likely be assembled without these instructions.

### Bottom
1. Start with the case bottom (thinner part with USB hole at the bottom).
2. Attach the joystick and rotary encoder with M3x6mm screws with male pins pointed at the bottom.
3. Place the ESP32 upside down with the antenna at the top. It should rest on the standoffs.
4. Attach the ESP32 holder with M3x6mm screws with the flat part up.  It should bend to apply some pressure to the ESP32 to hold it in place.
5. Create wire for sharing 3.3v connection with 4 ends, 2 on each side.  Verify lengths are appropriate and ensure there is extra length for removal of the top.  This can be done with solder seal heat shrink butt connectors, lever nuts, or just solder and heat shrink.
6. Run the wire under the bottom part of the ESP32 close to the USB end.
7. Connect the 3.3V wires to the appropriate pins on the ESP32, joystick, rotary encoder, and display.
8. Create wire for sharing GND connection with 3 ends.  This is for connecting the joystick and display. (The ESP32 has 3 GND connections. The capacitor and rotary encoder can use their own pins.)
9. Connect the GND wires to the appropriate pins on the ESP32 and joystick.
10. Connect a separate ground wire between the ESP32 and rotary encoder.
11. Connect the remaining wires for the joystick and rotary encoder as described in the pin mapping section.
12. Crimp connectors to the 10uF capacitor and attach to the ESP32 between the EN and GND pins towards the top of the connector. Ensure the short leg with the white line attaches to the GND pin.

### Top
1. Place the screen into the case top with the pins on the joystick side. The screen should be about 1mm below the surface.
2. Attach the screen with M3x6mm screws and printed washers (or 4mm screws or ~3 M3 washers).
3. Connect the wires for the display.  It is recommended to use a 9 or 10 pin dupont connector to create a single connection to the display.  This can be done making your own cables or by removing the single plastic housing and plugging it into the new larger connector.

### Finish
1. Attach the case bottom to the case top with 4 M3x6mm screws. These screws can be longer.
2. Press the printed knob onto the rotary encoder.

# Macros
The macro menu allows for up to 6 custom defined macros.  These are defined in a file that must be named "_screenmacros.gcode" and placed in the root on the SD card.

## Template
``` gcode
o100 if[#<_screenMacro> EQ 1]
  o101 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Macro 1 - Description)
  o101 else
    (MSG, Running Macro 1)
  o101 end if
o100 elseif[#<_screenMacro> EQ 2]
  o102 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Macro 2 - Description)
  o102 else
    (MSG, Running Macro 2)
  o102 end if
o100 elseif[#<_screenMacro> EQ 2]
  o103 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Macro 3 - Description)
  o103 else
    (MSG, Running Macro 3)
  o103 end if
o100 elseif[#<_screenMacro> EQ 4]
  o104 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Macro 4 - Description)
  o104 else
    (MSG, Running Macro 4)
  o104 end if
o100 elseif[#<_screenMacro> EQ 5]
  o105 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Macro 5 - Description)
  o105 else
    (MSG, Running Macro 5)
  o105 end if
o100 elseif[#<_screenMacro> EQ 6]
  o106 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Macro 6 - Description)
  o106 else
    (MSG, Running Macro 6)
  o106 end if
o100 else
  (PRINT, Macro %d#<_screenMacro>: Not Defined)
o100 end if
```

## How It Works
This utilizes the GCode Parameters and Expressions functionality in FluidNC: http://wiki.fluidnc.com/en/features/gcode_parameters_expressions

When highlighting a macro number on the screen, the following code is run to set the variables where "N" is the highlighted macro number:
``` gcode
#<_screenMacroInfoOnly>=1
#<_screenMacro>=N
$SD/Run=_screenmacros.gcode
```

## Example
``` gcode
o100 if[#<_screenMacro> EQ 1]
  o101 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Run Current GCode - Run the GCode file defined in /ScreenMacros/current.gcode.)
  o101 else
    (MSG, Running Current GCode Macro)
    $SD/Run=/ScreenMacros/current.gcode
  o101 end if
o100 elseif[#<_screenMacro> EQ 2]
  o102 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, View Pulloffs - View the current X, Y, and Z pulloffs.)
  o102 else
    (MSG, Running View Pulloffs Macro)
    M0 (PRINT, Motor Pulloffs: X=%.3f#</axes/x/motor0/pulloff_mm>  Y0=%.3f#</axes/y/motor0/pulloff_mm>  Y1=%.3f#</axes/y/motor1/pulloff_mm>  Z0=%.3f#</axes/z/motor0/pulloff_mm>  Z1=%.3f#</axes/z/motor1/pulloff_mm>)
  o102 end if
o100 elseif[#<_screenMacro> EQ 3]
  o103 if[#<_screenMacroInfoOnly> EQ 1]
    (MSG, Z Level - Z level the gantry by probing Z on both sides and setting set Z pulloffs.)
  o103 else
    (MSG, Running Z Level Macro)
    $SD/Run=_zlevel.gcode
  o103 end if
o100 else
  (PRINT, Macro %d#<_screenMacro>: Not Defined)
o100 end if
```


When clicking on the highlighted macro, the same code is run except the _screenMacroInfoOnly variable is set to 0 (false).

This allows the script to know which macro and whether or not it should just display a description message or actually run the macro.


# Notes

## Future Enhancements
> None of these are guaranteed but are things I would like to do in the future.
- Add battery with charging circuit
- Add a dock (or even a magnetic charging dock)
- Make the case thinner possibly by using a mini ESP32 instead of a devkit
