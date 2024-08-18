#pragma once
#include "Arduino.h"
#include "Utils.h"
#include "QTRSensorsMulti.h"

const int P_SPEEDMAP_MAX_LENGTH = 128;
const int MAX_PRESET_NO = 8;     //1 - based
const int PRESET_HEADER = 8888;  //1 - based

struct ParamNames {
  //public fields - dynamic params
  //byte PRINT_MODE = 1;  //enum PrintMode ;
  int HEADER = PRESET_HEADER;
  bool DEBUG_MODE = true;
  byte PRINT_MODE = 0;  //{0 - Text, 1 - Graph,  2 - CSV buffer};

  //sensor
  int SAMPLES_PER_SENSOR = 2;
  int SENSOR_SMOOTHING = 0;  //0-none, 1 - double average, 2-triple average, 3-filter
  int SENSOR_BLACK_LEVEL = 200;
  int SENSOR_FILTER_LEVEL = 5000;
  int SENSOR_BRIGHTNESS_MODE = 1; 
 
  //bot speed
  int BRAKE_SPEED = -50;   //negative speed for one wheel in fast turns
  int LOW_SPEED = 50;      //speed to go trough complex turns
  int NORMAL_SPEED = 190;  //base speed for most of route
  int FAST_SPEED = 220;    //max speed to fasten some parts of route, also used for turns
  int TOP_SPEED = 240;     //maximum available speed
  int TURN_SPEED = 220;    //maximum speed for turns
  int RAMPUP_TIME = 1000;  //time for smooth start

  bool USE_FAN = false;
  //tourbine speed (0..255 based)
  int FAN_SPEED_IDLE = 50;  //tourbine speed while prepared
  int FAN_SPEED_MAX = 100;  //tourbine speed while move

  float kP = 0.1;  //начальный коэффициент пропорциональной обратной связи
  float kD = 0.2;  //начальный коэффициент дифференциальной обратной связи 0.2
  int D_AGE = 5;

  float kI = 0.000;  //начальный коэффициент дифференциальной обратной связи 0.2
  int I_AGE = 15;
  float TURN_SPEED_CORRECTION = 2;  //

  long TIME_TO_STOP = 16000;

  int __RESERVED_0 = 0;
  int __RESERVED_1 = 0;
  int __RESERVED_2 = 0;
  int __RESERVED_3 = 0;
  int __RESERVED_4 = 0;
  int __RESERVED_5 = 0;

  char SPEED_MAP[P_SPEEDMAP_MAX_LENGTH];
};

struct ParamLabels {
  const String PRESET_NO = "PRESET_NO";
  const String PRINT_MODE = "PRINT_MODE";
  const String DEBUG_MODE = "DEBUG_MODE";
  //const String GRAPH_DEBUG = "GRAPH_DEBUG";

  const String SAMPLES_PER_SENSOR = "SAMPLES_PER_SENSOR";
  const String SENSOR_SMOOTHING = "SENSOR_SMOOTHING";
  const String SENSOR_BLACK_LEVEL = "SENSOR_BLACK_LEVEL";
  const String SENSOR_FILTER_LEVEL = "SENSOR_FILTER_LEVEL";
  const String SENSOR_BRIGHTNESS = "SENSOR_BRIGHTNESS";

  const String BRAKE_SPEED = "BRAKE_SPEED";
  const String LOW_SPEED = "LOW_SPEED";
  const String NORMAL_SPEED = "NORMAL_SPEED";
  const String FAST_SPEED = "FAST_SPEED";
  const String TOP_SPEED = "TOP_SPEED";
  const String TURN_SPEED = "TURN_SPEED";
  const String TURN_SPEED_CORRECTION = "TURNSPEED_CORR";
  const String RAMPUP_TIME = "RAMPUP_TIME";

  const String USE_FAN = "USE_FAN";
  const String FAN_SPEED_IDLE = "FAN_SPEED_IDLE";
  const String FAN_SPEED_MAX = "FAN_SPEED_MAX";

  const String kP = "kP";
  const String kD = "kD";
  const String D_AGE = "D_AGE";
  const String kI = "kI";
  const String I_AGE = "I_AGE";


