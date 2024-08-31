#include "Utils.h"
#include "Params.h"
//type==0 all params
//type==1 base speed only
//type==2 kP and kD
//type==3 D and D age

// enum  {
const byte PARAMS_ALL = 0;
const byte PARAMS_SPEED = 1;
const byte PARAMS_KP_KD = 2;
const byte PARAMS_KP_KD_KI = 3;
const byte PARAMS_SPEEDMAP = 4;
const byte PARAMS_BRIGHTNESS = 5;
// } EnterMode;

// const byte ENTER_PARAMS_ALL = 0;
// const byte ENTER_PARAMS_BASE_SPEED = 1;
// const byte ENTER_PARAMS_KP_KD = 2;
// const byte ENTER_PARAMS_D_AGE = 3;
// const byte ENTER_PARAMS_KP_KD_KI = 4;
// const byte ENTER_PARAMS_SPEEDMAP = 5;

void readButtons() {
  int reading = digitalRead(BUTTON1);

  if (reading != LAST_B1_STATE)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > 10) {
    if (reading != B1_STATE) {
      B1_STATE = reading;
      if (B1_STATE == HIGH)
        B1_PRESSED = !B1_PRESSED;
    }
  }
  LAST_B1_STATE = reading;

  reading = digitalRead(BUTTON2);
  if (reading != LAST_B2_STATE)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > 10) {
    if (reading != B2_STATE) {
      B2_STATE = reading;
      if (B2_STATE == HIGH)
        B2_PRESSED = !B2_PRESSED;
    }
  }
  LAST_B2_STATE = reading;
}


char menu() {
  u.info("");
  u.info(S_ENTER_CHOICE);
  char res = '0';
  while (res == '0') {
    readButtons();
    if (B1_PRESSED) {
      res = 'c';
    } else if (B2_PRESSED) {
      res = 'x';
    }
    if (res != '0') {
      B1_PRESSED = false;
      B2_PRESSED = false;
    }

    switch (u.read()) {

      case 'h': res = 'h'; break;  //help
      case 'd': res = 'd'; break;  //set debug info (on/off)
      case 'g': res = 'g'; break;  //log for graphics (graph - text - csv buffer)

      case 'l': res = 'l'; break;  //list params
      case 'e': res = 'e'; break;  //erase params
      case 'n': res = 'n'; break;  //set current preset No
      case 'w': res = 'w'; break;  //wash wheels

      case 'c': res = 'c'; break;  //calibration
      case 'q': res = 'q'; break;  //test sensors
      case 'b': res = 'b'; break;  //set sensor brightness /////////////////////////////////////////////////////////////////////////////////////////////

      case 'f': res = 'f'; break;  //switch on/off fan
      case 't': res = 't'; break;  //test Fan !!

      case 'a': res = 'a'; break;  //enter all params
      case 's': res = 's'; break;  //set speed params
      case 'p': res = 'p'; break;  //set P/D param
      case 'k': res = 'k'; break;  //set all PID params
      case 'm': res = 'm'; break;  //set speed map params

      case 10: res = 'x'; break;  //
      default: continue;
    }
  }
  delay(10);
  u.read();
  return res;
}

void displayHelp() {
  u.info(S_MENU_TEXT);
}

