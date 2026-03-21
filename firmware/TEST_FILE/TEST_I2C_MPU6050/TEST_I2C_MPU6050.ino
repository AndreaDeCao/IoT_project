#include <Wire.h>

void setup() {
  Wire.begin();

  Serial.begin(9600);
  Serial.println("\nI2C Scanner");
}


void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ ) { // 7 BIT DEDICated to i2c adresses to adresses the devices that has this protocol communication 
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // error== 0 device found
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }

    else if (error == 4) { // error== 4 hardware problem with connections
      Serial.print("Unknow error at address 0x");
      if (address < 16) // adresses <16 means that i see 0x8 instead of 0x08 so in this situation i add a 0 to better visualize the adress
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000);           // wait 5 seconds for next scan
}