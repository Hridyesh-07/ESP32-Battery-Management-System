# ESP32-Battery-Management-System
Real-Time ESP32 Battery Management System featuring intelligent fault detection, safety protection, LCD diagnostics, relay control, and Blynk IoT monitoring using Wokwi simulation.

# 🔋 ESP32 Battery Management System

### Real-Time Intelligent IoT Battery Monitoring & Safety Protection System

<p>
An enterprise-grade embedded Battery Management System (BMS) developed using ESP32, designed to monitor a simulated 4-cell lithium battery pack in real time. The system combines intelligent battery analytics, event-driven safety protection, fault-tolerant runtime management, cloud telemetry, and a professional IoT dashboard into a single integrated embedded solution.
</p>

![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge)
![Language](https://img.shields.io/badge/Language-Arduino%20C++-00979D?style=for-the-badge)
![IoT](https://img.shields.io/badge/IoT-Blynk-orange?style=for-the-badge)
![Simulation](https://img.shields.io/badge/Simulation-Wokwi-green?style=for-the-badge)
![Architecture](https://img.shields.io/badge/Architecture-Event--Driven-red?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Completed-success?style=for-the-badge)

</div>

---

# 📖 Project Overview

This project was developed as part of the **Elevance Skills Embedded Systems Internship Program**. The objective was to design and implement a **production-inspired Battery Management System (BMS)** capable of monitoring a simulated **4-cell lithium battery pack** while demonstrating professional embedded software engineering practices.

Unlike a conventional Arduino demonstration project, this implementation integrates multiple embedded subsystems into a single modular application, including:

- Real-time battery intelligence and analytics
- Event-driven safety protection
- Intelligent Human-Machine Interface (HMI)
- Fault-tolerant runtime management
- Cloud-based IoT telemetry
- Executive battery monitoring dashboard

The system continuously monitors individual battery cells, evaluates battery health, detects abnormal operating conditions, transitions between runtime modes, and communicates operational data to the cloud through **Blynk IoT**. The entire software architecture is built around a **non-blocking event-driven design**, ensuring responsive operation without using `delay()`.

---

# 🎯 Project Objectives

The project was designed to accomplish the following objectives:

- Monitor four simulated lithium battery cells in real time.
- Calculate pack voltage, average voltage, imbalance percentage, and battery statistics.
- Classify battery health into operational states.
- Detect and isolate battery faults using intelligent safety logic.
- Protect the battery pack through automatic relay control and audible alarms.
- Display live diagnostics using a rotating LCD-based HMI.
- Transmit operational telemetry to the Blynk IoT cloud platform.
- Maintain stable operation during Wi-Fi failures and automatically synchronize after reconnection.
- Provide operators with real-time recommendations, risk analysis, and fault history through an executive monitoring dashboard.

---

# ✅ Internship Tasks Completed

This project successfully integrates **all six internship tasks** into a single modular embedded application. Instead of developing six independent programs, every task has been implemented as part of one production-style Battery Management System.

| Task | Description | Status |
|------|-------------|:------:|
| **Task 1** | Adaptive Multi-Cell Battery Intelligence Engine | ✅ Completed |
| **Task 2** | Event-Driven Safety Protection Kernel | ✅ Completed |
| **Task 3** | Intelligent Embedded HMI & Diagnostic Interface | ✅ Completed |
| **Task 4** | Fault-Tolerant Embedded Runtime System | ✅ Completed |
| **Task 5** | Intelligent Cloud Telemetry Architecture | ✅ Completed |
| **Task 6** | Executive Battery Intelligence Dashboard | ✅ Completed |

---

# ⭐ Key Features

### 🔋 Adaptive Battery Intelligence
- Real-time monitoring of 4 simulated lithium battery cells
- Individual cell voltage measurement
- Pack voltage calculation
- Average cell voltage calculation
- Weakest and strongest cell identification
- Cell imbalance percentage analysis
- Intelligent battery health evaluation

---

### 🛡 Event-Driven Safety Protection
- Fully non-blocking architecture using `millis()`
- Zero use of `delay()`
- Automatic relay protection
- Buzzer warning system
- Fault-priority handling
- Stable state transitions
- Anti-relay chatter protection
- Automatic recovery logic

---

### ⚙ Fault-Tolerant Runtime System

Supports intelligent runtime transitions between:

- 🟢 NORMAL
- 🟡 DEGRADED
- 🟠 FAILSAFE
- 🔴 SHUTDOWN

Implemented fault detection:

- Weak Cell Fault
- Overvoltage Fault
- Sensor Fault
- Frozen ADC Detection
- Voltage Fluctuation Detection
- Relay Mismatch Detection

Additional runtime capabilities:

- Fault isolation
- Timestamped fault logging
- Fault history
- Recovery management
- Runtime diagnostics

---

### 📺 Intelligent LCD Human-Machine Interface

- Five rotating diagnostic screens
- Smooth flicker-free display updates
- Live battery statistics
- Runtime information
- Safety status
- Automatic fault-priority screen override

---

### ☁ Intelligent Cloud Telemetry

- Blynk IoT cloud integration
- State-change-based telemetry transmission
- Wi-Fi auto reconnect
- Event queue synchronization
- Offline-safe embedded operation
- Asynchronous cloud communication

---

### 📊 Executive Monitoring Dashboard

Professional IoT dashboard displaying:

- Individual cell voltages
- Pack voltage
- Average voltage
- Weakest & strongest cell
- Battery health
- Runtime mode
- Safety status
- Fault counter
- Last detected fault
- Risk level
- Operator recommendation
- System uptime

---

# 📊 Project Statistics

| Parameter | Value |
|-----------|------:|
| **Platform** | ESP32 |
| **Programming Language** | Arduino C++ |
| **Simulation Platform** | Wokwi |
| **IoT Platform** | Blynk |
| **Embedded Architecture** | Event-Driven |
| **Battery Cells** | 4 |
| **Internship Tasks Completed** | 6 / 6 |
| **Runtime Modes** | 4 |
| **Fault Types Implemented** | 6 |
| **LCD Diagnostic Screens** | 5 |
| **Cloud Dashboard** | Executive Blynk Dashboard |
| **Code Size** | ~1372 Lines |
| **Project Status** | Completed |

---

# 🛠 Hardware Components

| Component | Quantity |
|-----------|:--------:|
| ESP32 Development Board | 1 |
| 16x2 I2C LCD Display | 1 |
| Potentiometers (Simulated Battery Cells) | 4 |
| Relay Module | 1 |
| Active Buzzer | 1 |

---

# 💻 Software & Technologies

| Technology | Purpose |
|------------|---------|
| Arduino IDE | Embedded firmware development |
| ESP32 Framework | Microcontroller platform |
| Wokwi | Circuit simulation |
| Blynk IoT | Cloud dashboard & telemetry |
| GitHub | Version control & documentation |

---

# 📂 Repository Structure

```text
ESP32-Battery-Management-System/
│
├── Code/
│   └── Battery_Management_System.ino
│
├── Diagrams/
│   ├── Architecture_Diagram.png
│   └── Workflow_Diagram.png
│
├── Images/
│   ├── Dashboard_Healthy_1.png
│   ├── Dashboard_Healthy_2.png
│   ├── Dashboard_Healthy_3.png
│   ├── Dashboard_Healthy_4.png
│   ├── Dashboard_Fault_1.png
│   ├── Dashboard_Fault_2.png
│   ├── Dashboard_Fault_3.png
│   ├── Dashboard_Fault_4.png
│   ├── LCD_Healthy_1.png
│   ├── LCD_Healthy_2.png
│   ├── LCD_Healthy_3.png
│   ├── LCD_Healthy_4.png
│   ├── LCD_Healthy_5.png
│   ├── LCD_Fault.png
│   ├── Fault_Count.png
│   ├── Dashboar_Fault_Count.png
│
├── Wokwi/
│   ├── diagram.json
│   └── libraries.txt
│
├── README.md
├── LICENSE
└── .gitignore
```

---
