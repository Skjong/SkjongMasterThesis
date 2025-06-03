#ifndef USERINPUT_H
#define USERINPUT_H

#include <Wire.h>
#include <MPU6050.h>
#include "HX711.h"

// --- External RPM variables ---
extern float RPM_L;
extern float RPM_R;

// --- Sensor state ---
int16_t accelX = 0, accelY = 0, accelZ = 0;
float pitch = 0;

float load1 = 0;
float load2 = 0;
float prevLoad1 = 0;
float prevLoad2 = 0; 

bool bodyDetected = false;
bool handDetected = false;
bool userDetected = false;

// --- Timing ---
float lastUpdateTimeUserInput = 0;
unsigned long lastUserDetectedTime = 0;
unsigned long lastBodyTime = 0;
unsigned long lastHandsTime = 0;
const unsigned long maxSensorSilence = 5000; // 5 seconds grace

const unsigned long gracePeriodMs = 600;  //Experiment as low as is still smooth

// --- IMU and load cells ---
MPU6050 imu(0x68);
#define DT1 35
#define SCK1 16
#define DT2 39
#define SCK2 17
HX711 scale1;
HX711 scale2;

// --- Ultrasonic sensor ---
#define TRIG_PIN 15
#define ECHO_PIN 36
#define DISTANCE_THRESHOLD_CM 80.0
#define LOAD_THRESHOLD_G 5.0

// --- Setup ---
void setupUserInput() {
    imu.initialize();
    if (!imu.testConnection()) {
        Serial.println("Failed to connect to MPU6050.");
        while (1);
    }
    Serial.println("IMU initialized.");

    scale1.begin(DT1, SCK1);
    scale2.begin(DT2, SCK2);
    scale1.set_scale(1650); scale1.tare();
    scale2.set_scale(1650); scale2.tare();
    Serial.println("Load cells initialized.");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    Serial.println("Ultrasonic sensor initialized.");
}


// --- Main update loop ---
void updateUserInput() {
    // Get sensor readings
    imu.getAcceleration(&accelX, &accelY, &accelZ);

    load1 = scale1.get_units(1);
    load2 = scale2.get_units(1);
    float delta1 = abs(load1 - prevLoad1);
    float delta2 = abs(load2 - prevLoad2);
    prevLoad1 = load1;
    prevLoad2 = load2;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    float distance_cm = (duration > 0 && duration < 30000) ? duration * 0.034 / 2.0 : 9999;

    // IMU Slope Calculation
    float rawPitch = atan2(accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI - 12;
    static float filteredPitch = 0.0;
    float alpha = 0.1;
    filteredPitch = alpha * rawPitch + (1 - alpha) * filteredPitch;
    pitch = filteredPitch;

    // Sensor conditions
    bodyDetected = (distance_cm < DISTANCE_THRESHOLD_CM);
    handDetected = (delta1 > 0.3f || delta2 > 0.3f);

    // --- New: Cross-verification timers ---
    static unsigned long lastBodyTime = 0;
    static unsigned long lastHandsTime = 0;
    const unsigned long maxSensorSilence = 30000;  // 5s timeout

    if (bodyDetected) lastBodyTime = millis();
    if (handDetected) lastHandsTime = millis();

    bool bodyRecentlySeen = (millis() - lastBodyTime <= maxSensorSilence);
    bool handsRecentlySeen = (millis() - lastHandsTime <= maxSensorSilence);

    // Main user detection
    if (bodyDetected || handDetected) {
        lastUserDetectedTime = millis();
    }

    bool recentlyActive = (millis() - lastUserDetectedTime <= gracePeriodMs);
    bool bothSensorsHealthy = bodyRecentlySeen && handsRecentlySeen;

    userDetected = recentlyActive && bothSensorsHealthy;

    // Time delta
    unsigned long now = millis();
    float deltaTime = (now - lastUpdateTimeUserInput) / 1000.0;
    lastUpdateTimeUserInput = now;
}

#endif
