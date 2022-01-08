/**
	@file LCD.cpp
	@author Alberto
	@date 02/03/2018
	@brief LCD library.
 */
 
#include "LCD.h"

/*******************************************************************************
 ** Initializations
 ******************************************************************************/
 
LCD::LCD(
	uint8_t pin_rs, uint8_t pin_rw, uint8_t pin_en,
	uint8_t pin_db4, uint8_t pin_db5, uint8_t pin_db6, uint8_t pin_db7
){
	// save pins
	this->pin_rs = pin_rs;
	this->pin_rw = pin_rw;
	this->pin_en = pin_en;

	this->pin_db4 = pin_db4;
	this->pin_db5 = pin_db5;
	this->pin_db6 = pin_db6;
	this->pin_db7 = pin_db7;

	pinMode(pin_rs, OUTPUT);
	pinMode(pin_rw, OUTPUT);
	pinMode(pin_en, OUTPUT); digitalWrite(pin_en, LOW);
}

void LCD::init()
{
	/* See page 46 */
	
	delay(15);

	// Function set: 8 bits
	write4bits(0b0011);
	delayMicroseconds(5000);

	// Function set: 8 bits
	write4bits(0b0011);
	delayMicroseconds(150);

	// Function set: 8 bits
	write4bits(0b0011);
	wait();

	// Function set: 4 bits
	write4bits(0b0010);
	wait();

	// Funtion set: 4 bits, 2 lines and character font 5x8
	send(INSTR, 0b00101000);

	// Display off
	off();

	// Display clear
	clear();

	// Entry mode set: Cursor move direction increment and no shifting
	send(INSTR, 0b00000110);
	
	
	/* INITIALIZATION FINISHED */
	
	// Display on
	on(CURSOR_ON, BLINK_ON);
	
}


/*******************************************************************************
 ** Print
 ******************************************************************************/
 
void LCD::print(char c) {
	lcd_display_limit_t limit = displayLimit();
	
	if(limit != BOTTOM_RIGHT) {
		send(DATA, c);
		wait();

		if(limit == TOP_RIGHT) moveCursor(2,1);
	}
}

void LCD::print(const char* text) {
	while(*text != '\0'){
		print(*text);
		text++;
	}
}

void LCD::printDelay(const char* text, uint16_t delay_ms) {
	while(*text != '\0'){
		print(*text);
		text++;
		delay(delay_ms);
	}
}

void LCD::printLeft(const char* text) {
	
	// Formatting the format string (white space padding left)
	// i.e.: for LCD_DISPLAY_SIZE = 32 -> "%32s"
	char format_str[6];
	sprintf(format_str, "%%%ds", LCD_DISPLAY_SIZE);
	
	// Formatting the string
	char string[LCD_DISPLAY_SIZE + 1];
	sprintf(string, format_str, text);

	print(string);
}

void LCD::delete_char() {
	lcd_display_limit_t limit = displayLimit();
	
	if(limit != TOP_LEFT) {
		if(limit == BOTTOM_LEFT) moveCursor(1,17);

		moveCursorLeft();
		send(DATA, ' ');
		wait();
		moveCursorLeft();
	}
}

/*******************************************************************************
 ** Read
 ******************************************************************************/

