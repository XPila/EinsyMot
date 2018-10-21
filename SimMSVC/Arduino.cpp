#include "Arduino.h"

//class HardwareSerial
void HardwareSerial::begin(unsigned long baud, uint8_t config)
{
}

size_t HardwareSerial::write(uint8_t)
{
	return 1;
}

int HardwareSerial::read()
{
	return -1;
}

HardwareSerial Serial = HardwareSerial();
HardwareSerial Serial1 = HardwareSerial();

extern "C" {
extern void sim_cycle(void);
extern void sim_init(void);
}

extern void setup(void);
extern void loop(void);

int main(int argc, void* argv)
{
	sim_init();
	setup();
	while (1)
	{
		sim_cycle();
		loop();
	}
	return 0;
}

