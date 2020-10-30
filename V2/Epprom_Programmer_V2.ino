// Thank you to the members of arduino: J-M-L for the hex function. jremington for making me read the datasheet. chucktodd for writing the read function in 2015
#include <SPI.h>
#define csPIN 10 // Chip Select Pin aka Slave Select
#define posPIN 9 // Pin to turn on vcc
#define negPIN 8 // Pin to turn on grnd

//opcodes
#define WREN  6
#define WRDI  4
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2
#define EEPROMSIZE 256

//Function to turn ASCII into Hex
int8_t intValueOfHexaKey(char c)
{
  if ((c >= '0') && (c <= '9')) return c - '0';
  if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
  return -1; // error should not happen though...
}

// start of SPI read
uint8_t readByteAt(uint8_t cs, uint16_t adr)
{
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  uint8_t b = highByte(adr);
  b = b << 3;  //move address bit 8 to bit 11
  b = b | READ;  // read command
  SPI.transfer(b); // send cmd + address bit 8
  SPI.transfer(lowByte(adr)); // low byte of address
  b = SPI.transfer(0); // read the actual byte
  digitalWrite(cs, HIGH);
  SPI.endTransaction();
  return b;
}
//end

// start of SPI write
//uint8_t writeByteAt(uint8_t cs, uint16_t adr, char d)
uint8_t writeByteAt(uint8_t cs, uint16_t adr, char d)
{
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  SPI.transfer(WREN); //write enable
  digitalWrite(cs, HIGH);
  delay(10);
  digitalWrite(cs, LOW);
  uint8_t a = highByte(adr);
  a = a << 3;  //move address bit 8 to bit 11
  a = a | WRITE;  // write command
  SPI.transfer(a); // send cmd + address bit 8
  SPI.transfer(lowByte(adr)); // low byte of address
  a = SPI.transfer(d); // read the actual byte
  digitalWrite(cs, HIGH);
  delay(15); // delay because I was to lazy to make read if it was done writing the byte you could use the op code to check..
  SPI.endTransaction();

  return a;
}
//end

//setup 
void setup()
{
  Serial.begin(9600);
  digitalWrite(csPIN, HIGH);
  pinMode(csPIN, OUTPUT);
  pinMode(posPIN, OUTPUT);
  digitalWrite(posPIN, HIGH);
  pinMode(negPIN, OUTPUT);
  digitalWrite(negPIN, LOW);
  SPI.begin();
  Serial.println("Send 1 to read EEprom or 2 to write");
  Serial.setTimeout(7000); //sets the readbyte to wait 7 seconds
}

//main loop
void loop()
{
  if (Serial.available() > 0)
  {
    char command = Serial.read(); //listening to the serial port
    if (command == '1')
    {
      digitalWrite(posPIN, LOW);
      digitalWrite(negPIN, HIGH);
      delay(20);
      char ch[30];
      for (uint16_t i = 0; i < EEPROMSIZE; i++)
      {//how deep to read
        sprintf(ch, "%02X", readByteAt(csPIN, i));  
        Serial.print(ch);
      }
      digitalWrite(negPIN, LOW);
      digitalWrite(posPIN, HIGH);
    }
    else if (command == '2')
    {
      digitalWrite(posPIN, LOW);
      digitalWrite(negPIN, HIGH);
      delay(20);
      Serial.println("paste bin now you have 7 seconds");
      char dump[EEPROMSIZE] = "0";
      char readData[EEPROMSIZE * 2] = "0"; //The character array is used as buffer to read into.
      Serial.readBytes(readData, EEPROMSIZE * 2); //Reads in the pasted bin from the serial port
      for (int i = 0; i < EEPROMSIZE; i++)
      {
        dump[i] = intValueOfHexaKey(readData[i * 2 + 1]) + 16 * intValueOfHexaKey(readData[i * 2]); //math to turn the 2 hex values into 1 byte
        Serial.print(dump[i]);  //just for debug its going to print the ascii chars of each byte in the monitor 
      }
      char ch[30];
      for (uint16_t i = 0; i < EEPROMSIZE; i++)
      {//how deep to write
        sprintf(ch, "%02X", writeByteAt(csPIN, i, dump[i]));
      }

      delay(500);
      digitalWrite(negPIN, LOW);
      digitalWrite(posPIN, HIGH);
    }
  }
  delay(10);
}
