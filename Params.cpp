#include "Arduino.h"
#include <EEPROM.h>
#include "Params.h"
#include "Utils.h"
#include "QTRSensorsMulti.h"
#include "Const.h"


Params::Params(int aSensorBufSize, Utils &aUtils, QTRSensors &aSensor)
  : util(aUtils), sens(aSensor) {
  //очищаем на всякий случай память под спидмапы чтоб не мусорило
  memset(P.SPEED_MAP, 0, sizeof(P.SPEED_MAP));
  memset(speedMap, 0, sizeof(speedMap));
  sensorBufSize = aSensorBufSize;
}

Params::~Params() {}


int Params::getSpeedByMode(char mode) {
  switch (mode) {
    case SPEEDMODE_STOP: return 0;
    case SPEEDMODE_ZERO: return 0;
    case SPEEDMODE_LOW: return P.LOW_SPEED;
    case SPEEDMODE_NORMAL: return P.NORMAL_SPEED;
    case SPEEDMODE_FAST: return P.FAST_SPEED;
    case SPEEDMODE_TOP: return P.TOP_SPEED;
    default: return P.NORMAL_SPEED;
  }
}

int Params::getTurnSpeedByMode(char mode) {
  switch (mode) {
    case SPEEDMODE_STOP: return 0;
    case SPEEDMODE_ZERO: return P.TURN_SPEED;  //это для режима Idle с верчением на месте
    case SPEEDMODE_LOW: return P.NORMAL_SPEED;
    case SPEEDMODE_NORMAL: return P.TURN_SPEED;
    case SPEEDMODE_FAST: return P.TURN_SPEED;
    case SPEEDMODE_TOP: return MAX_MOTOR_SPEED;
    default: return P.TURN_SPEED;
  }
}

void Params::parseSpeedMap() {
  speedMapLength = 0;
  String speedMapStr = P.SPEED_MAP;
  speedMapStr.trim();
  if (speedMapStr == "") return;
  const String NOT_FOUND = "_";

  for (int i = 0; i < P_SPEEDMAP_MAX_LENGTH; i++) {
    String elem = util.getValueAtIndex(P.SPEED_MAP, ',', i, NOT_FOUND);
    if (elem == NOT_FOUND) break;
    elem.trim();
    int duration = util.getValueAtIndex(elem, ':', 0).toInt();
    char mode = util.getValueAtIndex(elem, ':', 1).charAt(0);
    if (!(mode == SPEEDMODE_TOP || mode == SPEEDMODE_FAST || mode == SPEEDMODE_NORMAL || mode == SPEEDMODE_LOW || mode == SPEEDMODE_ZERO)) {
      util.info(S_ERROR + ": unknown speed mode - '" + mode + "', speed map is not working");
      speedMapLength=0;
      break;
    }
    speedMapLength++;
    float kP = P.kP;
    float kD = P.kD;
    int ageD = P.D_AGE;
    float kI = P.kI;
    int ageI = P.I_AGE;
    //определяем какая скорость должна быть по умолчанию сначала,
    //в зависомости от режима скорости
    int runSpeed = getSpeedByMode(mode);
    int turnSpeed = getTurnSpeedByMode(mode);
    int brakeSpeed = P.BRAKE_SPEED;

    String s = util.getValueAtIndex(elem, ':', 2, NOT_FOUND);
    if (s != NOT_FOUND) kP = s.toFloat();
    s = util.getValueAtIndex(elem, ':', 3, NOT_FOUND);
    if (s != NOT_FOUND) kD = s.toFloat();
    s = util.getValueAtIndex(elem, ':', 4, NOT_FOUND);
    if (s != NOT_FOUND) ageD = s.toInt();
    s = util.getValueAtIndex(elem, ':', 5, NOT_FOUND);
    if (s != NOT_FOUND) kI = s.toFloat();
    s = util.getValueAtIndex(elem, ':', 6, NOT_FOUND);
    if (s != NOT_FOUND) ageI = s.toInt();
    s = util.getValueAtIndex(elem, ':', 7, NOT_FOUND);
    if (s != NOT_FOUND) turnSpeed = s.toInt();
    s = util.getValueAtIndex(elem, ':', 8, NOT_FOUND);
    if (s != NOT_FOUND) brakeSpeed = s.toInt();
    long timeStart = i == 0 ? 0 : speedMap[i - 1].timeEnd;  //значение времени из предыдущего шага
    long timeEnd = i == 0 ? duration : timeStart + duration;
    speedMap[i] = (SpeedMapEntry){ timeStart, timeEnd, mode, kP, kD, ageD, kI, ageI, runSpeed, turnSpeed, brakeSpeed };
  }
}

