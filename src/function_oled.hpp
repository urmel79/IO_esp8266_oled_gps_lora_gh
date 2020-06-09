#ifndef function_oled_hpp
#define function_oled_hpp

// Libs for OLED display
#include <Wire.h>

#if defined(OLED_0_96_IN)
  #include <SSD1306Wire.h> // legacy include: #include "SSD1306.h" for 0.96" displays
#elif defined(OLED_1_3_IN)
  #include <SH1106Wire.h> // legacy include: #include "SH1106.h" for 1.3" displays
#endif

/*
 * WeMos D1 Mini / NodeMCU I2C bus uses pins:
 * D1 = SCL
 * D2 = SDA
 *
 * Deauther Oled V5 I2C bus uses pins:
 * D1 = SDA
 * D2 = SCL
*/
#if defined(WEMOS_D1_MINI_PRO) || defined(NODE_MCU_LORA_1) || defined(NODE_MCU_LORA_2)
  const int sdaPin = D2;
  const int sclPin = D1;
#elif defined(NODE_MCU_LORA_2_ESP32)
  const int sdaPin = 21;
  const int sclPin = 22;
#elif defined(DEAUTHER_OLED)
  const int sdaPin = D1;
  const int sclPin = D2;
#endif

bool connectOLEDiic();

void printOLED_begin();
void printOLED_end();

void printOLED_str(int line, String str);
void printOLED_values_str(int line, String identifier, String value);
void printOLED_values_flt(int line, String identifier, double value, uint flt_positions, uint precision, String unit);

#endif // function_oled_hpp
