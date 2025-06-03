#ifndef PID_H
#define PID_H

// Motor 1 (Left)
#define MOTOR_PWM_L 33
#define MOTOR_DIR_L 27 

// Motor 2 (Right)
#define MOTOR_PWM_R 32
#define MOTOR_DIR_R 22

// Shared PID constants
float Kp = 0.85;
float Ki = 0.06;
float Kd = 0.0012;

// Motor 1 variables
float error_L = 0.0, previousError_L = 0.0;
float integral_L = 0.0;
float derivative_L = 0.0;
float outputPWM_L = 0.0;
static float holdPWM_L = 0;

// Motor 2 variables
float error_R = 0.0, previousError_R = 0.0;
float integral_R = 0.0;
float derivative_R = 0.0;
float outputPWM_R = 0.0;
static float holdPWM_R = 0;

// Setpoints
float setpointRPM_L = 0;
float setpointRPM_R = 0;

void setupPID() {
    pinMode(MOTOR_PWM_L, OUTPUT);
    pinMode(MOTOR_DIR_L, OUTPUT);
    pinMode(MOTOR_PWM_R, OUTPUT);
    pinMode(MOTOR_DIR_R, OUTPUT);
}

void updatePID() {
    // === LEFT MOTOR ===
    float error_L = setpointRPM_L - RPM_L;
    integral_L += error_L * 0.25;
    derivative_L = (error_L - previousError_L) / 0.25;
    previousError_L = error_L;

    outputPWM_L = (Kp * error_L) + (Ki * integral_L) + (Kd * derivative_L);
    outputPWM_L = constrain(abs(outputPWM_L), 0, 80);

    if (abs(RPM_L) > 0.3f) {
        holdPWM_L = outputPWM_L;
    } else {
        holdPWM_L *= 0.8f;
        if (holdPWM_L < 2.0f) holdPWM_L = 0.0f;
    }

    outputPWM_L = holdPWM_L;
    digitalWrite(MOTOR_DIR_L, setpointRPM_L >= 0 ? LOW : HIGH);
    analogWrite(MOTOR_PWM_L, (int)outputPWM_L);

    // === RIGHT MOTOR ===
    float error_R = setpointRPM_R - RPM_R;
    integral_R += error_R * 0.25;
    derivative_R = (error_R - previousError_R) / 0.25;
    previousError_R = error_R;

    outputPWM_R = (Kp * error_R) + (Ki * integral_R) + (Kd * derivative_R);
    outputPWM_R = constrain(abs(outputPWM_R), 0, 80);

    if (abs(RPM_R) > 0.3f) {
        holdPWM_R = outputPWM_R;
    } else {
        holdPWM_R *= 0.8f;
        if (holdPWM_R < 2.0f) holdPWM_R = 0.0f;
    }

    outputPWM_R = holdPWM_R;
    digitalWrite(MOTOR_DIR_R, setpointRPM_R >= 0 ? HIGH : LOW); // flipped for right
    analogWrite(MOTOR_PWM_R, (int)outputPWM_R);
}

#endif
