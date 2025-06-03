#include <Arduino.h>
#include "feedback.ino"
#include "pid.ino"
#include "userinput.ino"
#include "logger.ino"
#include "RPMtoPWM.ino"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Booting...");

    Wire.begin(23, 19);
    setupFeedback();
    setupPWMControl();   // Initialize Hall sensors
    setupPID();        // Initialize both motors
    setupUserInput();
    setupOLED();  
    setupLogger();
}

void loop() {
    readFeedback();  
    assistRampLogic();
    //testBraking();
    updateUserInput();
    //updatePID();
    updateOLED();
    updateBattery();
    updateLogger();
}




