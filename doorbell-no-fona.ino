// digital ding dong ditch
// by samy kamkar
// dec 7, 2014
//
// this arduino sketch connects to an RF transmitter (pin 9) and sends a signal every 30 seconds to trigger a wireless doorbell based off a transmission we sniffed previously with RTL-SDR
//
// more details at http://samy.pl/dingdong

#define TX_PIN 9
#define BIT_PERIOD 700
#define TIMES 21

float times[TIMES] = {
 .0000, .0015, .0030, .0045,  .0092, .0122, .0161, .0214,
 .0268, .0298, .0352, .0413,  .0436, .0505, .0535, .0574,
 .0620, .0673, .0719, .0757,  .0803
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
	int last = 0;

	// go through each "1" bit
	for (int i = 0; i < TIMES; i++)
	{
		// calculate microseconds (us)
		int us = times[i] * 1000000;
		if (i != 0)
			delayMicroseconds(us - last - BIT_PERIOD);

		// send a "1" for our BIT_PERIOD which is around 700-800us 
		digitalWrite(TX_PIN, HIGH);
		delayMicroseconds(BIT_PERIOD);
		digitalWrite(TX_PIN, LOW);

		last = us;
	}
	delay(20);
}


void loop()
{
	ring_bell();
  delay(30000);
}

