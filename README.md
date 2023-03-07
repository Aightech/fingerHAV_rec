# fingerHAV_rec
fingerHAV_rec is a module for recording and analyzing finger motion data. It consists of several sub-programs that work together to capture and process data from multiple sources, including Kistler sensors, load cells, and stereoscopic cameras.

##  Programs
### kistler
This sub-program is used to capture accelerometer and audio data from Kistler sensors. It can be set up for continuous recording, and is designed to ensure that any run will work.

### loadcells
The loadcells sub-program is used to display interpolated force torque data and stream data from eight loads and six force torque sensors.

###finger_pos
The finger_pos sub-program is used to stream the position of the finger tip from stereoscopic cameras. It can display data from both cameras and includes the ability to calibrate nail detection color.

### lsldbb
The lsldb sub-program is used to manage the LSL database.

### lslplot
The lslplot sub-program is used to plot data streams from the other sub-programs.

## Starting script
The starting script is used to run all of the sub-programs simultaneously. It includes commands to run kistler continuous, loadcells, finger_pos, and lslplot for each sub-program.

## GUI

The GUI is a Windows application built using Qt. It allows users to check that all three LSL streams are available and create a trial. Users can select materials from a random list and keep track of the number of trials and time. They can also add comments for each trial and start the trial. The GUI runs the lsl_dbfeeder program with the [user_material_trialID] parameter. The motion to follow is started, and the OpenCV window displays the movement to follow. The GUI gets motion mode from the experiment GUI and reads force and position from the stream. It displays whether the force is in the working force range.

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
