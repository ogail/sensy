#pragma once

#include <iostream>
#include <cwctype>
#include <ppltasks.h>
#include "I2CDisplay.h"

using namespace Windows::Foundation;
using namespace Platform;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::I2c;
using namespace DefaultApp::Data;
using namespace Concurrency;
using namespace std;

I2CDisplay::I2CDisplay()
{
    displayDevice = nullptr;
    for(unsigned int n = 0; n < 4; n++)
    {
        LineBuf[0] = ref new String(L"");
    }
    lcdInterfaceData = lcdBacklight;

	String^ aqs;
	aqs = I2cDevice::GetDeviceSelector();

	auto dis = concurrency::create_task(DeviceInformation::FindAllAsync(aqs)).get();
	if (dis->Size != 1) {
		//throw wexception(L"I2C bus not found");
	}

	String^ id = dis->GetAt(0)->Id;
	auto device = concurrency::create_task(I2cDevice::FromIdAsync(
		id,
		ref new I2cConnectionSettings(deviceAddress))).get();

	if (!device) {
		//wostringstream msg;
		//msg << L"Slave address 0x" << hex << deviceAddress << L" on bus " << id->Data() <<
		//	L" is in use. Please ensure that no other applications are using I2C.";
		//throw wexception(msg.str());
	}

	displayDevice = device;
}

I2CDisplay::~I2CDisplay()
{
    ClearScreen();
    Backlight(FALSE);
}

void I2CDisplay::I2CWriteIOPort()
{
    if(displayDevice != nullptr)
    {
        Array<byte>^ dataBuffer = ref new Array<byte>(1);
        dataBuffer[0] = lcdInterfaceData;
        displayDevice->Write(dataBuffer);
    }
}

void I2CDisplay::WriteNibble(
    boolean registerSelect,
    UINT8 dataNibble)
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

void I2CDisplay::Backlight(
    boolean backlightOn)
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
    boolean registerSelect,
    UINT8 dataByte)
{
    UINT8 nibble = ((dataByte & 0xF0) >> 4);
    WriteNibble(registerSelect, nibble);
    nibble = (dataByte & 0x0F);
    WriteNibble(registerSelect, nibble);
}

void I2CDisplay::LineSelect(
    unsigned int line)
{
    UINT8 controlByte = lcdSetDDRAMAddress;
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

void I2CDisplay::Write(String ^str)
{
    unsigned int strLen = min(str->Length(), 20);
    for(unsigned int n = 0; n < strLen; n++)
    {
        // Write the string
        WriteByte(TRUE, (UINT8)str->Data()[n]);
    }
    for(unsigned int n = strLen; n < 20; n++)
    {
        // delete the rest of the line
        WriteByte(TRUE, (UINT8)' ');
    }
}

void I2CDisplay::Update(unsigned int line)
{
    // Do we have a functional display?
    if(displayDevice == nullptr)
    {
        return;
    }

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
