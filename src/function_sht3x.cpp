#include "function_sht3x.hpp"

// create only a global pointer to the class Adafruit_SHT31 (for later instantiation)
Adafruit_SHT31* ptr_sht31 = NULL;

bool function_SHTx_connect_iic() {
  bool status;

  // choose between 100 kHz and 400 kHz I2C clock rate
  // Important: clock rate has to comply with the values of the pullup resistors
  Wire.setClock(100000);

  // for defining the I2C pins pass in a Wire library object like &Wire
  Wire.begin(sdaPin, sclPin);
  ptr_sht31 = new Adafruit_SHT31(&Wire);

  ptr_sht31 = new Adafruit_SHT31();

  // 0x44: default i2c address of sht31
  // set to 0x45 for alternate i2c addr
  status = ptr_sht31->begin(0x44);
  if (! status) {
    Serial.println("[SHTx] Could not find a valid SHT3x sensor, check wiring");
  }

  return status;
}

// An ~3.0 °C temperature increase can be noted when heater is enabled
void function_SHTx_enable_heater(bool enable) {
  ptr_sht31->heater(enable);
}

bool function_SHTx_get_heater() {
  bool heater_enabled_state = ptr_sht31->isHeaterEnabled();

  Serial.print("[SHTx] Heater Enabled State: ");
  if (heater_enabled_state)
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");

  return heater_enabled_state;
}

float function_SHTx_get_temperature() {
  float t = ptr_sht31->readTemperature();

  return t;
}

float function_SHTx_get_humidity() {
  float h = ptr_sht31->readHumidity();

  return h;
}











//
