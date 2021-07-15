// digital ding dong ditch
// by samy kamkar
// dec 7, 2014
//
// this arduino sketch connects to an Adafruit FONA (for GSM) and an RF transmitter (pin 9) -- when we receive a text message with the word "doorbell", we send an RF signal out to ring a doorbell based off a transmission we sniffed previously with RTL-SDR
//
// more details at http://samy.pl/dingdong

#define TX_PIN 9
#define BIT_PERIOD 700

long times[] = {
  0, 1500, 3000, 4500, 9200, 12200, 16100, 21400,
  26800, 29800, 35200, 41300, 43600, 50500, 53500,
  57400, 62000, 67300, 71900, 75700, 80300
};

#include <SoftwareSerial.h>

// You'll need to use my modified FONA library here:
// https://github.com/samyk/Adafruit_FONA_Library
#include "Adafruit_FONA.h"

#define FONA_RX 5 // was 2, but we're going to use that for an interrupt (RI)
#define FONA_TX 3
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(&fonaSS, FONA_RST);

int8_t last_sms;
int8_t sms_interrupt = 0;

void got_sms()
{
  sms_interrupt = 1;
}

void handle_sms()
{
  sms_interrupt = 0;

  // we got an interrupt, so check for new SMS
  // it's possible we got nothing as a phone call will also call this function
  //
  // don't call me bro

  // now let's compare with the number of SMS' we have
  int8_t smsnum = fona.getNumSMS();

  if (smsnum < 0) {
    Serial.println(F("G Could not read # SMS"));
  }
  else {
    Serial.print(smsnum); 
    Serial.println(F(" SMS's on SIM card!"));
  }

  // we got at least one new SMS!!!!
  if (last_sms < smsnum)
  {

    // go through each sms
    while (last_sms++ != smsnum)
    {
      // read this SMS and check for the word "doorbell"
      Serial.print(F("\n\rReading SMS #")); 
      Serial.println(last_sms);

      // Retrieve SMS sender address/phone number.
      if (! fona.getSMSSender(last_sms, replybuffer, 250)) {
        Serial.println("Failed!");
        break;
      }
      Serial.print(F("FROM: "));
      Serial.println(replybuffer);

      // Retrieve SMS value.
      uint16_t smslen;
      if (! fona.readSMS(last_sms, replybuffer, 250, &smslen)) { // pass in buffer and max len!
        Serial.println("Failed!");
        break;
      }
      Serial.print(F("***** SMS #")); 
      Serial.print(last_sms); 
      Serial.print(" ("); 
      Serial.print(smslen); 
      Serial.println(F(") bytes *****"));
      Serial.println(replybuffer);
      Serial.println(F("*****"));

      // now, should this SMS ring the doorbell??
      // we use the C function `strstr` because we're dealing with a character array,
      // not the typical arduino "string" object
      if (strstr(replybuffer, "doorbell\0") != NULL)
      {
        ring_bell();
      }
    }

    // save our sms number
    last_sms = smsnum;
  }
}

void setup()
{
  pinMode(TX_PIN, OUTPUT);

  // attach interrupt to pin 2 (interrupt 0) for ring interrupt (when we recv SMS)
  attachInterrupt(0, got_sms, LOW);

  Serial.begin(115200);
  Serial.println("");
  Serial.println(F("Digital Ding Dong Ditch"));
  Serial.println(F("by Samy Kamkar - https://twitter.com/samykamkar"));
  Serial.println(F("Initializing....(May take a few seconds)"));


  // See if the FONA is responding
  while (! fona.begin(4800)) {  // make it slow so its easy to read!
    Serial.println(F("Couldn't find FONA"));
    delay(1000);
  }
  Serial.println(F("FONA is OK"));

  // call our RI (ring interrrupt) upon new SMS messages!
  fona.setSMSInterrupt(1);

  // Print SIM card IMEI number.
  char imei[15] = { 0 }; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: ");
    Serial.println(imei);
  }

  // set the number of SMS's we currently have in EEPROM
  int8_t smsnum = fona.getNumSMS();

  if (smsnum < 0) {
    Serial.println(F("Could not read # SMS"));
  }
  else {
    Serial.print(smsnum);
    Serial.println(F(" SMS's on SIM card!"));
    // save our sms number in EEPROM
    last_sms = smsnum;
  }

}

void ring_bell()
{
  Serial.println("Ringing!\n");

  // the actual doorbell sends our signal 12 times, so we'll emulate it
  for (int j = 0; j < 12; j++)
  {
    single_ring();
  }
}

void single_ring()
{
  // go through each "1" bit
  for (int i = 1; i < sizeof(times) / sizeof(long); i++)
  {
    // send a "1" for our BIT_PERIOD which is around 700-800us 
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(BIT_PERIOD);
    digitalWrite(TX_PIN, LOW);
    // calculate microseconds (us)
    delayMicroseconds(times[i] - times[i - 1] - BIT_PERIOD);
  }
  delay(20);
}


void loop()
{
  // you'd think we'd just call handle_sms() from the interrupt, but the FONA will not behave properly if you access it from an interrupt, so we simply set a flag instead
  if (sms_interrupt)
    handle_sms();
  delay(1000);
}
