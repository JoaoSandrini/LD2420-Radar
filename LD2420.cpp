/*
  This code is used to interface with the HLK-LD2420 Radar Module
  The module is connected to UART2 (TX2/RX2) on the ESP32
  Developed By João Pedro Sandrini Milanezi
  Date: 06-03-2025
  Version: 1.0

  This project is part of the NERDSCrossing Project Developed by the NERDS Team, in Universidade Federal do Espírito Santo
  This study was financed, in part, by the São Paulo Research Foundation (FAPESP), Brasil. Process Number #2024/23727-8
 */

//libraries
#include <HardwareSerial.h>
#include <vector>
#include <cstddef>
#include <Arduino.h>

//Sensor Serial Communicaiton Ports
#define RX_PORT 16
#define TX_PORT 17

//Setup trigger and hold commands translation
#define TRIGGER_0 0x10
#define TRIGGER_1 0x11
#define TRIGGER_2 0x12
#define TRIGGER_3 0x13
#define TRIGGER_4 0x14
#define TRIGGER_5 0x15
#define TRIGGER_6 0x16
#define TRIGGER_7 0x17
#define TRIGGER_8 0x18
#define TRIGGER_9 0x19
#define TRIGGER_10 0x1A
#define TRIGGER_11 0x1B
#define TRIGGER_12 0x1C
#define TRIGGER_13 0x1D
#define TRIGGER_14 0x1E
#define TRIGGER_15 0x1F

#define HOLD_0 0x20
#define HOLD_1 0x21
#define HOLD_2 0x22
#define HOLD_3 0x23
#define HOLD_4 0x24
#define HOLD_5 0X25
#define HOLD_6 0x26
#define HOLD_7 0x27
#define HOLD_8 0x28
#define HOLD_9 0x29
#define HOLD_10 0x2A
#define HOLD_11 0x2B
#define HOLD_12 0x2C
#define HOLD_13 0x2D
#define HOLD_14 0x2E
#define HOLD_15 0x2F

//Setup module sysModes
#define DEBUG_MODE 0x00
#define REPORT_MODE 0x04
#define RUN_MODE 0x64

//Create Serial communication with the module
HardwareSerial mySerial(2);  // Use UART2 (TX2/RX2)

//Function Declaration
std::vector<byte> constructPackage(const std::vector<byte>& data);
std::vector<byte> enableCommandMode();
std::vector<byte> disableCommandMode();
std::vector<byte> readVersionNumber();
std::vector<byte> restartModule();
std::vector<byte> getConfigParams(const std::vector<byte>& params);
std::vector<byte> setConfigParams(const std::vector<byte>& params, const std::vector<int>& values);
std::vector<byte> sysMode(byte param);
std::vector<byte> getCommandResponse();

void sendCommand(const std::vector<byte>& command);
void printRunModeData();  
void bridgeSerial();

//Setup global variables
std::vector<byte> header = {0xFD, 0xFC, 0xFB, 0xFA};
std::vector<byte> footer = {0x04, 0x03, 0x02, 0x01};

byte sysModeParam = 0x00;

void setup() {
  Serial.begin(112500);       // Debug output
  mySerial.begin(112500, SERIAL_8N1, RX_PORT, TX_PORT);  // HLK-LD2420 baud rate (256000), RX2=16, TX2=17

  pinMode(23, INPUT); // OT2 Sensor Pin, outputs Digital Signal

  Serial.println("HLK-LD2420 Initialized");

  sendCommand(sysMode(RUN_MODE)); // Set the module sysMode

  getCommandResponse(); //Print the response of the command
}

void loop() {
  //Run the sensor
  if (sysModeParam == RUN_MODE) {
    printRunModeData();
  } else {
    bridgeSerial();
  }
}

