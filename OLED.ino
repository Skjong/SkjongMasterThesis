#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

extern float RPM_L;
extern float RPM_R;
extern float autoPWM;
extern bool isCoasting;
extern bool isBraking;
extern float pitch;
extern float load1;
extern float load2;

extern bool bodyDetected;
extern bool handDetected;
extern bool userDetected;
extern bool fallDetected;
unsigned long fallDisplayTime = 0;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Battery voltage input
#define BATTERY_PIN 34
const float voltageMultiplier = 8.58; //91k/12
const float MAX_BATTERY_VOLTAGE = 27.4;
const float MIN_BATTERY_VOLTAGE = 26.0;
float batteryPercent = 0.0;

void updateBattery() {
  int adc = analogRead(BATTERY_PIN);
  float vPin = (adc * 3.3) / 4095.0;
  float batteryVoltage = vPin * voltageMultiplier;
  batteryPercent = ((batteryVoltage - MIN_BATTERY_VOLTAGE) /
                    (MAX_BATTERY_VOLTAGE - MIN_BATTERY_VOLTAGE)) * 100.0;
  batteryPercent = constrain(batteryPercent, 0, 100);
}

void setupOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Init OK");
  display.display();
  delay(1000);
}

/*
void updateOLED() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Show current PWM value - use lastBrakePWM when braking
    display.setCursor(0, 0);
    display.print("PWM: ");
  display.println(isBraking ? lastBrakePWM : autoPWM, 1);

    // Debug info
    display.setCursor(0, 16);
    display.print("Pitch: ");
    display.print(pitch, 1);
    display.print("° RPM: ");
    display.print(avgRPM, 1);
    
    display.setCursor(0, 32);
    display.print("Accel: ");
    display.print(rpmAccel, 1);
    display.print(" Dir: ");
    display.print(digitalRead(MOTOR_DIR_L) ? "FWD" : "REV");

    // === User detection status (top right) ===
display.setCursor(0, 24);
if (bodyDetected && handDetected) {
  display.print("Active");
} else if (bodyDetected) {
  display.print("Body");
} else if (handDetected) {
  display.print("Hands");
} else {
  display.print("No user");
}

display.setCursor(70, 24);
display.print("LC: ");
display.print((int)load1);
display.print("g ");
display.print((int)load2);
display.print("g");
    
    display.display();
}
*/
void updateOLED() {
  static unsigned long lastOLED = 0;
  if (millis() - lastOLED < 100) return;
  lastOLED = millis();

  updateBattery();
  display.clearDisplay();

  // --- Handle fall display and timeout ---
  if (fallDetected) {
    display.setTextSize(2);
    display.setCursor(20, 30);
    display.print("FALL");
    display.setCursor(20, 48);
    display.print("DETECTED");
    display.display();

    if (fallDisplayTime == 0) fallDisplayTime = millis();  // Start timer
    else if (millis() - fallDisplayTime > 3000) {          // Show for 3 sec
      fallDetected = false;
      fallDisplayTime = 0;
    }
    return;
  }

  // === Speed and assist ===
  const float wheelDiameter = 0.165f; // meters
  const float wheelCirc = PI * wheelDiameter;
  float avgRPM = (RPM_L + RPM_R) * 0.5f;
  float speed_kmh = (avgRPM * wheelCirc / 60.0f) * 3.6f;
  int assistPercent = (int)((autoPWM / 50.0f) * 100.0f);

  display.setCursor(0, 0);
  display.print("Speed: ");
  display.print(speed_kmh, 1);
  display.print(" km/h");

  display.setCursor(0, 12);
  display.print("Assist: ");
  display.print(assistPercent);
  display.print("%");

  if (isBraking) {
  display.setCursor(80, 12);
  display.print("BRAKE");
} else if (isCoasting) {
  display.setCursor(80, 12);
  display.print("COAST");
}

// === User detection status (top right) ===
display.setCursor(0, 24);
if (bodyDetected && handDetected) {
  display.print("Active");
} else if (bodyDetected) {
  display.print("Body");
} else if (handDetected) {
  display.print("Hands");
} else {
  display.print("No user");
}

display.setCursor(70, 24);
display.print("LC: ");
display.print((int)load1);
display.print("g ");
display.print((int)load2);
display.print("g");


  // === Battery bar (bottom left) ===
  int batteryX = 10;
  int batteryY = 48;
  int batteryW = 50;
  int batteryH = 12;
  int batteryFill = map(batteryPercent, 0, 100, 0, batteryW - 4);

  display.drawRect(batteryX, batteryY, batteryW, batteryH, SSD1306_WHITE);
  display.drawRect(batteryX + batteryW, batteryY + 3, 3, 6, SSD1306_WHITE);
  display.fillRect(batteryX + 2, batteryY + 2, batteryFill, batteryH - 4, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(batteryX + 14, batteryY - 10);
  display.print((int)batteryPercent);
  display.print("%");

  // === Slope (bottom right) ===
  int slopeX = 74;
  int slopeY = 48;
  char slopeArrow = (pitch > 5) ? (char)0x18 : (pitch < -5) ? (char)0x19 : (char)0x1A;

  display.setTextSize(2);
  display.setCursor(slopeX, slopeY);
  display.print(slopeArrow);
  display.setTextSize(1);
  display.setCursor(slopeX + 18, slopeY + 5);
  display.print((int)pitch);
  display.print((char)247); // degree

  display.display();
}
