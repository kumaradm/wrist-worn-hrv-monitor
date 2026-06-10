# Wrist-Worn HRV Monitor (RMSSD-Based)

**This project is currently under active development.** Hardware, firmware, and documentation are subject to change.

A low-power, wrist-worn system designed for real-time **Heart Rate Variability (HRV)** monitoring. This device utilizes Photoplethysmography (PPG) to track peripheral blood volume changes and calculates autonomic nervous system health metrics directly on the edge.

---

## Project Overview

This project focuses on the intersection of embedded systems, power electronics, and digital signal processing. By using a wrist-based PPG sensor, the system captures blood volume changes, filters motion noise using an onboard IMU, and identifies Inter-Beat Intervals (IBI) to compute HRV using the **RMSSD** method. Processed metrics are logged locally to SPI flash and offloaded wirelessly over MQTT.

### Key Features

* **High-Sensitivity PPG Sensing:** Optical detection of blood flow via **MAX30101** with an 18-bit internal ADC.
* **Motion Artifact Rejection:** **BMI270** 6-axis IMU provides real-time inertial telemetry for mathematical noise cancellation on the PPG signal.
* **On-Device DSP:** Cascading biquad LPF/HPF, derivative filter, and moving window integrator running on an **ESP32-C3 (RISC-V)** for low-latency peak detection and IBI extraction.
* **Robust HRV Metric:** Implements **RMSSD** (Root Mean Square of Successive Differences) for reliable short-term autonomic nervous system analysis.
* **Precision Power Management:** Integrated TP4056 Li-Ion charging, TPS61023 boost conversion, dual AP2112K LDO regulation, and **MAX17048** fuel gauge for accurate battery State-of-Charge reporting.
* **Wireless Data Offloading:** Wi-Fi + BLE via the ESP32-C3's integrated antenna; cloud telemetry streamed over **MQTT** with NTP time synchronization.
* **Software-Controlled Power Latch:** MCU asserts a GPIO keep-alive signal post-boot, enabling graceful shutdown entirely from firmware.

---

## Technical Specifications

### Hardware Architecture

| Category | Component |
|---|---|
| **Microcontroller** | ESP32-C3-MINI-1-N4 (32-bit RISC-V, up to 160 MHz, Wi-Fi + BLE) |
| **PPG Sensor** | MAX30101 Pulse Oximeter & Heart Rate Monitor |
| **IMU** | Bosch BMI270 6-Axis (Accelerometer + Gyroscope) |
| **Battery Charger** | TP4056 Linear Charger (500mA) |
| **Boost Converter** | TPS61023DRLT (3.7V → 5V) |
| **LDOs** | AP2112K-3.3TRG1 (3.3V), AP2112K-1.8TRG1 (1.8V) |
| **Fuel Gauge** | MAX17048 (ModelGauge SoC + VCELL over I2C) |
| **Flash Memory** | Winbond W25Q128 128Mb (16MB) SPI Flash |
| **I2C Level Shifter** | PCA9306DCUT bidirectional translator (1.8V ↔ 3.3V) |
| **LED Driver** | IS31FL3193 3-Channel Smart RGB Driver |
| **ESD Protection** | PESD5V0 TVS Diodes on USB-C port |
| **Battery** | LP702530 LiPo 3.7V 500mAh |
| **PCB** | 4-Layer, wearable-optimized, 0603/0805 SMD components |

<p align="center">
  <img src="./docs/images/PCB 2D - Top.png" width="60%" alt="PCB 2D Top Layer"><br>
  <em>PCB 2D Top Layer</em>
  <br><br>
</p>

<p align="center">
  <img src="./docs/images/PCB 2D - Bottom.png" width="60%" alt="PCB 2D Bottom Layer"><br>
  <em>PCB 2D Bottom Layer</em>
  <br><br>
</p>

<p align="center">
  <img src="./docs/images/PCB 3D - Top.png" width="60%" alt="PCB 3D Top-View"><br>
  <em>PCB 3D Top-View</em>
  <br><br>
</p>

<p align="center">
  <img src="./docs/images/PCB 3D - Bottom.png" width="60%" alt="PCB 3D Bottom-View"><br>
  <em>PCB 3D Bottom-View</em>
  <br><br>
</p>

<p align="center">
  <img src="./docs/images/PCB 3D - Front.png" width="60%" alt="PCB 3D Front-View"><br>
  <em>PCB 3D Front-View</em>
  <br><br>
</p>

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

### Firmware Architecture

The firmware runs on **FreeRTOS** with a modular, event-driven design. Five functional blocks communicate through asynchronous queues, hardware ISRs, and a centralized state machine thread.

| Block | Responsibility |
|---|---|
| **System Manager** | Central orchestration; initializes I2C, SPI, GPIO; drives the FSM |
| **PPG Sensor Module** | ISR-driven FIFO capture from MAX30101; non-blocking queue push to avoid CPU polling |
| **Calculation Handler** | Biquad LPF/HPF → derivative filter → moving window integrator → peak detection → RR intervals → RMSSD |
| **Flash Memory Manager** | Page-by-page SPI write during acquisition; sequential read during offload |
| **Network Manager** | Wi-Fi station activation, NTP sync, persistent MQTT socket for cloud telemetry streaming |

### System State Machine

The firmware navigates three primary states driven by physical button press duration:

<p align="center">
  <img src="./docs/images/System State Diagram.png" width="60%" alt="System State Machine"><br>
  <em>State System Machine</em>
</p>

### The Algorithm (RMSSD)

The system calculates the Root Mean Square of Successive Differences between adjacent heartbeats ($RR$ intervals) to quantify parasympathetic activity:

$$\text{RMSSD} = \sqrt{\frac{1}{N-1} \sum_{i=1}^{N-1} (RR_{i+1} - RR_i)^2}$$

This calculation is performed in a rolling window, allowing the wearer to see real-time shifts in their stress levels or recovery state.

---

## Repository Structure

```text
wrist-worn-hrv-monitor/
├── docs/                # Full documentation, images, diagrams, and project photos 
├── hardware/            # PCB Schematics and BOM
├── simulation/          # Wokwi simulation files
└── README.md            # Project overview
```

## Disclaimer

This device is intended for **educational purposes only**. It is not a medical device and should not be used to diagnose or treat any health conditions.

## Author

**Kumara Drestanto Mubarokkhan** — Electronics Engineer  
[linkedin.com/in/kumaradm](https://linkedin.com/in/kumaradm) · [github.com/kumaradm](https://github.com/kumaradm) · kumaradrestantom@gmail.com