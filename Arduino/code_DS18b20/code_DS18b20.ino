#include <OneWire.h>

OneWire  ds(8);  //Connecting pin 8
 
void setup(void) {
  Serial.begin(9600);
}
 
void loop(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
   
  if ( !ds.search(addr)) //Search for 18b20 address sequence code stored in addr
  {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
   
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);//Read its address serial code and display it with the serial port
  }
 
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
  
  //Determine which model 18b20 by the first byte of ROM
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20"); 
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 
 
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // Start ds18b20 for temperature conversion, and store the result in internal 9-byte RAM.
   
  delay(1000);     //Conversion time is 1000ms to ensure sufficient conversion time
  
  present = ds.reset();
  ds.select(addr);   
  ds.write(0xBE);         //Reads 9 bytes in internal RAM
 
  Serial.print("  Data = ");
  Serial.print(present,HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {  
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
 
  //9 bytes of data are converted to temperature degrees
 
  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);//The accuracy is determined by the lower 4 bits, 
                               //because the lower 4 bits represent the decimals, 
                               //the lower 4 bits, and the lower 2^(-n) powers from high to low.
                               //So the left shift of the data can change the accuracy. The accuracy is 0.5,0.25,0.125,0.0625
    if (cfg == 0x00) raw = raw << 3;  //9 bit thermometer resolution, the maximum conversion time is 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; //10 bit thermometer resolution, the maximum conwersion time is 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; //11 bit thermometer resolution, the maximum conwersion time is 375 ms
    //The default is a 12-bit thermometer resolution with a maximum conversion time of 750 ms.
  }
  celsius = (float)raw / 16.0;//12 bit resolution ,1 bit respresent 0.0625，divide by 16 to get Celsius
  fahrenheit = celsius * 1.8 + 32.0;//Celsius to Fahrenheit
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, "); 
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
}