void processMenu() {
  char menuItem = menu();
  int intValue = 0;
  switch (menuItem) {
    case 'h':
      displayHelp();
      break;
    case 'd':
      p.P.DEBUG_MODE = !p.P.DEBUG_MODE;
      u.info(p.S.DEBUG_MODE + S_SET_TO + p.trueFalseLabel(p.P.DEBUG_MODE));
      u.setDebugMode(p.P.DEBUG_MODE);
      p.saveParams(p.presetNo);
      break;
    case 'g':
      if (p.P.PRINT_MODE == 0) {
        p.P.PRINT_MODE = 1;
        u.setPrintMode(PrintMode::CSV);  //!!!!!! тут какая то жесть, на самом деле ставится Graph
      } else if (p.P.PRINT_MODE == 1) {
        p.P.PRINT_MODE = 2;
        u.setPrintMode(PrintMode::CSV);
      } else if (p.P.PRINT_MODE == 2) {
        p.P.PRINT_MODE = 0;
        u.setPrintMode(PrintMode::Text);
      }
      u.info(p.S.PRINT_MODE + S_SET_TO + p.printModeLabel());
      p.saveParams(p.presetNo);
      break;
    case 'e':
      p.eraseParams();
      break;
    case 'l':
      p.loadParams(p.presetNo);
      sensor.setSamplesPerSensor(p.P.SAMPLES_PER_SENSOR);
      sensor.setBlackLevel(p.P.SENSOR_BLACK_LEVEL);
      p.displayParams();
      break;
    case 'n':
      u.info(S_ENTER + p.S.PRESET_NO + "(MAXIMUM PRESETS: " + String(MAX_PRESET_NO) + ", " + S_ENTER_TO_LEAVE + String(p.presetNo) + ")");
      intValue = u.readInteger();
      if (intValue != 0 && intValue != p.presetNo) {
        //сначала сохраняем текущие параметры
        p.saveParams(p.presetNo);
        //НЕ загружаем другой пресет - только переключаем номер!
        p.presetNo = constrain(intValue, 1, MAX_PRESET_NO);
        u.info(p.S.PRESET_NO + S_SET_TO + String(p.presetNo));
      }
      break;
    case 'c':
      sensor.resetCalibration();
      doCalibration();
      p.saveCalibration();
      break;
    case 'q':
      testSensors();
      break;
    // case 'b':  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //   u.info(S_ENTER_SENSOR_BRIGHTNESS);
    //   enterParams(PARAMS_BRIGHTNESS);
    //   p.saveParams(p.presetNo);
    //   break;
    case 'f':
      p.P.USE_FAN = !p.P.USE_FAN;
      u.info(p.S.USE_FAN + S_SET_TO + String(p.P.USE_FAN));
      p.saveParams(p.presetNo);
      motors.allowFan(p.P.USE_FAN);
      if (p.P.USE_FAN)
        u.info(S_RESTART_BOT);
      break;
    case 't':
      if (!p.P.USE_FAN) {
        u.info(S_ESC_NO_ARMING);
        break;
      }
      u.info(S_FAN_TEST);
      motors.testFan(p.P.FAN_SPEED_IDLE, p.P.FAN_SPEED_MAX);
      u.info(S_DONE);
      break;
    case 'w':
      washWheels();
      break;
    case 'a':
      u.info(S_ENTER_ALL_PARAMS);
      enterParams(PARAMS_ALL);
      p.saveParams(p.presetNo);
      break;
    case 's':
      u.info(S_ENTER_NORMAL_SPEED);
      enterParams(PARAMS_SPEED);
      p.saveParams(p.presetNo);
      break;
    case 'p':
      u.info(S_ENTER_P_D);
      enterParams(PARAMS_KP_KD);
      p.saveParams(p.presetNo);
      break;
    case 'k':
      u.info(S_ENTER_P_D_I);
      enterParams(PARAMS_KP_KD_KI);
      p.saveParams(p.presetNo);
      break;
    case 'm':
      u.info(S_ENTER_SPEEDMAP);
      u.info(S_ENTER_SPEEDMAP2);
      u.info(S_ENTER_SPEEDMAP3);
      enterParams(PARAMS_SPEEDMAP);
      p.saveParams(p.presetNo);
      break;
    case 'x':
      u.info(S_X_TO_PREPARE);
      MOVE_BOT = true;
    default:;
  }
}

