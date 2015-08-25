#pragma once

#include <string>
#include <strsafe.h>
#include <list>
#include "internal.h"
#include "command.h"

class I2CDisplay
{
public:
	void SetLine0(std::string data) { LineBuf[0] = data; Update(0); };
	void SetLine1(std::string data) { LineBuf[1] = data; Update(1); };
	void SetLine2(std::string data) { LineBuf[2] = data; Update(2); };
	void SetLine3(std::string data) { LineBuf[3] = data; Update(3); };
	I2CDisplay();
	~I2CDisplay();

private:
	static BYTE const deviceAddress = 0x27;

	static BYTE const lcdBacklight = 0x08;
	static BYTE const lcdEnable = 0x04;
	static BYTE const lcdReadWrite = 0x02;
	static BYTE const lcdRegisterSelect = 0x01;

	static BYTE const lcdClearDisplay = 0x01;
	static BYTE const lcdReturnHome = 0x02;
	static BYTE const lcdEntryModeSet = 0x04;
	static BYTE const lcdDisplayControl = 0x08;
	static BYTE const lcdCursorShift = 0x10;
	static BYTE const lcdFunctionSet = 0x20;
	static BYTE const lcdSetCGRAMAddress = 0x40;
	static BYTE const lcdSetDDRAMAddress = 0x80;

	// flags for display entry mode
	static BYTE const lcdEntryModeShiftDecrement = 0x00;
	static BYTE const lcdEntryModeShiftIncrement = 0x01;
	static BYTE const lcdEntryModeEntryRight = 0x00;
	static BYTE const lcdEntryModeEntryLeft = 0x02;

	// flags for display on/off control
	static BYTE const lcdDisplayControlOn = 0x04;
	static BYTE const lcdDisplayControlOff = 0x00;
	static BYTE const lcdDisplayControlCursorOn = 0x02;
	static BYTE const lcdDisplayControlCursorOff = 0x00;
	static BYTE const lcdDisplayControlBlinkOn = 0x01;
	static BYTE const lcdDisplayControlBlinkOff = 0x00;

	// flags for display/cursor shift
	static BYTE const lcdCursorShiftDisplayMove = 0x08;
	static BYTE const lcdCursorShiftCursorMove = 0x00;
	static BYTE const lcdCursorShiftMoveRight = 0x04;
	static BYTE const lcdCursorShiftMoveLeft = 0x00;

	// flags for function set
	static BYTE const lcdFunctionSet8BitMode = 0x10;
	static BYTE const lcdFunctionSet4BitMode = 0x00;
	static BYTE const lcdFunctionSet2LineMode = 0x80;
	static BYTE const lcdFunctionSet1LineMode = 0x00;
	static BYTE const lcdFunctionSet5x10Dots = 0x04;
	static BYTE const lcdFunctionSet5x8Dots = 0x00;

	std::string LineBuf[4];
	BYTE lcdInterfaceData = 0;
	bool IsInitialized = FALSE;
	bool InProgress = FALSE;
	HANDLE gEvent = nullptr;

	void I2CWriteIOPort();
	void WriteNibble(bool registerSelect, BYTE dataNibble);
	void WriteByte(bool registerSelect, BYTE dataByte);
	void Backlight(bool backlightOn);
	void ClearScreen();
	void Initialize();
	void LineSelect(unsigned int line);
	void Write(std::string str);
	void Update(unsigned int line);
	VOID ExecuteCommand(string name, std::list<string> *params);
	DWORD RunCommand(_In_ PCCommand Command);
};