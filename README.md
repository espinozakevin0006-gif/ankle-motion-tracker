# Wearable Ankle Motion Tracking System

An embedded wearable sensing prototype that measures ankle motion using an MPU-6050 and streams real-time biomechanical data to a browser-based dashboard over Wi-Fi.

---

## Overview

This project was built as an embedded biomechanics prototype for ankle motion tracking. An ESP32 reads accelerometer and gyroscope data from an MPU-6050, estimates ankle orientation, detects movement events, and publishes live data to a web interface hosted directly on the microcontroller.

The system is designed as a foundation for wearable gait monitoring, movement analysis, and real-time feedback applications.

---

## Key Features

- Reads 3-axis acceleration and angular velocity from the MPU-6050 IMU
- Computes ankle angle estimates using accelerometer data
- Detects motion events using gyroscope threshold logic
- Tracks step count and estimates cadence
- Hosts a live browser dashboard using the ESP32 as a Wi-Fi access point
- Streams real-time sensor values and plots motion data in the web interface

---

## What I Built

- Interfaced the MPU-6050 with an ESP32 using I2C communication
- Developed embedded firmware to acquire, process, and interpret inertial sensor data
- Implemented angle estimation using `atan2()` on accelerometer readings
- Built threshold-based motion detection logic to track steps and estimate cadence
- Configured the ESP32 as a standalone Wi-Fi hotspot and hosted an onboard web server
- Created a browser dashboard with live JSON updates and real-time graphing of motion signals

---

## Hardware

- ESP32 Development Board
- MPU-6050 6-axis IMU
- Breadboard
- Jumper wires

---

## Software and Tools

- Arduino IDE
- C/C++
- Wire library
- WiFi library
- WebServer library
- Embedded web interface using HTML, CSS, and JavaScript

---

## System Architecture

```text
MPU-6050 IMU
   ↓
I2C Communication
   ↓
ESP32
   ├── Sensor acquisition
   ├── Angle estimation
   ├── Motion detection
   ├── Step counting and cadence estimation
   └── Web server + JSON data endpoint
   ↓
Wi-Fi dashboard in browser
