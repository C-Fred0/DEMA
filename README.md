## DEMA
DEMA (Donde Esta Mi Abuelito [Where Is My Grandad]) is a project for the ESP32 that involves Machine Learning to indicate the status of a person that can wear a gadget on their belt.

The model is trained in Python using TensorFlow and adapted to TensorFlowMicro. 
![image](https://github.com/user-attachments/assets/1f280782-aa27-48ba-a641-6e9801b5ef04)

It is adapted to predict if a person is:
- Standing
- Sitting
- Laying down
- Walking
- Switching positions
- Falling*

Our interest is detecting if an elderly person has fallen and requires assistance. So with the MPU6050 it reads a string of 3 values [acceleration of the X,Y and Z axis] each 100 miliseconds for 3 seconds. Then, the ESP32 makes a prediction and if it guesses that the person has fallen, it will send an SMS [using the SIM7600] to a saved number with the location of the user [with the NEO-M8] on Google Maps.

![image](https://github.com/user-attachments/assets/80ff7b5d-ecc6-4817-ba54-af0502caf46d)

![image](https://github.com/user-attachments/assets/b465546e-93c0-43e0-8f18-53f82fd8955b)


(The web page is not included, but the labels for each prediction are in managed_components).
In order to aquire data for the training, there are 2 buttons, one for aquiring data and another to send the data via MQTT protocol. In a web server, the 90 values (that consist of a sample) are sent to a PC and the values are organized (columns of X, Y and Z values) with a pythonscript and saved in a .csv file for each category. A total of 160 samples per category were used for the training.

## How to use example
We encourage the users to use the example as a template for the new projects.
A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

