# HomeKit Gate Controller for Centurion D5-Evo Gate Motor

**HomeKit gate controller for Centurion (Centsys) D5 Evo gate motor**

This project implements a HomeKit-compatible gate controller using an ESP8266 microcontroller and the Arduino-HomeKit-ESP8266 library, specifically designed for Centurion (Centsys) D5-Evo gate motors. It enables remote control and status monitoring of the gate with Apple HomeKit.

## Features
- ✅ **HomeKit Integration:** Seamless control via the Apple Home app using the HomeSpan library.
- ✅ **Gate Control:** Open and close the gate with a relay trigger.
- ✅ **Status Detection:** Real-time detection of gate states (open, closed, opening, closing).
- ✅ **Gate Status** Detects when gate opens/closes from a different source (e.g. remote control, intercom, keypad).

## How It Works
The system uses an ESP8266 microcontroller to manage a relay and monitor the gate's operational state:
- **Relay Control:** The relay pin (GPIO 14) triggers the gate motor.
- **Status Monitoring:** The status pin (GPIO 12) monitors the gate's operation based on pulse patterns sent by the gate controller.
- **Flashing Patterns:** The gate signals its state using pulse patterns, where the duration of HIGH and LOW signals indicates whether the gate is opening or closing.
- **Logic Level Conversion:** A logic level converter is used to shift the gate's 5V status signal to a 3.3V input suitable for the ESP8266.

The `main.ino` class implements:
- `update()` – Triggered by HomeKit to open/close the gate.
- `loop()` – Continuously monitors the gate's state based on the input pin.

## Hardware Requirements
- ESP8266 microcontroller
- Relay module (for gate control)
- Logic level converter (5V to 3.3V signal conversion on status pin)

## Wiring

This section details the connections between the gate motor, logic level converter (LLC), relay module, buck converter, and ESP32 using individual tables for each component.

### 1. Gate Motor Connections

| **Gate Motor** | **Connection** | **Destination** | **Notes** |
| :------------- | :------------- | :-------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| GND (Safe COM) | -------->      | Common Ground   | Common ground for all components. Use "Safe COM" ground from the gate controller for isolation. And Connect to relay                                                                  |
| +12V Out       | -------->      | Buck Converter  | 12V output from the gate motor, used as input for the buck converter.                                                                                            |
| Status         | -------->      | LLC HV IN       | Status output from the gate motor (typically 5V), connected to the high-voltage input of the LLC.                                                                   |
| TRIG           | <--------      | Relay NO        | Trigger input of the gate motor, connected to the Normally Open (NO) terminal of the relay to activate gate movement.                                                |

### 2. Logic Level Converter (LLC) Connections

| **LLC** | **Connection** | **Source/Destination** | **Notes** |
| :------ | :------------- | :--------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HV IN   | <--------      | Gate Motor Status      | Status output from the gate motor (typically 5V).                                                                                                                   |
| HV      | -------->      | Buck Converter 5V      | High-voltage side of the LLC, powered by 5V from the buck converter.                                                                                              |
| LV      | -------->      | ESP32 3.3V             | Low-voltage side of the LLC, powered by the ESP32's 3.3V output.                                                                                                |
| LV OUT  | -------->      | ESP32 GPIO 39          | Logic level converted signal (3.3V), input to the ESP32's GPIO 39 for gate status monitoring.                                                                       |
| GND     | -------->      | Common Ground          | Common ground for all components.                                                                                                                                  |

### 3. Relay Module Connections

| **Relay Module** | **Connection** | **Source/Destination** | **Notes** |
| :--------------- | :------------- | :--------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IN               | <--------      | ESP32 GPIO 35          | Control signal (3.3V) from the ESP32's GPIO 35, triggering the relay's operation.                                                                                 |
| NO               | -------->      | Gate Motor TRIG         | Normally Open (NO) terminal, connected to the gate motor's trigger input.                                                                                            |
| COM (GND)        | -------->      | Common Ground          | Common ground (GND) of the relay module.                                                                                                                     |
| VCC              | -------->      | Buck Converter 5V      | Power supply for the relay module (typically 5V), from the buck converter's 5V output.                                                                           |
| GND              | -------->      | Common Ground          | Common ground for all components.                                                                                                                                  |

### 4. Buck Converter Connections

| **Buck Converter** | **Connection** | **Source/Destination** | **Notes** |
| :---------------- | :------------- | :--------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| VIN (+)           | <--------      | Gate Motor +12V Out    | 12V input from the gate motor.                                                                                                                                   |
| VOUT (+) 5V       | -------->      | LLC HV, Relay VCC      | 5V output, powers the relay module and the high-voltage side of the LLC.                                                                                           |
| VOUT (+) (Adjusted)| -------->      | ESP32 Power            | ESP32 power from buck converter (if adjusted).                                                                                                                     |
| GND               | -------->      | Common Ground          | Common ground for all components.                                                                                                                                  |

### 5. ESP32 Connections

| **ESP32** | **Connection** | **Source/Destination** | **Notes** |
| :-------- | :------------- | :--------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| GND       | -------->      | Common Ground          | Common ground for all components.                                                                                                                                  |
| 3.3V      | -------->      | LLC LV                 | Power supply for the low-voltage side of the LLC.                                                                                                                   |
| 5V       | -------->      | Relay VCC, LLC HV      | Power supply for the relay and LLC (if not using buck converter).                                                                                                    |
| GPIO 35   | -------->      | Relay IN               | Control signal to activate the relay.                                                                                                                               |
| GPIO 39   | <--------      | LLC LV OUT             | Status signal from the gate motor (converted by LLC).                                                                                                              |

**Notes:**

* **Common Ground:** All components share a common ground connection.
* **Buck Converter:** The buck converter efficiently steps down the 12V from the gate motor to 5V (and potentially 3.3V) for the other components.
* **LLC:** The logic level converter ensures compatibility between the 5V gate status signal and the ESP32's 3.3V logic.
* **Relay:** The relay acts as the switch to trigger the gate motor's movement.


![428311938-f44cfe6e-a5c6-495d-8480-1c9bc4ff0227](https://github.com/user-attachments/assets/59c98669-cf6b-4768-b37a-f5637a8d8164)

Only three connections need to be made to the D5 Evo Gate Motor:
1. Trig: this needs to be connected to ground for the gate to activate. I have connected this to a relay controlled by the ESP32
2. Status: this needs to be logic-level-converted from 5V to 3.3V. Connect to a GPIO pin of the ESP32
3. Com (ground, 0V): this is important for two reason: (1) status decoding and (2) triggering (activating) the gate. Because of (1), **the gate must share a ground with the esp32**. 


## Software Requirements
- [Arduino-HomeKit-ESP8266](https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266)
- Arduino IDE (for programming the ESP8266)

## Configuration
- **Wi-Fi:** Set credentials in `wifi_info.h`.
- **HomeKit Pin:** Default pairing pin is `11111111 (8 ones)`.
- **Pins:** Adjust `statusPin` and `relayPin` if necessary.
  
## Installation
1. Install the HomeSpan library in Arduino IDE.
2. Download and open both `main.ino`, `my_accessory.c ` and `wifi_info.h` in an IDE.
3. Configure Wi-Fi credentials in `wifi_info.h`, HomeKit pairing code in `my_accessory.c`.
4. Upload the files to your ESP8266.
5. Pair the device with HomeKit using the code.

## Future Improvements
- **Obstruction Detection:** Currently not implemented.

## License
This project is open-source under the MIT License.

## Contributing
Contributions are welcome! Feel free to submit pull requests or open issues for improvements and bug fixes.
