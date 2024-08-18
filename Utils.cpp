#include "Print.h"
#include "Arduino.h"
#include "Utils.h"

Utils::Utils(Stream &aSerialPort, Stream &aBluetoothPort)
  : serialPort(aSerialPort), bluetoothPort(aBluetoothPort) {
}

Utils::Utils(PrintMode aPrintMode, Stream &aSerialPort, Stream &aBluetoothPort)
  : serialPort(aSerialPort), bluetoothPort(aBluetoothPort) {
  printMode = aPrintMode;
}

void Utils::setWorkMode(WorkMode aWorkMode) {
  workMode = aWorkMode;
}


void Utils::setTimeout(unsigned long timeout) {
  serialPort.setTimeout(timeout);
  bluetoothPort.setTimeout(timeout);
}

void Utils::setPrintMode(PrintMode aPrintMode) {
  // info("here");
  // info(aPrintMode);
  printMode = aPrintMode;
  // info("here2");
}

PrintMode Utils::getPrintMode() {
  return printMode;
}

boolean Utils::isGraphDebug() {
  return printMode == PrintMode::Graph;
}

void Utils::setDebugMode(boolean mode) {
  debugMode = mode;
}


void Utils::info(String s) {
  if (printMode == PrintMode::Text || printMode == PrintMode::Graph) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both) {
      serialPort.println(s);
    }
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
      bluetoothPort.println(s);
    }
  }
}

void Utils::info_(String s) {
  //Serial.println("here");
  if (printMode == PrintMode::Text || printMode == PrintMode::Graph) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both)
      serialPort.print(s);
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
      bluetoothPort.print(s);
  }
}

void Utils::debug(String s) {
  if (!debugMode) return;

  // if (USE_COMMA && printMode == PrintMode::Graph) {
  //   s.replace(",", ";");
  //   s.replace(".", ",");
  // }

  // if (millis() - lastDebugInfoTime > (printMode == PrintMode::Graph ? debugInfoPeriodGraph : debugInfoPeriodText)) {
  if (printMode == PrintMode::Text || printMode == PrintMode::Graph) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both)
      serialPort.println(s);
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
      bluetoothPort.println(s);
  }
  lastDebugInfoTime = millis();
  // }
  // info(s);
}

void Utils::debug_(String s) {
  if (!debugMode) return;

  // if (USE_COMMA && printMode == PrintMode::Graph) {
  //   s.replace(",", ";");
  //   s.replace(".", ",");
  // }

  if (millis() - lastDebugInfoTime > printMode == PrintMode::Graph ? debugInfoPeriodGraph : debugInfoPeriodText) {
    if (printMode == PrintMode::Text || printMode == PrintMode::Graph) {
      if (workMode == WorkMode::SerialPort || WorkMode::Both)
        serialPort.print(s);
      if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
        bluetoothPort.print(s);
    }
    lastDebugInfoTime = millis();
  }
}

void Utils::info(int i) {
  info(String(i));
}

void Utils::info_(int i) {
  info_(String(i));
}

void Utils::debug(int i) {
  debug(String(i));
}

void Utils::debug_(int i) {
  debug_(String(i));
}


void Utils::info(char c) {
  if (printMode == PrintMode::Text || printMode == PrintMode::Graph) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both) {
      serialPort.println(c);
    }
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
      bluetoothPort.println(c);
    }
  }
}

void Utils::info_(char c) {
  if (printMode == PrintMode::Text || printMode == PrintMode::Graph) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both) {
      serialPort.print(c);
    }
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
      bluetoothPort.print(c);
    }
  }
}

// void Utils::debug(char c) {
//   if (debugMode)
//     info(c);
// }

// void Utils::debug_(char c) {
//   if (debugMode)
//     info_(c);
// }


size_t Utils::write(uint8_t c) {
  size_t result = 0;
  if (workMode == WorkMode::SerialPort || WorkMode::Both)
    result = serialPort.write(c);
  if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
    result = bluetoothPort.write(c);
  return result;
}

size_t Utils::write(const uint8_t *buffer, size_t size) {
  size_t result = 0;
  if (workMode == WorkMode::SerialPort || WorkMode::Both)
    result = serialPort.write(buffer, size);
  if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
    result = bluetoothPort.write(buffer, size);
  return result;
}

boolean Utils::available() {
  if ((workMode == WorkMode::SerialPort || WorkMode::Both) && serialPort.available()) return true;
  if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
    return bluetoothPort.available();
}