void enterParams(int aMode) {  ///////////////////////////////////////////////////////////////////////////////////////
  int intValue = 0;
  String strValue;
  float floatValue = 0;

  if (aMode == PARAMS_ALL) {
    u.info(S_ENTER + p.S.BRAKE_SPEED + " (" + S_ENTER_TO_LEAVE + String(p.P.BRAKE_SPEED) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.BRAKE_SPEED = constrain(intValue, -150, 150);
      u.info(p.S.BRAKE_SPEED + S_SET_TO + p.P.BRAKE_SPEED);
    }

    u.info(S_ENTER + p.S.LOW_SPEED + " (" + S_ENTER_TO_LEAVE + String(p.P.LOW_SPEED) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.LOW_SPEED = intValue;
      u.info(p.S.LOW_SPEED + S_SET_TO + p.P.LOW_SPEED);
    }
  }

  if (aMode == PARAMS_ALL || aMode == PARAMS_SPEED) {
    u.info(S_ENTER + p.S.NORMAL_SPEED + " (" + S_ENTER_TO_LEAVE + String(p.P.NORMAL_SPEED) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.NORMAL_SPEED = constrain(intValue, 0, 255);
      if (aMode == PARAMS_SPEED && p.P.NORMAL_SPEED > p.P.FAST_SPEED) {
        p.P.FAST_SPEED = p.P.NORMAL_SPEED;
      }
      u.info(p.S.NORMAL_SPEED + S_SET_TO + p.P.NORMAL_SPEED);
    }
  }

  if (aMode == PARAMS_ALL) {
    u.info(S_ENTER + p.S.FAST_SPEED + " (" + S_ENTER_TO_LEAVE + String(p.P.FAST_SPEED) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.FAST_SPEED = constrain(intValue, 0, 255);
      u.info(p.S.FAST_SPEED + S_SET_TO + p.P.FAST_SPEED);
    }

    u.info(S_ENTER + p.S.TOP_SPEED + " (" + S_ENTER_TO_LEAVE + String(p.P.TOP_SPEED) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.TOP_SPEED = constrain(intValue, 0, 255);
      u.info(p.S.TOP_SPEED + S_SET_TO + p.P.TOP_SPEED);
    }

    u.info(S_ENTER + p.S.TURN_SPEED + " (" + S_ENTER_TO_LEAVE + String(p.P.TURN_SPEED) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.TURN_SPEED = constrain(intValue, 0, 255);
      u.info(p.S.TURN_SPEED + S_SET_TO + p.P.TURN_SPEED);
    }

    u.info(S_ENTER + p.S.TURN_SPEED_CORRECTION + " (" + S_ENTER_TO_LEAVE + String(p.P.TURN_SPEED_CORRECTION) + ")");
    floatValue = u.readFloat();
    if (floatValue != 0) {
      p.P.TURN_SPEED_CORRECTION = floatValue;
      u.info(p.S.TURN_SPEED_CORRECTION + S_SET_TO + p.P.TURN_SPEED_CORRECTION);
    }

    u.info(S_ENTER + p.S.RAMPUP_TIME + " (" + S_ENTER_TO_LEAVE + String(p.P.RAMPUP_TIME) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.RAMPUP_TIME = intValue;
      u.info(p.S.RAMPUP_TIME + S_SET_TO + p.P.RAMPUP_TIME);
    }

    u.info(S_ENTER + p.S.SAMPLES_PER_SENSOR + " (" + S_ENTER_TO_LEAVE + String(p.P.SAMPLES_PER_SENSOR) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.SAMPLES_PER_SENSOR = constrain(intValue, 0, 64);
      sensor.setSamplesPerSensor(p.P.SAMPLES_PER_SENSOR);
      u.info(p.S.SAMPLES_PER_SENSOR + S_SET_TO + p.P.SAMPLES_PER_SENSOR);
    }

    // u.info(S_ENTER + p.S.SENSOR_SMOOTHING + " (" + S_ENTER_TO_LEAVE + String(p.P.SENSOR_SMOOTHING) + ")");
    // intValue = u.readInteger();
    // if (intValue != 0) {
    //   p.P.SENSOR_SMOOTHING = constrain(intValue, 0, 4);
    //   sensor.setSmoothing(QTRSmoothing(p.P.SENSOR_SMOOTHING));
    //   u.info(p.S.SENSOR_BLACK_LEVEL + S_SET_TO + p.P.SENSOR_BLACK_LEVEL);
    // }

    u.info(S_ENTER + p.S.SENSOR_BLACK_LEVEL + " (" + S_ENTER_TO_LEAVE + String(p.P.SENSOR_BLACK_LEVEL) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.SENSOR_BLACK_LEVEL = constrain(intValue, 0, 800);
      sensor.setBlackLevel(p.P.SENSOR_BLACK_LEVEL);
      u.info(p.S.SENSOR_BLACK_LEVEL + S_SET_TO + p.P.SENSOR_BLACK_LEVEL);
    }
  }

  if (aMode == PARAMS_ALL || aMode == PARAMS_KP_KD || aMode == PARAMS_KP_KD_KI) {
    u.info(S_ENTER + p.S.kP + " (" + S_ENTER_TO_LEAVE + String(p.P.kP) + ")");
    floatValue = u.readFloat();
    if (floatValue != 0) {
      p.P.kP = floatValue;
      u.info(p.S.kP + S_SET_TO + p.P.kP);
    }
    u.info(S_ENTER + p.S.kD + " (" + S_ENTER_TO_LEAVE + String(p.P.kD) + ")");
    floatValue = u.readFloat();
    if (floatValue != 0) {
      p.P.kD = floatValue;
      u.info(p.S.kD + S_SET_TO + p.P.kD);
    }
  }

  if (aMode == PARAMS_ALL || aMode == PARAMS_KP_KD_KI) {
    u.info(S_ENTER + p.S.D_AGE + " (" + S_ENTER_TO_LEAVE + String(p.P.D_AGE) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.D_AGE = intValue;
      u.info(p.S.D_AGE + S_SET_TO + p.P.D_AGE);
    }

    u.info(S_ENTER + p.S.kI + " (" + S_ENTER_TO_LEAVE + String(p.P.kI) + ")");
    floatValue = u.readFloat();
    if (floatValue != 0) {
      p.P.kI = floatValue;
      u.info(p.S.kI + S_SET_TO + p.P.kI);
    }

    u.info(S_ENTER + p.S.I_AGE + " (" + S_ENTER_TO_LEAVE + String(p.P.I_AGE) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.I_AGE = intValue;
      u.info(p.S.I_AGE + S_SET_TO + p.P.I_AGE);
    }
  }

  if (aMode == PARAMS_ALL) {
    u.info(S_ENTER + p.S.FAN_SPEED_IDLE + " (" + S_ENTER_TO_LEAVE + String(p.P.FAN_SPEED_IDLE) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.FAN_SPEED_IDLE = constrain(intValue, 0, 255);
      u.info(p.S.FAN_SPEED_IDLE + S_SET_TO + p.P.FAN_SPEED_IDLE);
    }

    u.info(S_ENTER + p.S.FAN_SPEED_MAX + " (" + S_ENTER_TO_LEAVE + String(p.P.FAN_SPEED_MAX) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.FAN_SPEED_MAX = constrain(intValue, 0, 255);
      u.info(p.S.FAN_SPEED_MAX + S_SET_TO + p.P.FAN_SPEED_MAX);
    }


    u.info(S_ENTER + p.S.TIME_TO_STOP + " (" + S_ENTER_TO_LEAVE + String(p.P.TIME_TO_STOP) + ")");
    intValue = u.readInteger();
    if (intValue != 0) {
      p.P.TIME_TO_STOP = intValue;
      u.info(p.S.TIME_TO_STOP + S_SET_TO + p.P.TIME_TO_STOP);
    }
  }

  if (aMode == PARAMS_ALL || aMode == PARAMS_BRIGHTNESS) {
    u.info(S_ENTER + p.S.SENSOR_BRIGHTNESS + " (" + S_ENTER_TO_LEAVE + String(p.P.SENSOR_BRIGHTNESS_MODE) + ")");
    intValue = u.readInteger();
    if (intValue >= p.P.SENSOR_BRIGHTNESS_MODE) {
      // Serial.println("INONE");
      for (int i = 0; i < (intValue - p.P.SENSOR_BRIGHTNESS_MODE); i++) {
        digitalWrite(18, LOW);  //--
        digitalWrite(19, LOW);  //--
        for (int j; j > 5; i++) {
          analogRead(SENSOR_IN);
        }
        digitalWrite(18, HIGH);  //--
        digitalWrite(19, HIGH);  //--
        // Serial.print("a");
        u.info("I");
      }
      u.info("IF");
    } else if (intValue < p.P.SENSOR_BRIGHTNESS_MODE) {
      // Serial.println("INTWO");
      int num = 32 - p.P.SENSOR_BRIGHTNESS_MODE + intValue;
      for (int i = 0; i < num; i++) {
        digitalWrite(18, HIGH);  //--
        digitalWrite(19, HIGH);  //--
        for (int j; j > 5; i++) {
          analogRead(SENSOR_IN);
        }
        digitalWrite(18, LOW);  //--
        digitalWrite(19, LOW);  //--
        // Serial.print("b");
        u.info("E");
      }
      u.info("ELIF");
    }
    p.P.SENSOR_BRIGHTNESS_MODE = intValue;
  }

  if (aMode == PARAMS_SPEEDMAP) {
    u.info("CURRENT " + p.S.SPEED_MAP + " IS: " + p.P.SPEED_MAP);
    strValue = u.readStr();
    if ((strValue != "") && (strValue != "\n") && (strValue != "\r") && (strValue != "\r\n")) {
      strValue.trim();
      strValue.toCharArray(p.P.SPEED_MAP, P_SPEEDMAP_MAX_LENGTH);
    }
    p.parseSpeedMap();
    u.info("NEW " + p.S.SPEED_MAP + ":");
    p.displaySpeedMap();
  }
}

