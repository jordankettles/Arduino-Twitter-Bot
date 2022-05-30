#include <Wire.h>
#include <SPI.h>
//#include <TwitterApi.h>

int aht10_addr = 0x38;
int cs_pin = 5;

/**
 * maxTransfer sends a value to the MAX7219 LED screen.
 * Expects the screen to be in decode mode.
 */
void maxTransfer(uint8_t address, uint8_t value) {
  digitalWrite(cs_pin, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(cs_pin, HIGH);
}
/**
 * getTemperature gets the current temperature in celsius from
 * the aht10 sensor.
 */
float getTemperature() {
  int reading;
  float temp;
  char rawdata[6];
  Wire.beginTransmission(aht10_addr);
  Wire.write(0xac);
  Wire.write(0x33);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(150);
  
  Wire.requestFrom(aht10_addr, 6);
  for (int idx=0; idx<6; idx++) {
    rawdata[idx] = Wire.read();
  }
  reading = (rawdata[3] & 0x0F) << 16 | rawdata[4] << 8 | rawdata[5];
  temp = (reading / (float) 1048576) * 200 - 50;
  return temp;
}

void setup() {
  Wire.begin();
  Serial.begin(115200);

  pinMode(cs_pin, OUTPUT);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();

  // Flash all the LEDs.
  maxTransfer(0x0F, 0x01);
  delay(1000);
  maxTransfer(0x0F, 0x00);

  // Turn on decode mode so it's easy to display numbers.
  maxTransfer(0x0c, 0x01);
  maxTransfer(0x09, 0xff);

  maxTransfer(0x0b, 0x07);
  maxTransfer(0x0a, 0x08);
}

void loop() {
  float temp;
  Serial.println("Triggering measurement");
  temp = getTemperature();
  Serial.print(temp);
  Serial.println(" degrees");

  // Set the first 4 digits of the 8 digit screen to blank = 15.
  for(int screen_pos=8;screen_pos>4;screen_pos--) {
    maxTransfer(screen_pos, 15);
  }
  maxTransfer(4,  temp / 10); // Ten.
  maxTransfer(3, (( (int) temp) % 10) + 128); // One. + 128 turns on the decimal point for that number.
  maxTransfer(2, ( (int) (temp * 10)) % 10); // Tenths.
  maxTransfer(1, ( (int) (temp * 100)) % 10); // Hundredths.
  delay(2000);
}