char LCD::readChar(uint8_t row, uint8_t col) {
	
	// Save current DDRAM address
	uint8_t ddram_address = read_AC();

	// Set DDRAM
	uint8_t address = (col-1) | ((row-1) << 6);
	uint8_t instruction = 0b10000000 | address;
	send(INSTR, instruction);

	// Character to write
	char c = '\0';
	
	// Set data pins to input
	pinMode(this->pin_db4, INPUT);
	pinMode(this->pin_db5, INPUT);
	pinMode(this->pin_db6, INPUT);
	pinMode(this->pin_db7, INPUT);
	
	// Set RS and RW pins to 1
	digitalWrite(this->pin_rs, HIGH);
	digitalWrite(this->pin_rw, HIGH);
	
	uint8_t i = 8;

	// Read first 4 bits
	digitalWrite(this->pin_en, HIGH);
	c |= (digitalRead(this->pin_db7) == HIGH) << --i;
	c |= (digitalRead(this->pin_db6) == HIGH) << --i;
	c |= (digitalRead(this->pin_db5) == HIGH) << --i;
	c |= (digitalRead(this->pin_db4) == HIGH) << --i;
	digitalWrite(this->pin_en, LOW);
	delayMicroseconds(1);

	// Read last 4 bits
	digitalWrite(this->pin_en, HIGH);
	c |= (digitalRead(this->pin_db7) == HIGH) << --i;
	c |= (digitalRead(this->pin_db6) == HIGH) << --i;
	c |= (digitalRead(this->pin_db5) == HIGH) << --i;
	c |= (digitalRead(this->pin_db4) == HIGH) << --i;
	digitalWrite(this->pin_en, LOW);
	delayMicroseconds(100);
	
	wait();
	
	// Restore DDRAM address
	instruction = 0b10000000 | ddram_address;
	send(INSTR, instruction);
	
	return c;
}
 
char* LCD::readString(){
	
	char* string = (char*) malloc(LCD_DISPLAY_SIZE + 1);
	
	char current = '\0';
	uint8_t i = 0, row, col;
	
	// Read until LCD limit or first white space (break)
	while(i < LCD_DISPLAY_SIZE) {
		row = (i / LCD_DISPLAY_COLS) + 1;
		col = (i % LCD_DISPLAY_COLS) + 1;
		current = readChar(row, col);
		if(current == ' ') break;
		string[i++] = current;
	}
	
	// Null terminated string
	string[i] = '\0';
	
	return string;
}

/*******************************************************************************
 ** Display control
 ******************************************************************************/
 
void LCD::on(lcd_cursor_state_t cursor_state, lcd_blink_state_t blink_state)
{
	uint8_t instruction = 0b00001100;

	if(cursor_state == CURSOR_ON) {
		instruction |= 0b00000010;
	}
	
	if(blink_state == BLINK_ON) {
		instruction |= 0b00000001;
	}
	
	send(INSTR, instruction);
}

void LCD::off()
{
	send(INSTR, 0b00001000);
}

void LCD::clear()
{
	send(INSTR, 0b00000001);
}


/*******************************************************************************
 ** Cursor control
 ******************************************************************************/
 
void LCD::moveCursor(uint8_t row, uint8_t col)
{
	uint8_t address = (col-1) | ((row-1) << 6);
	uint8_t instruction = 0b10000000 | address;
	send(INSTR, instruction);
}

void LCD::moveCursorRight()
{
	send(INSTR, 0b00010100);
}

void LCD::moveCursorLeft()
{
	send(INSTR, 0b00010000);
}


/*******************************************************************************
 ** Shift control
 ******************************************************************************/
 
void LCD::shiftDisplayRight()
{
	send(INSTR, 0b00011100);
}

void LCD::shiftDisplayLeft()
{
	send(INSTR, 0b00011000);
}


/*******************************************************************************
 ** Display limits
 ******************************************************************************/

lcd_display_limit_t LCD::displayLimit() {
	uint8_t ac = read_AC();
	switch(ac) {
		case LCD_TL_LIMIT:   return TOP_LEFT;
		case LCD_TR_LIMIT:   return TOP_RIGHT;
		case LCD_BL_LIMIT:   return BOTTOM_LEFT;
		case LCD_BR_LIMIT+1: return BOTTOM_RIGHT;
	}
	return NOT_LIMIT;
}

/*******************************************************************************
 ** Custom characters
 ******************************************************************************/

void LCD::storeCharPattern(const uint8_t* bitmap, uint8_t index) {

	// Save current DDRAM address
	uint8_t ddram_address = read_AC();
	
	// Fill character address in instruction and set CGRAM
	uint8_t instruction = 0b01000000 | (index << 3);
	send(INSTR, instruction);
	
	for(uint8_t i = 0; i < 8; i++) {
		// Write data to CGRAM
		send(DATA, bitmap[i]);
	}

	// Restore DDRAM address
	instruction = 0b10000000 | ddram_address;
	send(INSTR, instruction);
	
}

void LCD::showCharPattern(uint8_t index) {
	print(index);
}

