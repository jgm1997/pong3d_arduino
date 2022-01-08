// Joystick.h
#include <Arduino.h>

#ifndef _JOYSTICK_h
#define _JOYSTICK_h

class Joystick
{
public:
	
	Joystick(uint8_t pin_vrx, uint8_t pin_vry, uint8_t pin_sw);

	void init();
	int PosX();
	int PosY();
	int swStatus();


private:

	uint8_t pin_vrx;
	uint8_t pin_vry;
	uint8_t pin_sw;


};



#endif

