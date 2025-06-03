#ifndef FEEDBACK_H
#define FEEDBACK_H

// --- Pin Definitions ---
#define HALL_1_L 13
#define HALL_2_L 25
#define HALL_3_L 14

#define HALL1_R 5
#define HALL2_R 4
#define HALL3_R 21

// --- Constants ---
const int statesPerRotation = 90;
const int hallStates[6] = {5, 1, 3, 2, 6, 4};  // Valid Hall state sequence
const float alpha = 0.3;  // Smoothing factor

// --- RPM outputs ---
float RPM_L = 0.0;
float RPM_R = 0.0;

// --- Internal tracking (Left) ---
volatile int lastDir_L = 1;
volatile unsigned long lastDelta_L = 100;
volatile bool updateFlag_L = false;
int lastHallState_L = -1;
int prevIndex_L = -1;

// --- Internal tracking (Right) ---
volatile int lastDir_R = 1;
volatile unsigned long lastDelta_R = 100;
volatile bool updateFlag_R = false;
int lastHallState_R = -1;
int prevIndex_R = -1;

// --- Hall decoding ---
int getHallIndex(int state) {
  for (int i = 0; i < 6; i++) {
    if (hallStates[i] == state) return i;
  }
  return -1;
}

// --- ISR: Left wheel ---
void IRAM_ATTR onHallChange_L() {
  int s1 = digitalRead(HALL_1_L);
  int s2 = digitalRead(HALL_2_L);
  int s3 = digitalRead(HALL_3_L);
  int hallState = (s1 << 2) | (s2 << 1) | s3;

  int idx = getHallIndex(hallState);
  if (idx != -1 && idx != prevIndex_L) {
    static unsigned long lastTime_L = 0;
    unsigned long now = millis();
    unsigned long dt = now - lastTime_L;
    lastTime_L = now;

    int delta = (idx - prevIndex_L + 6) % 6;
    int dir = (delta == 1 || delta == 2) ? 1 : -1;

    if (dt > 0 && dt < 200) {
      lastDelta_L = dt;
      lastDir_L = dir;
      updateFlag_L = true;
    }

    prevIndex_L = idx;
  }
}

// --- ISR: Right wheel ---
void IRAM_ATTR onHallChange_R() {
  int s1 = digitalRead(HALL1_R);
  int s2 = digitalRead(HALL2_R);
  int s3 = digitalRead(HALL3_R);
  int hallState = (s1 << 2) | (s2 << 1) | s3;

  int idx = getHallIndex(hallState);
  if (idx != -1 && idx != prevIndex_R) {
    static unsigned long lastTime_R = 0;
    unsigned long now = millis();
    unsigned long dt = now - lastTime_R;
    lastTime_R = now;

    int delta = (idx - prevIndex_R + 6) % 6;
    int dir = (delta == 1 || delta == 2) ? 1 : -1;

    if (dt > 0 && dt < 200) {
      lastDelta_R = dt;
      lastDir_R = dir;
      updateFlag_R = true;
    }

    prevIndex_R = idx;
  }
}

// --- Setup ---
void setupFeedback() {
  pinMode(HALL_1_L, INPUT_PULLUP);
  pinMode(HALL_2_L, INPUT_PULLUP);
  pinMode(HALL_3_L, INPUT_PULLUP);
  pinMode(HALL1_R, INPUT_PULLUP);
  pinMode(HALL2_R, INPUT_PULLUP);
  pinMode(HALL3_R, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(HALL_1_L), onHallChange_L, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HALL_2_L), onHallChange_L, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HALL_3_L), onHallChange_L, CHANGE);

  attachInterrupt(digitalPinToInterrupt(HALL1_R), onHallChange_R, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HALL2_R), onHallChange_R, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HALL3_R), onHallChange_R, CHANGE);
}

// --- Feedback update (called in loop) ---
void readFeedback() {
  static unsigned long lastUpdate_L = 0;
  static unsigned long lastUpdate_R = 0;
  unsigned long now = millis();

  if (updateFlag_L) {
    float instantRPM = lastDir_L * (1000.0 / lastDelta_L) * (60.0 / statesPerRotation);
    RPM_L = alpha * instantRPM + (1.0 - alpha) * RPM_L;
    updateFlag_L = false;
    lastUpdate_L = now;
  } else if (now - lastUpdate_L > 300) {
    RPM_L = 0.0;
  }

  if (updateFlag_R) {
    float instantRPM = lastDir_R * (1000.0 / lastDelta_R) * (60.0 / statesPerRotation);
    instantRPM = -instantRPM;  //Flip speed of mirrored wheel
    RPM_R = alpha * instantRPM + (1.0 - alpha) * RPM_R;
    updateFlag_R = false;
    lastUpdate_R = now;
  } else if (now - lastUpdate_R > 300) {
    RPM_R = 0.0;
  }


}

#endif
