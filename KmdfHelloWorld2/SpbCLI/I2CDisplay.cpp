#pragma once

#include <iostream>
#include <cwctype>
#include <ppltasks.h>
#include <assert.h>
#include <sstream>
#include "I2CDisplay.h"

using namespace std;

I2CDisplay::I2CDisplay()
{
    for(unsigned int n = 0; n < 4; n++)
    {
        LineBuf[0] = "";
    }
    lcdInterfaceData = lcdBacklight;
	gEvent = CreateEvent(nullptr, true, false, nullptr);
}

I2CDisplay::~I2CDisplay()
{
    ClearScreen();
    Backlight(FALSE);
	if (gEvent != nullptr)
	{
		CloseHandle(gEvent);
	}
}

DWORD I2CDisplay::RunCommand(_In_ PCCommand Command)
{
	DWORD status;
	DWORD bytesTransferred;
	
	Command->Overlapped.hEvent = gEvent;

	if (Command->Execute() == true)
	{
		if (GetOverlappedResult(Command->File,
			&(Command->Overlapped),
			&bytesTransferred,
			true) == FALSE)
		{
			status = GetLastError();
		}
		else
		{
			status = NO_ERROR;
		}

		Command->Complete(status, bytesTransferred);
	}
	else
	{
		status = GetLastError();
	}

	return status;
}

VOID I2CDisplay::ExecuteCommand(string name, list<string> *params)
{
	assert(!_stricmp(name.c_str(), "write"));
	string tag;
	CWriteCommand command(params, tag);

	if (command.Parse())
	{
		RunCommand(&command);
	}
	else
	{
		command.DetachParameter(); //avoid double deletion
	}
}

void I2CDisplay::I2CWriteIOPort()
{
	std::stringstream stream; // lcdInterfaceData
	stream << std::hex;
	stream << std::setw(2) << std::setfill('0') << (int)lcdInterfaceData;
	string tmp(stream.str());
	list<string> * dataBuffer = new list<string>{ "{", tmp, "}" };
	ExecuteCommand("write", dataBuffer);
}

void I2CDisplay::WriteNibble(
    bool registerSelect,
    BYTE dataNibble)
{
    // Set the data...
    lcdInterfaceData &= lcdBacklight; // Zero everything but the backlight state
    lcdInterfaceData |= ((0x0F & dataNibble) << 4) | (registerSelect ? lcdRegisterSelect : 0);
    I2CWriteIOPort();

    // ...arm...
    lcdInterfaceData |= lcdEnable;
    I2CWriteIOPort();
//    Sleep(1); // wait for 450ns

    // ...fire!
    lcdInterfaceData &= ~lcdEnable;
    I2CWriteIOPort();
//    Sleep(1); // wait for 37us
}

void I2CDisplay::Backlight(bool backlightOn)
{
    if(backlightOn)
    {
        lcdInterfaceData |= lcdBacklight;
    }
    else
    {
        lcdInterfaceData &= ~lcdBacklight;
    }
    I2CWriteIOPort();
}

void I2CDisplay::WriteByte(
    bool registerSelect,
    BYTE dataByte)
{
    BYTE nibble = ((dataByte & 0xF0) >> 4);
    WriteNibble(registerSelect, nibble);
    nibble = (dataByte & 0x0F);
    WriteNibble(registerSelect, nibble);
}

void I2CDisplay::LineSelect(
    unsigned int line)
{
    BYTE controlByte = lcdSetDDRAMAddress;
    switch(line)
    {
    case 0:
        controlByte |= 0x00;
        break;
    case 1:
        controlByte |= 0x40;
        break;
    case 2:
        controlByte |= 0x14;
        break;
    case 3:
        controlByte |= 0x54;
        break;
    default:
        return;
    }
    WriteByte(FALSE, controlByte);
}

void I2CDisplay::ClearScreen()
{
    WriteByte(FALSE, lcdClearDisplay);
}

void I2CDisplay::Initialize()
{
    // Low level initialization
    WriteNibble(FALSE, 0x03);
    Sleep(5); // wait for 4.1ms
    WriteNibble(FALSE, 0x03);
    WriteNibble(FALSE, 0x03);
    WriteNibble(FALSE, (lcdFunctionSet >> 4));

    // Display initialization
    Backlight(TRUE);
    WriteByte(FALSE, lcdFunctionSet | lcdFunctionSet4BitMode | lcdFunctionSet2LineMode | lcdFunctionSet5x10Dots);
    WriteByte(FALSE, lcdDisplayControl | lcdDisplayControlOn | lcdDisplayControlCursorOff | lcdDisplayControlBlinkOff);
    WriteByte(FALSE, lcdCursorShift | lcdCursorShiftCursorMove | lcdCursorShiftMoveRight);
}

void I2CDisplay::Write(string str)
{
    unsigned int strLen = min(str.size(), 20);
    for(unsigned int n = 0; n < strLen; n++)
    {
        // Write the string
        WriteByte(TRUE, (BYTE)str[n]);
    }
    for(unsigned int n = strLen; n < 20; n++)
    {
        // delete the rest of the line
        WriteByte(TRUE, (BYTE)' ');
    }
}

void I2CDisplay::Update(unsigned int line)
{
    // Initialize display on first use
    if(!IsInitialized)
    {
        Initialize();
        IsInitialized = TRUE;
    }

    // Simple syncronization so only one update happens at a time
    while(InProgress)
    {
        Sleep(100);
    }

    // Write line
    InProgress = TRUE;
    LineSelect(line);
    Write(LineBuf[line]);
    InProgress = FALSE;
}
