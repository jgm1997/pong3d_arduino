#include "Timer.h"

Timer::Timer(uint8_t timer) {
    this->timer = timer;
}

// SETTERS
void Timer::setTime(uint16_t time_ms) {
	// Prescaling values, initial time and initial period
	uint16_t ps[] = {1, 8, 64, 256, 1024};
	double time0 = 4.0959;
	double period0 = 0.0625;

	// Calculate register value
	uint8_t cs;
	double max_time;

	for(cs = 0; cs < 5; cs++) {
		max_time = time0 * ps[cs];
		if(time_ms < (uint16_t) max_time) break;
	}
	
	double period = period0 * ps[cs];
	this->clock_select = cs + 1;
	this->output_compare = (uint16_t) ((double) time_ms * 1000 / period);
}

void Timer::setFunction(void (*function)()) {
	noInterrupts();
    this->function = function;
	interrupts();
}

void Timer::load() {
	noInterrupts();

	switch(this->timer) {
		case 1:
			TCCR1A = 0;
			TCCR1B = 0;
			
			// Configurar modo CTC
			TCCR1B |= (1 << WGM12);

			OCR1A = this->output_compare;

			// Configurar interrupciones
			TIMSK1 |= (1 << OCIE1A);

			// Configurar Pre-escalado ()
			TCCR1B |= this->clock_select;
			break;
		case 3:
			TCCR3A = 0;
			TCCR3B = 0;
			TCCR3B |= (1 << WGM32);
			OCR3A = this->output_compare;
			TIMSK3 |= (1 << OCIE3A);
			TCCR3B |= this->clock_select;
			break;
		case 4:
			TCCR4A = 0;
			TCCR4B = 0;
			TCCR4B |= (1 << WGM42);
			OCR4A = this->output_compare;
			TIMSK4 |= (1 << OCIE4A);
			TCCR4B |= this->clock_select;
			break;
		case 5:
			TCCR5A = 0;
			TCCR5B = 0;
			TCCR5B |= (1 << WGM52);
			OCR5A = this->output_compare;
			TIMSK5 |= (1 << OCIE5A);
			TCCR5B |= this->clock_select;
			break;
	}
	interrupts();
}

void Timer::action() {
    this->function();
}

// PRIVATE
uint8_t Timer::getBits(uint8_t timer) {
	switch(timer) {
		case 1: case 3: case 4: case 5:
			return 16;
		case 0: case 2:
			return 8;
	}
	return 0;
}