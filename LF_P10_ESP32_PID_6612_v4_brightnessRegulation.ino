#include "QTRSensorsMulti.h"
#include "Const.h"
#include <EEPROM.h>
#include <driver/adc.h>
#include "Utils.h"
#include "DCMotor.h"
#include "BluetoothSerial.h"
#include "Params.h"

//#define USE_PIN            // Uncomment this to use PIN during pairing. The pin is specified on the line below
const char *pin = "1234";  // Change this to more secure PIN.

String device_name = "ESP32-V10_DC6612";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

uint16_t SENSOR_BUF[SENSOR_BUF_SIZE];
float ERROR_BUF_D[128];
float ERROR_BUF_I[128];

const bool ALLOW_MOVE = true;  //разрешить движение
const bool ALLOW_FAN = true;   //разрешить турбину
const bool EMULATION = true;   //тестовый сигнал датчика

bool MOVE_BOT = false;
bool PREPARED = false;
bool STARTED = false;
bool WAS_STARTED = false;

bool B1_PRESSED = false;
int LAST_B1_STATE = LOW;
int B1_STATE = 1;

bool B2_PRESSED = false;
int LAST_B2_STATE = LOW;
int B2_STATE = 1;
long lastDebounceTime = 0;

bool TIME_ENDED = false;

int errorBufPosD = -1;
int errorBufPosI = -1;
long lastDebugInfoTime = 0;
long lastCycleTime = 0;
long cycleDuration = 0;
long gStartTime = 0;

QTRSensors sensor;
BluetoothSerial SerialBT;
Utils u = Utils(Serial, SerialBT);
Params p = Params(SENSOR_BUF_SIZE, u, sensor);
DCMotor6612 motors = DCMotor6612(LEFT_MOTOR_PWM, LEFT_MOTOR_IN1, LEFT_MOTOR_IN2, RIGHT_MOTOR_PWM, RIGHT_MOTOR_IN1, RIGHT_MOTOR_IN2, TOURBINE_PIN, u);

int leftSpeed = 0;
int rightSpeed = 0;
float emulIinitial = 0;
float emulIncrement = 3.14159 / 300;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  String filePath = __FILE__;
  String fileName = filePath.substring(filePath.lastIndexOf("\\") + 1);
  pinMode(SENSOR_IN, INPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);


  EEPROM.begin(EEPROM_SIZE);
  Serial.setTimeout(200);
  Serial.begin(115200);
  Serial.println("STARTED");
  SerialBT.setTimeout(200);
  SerialBT.begin(device_name);
  // #ifdef USE_PIN
  //   SerialBT.setPin(pin);
  //   Serial.println("Using PIN");
  // #endif
  u.setWorkMode(WorkMode::Both);

  memset(ERROR_BUF_D, 0, sizeof(ERROR_BUF_D));
  memset(ERROR_BUF_I, 0, sizeof(ERROR_BUF_I));

  u.info("");
  u.info("---------------------------");
  u.info(fileName);
  u.info("----------START------------");
  u.info("");

  p.loadParams(1);

  // u.setDebugMode(p.P.DEBUG_MODE);
  // u.setPrintMode(PrintMode(p.P.PRINT_MODE));

  motors.allowMove(ALLOW_MOVE);
  motors.allowFan(ALLOW_FAN);
  if (p.P.USE_FAN) {
    motors.armFan();
  } else {
    u.info(S_ESC_NO_ARMING);
  }
  //!! при setSamplesPerSensor(2) весь цикл программы занимает 1.2 миллисекунды;
  sensor.setTypeAnalogMX();
  sensor.setSensorPins((const uint8_t[]){ SENSOR_IN, SENSOR_S0, SENSOR_S1, SENSOR_S2, SENSOR_S3 }, SENSOR_BUF_SIZE);
  sensor.setSamplesPerSensor(p.P.SAMPLES_PER_SENSOR);
  sensor.setBlackLevel(p.P.SENSOR_BLACK_LEVEL);
  //sensor.setSmoothing(QTRSmoothing(p.P.SENSOR_SMOOTHING));
  p.loadCalibration();
  u.info("SETUP DONE");
  p.displayCalibration();
  p.displayParams();
  displayHelp();
  delay(500);
}


float bot_position() {
  int pos1 = (sensor.readLineBlack(SENSOR_BUF) - 7000);
  float pos2 = pos1 / 1000.00;
  if (EMULATION) {
    emulIinitial = emulIinitial + emulIncrement;
    pos2 = constrain(sin(emulIinitial) * 7, -7, 7);
    pos2 = 0;
  }
  return pos2;
}