/******************************************************************************/
/** PRIVATE FUNCTIONS                                                        **/
/******************************************************************************/

void LCD::write4bits(uint8_t value)
{
	// Set data pins as output
	pinMode(this->pin_db4, OUTPUT);
	pinMode(this->pin_db5, OUTPUT);
	pinMode(this->pin_db6, OUTPUT);
	pinMode(this->pin_db7, OUTPUT);

	// Set RS and RW to send instruction
	digitalWrite(this->pin_rs, LOW);
	digitalWrite(this->pin_rw, LOW);
	
	// Write value to data pins
	uint8_t i = 4;
	digitalWrite(this->pin_db7, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db6, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db5, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db4, (value & (1 << --i)) ? HIGH : LOW);

	enable_pulse();
}

void LCD::send(lcd_send_mode_t mode, uint8_t value)
{
	// Set data pins as output
	pinMode(this->pin_db4, OUTPUT);
	pinMode(this->pin_db5, OUTPUT);
	pinMode(this->pin_db6, OUTPUT);
	pinMode(this->pin_db7, OUTPUT);

	// Set RS and RW to send instruction/data depending on mode
	digitalWrite(this->pin_rs, mode == DATA ? HIGH : LOW);
	digitalWrite(this->pin_rw, LOW);

	// Write first nibble (most significant) from value to data pins
	uint8_t i = 8;
	digitalWrite(this->pin_db7, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db6, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db5, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db4, (value & (1 << --i)) ? HIGH : LOW);

	enable_pulse();

	// Write second nibble (less significant) from value to data pins
	digitalWrite(this->pin_db7, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db6, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db5, (value & (1 << --i)) ? HIGH : LOW);
	digitalWrite(this->pin_db4, (value & (1 << --i)) ? HIGH : LOW);

	enable_pulse();

	wait();
}

void LCD::wait()
{
	uint8_t busy;

	// Set data pins as input
	pinMode(this->pin_db4, INPUT);
	pinMode(this->pin_db5, INPUT);
	pinMode(this->pin_db6, INPUT);
	pinMode(this->pin_db7, INPUT);

	// Set RS and RW to read busy flag
	digitalWrite(this->pin_rs, LOW);
	digitalWrite(this->pin_rw, HIGH);

	do {
		digitalWrite(pin_en, HIGH);
		
		// Read busy flag (db7)
		busy = digitalRead(this->pin_db7);
		
		enable_pulse();
	} while (busy == HIGH);
}

uint8_t LCD::read_AC() {

	uint8_t ac = 0b0000000;
	
	// Set data pins as input
	pinMode(this->pin_db4, INPUT);
	pinMode(this->pin_db5, INPUT);
	pinMode(this->pin_db6, INPUT);
	pinMode(this->pin_db7, INPUT);

	// Set RS and RW to read address
	digitalWrite(this->pin_rs, LOW);
	digitalWrite(this->pin_rw, HIGH);

	uint8_t i = 7;
	
	// First 4 bits
	digitalWrite(pin_en, HIGH);
	ac |= (digitalRead(this->pin_db6) == HIGH) << --i;
	ac |= (digitalRead(this->pin_db5) == HIGH) << --i;
	ac |= (digitalRead(this->pin_db4) == HIGH) << --i;
	digitalWrite(pin_en, LOW);
	delayMicroseconds(1);
	
	// Next 4 bits
	digitalWrite(pin_en, HIGH);
	ac |= (digitalRead(this->pin_db7) == HIGH) << --i;
	ac |= (digitalRead(this->pin_db6) == HIGH) << --i;
	ac |= (digitalRead(this->pin_db5) == HIGH) << --i;
	ac |= (digitalRead(this->pin_db4) == HIGH) << --i;
	digitalWrite(pin_en, LOW);
	delayMicroseconds(100);
	
	wait();
	
	return ac;
}

void LCD::enable_pulse()
{
	digitalWrite(pin_en, LOW);
	delayMicroseconds(1);
	digitalWrite(pin_en, HIGH);
	delayMicroseconds(1);
	digitalWrite(pin_en, LOW);
	delayMicroseconds(100);
}