  const String TIME_TO_STOP = "TIME_TO_STOP";
  const String SPEED_MAP = "SPEED_MAP";

  const String COLON = ": ";
  const String COLON2 = ":";
};


struct SpeedMapEntry {
  long timeStart;
  long timeEnd;
  char speedMode;  //'t'-top, 'f'-fast, 'n'-normal, 'l'-low
  float kP;
  float kD;
  int ageD;
  float kI;
  int ageI;
  int runSpeed;    //просто текущая скорость, с которой нужно ехать.
  int turnSpeed;   //верхняя граница скорости при поворотах
  int brakeSpeed;  //нижняя граница скорости при поворотах (торможение)
};


class Params {
public:
  const static char SPEEDMODE_STOP = 's';  //стоим не шевелимся
  const static char SPEEDMODE_ZERO = '0';  //едем на месте
  const static char SPEEDMODE_NORMAL = 'n';
  const static char SPEEDMODE_TOP = 't';
  const static char SPEEDMODE_FAST = 'f';
  const static char SPEEDMODE_LOW = 'l';
  //const static float CORR_LEFT_MOTOR = 1.0;

  ParamNames P;
  ParamLabels S;
  int presetNo = 1;

  Params(int aSensorBufSize, Utils &aUtils, QTRSensors &aSensor);
  ~Params();
  void setDebugMode(boolean aMode);

  void saveCalibration();
  void loadCalibration();

  // void loadParams();
  void loadParams(int aPreset);
  // void saveParams();
  void saveParams(int aPreset);
  void parseSpeedMap();
  void eraseParams();

  //void parseRoute

  void displayParams();
  void displayCalibration();
  void displaySpeedMap();
  // int getSpeedMapSpeed();
  // char getSpeedMapMode();

  SpeedMapEntry getSpeedMapEntry();

  void setStartRunTime(long aStartRunTime) {
    startRunTime = aStartRunTime;
  }

  String trueFalseLabel(boolean aBool) {
    return aBool ? "Yes" : "No";
  }

  String printModeLabel() {
    switch (P.PRINT_MODE) {
      case 0: return "Text";
      case 1: return "Graph";
      case 2: return "CSV buffer";
      default: return "Unknown";
    }
    return "";
  }

private:
  const static int SPEED_MAP_MAX_SIZE = 16;
  const int EEPROM_PARAMS_OFFSET = 16 * 4 + 4;  //смещение начала блока параметров - место для калибровки датчика
  const int EEPROM_PARAMS_HEADER = 5555;

  Utils &util;
  QTRSensors &sens;
  boolean debugMode = true;
  int sensorBufSize = 0;
  SpeedMapEntry speedMap[SPEED_MAP_MAX_SIZE];
  long startRunTime = 0;  //время старта, нужно чтобы всегда задавалось при старте, чтобы работала карта скоростей
  int speedMapLength = 0;

  void validateParams();
  bool checkHeader() {
    int intValue = 0;
    EEPROM.get(EEPROM_PARAMS_OFFSET, intValue);
    return intValue == EEPROM_PARAMS_HEADER;
  }

  // int readPresetNo() {
  //   int intValue = 0;
  //   if (!checkHeader()) return -1;
  //   EEPROM.get(EEPROM_PARAMS_OFFSET + sizeof(EEPROM_PARAMS_HEADER), intValue);
  //   return constrain(intValue, 1, MAX_PRESET_NO);
  // }

  int getParamsOffset(int aPreset) {
    aPreset = constrain(aPreset, 1, MAX_PRESET_NO);
    // return EEPROM_PARAMS_OFFSET + sizeof(EEPROM_PARAMS_HEADER) + sizeof(presetNo) + (aPreset - 1) * sizeof(ParamNames);
    return EEPROM_PARAMS_OFFSET + sizeof(EEPROM_PARAMS_HEADER) + (aPreset - 1) * sizeof(ParamNames);
  }

  int getSpeedByMode(char mode);
  int getTurnSpeedByMode(char mode);
};
