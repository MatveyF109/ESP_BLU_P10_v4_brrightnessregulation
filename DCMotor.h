#pragma once
#include "Arduino.h"
#include "ESP32Servo.h"
#include "Utils.h"

class DCMotor6612 {
public:
  DCMotor6612(int aLeft_motor_pwm, int aLeft_motor_in1, int aLeft_motor_in2, int aRight_motor_pwm, int aRight_motor_in1, int aRight_motor_in2, int aFan_esc_pin, Utils &aUtils);
  ~DCMotor6612();

  void armFan();
  void goMotors(int m_left_speed, int m_right_speed);
  void stopMotors();
  void smoothStart(int m_speed);
  void allowMove(boolean aAllow);
  void allowFan(boolean aAllow);

  void goFan(int fan_speed);
  void testFan(int speed_from, int speed_to);


private:
  int LEFT_MOTOR_IN1 = -1;
  int LEFT_MOTOR_IN2 = -1;
  int LEFT_MOTOR_PWM = -1;
  int RIGHT_MOTOR_IN1 = -1;
  int RIGHT_MOTOR_IN2 = -1;
  int RIGHT_MOTOR_PWM = -1;
  int SERVO_FREQ = 490;
  int FAN_ESC_PIN = -1;

  int current_speed_left = 0;
  int current_speed_right = 0;
  int current_speed_fan = 0;
  boolean move_allowed = true;
  boolean fan_allowed = true;

  Utils &u;
  Servo fan_esc;

};