void doCalibration() {
  u.info_("CALIBRATION...");
  sensor.setSamplesPerSensor(8);
  for (uint16_t i = 0; i < 500; i++) {
    if ((u.available() > 0 && u.read() == 10) || B1_PRESSED)
      break;
    if (i % 10 == 0)
      digitalWrite(LED_BUILTIN, HIGH);
    if (i % 20 == 0)
      digitalWrite(LED_BUILTIN, LOW);
    sensor.calibrate();
    for (int i = 0; i < SENSOR_BUF_SIZE; i++) {
      u.info_("[" + String(sensor.calibrationOn.minimum[i]) + ":" + String(sensor.calibrationOn.maximum[i]) + "] ");
    }
    u.info("");
  }
  sensor.setSamplesPerSensor(p.P.SAMPLES_PER_SENSOR);
  digitalWrite(LED_BUILTIN, LOW);
  u.info("DONE");
}

void testSensors() {
  u.info(S_TESTSENSORS);
  while (true) {
    if ((u.available() > 0 && u.read() == 10) || B1_PRESSED)
      break;
    uint16_t position = sensor.readLineBlack(SENSOR_BUF);
    for (uint8_t i = 0; i < SENSOR_BUF_SIZE; i++) {
      u.info_(SENSOR_BUF[i]);
      u.info_('\t');
    }
    u.info(String(position));
    delay(100);
  }
  B1_PRESSED = false;
  u.info(S_DONE);
}