void loop() {
  //Serial.println("1");
  readButtons();
  if (!MOVE_BOT) {
    digitalWrite(LED_BUILTIN, LOW);
    motors.goFan(0);
    motors.stopMotors();
    STARTED = false;
    PREPARED = false;
    processMenu();
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    if (!STARTED && ((u.available() > 0) || B2_PRESSED || TIME_ENDED)) {
      int ch = u.read();
      B1_PRESSED = false;
      TIME_ENDED = false;
      if (!PREPARED && (ch == 120 || B2_PRESSED)) {
        ch = u.read();
        motors.goFan(p.P.FAN_SPEED_IDLE);
        PREPARED = true;
        B1_PRESSED = false;
        B2_PRESSED = false;
        u.info(S_BOT_PREPARED);
      } else if (PREPARED && (ch == 10 || B2_PRESSED)) {
        STARTED = true;
        B1_PRESSED = false;
        B2_PRESSED = false;
        u.info(S_BOT_STARTED);
        gStartTime = millis();
        p.setStartRunTime(gStartTime);
      }
      // else {
      //   STARTED = false;
      //   WAS_STARTED = false;
      //   PREPARED = false;
      //   MOVE_BOT = false;
      //   motors.goFan(0);
      //   motors.stopMotors();
      //   B1_PRESSED = false;
      //   B2_PRESSED = false;
      //   digitalWrite(LED_BUILTIN, LOW);
      //   u.info(S_ALL_STOPPED);
      //   u.info("Last cycle time: " + String(cycleDuration));
      // }
    }
    if (PREPARED && !STARTED) {
      doMove(false);
      cycleDuration = micros() - lastCycleTime;
      lastCycleTime = micros();
    }
    if (STARTED) {
      static byte prevBytesAm = 0;
      static uint32_t tmr = 0;
      byte bytesAm = u.available();
      if (bytesAm != prevBytesAm) {
        prevBytesAm = bytesAm;
        tmr = millis();
      }
      if ((bytesAm & millis() - tmr > 10) || bytesAm > 28) {
        char ch = u.read();
        // u.info("////////////////////////////////////////////////////////////////////////////////////////////////");
        // u.info(ch);
        // u.info("////////////////////////////////////////////////////////////////////////////////////////////////");
        processKoefsAdj(ch);
        //char ent = u.read();
        serialFlush();
      }
      //digitalWrite(LED_BUILTIN, HIGH);
      if (!WAS_STARTED) {
        //p.loadParams(p.presetNo);  //restore base speed etc. from EEPROM
        if (u.getPrintMode() == PrintMode::Graph) {
          //plot("ERROR; VAL_D; DIFF_D; VAL_I; SUM_I; TOTAL_CORR" );
          //plot("ERROR, VAL_D, DIFF_D, TIME_D, VAL_I, SUM_I, TIME_I, TOTAL_CORR, LEFT_S, RIGHT_S");
          u.plot("ERROR, VAL_P, VAL_D, DIFF_D, VAL_I, SUM_I, TOTAL_CORR, RES_SPEED");
        }
        WAS_STARTED = true;
      }
      doMove(true);
      if ((p.P.TIME_TO_STOP > 0) && (millis() - gStartTime > p.P.TIME_TO_STOP)) {
        TIME_ENDED = true;
      }
      cycleDuration = micros() - lastCycleTime;
      lastCycleTime = micros();
    }
  }
}

void doMove(bool allowMove) {
  float error = bot_position();
  motors.goFan(p.P.FAN_SPEED_MAX);
  SpeedMapEntry e = p.getSpeedMapEntry();
  doPIDMove(error, allowMove, e.speedMode, e.kP, e.kD, e.ageD, e.kI, e.ageI, e.runSpeed, e.turnSpeed, e.brakeSpeed);
}