int Utils::read() {
  if ((workMode == WorkMode::SerialPort || WorkMode::Both) && serialPort.available())
    return serialPort.read();
  if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
    return bluetoothPort.read();
}

int Utils::readInteger() {
  int v = -1;
  for (;;) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both) {
      if (serialPort.available()) {
        v = serialPort.parseInt();
        break;
      }
    }
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
      if (bluetoothPort.available()) {
        v = bluetoothPort.parseInt();
        break;
      }
    }
  }
  delay(100);
  if (workMode == WorkMode::SerialPort || WorkMode::Both)
    serialPort.read();
  if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
    bluetoothPort.read();
  return v;
}

float Utils::readFloat() {
  float v = -1;
  while (true) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both) {
      if (serialPort.available()) {
        v = serialPort.parseFloat();
        break;
      }
    }
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
      if (bluetoothPort.available()) {
        v = bluetoothPort.parseFloat();
        break;
      }
    }
    delay(10);
  }
  delay(100);
  if (workMode == WorkMode::SerialPort || WorkMode::Both)
    serialPort.read();
  if (workMode == WorkMode::BluetoothPort || WorkMode::Both)
    bluetoothPort.read();

  return v;
}

String Utils::readStr() {
  boolean serialWasRead = false;
  boolean btWasRead = false;
  delay(100);
  String s;
  serialPort.read();
  bluetoothPort.read();
  while (true) {
    if (serialPort.available() > 0) {
      serialWasRead = true;
      break;
    };
    if (bluetoothPort.available() > 0) {
      btWasRead = true;
      break;
    };
  }
  if (serialWasRead)
    s = serialPort.readString();
  else
    s = bluetoothPort.readString();
  delay(100);
  if (serialWasRead)
    serialPort.read();
  else
    bluetoothPort.read();
  return s;
}

String Utils::fillChars(char ch, int cnt) {
  String res;
  for (int i = 0; i < cnt; i++) {
    res += ch;
  }
  return res;
}

String Utils::fullFillChars(String s, char ch, int cnt) {
  if (s.length() >= cnt) return s;
  String res;

  for (int i = s.length(); i < cnt; i++) {
    res += ch;
  }
  return s + res;
}

// String Utils::int2str(int val) {
//   char buff[10];
//   sprintf(buff, "%6d", val);
//   return buff;
// }

void Utils::plot(String s) {
  // if (USE_COMMA) {
  //   s.replace(",", ";");
  //   s.replace(".", ",");
  // }
  info(s);

  // if (printMode == PrintMode::Graph) {
  //   if (workMode == WorkMode::SerialPort || WorkMode::Both)
  //     serialPort.println(s);
  //   if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
  //     bluetoothPort.println(s);
  //   }
  // }
}

void Utils::plot(int v1, int v2) {
  if (printMode == PrintMode::Graph) {
    if (workMode == WorkMode::SerialPort || WorkMode::Both)
      serialPort.println(String(v1) + " " + String(v2));
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
      bluetoothPort.println(String(v1) + " " + String(v2));
    }
  }
}

void Utils::plot(int v1, int v2, int v3, int v4) {
  if (printMode == PrintMode::Graph) {
    String s = String(v1) + " " + String(v2) + " " + String(v3) + " " + String(v4);
    if (workMode == WorkMode::SerialPort || WorkMode::Both)
      serialPort.println(s);
    if (workMode == WorkMode::BluetoothPort || WorkMode::Both) {
      bluetoothPort.println(s);
    }
  }
}

String Utils::float2str(float val) {  //, byte decimals
  char buff[10];
  dtostrf(val, 6, 2, buff);
  return buff;
}

int Utils::maxVal(int a, int b) {
  return a > b ? a : b;
}

String Utils::int2str(int val) {
  char buff[10];
  sprintf(buff, "%6d", val);
  return buff;
}

String Utils::getValueAtIndex(String data, char separator, int index) {
  return getValueAtIndex(data, separator, index, "");
}


String Utils::getValueAtIndex(String data, char separator, int index, String ifNotFound) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : ifNotFound;
}



// String float2strComma(float val) {
//   return float2strComma(val, 2);
// }


// String float2strComma(float val, byte decimals) {
//   String s = float2str(val, decimals);
//   s.replace(".", ",");
//   return s;
// }


// String Utils::binToStr(uint16_t number) {
//   char binstr[] = "0000000000000000";
//   uint8_t i = 0;
//   uint16_t n = number;

//   while (n > 0 && i < 16) {
//     binstr[16 - 1 - i] = n % 2 + '0';
//     ++i;
//     n /= 2;
//   }
//   return binstr;
// }
