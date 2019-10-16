#include "function_oled.hpp"

// Initialize the OLED display using Wire library
//SSD1306Wire display(0x3c, sdaPin, sclPin);
SH1106Wire display(0x3c, sdaPin, sclPin);

bool connectOLEDiic() {
  bool status;

  status = display.init();
  if (!status) Serial.println("Could not find a valid OLED SSD1306/SH1106, check wiring!");

  display.flipScreenVertically();
  display.display();
  delay(200);
  display.clear();

  return status;
}

void printOLED_begin() {
  display.clear();

  // Sets the current font. Available default fonts:
  // ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
  display.setFont(ArialMT_Plain_10); //I like fonts

  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void printOLED_end() {
  display.display();
}

void printOLED_str(int line, String str) {
  display.drawString(0, line, str);
}

void printOLED_values(int line, String identifier, String value) {
  String str_display = identifier+value;
  display.drawString(0, line, str_display);
}
