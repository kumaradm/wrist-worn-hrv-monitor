# Wearable HRV Monitor (RMSSD-Based)

A low-power, wrist-worn system designed for real-time **Heart Rate Variability (HRV)** monitoring. This device utilizes Photoplethysmography (PPG) to track peripheral blood volume changes and calculates autonomic nervous system health metrics directly on the edge.

---

## Project Overview

This project focuses on the intersection of embedded systems and digital signal processing. By using a wrist-based PPG sensor, the system captures blood volume changes, filters noise, and identifies Inter-Beat Intervals (IBI) to compute HRV using the **RMSSD** method.

### Key Features

* **High-Sensitivity PPG Sensing:** Optical detection of blood flow via **MAX30102**.
* **On-Device Computation:** Using an **ESP32-C3 (RISC-V)** to ensure low latency.
* **Precision Power Management:** Integrated Li-Ion charging and **MAX17048** fuel gauge for accurate battery measurement.
* **Robust Metric:** Implements the **RMSSD** (Root Mean Square of Successive Differences) for robust short-term HRV analysis.

---

## Technical Specifications

### Hardware Architecture

* **MCU:** ESP32-C3-MINI-N1 (Wi-Fi/BLE enabled).
* **Level Shifting:** PCA9306 bidirectional translator to bridge $1.8V$ (Sensor) and $3.3V$ (MCU) $I^2C$ domains.
* **Memory:** **128Mb W25Q128** Flash via SPI for high-frequency data logging.
* **Debugging:** **Native USB Serial/JTAG Controller** for real-time debugging.
* **Power:** Dual LDO architecture (**AP2112K**) to isolate sensitive analog sensor rails from digital switching noise.

<p align="center">
  <img src="./docs/images/High%20Level%20System%20Architecture.png" width="60%" alt="High Level System Architecture"><br>
  <em>High Level System Architecture</em>
  <br><br>
</p>

<p align="center">
  <img src="./docs/images/Power%20Management%20Subsystem%20Architecture.png" width="60%" alt="Power Management Subsystem Architecture"><br>
  <em>Power Management Subsystem Architecture</em>
  <br><br>
</p>

<p align="center">
  <img src="./docs/images/Sensors%20&%20User%20Interface%20Subsystem%20Architecture.png" width="60%" alt="Sensors & User Interface Subsystem Architecture"><br>
  <em>Sensors & User Interface Subsystem Architecture</em>
  <br><br>
</p>

<p align="center">
  <img src="./docs/images/Core%20Processing%20&%20Memory%20Subsystem%20Architecture.png" width="60%" alt="Core Processing & Memory Subsystem Architecture"><br>
  <em>Core Processing & Memory Subsystem Architecture</em>
</p>

### The Algorithm (RMSSD)

The system calculates the Root Mean Square of Successive Differences between adjacent heartbeats ($RR$ intervals) to quantify parasympathetic activity:

$$\text{RMSSD} = \sqrt{\frac{1}{N-1} \sum_{i=1}^{N-1} (RR_{i+1} - RR_i)^2}$$

This calculation is performed in a rolling window, allowing the wearer to see real-time shifts in their stress levels or recovery state.

---

## Repository Structure

```text
wearable-hrv-monitor/
├── hardware/        # PCB Schematics, Layouts (KiCad), and BOM
├── firmware/        # MCU source code (C++/Arduino) and Libraries
├── docs/            # Technical datasheets and RMSSD math derivation
├── assets/          # Images, diagrams, and project photos
└── README.md        # Project documentation
```

## Disclaimer

This device is intended for educational and research purposes only. It is not a medical device and should not be used to diagnose or treat any health conditions.