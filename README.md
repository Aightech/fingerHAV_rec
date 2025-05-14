# fingerHAV_rec
fingerHAV_rec is a module for recording and analyzing finger motion data. It consists of several sub-programs that work together to capture and process data from multiple sources, including Kistler sensors, load cells, and stereoscopic cameras.

# Requirements
- CMake
- qt6 (for GUI: sudo apt install qt6-base-dev qt6-tools-dev qtbase5-private-dev)
- postgresql (for lslsub_dbfeeder: sudo apt install postgresql libpq-dev libboost-all-dev)
- portaudio (for kistler: sudo apt install libasound-dev portaudio19-dev)
- sfml (sudo apt install libsfml-dev)

##  Programs (in prgm/ folder)
### kistler
This sub-program is used to capture accelerometer and audio data from Kistler sensors. It can be set up for continuous recording, and is designed to ensure that any run will work.

### loadcells
The loadcells sub-program is used to display interpolated force torque data and stream data from eight loads and six force torque sensors.

### finger_pos
The finger_pos sub-program is used to stream the position of the finger tip from stereoscopic cameras. It can display data from both cameras and includes the ability to calibrate nail detection color.

### lslsub_dbfeeder
The lsldb sub-program is used to manage the LSL database.

### lslplot
The lslplot sub-program is used to plot data streams from the other sub-programs.

### Starting script
The starting script is used to run all of the sub-programs simultaneously. It includes commands to run kistler continuous, loadcells, finger_pos, and lslplot for each sub-program.

### GUI

The GUI is a Windows application built using Qt. It allows users to check that all three LSL streams are available and create a trial. Users can select materials from a random list and keep track of the number of trials and time. They can also add comments for each trial and start the trial. The GUI runs the lsl_dbfeeder program with the [user_material_trialID] parameter. The motion to follow is started, and the OpenCV window displays the movement to follow. The GUI gets motion mode from the experiment GUI and reads force and position from the stream. It displays whether the force is in the working force range.


## Recording protocol
In each trial, the volunteer sat in front of the acquisition setup and waited for the start of the recording. At the beginning of each trial, the finger was placed above the texture without touching it, enabling the calibration of the load measurement. 

### Motions
To provide variability in the recorded movements, each trial included,
- 3s above the texture without touching
- 10s of lateral back and forth sliding. 
- 10s of proximal/distal back and forth sliding. 
- 10s of clockwise circular sliding.
- 10s of anti-clockwise circular sliding.
- 20s of unconstrained sliding.


The volunteer made best efforts to follow the displayed target in the GUI during the constrained motion phase. During the last 20~s of unconstrained motion, volunteer was free to vary the load and the scanning speed.

### Textures
The textures are selected randomly and changed after each trial to limit any "learning" and limit the disconfort of touching rougth texture for a prolonged time.


### Submodules dependencies
#### libraries
- [built_lsl](lib/built_lsl/README.md)

### Program dependencies
- [finger_pos](prgm/finger_pos/README.md)
- [loadcells](prgm/loadcells/README.md)
- [kistler_DAQ](prgm/kistler_DAQ/README.md)
- [lslsub_dbfeeder](prgm/lslsub_dbfeeder/README.md)
- [lslsub_plotter](prgm/lslsub_plotter/README.md)



# Building source code

To build the project run:
```bash
cd fingerHAV_rec
mkdir build && cd build
cmake .. && make
```

# Demonstration app

When the project have been built, you can run:
```bash
./fingerHAV_rec -h
```
to get the demonstration app usage.

# Example
Open the ![main.cpp](cpp:src/main.cpp) file to get an example how to use the lib.



# Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.