//Function to bridge the serial communication between the module and the serial monitor
//Checks the running mode and prints formated data in HEX, check https://github.com/JoaoSandrini/LD2420-Radar/blob/main/HLK-LD2420%20Protocol%20(Translated).pdf
void bridgeSerial() {
  std::vector<byte> debugFooter = {0xFD, 0xFC, 0xFB, 0xFA};
  std::vector<byte> reportFooter = {0xF8, 0xF7, 0xF6, 0xF5};
  std::vector<byte> checkFooter;

  if (mySerial.available() > 0) {
    Serial.print(mySerial.read(), HEX);
    Serial.print(" ");

    if (mySerial.peek() == 0xF8) {
      for (int i = 0; i < 4; i++) {
        checkFooter.push_back(mySerial.read());
      }
      if (checkFooter == reportFooter) {
        for (byte b : checkFooter) {
          Serial.print(b, HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
    }

    if (mySerial.peek() == 0xFD) {
      for (int i = 0; i < 4; i++) {
        checkFooter.push_back(mySerial.read());
      }
      if (checkFooter == debugFooter) {
        for (byte b : checkFooter) {
          Serial.print(b, HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
    }
  }
}

//Function to print the data in RUN_MODE, prints ascii
void printRunModeData() {
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}

//Function to construct the package to be sent to the module
std::vector<byte> constructPackage(const std::vector<byte>& data) {
  std::vector<byte> package;

  // Add header
  package.insert(package.end(), header.begin(), header.end());

  // Data Length
  package.push_back(static_cast<byte>(data.size()));
  package.push_back(0x00);

  // Add data
  package.insert(package.end(), data.begin(), data.end());

  // Add footer
  package.insert(package.end(), footer.begin(), footer.end());

  return package;
}

//Function to Construct the command to enable the command mode
std::vector<byte> enableCommandMode() {
  std::vector<byte> data = {0xFF, 0x00, 0x01, 0x00};
  return constructPackage(data);
}

//Function to Construct the command to disable the command mode
std::vector<byte> disableCommandMode() {
  std::vector<byte> data = {0xFE, 0x00};
  return constructPackage(data);
}

//Function to Construct the command to read the version number
std::vector<byte> readVersionNumber() {
  std::vector<byte> data = {0x00, 0x00};
  return constructPackage(data);
}

//Function to Construct the command to restart the module
std::vector<byte> restartModule() {
  std::vector<byte> data = {0x68, 0x00};
  return constructPackage(data);
}

//Function to Construct the command to get the configuration parameters
std::vector<byte> getConfigParams(const std::vector<byte>& params) {
  std::vector<byte> data = {0x08, 0x00};
  
  for (byte b : params) {
    data.push_back(b);  
    data.push_back(0x00);
  }

  return constructPackage(data);
}

//Function to Construct the command to set the configuration parameters
std::vector<byte> setConfigParams(const std::vector<byte>& params, const std::vector<int>& values) {
    std::vector<byte> data = {0x07, 0x00}; // Initialize with specific header bytes

    for (size_t i = 0; i < params.size(); i++) {
        // Add the parameter byte
        data.push_back(params[i]);
        data.push_back(0x00);

        // Add the 4 bytes for the corresponding value (little-endian)
        int value = values[i];
        for (int j = 0; j < 4; j++) {
            byte byteValue = (value >> (8 * j)) & 0xFF; // Little-endian: LSB first
            data.push_back(byteValue);
        }

        // Add padding byte after each value
    }

    return constructPackage(data); // Return the constructed package
}

//Function to Construct the command to set the system mode
std::vector<byte> sysMode(byte param) {
    std::vector<byte> data = {0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    data.insert(data.begin() + 4, param);
    sysModeParam = param;
    return constructPackage(data);
}

//Function to send the command to the module
void sendCommand(const std::vector<byte>& command) {
  mySerial.write(command.data(), command.size());
  Serial.print("Sent: ");
  for (byte b : command) {
    Serial.print(b, HEX);
    Serial.print(" ");
  }
  Serial.println();
}

//Function to get the response of the command
std::vector<byte> getCommandResponse() {
  std::vector<byte> response;

  // Wait for data to be available
  delay(100); // Adjust the delay as needed

  if (mySerial.peek() == 0xFD) {
    while (mySerial.available()) {
      response.push_back(mySerial.read());
      if (response.size() >= 4 && response[response.size() - 4] == 0x04 && response[response.size() - 3] == 0x03 && response[response.size() - 2] == 0x02 && response[response.size() - 1] == 0x01) {
        break;
      }
    }
  }

  if (response.size() > 0) {
    Serial.print("Received: ");
    for (byte b : response) {
      Serial.print(b, HEX);
      Serial.print(" ");
    }
    Serial.println();

    if (response.size() > 7) {
      Serial.print("Message Status: ");
      if (response[7] == 0x01) {
        Serial.println("Success");
      } else {
        Serial.println("Unsuccessful Command");
      }
    } else {
      Serial.println("Error: Response too short");
    }
  } else {
    Serial.println("No response received");
  }

  return response;
}