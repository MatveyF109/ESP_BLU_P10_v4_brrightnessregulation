#pragma once
//static params
const int LED_BUILTIN = GPIO_NUM_2;
//SENSOR
const uint8_t SENSOR_BUF_SIZE = 15;

const int MAX_MOTOR_SPEED = 255;

const int SENSOR_IN = 25;
const int SENSOR_S0 = 0;
const int SENSOR_S1 = 4;
const int SENSOR_S2 = 12;
const int SENSOR_S3 = 32;

//MOTORS
const int LEFT_MOTOR_PWM = GPIO_NUM_14;
const int LEFT_MOTOR_IN1 = GPIO_NUM_18;
const int LEFT_MOTOR_IN2 = GPIO_NUM_26;

const int RIGHT_MOTOR_PWM = GPIO_NUM_19;
const int RIGHT_MOTOR_IN1 = GPIO_NUM_5;
const int RIGHT_MOTOR_IN2 = GPIO_NUM_23;
const float CORR_LEFT_MOTOR = 1.0;

const int TOURBINE_PIN = GPIO_NUM_33;

//BUTTONS
//B1- calibration
//B2 - start stop


const int BUTTON1 = GPIO_NUM_27;

const int BUTTON2 = GPIO_NUM_21;

//OTHER
const int EEPROM_SIZE = 2000;

//string messages
const String S_MENU_TEXT = "(h)elp, (d)ebug on|off, (g)raph debug on|off, (l)ist params, preset (n)umber, (e)rase params, (c)alibration, test (q)tr sensors\n(w)ash wheels, (t)est fan, (f)an on|off, (a)ll params, (s)peed, (p) and d, (k) all PID params, speed (m)ap";
const String S_ENTER_CHOICE = "YOUR CHOICE?";
const String S_FAN_NOT_ALLOWED = "FAN IS NOT ALLOWED";
const String S_FAN_TEST = "FAN TEST...";
const String S_ENTER_ALL_PARAMS = "ENTER ALL PARAMS";
const String S_ENTER_NORMAL_SPEED = "ENTER NORMAL SPEED";
const String S_ENTER_SAMPLES_PER_SENSOR = "ENTER SAMPLES PER SENSOR";
const String S_ENTER_P_D = "ENTER P and D";
const String S_ENTER_P_D_I = "ENTER P, D and I";
const String S_ENTER_D_D_AGE = "ENTER D and D age";
const String S_ENTER_SPEEDMAP = "ENTER SPEEDMAP (max length 128 symbols), ENTER TO LEAVE AS IS";
const String S_ENTER_SPEEDMAP2 = "Format: 'time:mode:kP:kD:ageD:kI:ageI:turnSpeed:brakeSpeed', mode=(z)ero, (l)ow, (n)ormal, (f)ast, (t)op";
const String S_ENTER_SPEEDMAP3 = "Example: 1000:n,2000:f:0.22,3000:l:0.22:0.33:16";



const String S_ESC_ARMING = "ESC ARMING...";
const String S_ESC_NO_ARMING = "FAN WAS OFF, ESC NOT ARMED";
const String S_RESTART_BOT = "NEED TO RESTART NOW";
const String S_BOT_PREPARED = "BOT PREPARED";
const String S_BOT_STARTED = "BOT STARTED";
const String S_ALL_STOPPED = "ALL STOPPED";
const String S_LOADING_SAVED_PARAMS = "LOADING PARAMS...";
const String S_NO_SAVED_PARAMS = "NO SAVED PARAMS FOUND";
const String S_SAVING_PARAMS = "SAVING...";
const String S_ERASING_PARAMS = "ERASING...";
const String S_WASHING = "WASHING...";
const String S_TESTSENSORS = "TESTING SENSORS...";
const String S_CALIBRATION = "CALIBRATION:";
const String S_SAVING_CALIBRATION = "SAVING CALIBRATION...";
const String S_LOADING_CALIBRATION = "LOADING CALIBRATION:";
const String S_ENTER_SENSOR_BRIGHTNESS = "ENTER SENSOR BRIGHTNEESS";

const String S_DONE = "DONE.";
const String S_ENTER_TO_LEAVE = "ENTER TO LEAVE ";
const String S_X_TO_PREPARE = "'X' TO PREPARE";
const String S_NEED_RESTART = " RESTART BOARD!";
const String S_SET_TO = " is set to ";
const String S_IS = " is ";
const String S_ENTER = "ENTER ";
const String S_ERROR = "ERROR ";
