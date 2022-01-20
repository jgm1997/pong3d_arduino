/**
	@file LCD.cpp
	@author Alberto
	@date 02/03/2018
	@brief LCD library.
 */
 
#include <Arduino.h>
#include <stdint.h>

#ifndef LCD_H
#define LCD_H

#define LCD_DISPLAY_ROWS 2
#define LCD_DISPLAY_COLS 16
#define LCD_DISPLAY_SIZE LCD_DISPLAY_ROWS * LCD_DISPLAY_COLS

#define LCD_TL_LIMIT 0
#define LCD_TR_LIMIT LCD_TL_LIMIT + LCD_DISPLAY_COLS - 1
#define LCD_BL_LIMIT 0x40
#define LCD_BR_LIMIT LCD_BL_LIMIT + LCD_DISPLAY_COLS - 1

typedef enum {CURSOR_ON, CURSOR_OFF} lcd_cursor_state_t;
typedef enum {BLINK_ON,  BLINK_OFF } lcd_blink_state_t;

typedef enum {INSTR, DATA} lcd_send_mode_t;

typedef enum {NOT_LIMIT, TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT} lcd_display_limit_t;

class LCD {
public:
	// Initializations
	LCD(uint8_t pin_rs, uint8_t pin_rw, uint8_t pin_en,
		uint8_t pin_db4, uint8_t pin_db5, uint8_t pin_db6, uint8_t pin_db7);
		
	void init();

	// Print
	void print(const char* text);
	void printDelay(const char* text, uint16_t delay_ms);
	void printLeft(const char* text);

	void print(char c);
	void delete_char();
	void delete_row(uint8_t row);
	
	// Read
	char readChar(uint8_t row, uint8_t col);
	char* readString();
	char* readSecondRow();

	// Display control
	void on(lcd_cursor_state_t cursor_state, lcd_blink_state_t blink_state);
	void off();
	
	void clear();

	// Cursor control
	void moveCursor(uint8_t row, uint8_t col);
	void moveCursorRight();
	void moveCursorLeft();

	// Shift control
	void shiftDisplayRight();
	void shiftDisplayLeft();
	
	// Display limits
	lcd_display_limit_t displayLimit();
	
	// Custom characters
	void storeCharPattern(const uint8_t* bitmap, uint8_t index);
	void showCharPattern(uint8_t index);

private:
	void write4bits(uint8_t value);

	void send(lcd_send_mode_t mode, uint8_t value);
	void wait();
	
	uint8_t read_AC();
	
	void enable_pulse();

	uint8_t pin_rs; // LOW: instruction.  HIGH: data.
	uint8_t pin_rw; // LOW: write to LCD.  HIGH: read from LCD.
	uint8_t pin_en; // activated by a HIGH pulse.
	
	uint8_t pin_db4;
	uint8_t pin_db5;
	uint8_t pin_db6;
	uint8_t pin_db7;
};

#endif
