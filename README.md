
# IoT project: Speed Camera and remote control car

Our project consists of a small remote‑controlled rover that uses a gyroscope for steering and movement. The rover passes through a scaled-down speed trap (autovelox) that measures its speed. Depending on whether the speed limit is exceeded, an LED changes color (for example green if under the limit, red if over). The system also sends a JSON file to a web server, which logs and stores all recorded speed‑check data for further analysis.

## Table of contents
* [Hardware and software reuirements](#hardware-and-software-requirements)
* [Project layout](#project-layout)
* [How to build, burn and run the project](#build-proj)
* [User guide](#user-guide)
* [Links](#links)
* [Team members](#team-members)

## Hardware and software requirements

### Hardware requirements
- Arduino UNO for our car
- ESP32 for speed camera
- Web dashboard for live monitoring
- Infrared sensors (?)
- LED (/display?) to show the speed (above or below the limit)
- Breadboard and jumper wires
- Power source(s)

![Porva immagine](./images/schematic_car.png)

### Software requirements
- Arduino IDE
- Common libraries (...)
- Web server (...)

## Project layout
~~~
/project-root
├── README.md
├── docs/
├── firmware/
│   ├── arduino_car/
│   ├── esp32/
│   ├── fsm_velox/
│   ├── progettoRX_TX_con_NRF04_TX_RX_RF/
│   └── TEST_FILE/
├── hardware/
├── images/
├── report/
├── .gitignore
└── LICENSE
~~~
- `docs/` contains the web page and supporting scripts used for the project presentation.
- `firmware/` contains all embedded software sources.
- `arduino_car/` contains car tests
- `esp32/` includes the ESP32 firmware, web server files, and the main sketch.
- `fsm_velox/` contains the Velox state machine firmware and the code for sending data to the web server.
- `progettoRX_TX_con_NRF04_TX_RX_RF/` ??
- `TEST_FILE/` includes isolated tests for motors, sensors etc...
- `hardware/` contains schematics.
- `images/` stores pictures used in documentation.
- `report/` contains the final report (and related material).



## User guide
This is how to build and run the project
### 1. Setup Arduino IDE
- Arduino IDE 2.x installation.
- adding ESP32 Dev Module con COM5 port (Select the right USB).

### 2. Firmware ESP32 (for the speed camera)
- Open `firmware/fsm_velox/fsm_velox.ino` in Arduino IDE.
- **Sketch > Verify/Compile** for building.
- **Sketch > Upload** for ESP32.
- Open **Serial Monitor** (9600 baud) for log
- Access at `http://192.168.1.4` from the browser.

### 3. Firmware Arduino (for the car)
x ste

##  Links
- PowerPoint presentation:
- YouTube video:

## Team members and contributions
- Luciani Stefano - 
- De Cao Andrea - 
- Boscardin Denise - 
- Heenatigala Devmin - 