void Params::displaySpeedMap() {
  String s;
  if (speedMap[0].timeEnd > 0) {
    for (int i = 0; i < speedMapLength; i++) {
      if (speedMap[i].timeEnd > 0) {
        s = "  " + util.fullFillChars(String(speedMap[i].timeStart) + '-' + String(speedMap[i].timeEnd), ' ', 12) + " mode:" + speedMap[i].speedMode;
        s = s + " " + S.kP + ":" + String(speedMap[i].kP) + " " + S.kD + ":" + String(speedMap[i].kD) + " " + S.D_AGE + ":" + String(speedMap[i].ageD);
        s = s + " " + S.kI + ":" + String(speedMap[i].kI) + " " + S.I_AGE + ":" + String(speedMap[i].ageI);
        s = s + " SPEED:" + String(speedMap[i].runSpeed) + " " + S.TURN_SPEED + ":" + String(speedMap[i].turnSpeed) + " " + S.BRAKE_SPEED + ":" + String(speedMap[i].brakeSpeed);
        util.info(s);
      }
    }
  } else {
    util.info("NO PARSED SPEED MAP");
  }
}


// char Params::getSpeedMapMode() {
//   return getSpeedMapEntry().speedMode;
// }

SpeedMapEntry Params::getSpeedMapEntry() {
  SpeedMapEntry entry;
  entry.speedMode = SPEEDMODE_NORMAL;
  entry.kP = P.kP;
  entry.kD = P.kD;
  entry.ageD = P.D_AGE;
  entry.kI = P.kI;
  entry.ageI = P.I_AGE;
  entry.runSpeed = getSpeedByMode(SPEEDMODE_NORMAL);
  entry.turnSpeed = getTurnSpeedByMode(SPEEDMODE_NORMAL);
  entry.brakeSpeed = P.BRAKE_SPEED;
  if (speedMapLength == 0) return entry;
  long currTime = millis() - startRunTime;
  for (int i = 0; i < speedMapLength; i++) {
    if ((currTime > speedMap[i].timeStart) && (currTime < speedMap[i].timeEnd)) {
      entry = speedMap[i];
      break;
    }
  }
  return entry;
}

void Params::validateParams() {
  P.SAMPLES_PER_SENSOR = constrain(1, 32, P.SAMPLES_PER_SENSOR);
  P.SENSOR_SMOOTHING = constrain(0, 3, P.SENSOR_SMOOTHING);  //0-none, 1 - double average, 2-triple average, 3-filter
  P.SENSOR_BLACK_LEVEL = constrain(50, 800, P.SENSOR_BLACK_LEVEL);
  P.SENSOR_FILTER_LEVEL = constrain(0, 32000, P.SENSOR_FILTER_LEVEL);
}

void Params::loadParams(int aPreset) {
  ParamNames tmpP;

  util.info_("LOADING FROM PRESET " + String(aPreset) + "...");
  if (!checkHeader()) {
    util.info("NO SAVED PARAMS");
    return;
  }
  aPreset = constrain(aPreset, 1, MAX_PRESET_NO);
  EEPROM.get(getParamsOffset(aPreset), tmpP);

  if (tmpP.HEADER != PRESET_HEADER) {
    util.info("PRESET " + String(+aPreset) + " IS EMPTY, DEFAULTS WERE SET.");
    return;
  }
  P = tmpP;
  //validateParams();
  if (P.SAMPLES_PER_SENSOR <= 0) P.SAMPLES_PER_SENSOR = 2;
  util.setDebugMode(P.DEBUG_MODE);
  util.setPrintMode(PrintMode(P.PRINT_MODE));
  parseSpeedMap();
  util.info(S_DONE);
}

void Params::saveParams(int aPreset) {
  util.info_("SAVING AT PRESET " + String(aPreset) + "...");
  //всегда записываем заголовок
  EEPROM.put(EEPROM_PARAMS_OFFSET, EEPROM_PARAMS_HEADER);
  //EEPROM.put(EEPROM_PARAMS_OFFSET + sizeof(EEPROM_PARAMS_HEADER), aPreset);
  EEPROM.put(getParamsOffset(aPreset), P);
  EEPROM.commit();
  util.info(S_DONE);
}

void Params::eraseParams() {
  util.info_("ERASING PARAMS...");
  ParamNames temP;
  memset(&temP, 0, sizeof(ParamNames));
  for (int i = 1; i <= MAX_PRESET_NO; i++) {
    EEPROM.put(getParamsOffset(i), temP);
  }
  //стираем заголовок, как будто там ничего нет
  EEPROM.put(EEPROM_PARAMS_OFFSET, 0);
  // EEPROM.put(EEPROM_PARAMS_OFFSET + sizeof(EEPROM_PARAMS_HEADER), 0);
  EEPROM.commit();
  util.info(S_DONE);
  util.info(S_NEED_RESTART);
}

