#include "DCMotor.h"



// DCMotor6612::DCMotor6612(int aLeft_motor_pwm, int aLeft_motor_in1, int aLeft_motor_in2, int aRight_motor_pwm, int aRight_motor_in1, int aRight_motor_in2, Utils &aUtils)
//   : u(aUtils) {
//   u.info(aLeft_motor_pwm);
//   DCMotor6612(aLeft_motor_pwm, aLeft_motor_in1, aLeft_motor_in2, aRight_motor_pwm, aRight_motor_in1, aRight_motor_in2, -1, aUtils);
// }

DCMotor6612::DCMotor6612(int aLeft_motor_pwm, int aLeft_motor_in1, int aLeft_motor_in2, int aRight_motor_pwm, int aRight_motor_in1, int aRight_motor_in2, int aFan_esc_pin, Utils &aUtils)
  : u(aUtils) {
  LEFT_MOTOR_PWM = aLeft_motor_pwm;
  LEFT_MOTOR_IN1 = aLeft_motor_in1;
  LEFT_MOTOR_IN2 = aLeft_motor_in2;
  RIGHT_MOTOR_PWM = aRight_motor_pwm;
  RIGHT_MOTOR_IN1 = aRight_motor_in1;
  RIGHT_MOTOR_IN2 = aRight_motor_in2;
  FAN_ESC_PIN = aFan_esc_pin;
  if (FAN_ESC_PIN > -1)
    fan_esc.setPeriodHertz(SERVO_FREQ);
  pinMode(LEFT_MOTOR_PWM, OUTPUT);
  pinMode(LEFT_MOTOR_IN1, OUTPUT);
  pinMode(LEFT_MOTOR_IN2, OUTPUT);
  pinMode(RIGHT_MOTOR_PWM, OUTPUT);
  pinMode(RIGHT_MOTOR_IN1, OUTPUT);
  pinMode(RIGHT_MOTOR_IN2, OUTPUT);
  if (FAN_ESC_PIN > -1) {
    pinMode(FAN_ESC_PIN, OUTPUT);
  }
}


void DCMotor6612::allowMove(boolean aAllow) {
  move_allowed = aAllow;
}


void DCMotor6612::allowFan(boolean aAllow) {
  fan_allowed = aAllow;
}



DCMotor6612::~DCMotor6612() {
}

void DCMotor6612::armFan() {
  if (FAN_ESC_PIN > -1) {
    u.info_("ARMING FAN...");
    fan_esc.setPeriodHertz(SERVO_FREQ);
    fan_esc.attach(FAN_ESC_PIN);
    fan_esc.writeMicroseconds(500);
    delay(1000);
    fan_esc.writeMicroseconds(2000);
    delay(1000);
    fan_esc.writeMicroseconds(1000);
    u.info("DONE.");
    delay(5000);
  }
}

void DCMotor6612::goFan(int fan_speed) {
  int MIN_FORWARD = 1000;
  int MAX_FORWARD = 2000;
  int f_speed = constrain(fan_speed, 0, 255);
  f_speed = map(f_speed, 0, 255, MIN_FORWARD, MAX_FORWARD);
  fan_esc.writeMicroseconds(f_speed);
}


void DCMotor6612::goMotors(int m1_speed, int m2_speed) {
  if (m1_speed > 0) {
    digitalWrite(LEFT_MOTOR_IN1, HIGH);
    digitalWrite(LEFT_MOTOR_IN2, LOW);
  } else if (m1_speed == 0) {
    digitalWrite(LEFT_MOTOR_IN1, LOW);
    digitalWrite(LEFT_MOTOR_IN2, LOW);
  } else if (m1_speed < 0) {
    digitalWrite(LEFT_MOTOR_IN1, LOW);
    digitalWrite(LEFT_MOTOR_IN2, HIGH);
  }

  if (m2_speed > 0) {
    digitalWrite(RIGHT_MOTOR_IN1, HIGH);
    digitalWrite(RIGHT_MOTOR_IN2, LOW);
  } else if (m2_speed == 0) {
    digitalWrite(RIGHT_MOTOR_IN1, LOW);
    digitalWrite(RIGHT_MOTOR_IN2, LOW);
  } else if (m2_speed < 0) {
    digitalWrite(RIGHT_MOTOR_IN1, LOW);
    digitalWrite(RIGHT_MOTOR_IN2, HIGH);
  }

  m1_speed = constrain(abs(m1_speed), 0, 255);
  m2_speed = constrain(abs(m2_speed), 0, 255);
  m1_speed = map(m1_speed, 1, 255, 11, 255);
  m2_speed = map(m2_speed, 1, 255, 11, 255);
  analogWrite(LEFT_MOTOR_PWM, m1_speed);
  analogWrite(RIGHT_MOTOR_PWM, m2_speed);
}


void DCMotor6612::testFan(int speed_from, int speed_to) {
  int currSpeed = 0;
  for (currSpeed = speed_from; currSpeed <= speed_to; currSpeed += 1) {
    goFan(currSpeed);
    delay(10);
  }
  delay(1000);
  for (currSpeed = speed_to; currSpeed >= speed_from; currSpeed -= 1) {
    goFan(currSpeed);
    delay(10);
  }
  delay(1000);
}

void DCMotor6612::smoothStart(int m_speed) {
  if (!move_allowed) return;
  for (int i = 0; i < m_speed; i++) {
    goMotors(i, i);
    delay(4);
  }
}

void DCMotor6612::stopMotors() {
  if (!move_allowed) return;
  if (current_speed_left > 0 || current_speed_right > 0) {
    int l_speed = current_speed_left;
    int r_speed = current_speed_right;
    while (l_speed > 0 || r_speed > 0) {
      if (l_speed > 0)
        l_speed -= 1;
      if (r_speed > 0)
        r_speed -= 1;
      u.info(String(l_speed) + " .. " + String(r_speed));
      goMotors(l_speed, r_speed);
      delay(1);
    }
  }
  goMotors(0, 0);
}

