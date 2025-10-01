
# Overview

The Smart Greenhouse Project is an IoT-based system built with an ESP32 microcontroller to automate plant care by monitoring and controlling essential environmental conditions.
This project integrates multiple sensors and actuators to ensure optimal growth conditions for plants, while providing remote monitoring and control capabilities.

The goal is to create a low-cost, scalable, and educational solution that combines electronics, automation, and IoT concepts.

# Features

🌡 Temperature & Humidity Monitoring – via a DHT22 sensor, providing reliable data about the greenhouse microclimate.

🌱 Soil Moisture Measurement – ensures irrigation is performed only when necessary.

💧 Automatic Irrigation – a water pump controlled by the ESP32 activates when the soil moisture drops below a threshold.

☀️ Light Intensity Detection – using a photoresistor (LDR) to monitor sunlight exposure, which can be used to trigger lighting systems in extended setups.

🛢 Water Tank Level Monitoring – an HC-SR04 ultrasonic sensor measures the remaining water level to prevent the pump from running dry.

📡 Remote Monitoring – data can be sent to external platforms (such as Blynk, MQTT, or a custom webserver) for visualization and control.

# Hardware Components

ESP32 Development Board (main controller, Wi-Fi enabled)

DHT22 Sensor (temperature & humidity)

Soil Moisture Sensor

Photoresistor (LDR) (light intensity)

HC-SR04 Ultrasonic Sensor (water tank level)

Water Pump + Transistor Driver Circuit (controlled irrigation)

Power Supply Module (3.3V / 5V)

Jumper wires, breadboard, and optional relay/driver modules

# How It Works

## Data Collection

The DHT22 continuously measures ambient temperature and humidity.

The soil moisture sensor monitors the hydration level of the soil.

The photoresistor (LDR) detects the light intensity, helping evaluate plant exposure.

The HC-SR04 measures the distance to the water surface in the tank, providing an estimate of the available water volume.

# Decision Making

The ESP32 runs a control algorithm that compares the sensor values with pre-defined thresholds.

Example:

If soil moisture < threshold, the pump is activated.

If tank level is too low, the system prevents pump activation and issues a warning.

If temperature or humidity are critical, the system logs alerts or can trigger actuators (like fans or heaters in future upgrades).

# Actuation

A transistor or relay driver allows the ESP32 to switch the pump on and off safely, even if the pump requires higher current.

The irrigation cycle runs only for a set amount of time, avoiding overwatering.

# Communication

The ESP32 can log sensor values locally (serial monitor) or send them to external dashboards via Wi-Fi.

Remote platforms (e.g., Blynk, MQTT broker, or custom webserver) allow visualization and manual override of the system.


# Educational Value

This project is ideal for:

📘 Students learning about IoT, microcontrollers, and automation.

🔧 Hobbyists interested in DIY smart farming projects.

🌱 Gardeners who want to simplify and optimize plant care.

# License

This project is open-source and available for educational and personal use. Contributions and improvements are welcome!
