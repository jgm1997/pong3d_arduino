/**
	@file Keypad.h
	@author Alberto
	@date 20/02/2018
	@brief Keypad library.
 */
 
#include <Arduino.h>
#include <stdint.h>

#ifndef KEYPAD_H
#define KEYPAD_H

#define KEYPAD_NO_KEY 255

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define KEYPAD_REDS 4
#define KEYPAD_LEDS 8

#define KEYPAD_DELAY 10

typedef enum keypad_block {BLOCK, NO_BLOCK} keypad_block_t;

class Keypad {
public:
	Keypad();

	// Initializations
	void initKeypad(
		uint8_t pin_c0, uint8_t pin_c1, uint8_t pin_c2, uint8_t pin_c3,
		uint8_t pin_r0, uint8_t pin_r1, uint8_t pin_r2, uint8_t pin_r3
	);

	void initKeypadRed(
		uint8_t pin_kr0, uint8_t pin_kr1, uint8_t pin_kr2, uint8_t pin_kr3
	);

	void initLeds(
		uint8_t pin_l0, uint8_t pin_l1, uint8_t pin_l2, uint8_t pin_l3,
		uint8_t pin_l4, uint8_t pin_l5, uint8_t pin_l6, uint8_t pin_l7
	);

	// Keypad
	uint8_t readKeypad(keypad_block_t block);

	// LEDs
	void onLed  (uint8_t num_led);
	void offLed (uint8_t num_led);
	
private:
	// Pins
	uint8_t pin_row[KEYPAD_ROWS];
	uint8_t pin_col[KEYPAD_COLS];
	
	uint8_t pin_kred[KEYPAD_REDS];
	
	uint8_t pin_led[KEYPAD_LEDS];

	// Keypad control
	uint8_t prevKey;
	uint8_t pressed;
	
};

#endif /* KEYPAD_H */