void doPIDMove(float error, bool allowMove, char speedMode, float kP, float kD, int ageD, float kI, int ageI, int runSpeed, int turnSpeed, int brakeSpeed) {
  //Process P
  float P = kP * error;

  //Process D
  errorBufPosD = (errorBufPosD + 1) % ageD;  //позиция для сохранения текущего значения отклонения в массив (для D)
  ERROR_BUF_D[errorBufPosD] = error;
  float diffValue = (ERROR_BUF_D[errorBufPosD] - ERROR_BUF_D[(errorBufPosD + ageD + 1) % ageD]);
  float D = diffValue * kD;

  //Process I
  errorBufPosI = (errorBufPosI + 1) % ageI;  //позиция для сохранения текущего значения отклонения в массив (для D)
  ERROR_BUF_I[errorBufPosI] = error;

  int integralValue = 0;

  for (int i = 0; i < ageI; i++) {
    integralValue += ERROR_BUF_I[i];
  }

  float I = integralValue / ageI * kI;
  float correction = P + I + D;
  // int generalSpeed;

  // switch (speedMode) {
  //   case p.SPEEDMODE_STOP: generalSpeed = 0; break;
  //   case p.SPEEDMODE_ZERO: generalSpeed = 0; break;
  //   case p.SPEEDMODE_LOW: generalSpeed = p.P.LOW_SPEED; break;
  //   case p.SPEEDMODE_NORMAL: generalSpeed = p.P.NORMAL_SPEED; break;
  //   case p.SPEEDMODE_FAST: generalSpeed = p.P.FAST_SPEED; break;
  //   case p.SPEEDMODE_TOP: generalSpeed = p.P.TOP_SPEED; break;
  //   default: generalSpeed = p.P.NORMAL_SPEED;
  // }

  //if (p.P.NORMAL_SPEED == 0) generalSpeed = 0;  //зачем?

  leftSpeed = runSpeed - correction * 100;
  rightSpeed = runSpeed + correction * 100;

  //притормаживаем на резких поворотах
  int turnSpeedCorrection = runSpeed == 0 ? 0 : abs(error * p.P.TURN_SPEED_CORRECTION);
  leftSpeed = leftSpeed - turnSpeedCorrection;
  rightSpeed = rightSpeed - turnSpeedCorrection;

  if (!allowMove) {
    leftSpeed = 0;
    rightSpeed = 0;
  }

  // int limitSpeed;
  // switch (speedMode) {
  //   case p.SPEEDMODE_LOW: limitSpeed = p.P.NORMAL_SPEED; break;
  //   case p.SPEEDMODE_NORMAL: limitSpeed = p.P.TURN_SPEED; break;
  //   case p.SPEEDMODE_FAST: limitSpeed = p.P.TURN_SPEED; break;
  //   case p.SPEEDMODE_TOP: limitSpeed = MAX_MOTOR_SPEED; break;
  //   default: limitSpeed = p.P.TURN_SPEED;
  // }

  leftSpeed = constrain(leftSpeed, brakeSpeed, turnSpeed);
  rightSpeed = constrain(rightSpeed, brakeSpeed, turnSpeed);
  //корректируем если моторы по разному тянут
  leftSpeed = leftSpeed * CORR_LEFT_MOTOR;

  if (millis() - gStartTime < p.P.RAMPUP_TIME) {
    //типа плавный старт
    float rampupRate = (millis() - gStartTime) * 1.0 / p.P.RAMPUP_TIME * 1.0;
    leftSpeed = rampupRate * leftSpeed;
    rightSpeed = rampupRate * rightSpeed;
  }

  //DEBUGGING
  if (u.isTimeToPrintDebug()) {
    float timeForD = ageD * cycleDuration / 1000.0;
    float timeForI = ageI * cycleDuration / 1000.0;
    String debugStr;
    if (u.isGraphDebug() && STARTED)
      debugStr = u.float2str(error) + ',' + u.float2str(P) + ',' + u.float2str(D) + ',' + u.float2str(diffValue) + ',' + u.float2str(I) + ',' + u.float2str(integralValue / 10) + ',' + u.float2str(correction) + ',' + String((leftSpeed - rightSpeed) / 10);
    else
      debugStr = "ERR=" + String(error) + ", D=" + String(diffValue) + ":" + String(timeForD) + "ms, I=" + String(integralValue) + ":"
                 + String(timeForI) + "ms, CORR=" + String(correction) + ", LS=" + String(leftSpeed) + " RS=" + String(rightSpeed)
                 + " CYCLE=" + String(cycleDuration / 1000.0) + "ms, mode:" + speedMode;
    u.debug(debugStr);
    //char speedMode, float kP, float kD, int ageD, float kI, int ageI
    //3000:z,3000:l:0.11,3000:n:0.12:0.21,3000:f:0.13:0.22:10,3000:t:0.14:0.23:20:0.06

    String debugStr2;
    debugStr = String(millis() - gStartTime) + " mode:" + speedMode;
    debugStr = debugStr + " " + p.S.kP + ":" + String(kP) + " " + p.S.kD + ":" + String(kD) + " " + p.S.D_AGE + ":" + String(ageD);
    debugStr = debugStr + " " + p.S.kI + ":" + String(kI) + " " + p.S.I_AGE + ":" + String(ageI);
    debugStr = debugStr + " runSpeed:" + String(runSpeed) + " turnSpeed:" + String(turnSpeed) + " brakeSpeed:" + String(brakeSpeed);

    u.debug(debugStr);
  }
  motors.goMotors(leftSpeed, rightSpeed);
}
