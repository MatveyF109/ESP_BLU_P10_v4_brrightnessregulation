#pragma once
#include "Arduino.h"

//Debug types
enum PrintMode {
  Text,
  Graph,
  CSV
};

enum WorkMode {
  SerialPort,
  BluetoothPort,
  Both,
  BothBuffer
};


class Utils {
public:
  Utils(Stream &aSerialPort, Stream &aBluetoothPort);
  Utils(PrintMode aPrintMode, Stream &aSerialPort, Stream &aBluetoothPort);
  void setPrintMode(PrintMode aPrintMode);
  PrintMode getPrintMode();
  boolean isGraphDebug();


  void setWorkMode(WorkMode aWorkMode);
  void setTimeout(unsigned long timeout);
  void setDebugMode(boolean mode);

  boolean available();
  int read();
  int readInteger();
  float readFloat();
  String readStr();
  String fillChars(char ch, int cnt);
  String fullFillChars(String s, char ch, int cnt);  

  void info_(String s);
  void info(String s);
  void debug(String s);
  void debug_(String s);

  void info_(int i);
  void info(int i);
  void debug(int i);
  void debug_(int i);

  void info_(char c);
  void info(char c);
  void debug(char c);
  void debug_(char c);


  void plot(String s);
  void plot(int v1, int v2);
  void plot(int v1, int v2, int v3, int v4);
  String int2str(int val);
  String float2str(float val);
  size_t write(uint8_t c);
  size_t write(const uint8_t *buffer, size_t size);
  int maxVal(int a, int b);

  String getValueAtIndex(String data, char separator, int index, String ifNotFound);
  String getValueAtIndex(String data, char separator, int index);

  bool isTimeToPrintDebug() {
   return (millis() - lastDebugInfoTime > (printMode == PrintMode::Graph ? debugInfoPeriodGraph : debugInfoPeriodText));
  }

private:
  Stream &serialPort;
  Stream &bluetoothPort;
  PrintMode printMode = PrintMode::Text;
  WorkMode workMode = WorkMode::SerialPort;
  boolean debugMode = true;
  long lastDebugInfoTime = 0;
  int debugInfoPeriodText = 200;  //period in ms
  int debugInfoPeriodGraph = 10;  //period in ms  
  int buf[20000];
};
