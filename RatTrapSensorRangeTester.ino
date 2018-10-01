/*
    A range tester for the LoRat project - a
    Lo-Ra enabled rat trap sensor.
    Designed for a Arduino Uno attached to
    Lo-Ra radio via SPI and an LCD screen via I2C.

    Simply sends a random 4 digit number to the server and waits for the same
    number to be returned along with the remote RSSI.

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

#include <stdlib.h>

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

#include <SPI.h>
#include <RH_RF95.h>

#include <Bounce2.h>




Bounce pushbutton = Bounce();
#define PUSHBUTTON_PIN  4


#define MAX_REPLY_TRIES 3 // Try 3 times for a reply


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
  Serial.println("Sending test message");

  // Generate a four digit random number between 1001 and 9999
  long rand_number = random(1001, 10000);
  Serial.print("Random number to send = ");
  Serial.println(rand_number, DEC);
  char message[11] = "Test:";
  itoa( rand_number, message + 5, 10);
  message[10] = '\0';

  Serial.print("Sent - ");
  Serial.println(message);

  lcd.setCursor(0, 0);
  lcd.print("Sent:");
  lcd.setCursor(6, 0 );
  lcd.print(&(message[5]));
  rf95.send((uint8_t *)message, sizeof(message));


  delay(10);
  rf95.waitPacketSent();


  // Look for a response:
  bool got_reply = false;


  // Try three times to listen for a reply:
  for ( int attempt = 0; attempt < MAX_REPLY_TRIES; attempt++ ) {
    char buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.waitAvailableTimeout(1000))
    {
      if (rf95.recv(buf, &len))
      {

        // Now we have a reply. Check that it's the number we expect and
        // extract at the RSSI at the remote station.
        delay(1000);
        RH_RF95::printBuffer("Received: ", buf, len);
        Serial.println((char*)buf);
        Serial.print("Local RSSI: ");
        float local_rssi = rf95.lastRssi();
        Serial.println(local_rssi, DEC);
        got_reply = true;

        lcd.setCursor(0, 1);
        lcd.print("Rcvd: ");
        float remote_rssi = atof(&(buf[11]));
        buf[11] = '\0';
        long response = long(atoi(&(buf[4])));
        Serial.println( remote_rssi, DEC);
        Serial.println( response, DEC);
        lcd.setCursor(6, 1);
        lcd.print(response);
        if (  response != rand_number) {
          lcd.print("** Fail!");
        } else {
          lcd.print(" OK");

        }

        lcd.setCursor(0, 2);
        lcd.print("Local RSSI :");
        lcd.setCursor(12, 2);
        lcd.print(local_rssi);
        lcd.setCursor(0, 3);
        lcd.print("Remote RSSI:");
        lcd.setCursor(12, 3);
        lcd.print(remote_rssi);

        break;
      }
    }
  }


  if ( !got_reply) {
    lcd.setCursor(0, 1);
    lcd.print("Failed: no reply");
  }

}


void setup()
{
  int error;

  Serial.begin(9600);


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

  randomSeed(analogRead(0));

  setupLoRa();

  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
  pushbutton.attach(PUSHBUTTON_PIN);
  pushbutton.interval(50);

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

  if ( pushbutton.fell() ) {
    doTest();
  }



}
