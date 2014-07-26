#include "WProgram.h"

void setup()
{
	setup();
}

void loop()
{
	digitalWriteFast(13, HIGH);
	delay(500);
	digitalWriteFast(13, LOW);
	delay(500);
}