void Params::saveCalibration() {
  util.info_("SAVING CALIBRATION...");
  for (int i = 0; i < sensorBufSize; i++) {
    util.info_("[" + String(sens.calibrationOn.minimum[i]) + ":" + String(sens.calibrationOn.maximum[i]) + "]");
  }
  util.info("...");
  for (int i = 0; i < sensorBufSize; i++) {
    EEPROM.put(i * 2, sens.calibrationOn.minimum[i]);
    EEPROM.put(i * 2 + sensorBufSize * 2, sens.calibrationOn.maximum[i]);
  }
  EEPROM.commit();
  util.info("DONE");
  delay(1000);
}

void Params::loadCalibration() {
  util.info_("LOADING CALIBRATION...");
  sens.calibrate();  //init calibration array first
  for (int i = 0; i < sensorBufSize; i++) {
    EEPROM.get(i * 2, sens.calibrationOn.minimum[i]);
    EEPROM.get(i * 2 + sensorBufSize * 2, sens.calibrationOn.maximum[i]);
  }
  util.info("DONE.");
  delay(1000);
}

void Params::displayCalibration() {
  for (int i = 0; i < sensorBufSize; i++) {
    util.info_("[" + String(sens.calibrationOn.minimum[i]) + ":" + String(sens.calibrationOn.maximum[i]) + "]");
  }
  util.info("");
}


void Params::displayParams() {
  util.info("PARAMS PRESET # " + util.fillChars(' ', 4) + String(presetNo));
  util.info(S.BRAKE_SPEED + S.COLON + util.fillChars(' ', 7) + String(P.BRAKE_SPEED));
  util.info(S.LOW_SPEED + S.COLON + util.fillChars(' ', 9) + String(P.LOW_SPEED));
  util.info(S.NORMAL_SPEED + S.COLON + util.fillChars(' ', 6) + String(P.NORMAL_SPEED));
  util.info(S.FAST_SPEED + S.COLON + util.fillChars(' ', 8) + String(P.FAST_SPEED));
  util.info(S.TOP_SPEED + S.COLON + util.fillChars(' ', 9) + String(P.TOP_SPEED));
  util.info(S.TURN_SPEED + S.COLON + util.fillChars(' ', 8) + String(P.TURN_SPEED));
  util.info(S.TURN_SPEED_CORRECTION + util.fillChars(' ', 6) + String(P.TURN_SPEED_CORRECTION));
  util.info(S.RAMPUP_TIME + util.fillChars(' ', 9) + String(P.RAMPUP_TIME));
  // util.info("");
  util.info(S.kP + S.COLON + util.fillChars(' ', 16) + String(P.kP));
  util.info(S.kD + S.COLON + util.fillChars(' ', 16) + String(P.kD));
  util.info(S.D_AGE + S.COLON + util.fillChars(' ', 13) + String(P.D_AGE));
  util.info(S.kI + S.COLON + util.fillChars(' ', 16) + String(P.kI));
  util.info(S.I_AGE + S.COLON + util.fillChars(' ', 13) + String(P.I_AGE));
  // util.info("");
  util.info(S.USE_FAN + S.COLON + util.fillChars(' ', 11) + String(P.USE_FAN));
  util.info(S.FAN_SPEED_IDLE + S.COLON + util.fillChars(' ', 4) + String(P.FAN_SPEED_IDLE));
  util.info(S.FAN_SPEED_MAX + S.COLON + util.fillChars(' ', 5) + String(P.FAN_SPEED_MAX));
  // util.info("");
  util.info(S.SAMPLES_PER_SENSOR + S.COLON + String(P.SAMPLES_PER_SENSOR));
  util.info(S.SENSOR_SMOOTHING + S.COLON + util.fillChars(' ', 2) + String(P.SENSOR_SMOOTHING));
  util.info(S.SENSOR_BLACK_LEVEL + S.COLON + String(P.SENSOR_BLACK_LEVEL));
  //util.info(S.SENSOR_FILTER_LEVEL + S.COLON2 + String(P.SENSOR_FILTER_LEVEL));
  // util.info("");
  util.info(S.TIME_TO_STOP + util.fillChars(' ', 8) + String(P.TIME_TO_STOP));
  util.info(S.SPEED_MAP + util.fillChars(' ', 11) + String(P.SPEED_MAP));
  parseSpeedMap();
  displaySpeedMap();
  // util.info("");
  util.info(S.PRINT_MODE + S.COLON + util.fillChars(' ', 8) + printModeLabel());
  util.info(S.DEBUG_MODE + S.COLON + util.fillChars(' ', 8) + trueFalseLabel(P.DEBUG_MODE));
}

void Params::setDebugMode(boolean aMode) {
  debugMode = aMode;
}
