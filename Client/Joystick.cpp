// 
// 
// 

#include "Joystick.h"

Joystick::Joystick(uint8_t pin_vrx, uint8_t pin_vry, uint8_t pin_sw) {

	this->pin_vrx = pin_vrx;
	this->pin_vry = pin_vry;
	this->pin_sw = pin_sw;
}

void Joystick::init() {

	pinMode(pin_vrx, INPUT);
	pinMode(pin_vry, INPUT);
	pinMode(pin_sw, INPUT_PULLUP);
}

int Joystick::PosX() {

	return (analogRead(pin_vrx) - 512);
}

int Joystick::PosY() {

	return (analogRead(pin_vry) - 512);
}

int Joystick::swStatus() {

	return (!digitalRead(pin_sw));
}
