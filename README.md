# GRIP vision processing on an Intel Galileo

Girls of Steel, FIRST FRC Team 3504 (<http://girlsofsteelrobotics.com/>)

This repository contains documentation and source code for running GRIP vision processing pipelines on an Intel Galileo embedded computer. The primary audience is FIRST FRC teams interested in using the retro-reflective targets on the game field as a guide for autonomous robot operation.

This repository's Wiki has a series of detailed set up guides:
* [Why use the Intel Galileo for vision processing?](https://github.com/GirlsOfSteelRobotics/GRIPonGalileo/wiki/Why-Intel-Galileo)
* [Intel Galileo setup](https://github.com/GirlsOfSteelRobotics/GRIPonGalileo/wiki/Intel-Galileo-setup)
* [Creating the GRIP pipeline](https://github.com/GirlsOfSteelRobotics/GRIPonGalileo/wiki/Creating-the-GRIP-pipeline)
* [Compiling and installing GRIP-Runner](https://github.com/GirlsOfSteelRobotics/GRIPonGalileo/wiki/Compiling-and-installing-GRIP-Runner)
* [Microsoft Lifecam manual exposure details](https://github.com/GirlsOfSteelRobotics/GRIPonGalileo/wiki/Microsoft-Lifecam-manual-exposure)

As an introduction, here are the three main components of the project:

* GRIP is a graphical application for designing and debugging vision processing pipelines built on OpenCV. Vision inputs can be configured for webcams, network cameras, or collections of images taken in the field. As filtering operations are added and adjusted, the results can be seen immediately. The output is generated source code that implements the pipeline in Java, C++, or Python. With GRIP, computer vision is much more approachable for novices and significantly more convenient for experienced OpenCV users. (<https://github.com/WPIRoboticsProjects/GRIP/blob/master/README.md>)

* Intel Galileo is a single-board computer built for embedded and IOT applications. It is used in this project to offload the CPU-intensive image processing work from the main control computer on the robot. This removes the risk of sluggish or jittery robot response resulting from vision pipelines that take too long to execute. It is inexpensive (~$50) and comes with exactly the interfaces and OpenCV libraries needed for this use case. (<https://software.intel.com/en-us/iot/hardware/galileo>)

* Microsoft Lifecam HD-3000 is a USB webcam recommended by FIRST and supported on the Mac, Windows, and Linux. A key element of this project is custom-written software for obtaining images from the Lifecam with proper exposure. (The automatic exposure levels for this camera are far too high for FRC vision processing but manual controls are available via the driver API.)
