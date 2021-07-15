// digital ding dong ditch
// by samy kamkar
// dec 7, 2014
//
// this arduino sketch connects to an RF transmitter (pin 9) and sends a signal every 30 seconds to trigger a wireless doorbell based off a transmission we sniffed previously with RTL-SDR
//
// more details at http://samy.pl/dingdong

#define TX_PIN 9
#define BIT_PERIOD 700

long times[] = {
  0, 1500, 3000, 4500, 9200, 12200, 16100, 21400,
  26800, 29800, 35200, 41300, 43600, 50500, 53500,
  57400, 62000, 67300, 71900, 75700, 80300
};

void setup()
{
  pinMode(TX_PIN, OUTPUT);


  Serial.begin(115200);
  Serial.println("");
  Serial.println(F("Digital Ding Dong Ditch"));
  Serial.println(F("by Samy Kamkar - https://twitter.com/samykamkar"));
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
  ring_bell();
  delay(30000);
}
