#pragma once

// Define COMMAND and DATA LCD Rs (used by send method).
// ---------------------------------------------------------------------------
#define COMMAND                 0
#define DATA                    1
#define FOUR_BITS               2

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0B00000100  // Enable bit
#define Rw 0B00000010  // Read/Write bit
#define Rs 0B00000001  // Register select bit

// Default library configuration parameters used by class constructor with
// only the I2C address field.
// ---------------------------------------------------------------------------
/*!
@defined
@abstract   Enable bit of the LCD
@discussion Defines the IO of the expander connected to the LCD Enable
*/
#define EN 6  // Enable bit

/*!
@defined
@abstract   Read/Write bit of the LCD
@discussion Defines the IO of the expander connected to the LCD Rw pin
*/
#define RW 5  // Read/Write bit

/*!
@defined
@abstract   Register bit of the LCD
@discussion Defines the IO of the expander connected to the LCD Register select pin
*/
#define RS 4  // Register select bit

/*!
@defined
@abstract   LCD dataline allocation this library only supports 4 bit LCD control
mode.
@discussion D4, D5, D6, D7 LCD data lines pin mapping of the extender module
*/
#define D4 0
#define D5 1
#define D6 2
#define D7 3

/*!
@defined
@abstract   Defines the duration of the home and clear commands
@discussion This constant defines the time it takes for the home and clear
commands in the LCD - Time in microseconds.
*/
#define HOME_CLEAR_EXEC      2000

/*!
@defined
@abstract   Backlight off constant declaration
@discussion Used in combination with the setBacklight to swith off the
LCD backlight. @set setBacklight
*/
#define BACKLIGHT_OFF           0

/*!
@defined
@abstract   Backlight on constant declaration
@discussion Used in combination with the setBacklight to swith on the
LCD backlight. @set setBacklight
*/
#define BACKLIGHT_ON          255