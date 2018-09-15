/*
    A range tester for the LoRat project - a
    Lo-Ra enabled rat trap sensor.
    Designed for a Arduino Uno attached to
    Lo-Ra radio via SPI and an LCD screen via I2C.


    Pin Assignments:
     3 -> LoRa DOI0
     4 -> Push button (to ground)
     9 -> LoRa Reset
    10 -> LoRa Clock Select (NSS)
    11 -> LoRa MISO
    12 -> LoRa MOSI
    13 -> LoRa SCK
    SDA -> I2C LCD SDS
    SCL -> I2C LCD SCL

    Quentin McDonald
    September 2018
*/


#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

#include <SPI.h>
#include <RH_RF95.h>

#include <Bounce2.h>

Bounce pushbutton = Bounce();
#define PUSHBUTTON_PIN  4


#define RFM95_CS 10  // Clock select should be on 10
#define RFM95_RST 9  // Reset on 9
#define RFM95_INT 3  // Interrupt on 3

// Operating at 433 Mhz
#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display



// Configure the LoRa radio
void setupLoRa() {

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.println("Initializing LoRa radio");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }

  Serial.println("LoRa radio init OK");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }

  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);


  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  // Set to slow speed for longer range
  rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);
}


void doTest()
// Run the actual transmission test
{
  lcd.home();
  lcd.clear();
  lcd.print("Running Test");
  


  
}





void setup()
{
  int error;

  Serial.begin(9600);
  Serial.println("LCD...");

  while (! Serial);

  Serial.println("LoRat Range Tester: check for LCD");
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  if (error == 0) {
    Serial.println(" -- LCD found.");

  } else {
    Serial.println(" -- LCD NOT found.");
  } // if

  lcd.begin(16, 4); // initialize the lcd

  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("LoRat Range Tester");
  lcd.setCursor(0, 1);



  setupLoRa();

  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
  pushbutton.attach(PUSHBUTTON_PIN);
  pushbutton.interval(5);

  lcd.setCursor(0, 1);
  lcd.print("Radio Initialized OK");

  lcd.setCursor(0, 2);
  String message = "Freq. = ";
  String freq(RF95_FREQ);
  String mhz = " MHz";
  lcd.print(message + freq + mhz);

  lcd.setCursor(0, 3);
  lcd.print("Ready for test");

} // setup()

void loop() {

  pushbutton.update();

  int value = pushbutton.read();

  if ( value == LOW ) {
    doTest();
  }



}
