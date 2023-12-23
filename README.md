# Wireless Indoor Localization

## Introduction
This repository contains the implementation of the wireless indoor localization system I presented in my undergraduate thesis project. This is essentially an optimization of the work done in the phase 1 of the project ([Here](https://github.com)) to enable it run on resource contrained devices like microcontrollers.

### Boards and Devices
End Device:
- Target Board: ESP32-based microcontrollers
- Development Board: ESP32-WROOM-32
- Development Enviroment: ESP-IDF framework

Server:
- Target Board: Any single-board-computer with wireless communication capabilities.(General purpose computers will do) 
- Development Board: Rapsberry Pi-4
- Development Environment: vanilla python

## Layout of the Repository
Offline Phase
- `calibration`: This contains the application code for acquiring the digital map(fingerprints) and persisting the data in the offline phase of the localization by fingeprinting approach. 

Online Phase
- `device`: Contains the code for acquiring the real time fingerprints from the microcontroller and sending it  to a localization engine at intervals. 

- `server`: Contains an implementation of Localization engine running on the server. The engine runs inference on the fingeprint sent by the device and returns an estimated real time location of the device. 

Machine Learning Models
- `KNN-model`: KNN algorithm code for deriving the KNN model used for localization

- `KNN-model(compressed)`: `KNN-model` optimized for microcontrollers. 

On-device Inference
- `server-on-device`: Optimized  and minimalistic localization engine(based on the`KNN-model(compressed)`) to run inference on the same microcontroller where the fingerprint acquisition happens. 


## Getting Started
Each sub-project contains a detailed README on how to setup the project and its dependencies. 

## Prerequisites
- An indoor setting with preexisting WAPs (wireless access point) infrastructure installed in a regular and predictable pattern. 
- If porting to another microcontroller, the basic requirements are Wi-Fi capabilities and support for RTOS. To avoid stackoverflow exception during runtime, the reccommended stack size for the task running the websocket server and scanning for avaiable WAPs is about 8192K. 

## Read the Thesis
Find the thesis [Here](https://github.com)

## Contributing
- Ngari Crisphine: crisphine96@gmail.com
- Asogwa Emmanuel: asogwaemmanuel36@gmail.com 

## License

## Acknowledgments
Project Supervisor: Dr. T Nagajuna: ntelgram@gitam.in

