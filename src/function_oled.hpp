#ifndef function_oled_hpp
#define function_oled_hpp

// Libs for OLED display
#include <Wire.h>
//#include "SSD1306Wire.h" // legacy include: #include "SSD1306.h"
#include "SH1106Wire.h" /* legacy include: #include "SH1106.h"*/

/*
 * WeMos D1 Mini / NodeMCU I2C bus uses pins:
 * D1 = SCL
 * D2 = SDA
 *
 * Deauther Oled V5 I2C bus uses pins:
 * D1 = SDA
 * D2 = SCL
*/
#if defined(WEMOS_D1_MINI_PRO) || defined(NODE_MCU_V2)
  const int sdaPin = D2;
  const int sclPin = D1;
#elif defined(DEAUTHER_OLED)
  const int sdaPin = D1;
  const int sclPin = D2;
#endif

bool connectOLEDiic();

void printOLED_str(int line, String str);

void printOLED_begin();
void printOLED_end();
void printOLED_values(int line, String identifier, String value);

#endif // function_oled_hpp
