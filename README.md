# Light Switch Device

This is a small Arduino-based light switch that can flip a physical switch remotely.  
I built it to control my dorm room light from bed or across the room using an IR remote or push button.

## Overview

The device uses a servo motor to push or pull the switch lever when triggered.  
It stays in low-power sleep mode most of the time and only wakes up when the button or IR sensor detects a signal.

It’s simple but really fun — I learned how to make hardware react instantly while saving power.

## Hardware

| **Microcontroller** | Arduino Nano (or Pro Mini) |
| **Servo Motor** | MG995 micro servo |
| **IR Receiver** | HX1838 (reads remote signal) |
| **Button** | AE1027 Push button connected to interrupt pin |
| **Power** | 4 x AA battery |
| **Mount** | 3D-printed holder that presses the switch |

## How It Works

- The Arduino sleeps most of the time to save power.  
- When the IR receiver or button detects input, it wakes the board.  
- The servo moves between two positions (ON and OFF).  
- After switching, the system waits a short time and goes back to sleep.
