# Arduino IR Countdown Timer

A versatile countdown timer for Arduino controlled via an IR Remote, featuring a 4-digit 7-segment display.

## 🚀 Features

- **Countdown**: High-precision countdown from up to 99 minutes.
- **IR Control**: Fully operated via infrared remote control.
- **Visual Feedback**: Displays `---` when idle and blinks all segments when the time is up.
- **Dynamic adjustment**: Intelligent time adjustment (+/- 10 minutes or +/- 1 minute depending on the current time).

## 🛠️ Hardware Requirements

- **Arduino** (Uno, Nano, or similar)
- **4-Digit 7-Segment Display** (Common Cathode)
- **IR Receiver** (e.g., AX-1838)
- **IR Remote Control**
- Jumper wires and Breadboard

## 🔌 Pinout & Wiring

### 7-Segment Display (4 Digits)
Connect the segment and digit pins as follows:

| Component | Arduino Pin |
|-----------|-------------|
| Segment A | 2           |
| Segment B | 3           |
| Segment C | 4           |
| Segment D | 5           |
| Segment E | 6           |
| Segment F | 7           |
| Segment G | 8           |
| Digit 1   | 9           |
| Digit 2   | 10          |
| Digit 3   | 11          |
| Digit 4   | 12          |
| Decimal Pt| A5          |

### IR Receiver
| Pin | Arduino Pin |
|-----|-------------|
| OUT | 13          |
| VCC | 5V          |
| GND | GND         |

## 🎮 Remote Controls

The timer uses the following IR command hex codes (standard for many generic remotes):

- **0x43 (Play/Pause)**: Start or Pause the timer. If finished or idle, sets time to 60:00.
- **0x44 (Reset)**: Stops the timer and clears the display to idle state (`---`).
- **0x15 (+)**: Adds 10 minutes (or 1 minute if below 10 min).
- **0x07 (-)**: Subtracts 10 minutes (or 1 minute if below 10 min).

## 📦 Dependencies

This project requires the **IRremote** library. You can install it via the Arduino Library Manager.

---
*Created as a project demonstration.*