void washWheels() {
  u.info_(S_WASHING);
  long startTime = millis();
  while (true) {
    motors.goMotors(150, 150);
    if ((u.available() > 0 && u.read() == 10) || B1_PRESSED || millis() - startTime > 10000)
      break;
    delay(10);
  }
  B1_PRESSED = false;
  motors.stopMotors();
  u.info(S_DONE);
}

char res;
int dP = 0;
int dD = 0;
int dI = 0;
void processKoefsAdj(char mode) {
  switch (mode) {
    case 'a':
      res = 'a';
      u.info("KP++");
      p.P.kP = p.P.kP + 0.02;
      dP = dP + 0.02;
      break;
    case 'b':
      res = 'b';
      u.info("KP+");
      p.P.kP = p.P.kP + 0.01;
      dP = dP + 0.01;
      break;
    case 'c':
      res = 'c';
      u.info("KP-");
      p.P.kP = p.P.kP - 0.01;
      dP = dP - 0.01;
      break;
    case 'd':
      res = 'd';
      u.info("KP--");
      p.P.kP = p.P.kP - 0.02;
      dP = dP - 0.02;
      break;
    case 'e':
      res = 'e';
      u.info("KD++");
      p.P.kD = p.P.kD + 0.02;
      dD = dD + 0.02;
      break;
    case 'f':
      res = 'f';
      u.info("KD+");
      p.P.kD = p.P.kD + 0.01;
      dD = dD + 0.01;
      break;
    case 'g':
      res = 'g';
      u.info("KD-");
      p.P.kD = p.P.kD - 0.01;
      dD = dD - 0.01;
      break;
    case 'h':
      res = 'h';
      u.info("KD--");
      p.P.kD = p.P.kD - 0.02;
      dD = dD - 0.02;
      break;
    case 'i':
      res = 'i';
      u.info("KI++");
      p.P.kI = p.P.kI + 0.02;
      dI = dI + 0.02;
      break;
    case 'j':
      res = 'j';
      u.info("KI+");
      p.P.kI = p.P.kI + 0.01;
      dI = dI + 0.01;
      break;
    case 'k':
      res = 'k';
      u.info("KI-");
      p.P.kI = p.P.kI - 0.01;
      dI = dI - 0.01;
      break;
    case 'l':
      res = 'l';
      u.info("KI--");
      p.P.kI = p.P.kI - 0.02;
      dI = dI - 0.02;
      break;
    case 'm':
      res = 'm';
      u.info("KP=def");
      p.P.kP = p.P.kP - dP;
      dP = 0;
      break;
    case 'n':
      res = 'n';
      u.info("KD=def");
      p.P.kD = p.P.kD - dD;
      dD = 0;
      break;
    case 'o':
      res = 'o';
      u.info("KI=def");
      p.P.kI = p.P.kI - dI;
      dI = 0;
      break;
    case 'x':
      STARTED = false;
      WAS_STARTED = false;
      PREPARED = false;
      MOVE_BOT = false;
      motors.goFan(0);
      motors.stopMotors();
      B1_PRESSED = false;
      B2_PRESSED = false;
      digitalWrite(LED_BUILTIN, LOW);
      u.info(S_ALL_STOPPED);
      u.info("Last cycle time: " + String(cycleDuration));
    case 10:
      STARTED = false;
      WAS_STARTED = false;
      PREPARED = false;
      MOVE_BOT = false;
      motors.goFan(0);
      motors.stopMotors();
      B1_PRESSED = false;
      B2_PRESSED = false;
      digitalWrite(LED_BUILTIN, LOW);
      u.info(S_ALL_STOPPED);
      u.info("Last cycle time: " + String(cycleDuration));
    default:
      SerialBT.println("ERROR, NO MATCHING MODE FOUND");
  }
}

void serialFlush(){
  while(u.available() > 0) {
    char t = u.read();
  }
}