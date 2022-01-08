/**
	@file Keypad.cpp
	@author Alberto
	@date 20/02/2018
	@brief Keypad library.
 */
 
#include "Keypad.h"
#include <Arduino.h>

Keypad::Keypad() {
	this->pressed = KEYPAD_NO_KEY;
}

/*******************************************************************************
 ** Initializations
 ******************************************************************************/

void Keypad::initKeypad(
	uint8_t pin_c0, uint8_t pin_c1, uint8_t pin_c2, uint8_t pin_c3,
	uint8_t pin_r0, uint8_t pin_r1, uint8_t pin_r2, uint8_t pin_r3
){
	this->pin_col[0] = pin_c0;
	this->pin_col[1] = pin_c1;
	this->pin_col[2] = pin_c2;
	this->pin_col[3] = pin_c3;
	
	this->pin_row[0] = pin_r0;
	this->pin_row[1] = pin_r1;
	this->pin_row[2] = pin_r2;
	this->pin_row[3] = pin_r3;
	
	for(uint8_t i = 0; i < KEYPAD_COLS; i++) {
		pinMode(this->pin_col[i], OUTPUT);
	}
	
	for(uint8_t i = 0; i < KEYPAD_ROWS; i++) {
		pinMode(this->pin_row[i], INPUT_PULLUP);
	}
}

void Keypad::initKeypadRed(
	uint8_t pin_kr0, uint8_t pin_kr1, uint8_t pin_kr2, uint8_t pin_kr3
) {
	this->pin_kred[0] = pin_kr0;
	this->pin_kred[1] = pin_kr1;
	this->pin_kred[2] = pin_kr2;
	this->pin_kred[3] = pin_kr3;
	
	for(uint8_t i = 0; i < KEYPAD_ROWS; i++) {
		pinMode(this->pin_kred[i], INPUT_PULLUP);
	}
};

void Keypad::initLeds(
	uint8_t pin_l0, uint8_t pin_l1, uint8_t pin_l2, uint8_t pin_l3,
	uint8_t pin_l4, uint8_t pin_l5, uint8_t pin_l6, uint8_t pin_l7
){
	this->pin_led[0] = pin_l0;
	this->pin_led[1] = pin_l1;
	this->pin_led[2] = pin_l2;
	this->pin_led[3] = pin_l3;
	this->pin_led[4] = pin_l4;
	this->pin_led[5] = pin_l5;
	this->pin_led[6] = pin_l6;
	this->pin_led[7] = pin_l7;
	
	for(uint8_t i = 0; i < KEYPAD_LEDS; i++) {
		pinMode(pin_led[i], OUTPUT);
		this->offLed(i);
	}
}

/*******************************************************************************
 ** Keypad
 ******************************************************************************/

uint8_t Keypad::readKeypad(keypad_block_t block) {
	
	do {
		prevKey = pressed;
		pressed = KEYPAD_NO_KEY;

		// For each column
		for(uint8_t i = 0; i < KEYPAD_COLS; i++) {

			// Set current column to 0 and the rest to 1
			for(uint8_t j = 0; j < KEYPAD_COLS; j++) {
				if(i == j) {
					digitalWrite(this->pin_col[j], LOW);
				} else {
					digitalWrite(this->pin_col[j], HIGH);
				}
			}
			
			delay(KEYPAD_DELAY);

			// Check for all rows if any has a value of 0
			for(uint8_t j = 0; j < KEYPAD_ROWS; j++) {
				if(digitalRead(this->pin_row[j]) == LOW) {
					pressed = KEYPAD_COLS * j + i; // wrap 2D to 1D
					break;
				}
			}
		}

		// Red lower row
		for(uint8_t i = 0; i < KEYPAD_REDS; i++) {
			if(digitalRead(this->pin_kred[i]) == LOW) {
				pressed = KEYPAD_ROWS*KEYPAD_COLS + i;
				break;
			}
		}
		
	// Ignore loop if block state is NO_BLOCK
	} while(block == BLOCK && (pressed == KEYPAD_NO_KEY || prevKey == pressed));
	
	return pressed;
}


/*******************************************************************************
 ** LEDs
 ******************************************************************************/

void Keypad::onLed (uint8_t num_led) {
	if(num_led < KEYPAD_LEDS) {
		digitalWrite(pin_led[num_led], LOW);
	}
}

void Keypad::offLed(uint8_t num_led) {
	if(num_led < KEYPAD_LEDS) {
		digitalWrite(pin_led[num_led], HIGH);
	}
}