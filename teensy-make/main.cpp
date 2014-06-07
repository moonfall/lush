#include "WProgram.h"

#if 1
extern "C" int main(void)
{
	// To use Teensy 3.0 without Arduino, simply put your code here.
	// For example:

	pinMode(13, OUTPUT);
	while (1) {
		digitalWriteFast(13, HIGH);
		delay(2500);
		digitalWriteFast(13, LOW);
		delay(500);
	}

}
#else
void setup()
{
	usb_init();
	Delay(5000);
	Serial.begin(38400);
	pinMode(13, OUTPUT);
}
void loop()
{
	digitalWriteFast(13, HIGH);
	delay(500);
	digitalWriteFast(13, LOW);
	delay(500);
	Serial.print("x");
}

#endif
