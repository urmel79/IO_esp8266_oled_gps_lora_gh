#include "function_oled.hpp"

// Initialize the OLED display using Wire library
#if defined(OLED_0_96_IN)
  SSD1306Wire display(0x3c, sdaPin, sclPin); // for 0.96" displays
#elif defined(OLED_1_3_IN)
  SH1106Wire display(0x3c, sdaPin, sclPin); // for 1.3" displays
#endif

bool connectOLEDiic() {
  bool status;

  // choose between 100 kHz and 400 kHz I2C clock rate
  // Important: clock rate has to comply with the values of the pullup resistors
  Wire.setClock(100000);

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

void printOLED_values_str(int line, String identifier, String value) {
  String str_display = identifier+value;
  display.drawString(0, line, str_display);
}

void printOLED_values_flt(int line, String identifier, double value, uint flt_positions, uint precision) {
  char str_value[21];     //result string 20 positions + \0 at the end

  if (flt_positions < precision) precision = flt_positions + 2; // sanitize precision
  if (flt_positions > 20) flt_positions = 20; // validate flt_positions (max 20 digits)
  if (precision > 10) precision = 10; // validate precision (max 10 precision digits)
  dtostrf( value, flt_positions, precision, str_value ); // convert float/double to string with desired precision

  String str_display = identifier+str_value;
  display.drawString(0, line, str_display);
}




















//
