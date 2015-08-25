#pragma once

#include <appmodel.h>
#include <strsafe.h>

namespace DefaultApp
{
    namespace Data
    {
        public ref class I2CDisplay sealed
        {
        public:
            property Platform::String^ Line0 { void set(Platform::String^ data) { LineBuf[0] = data; Update(0); } };
            property Platform::String^ Line1 { void set(Platform::String^ data) { LineBuf[1] = data; Update(1); } };
            property Platform::String^ Line2 { void set(Platform::String^ data) { LineBuf[2] = data; Update(2); } };
            property Platform::String^ Line3 { void set(Platform::String^ data) { LineBuf[3] = data; Update(3); } };
            I2CDisplay();

        private:
            static UINT8 const deviceAddress = 0x27;

            static UINT8 const lcdBacklight = 0x08;
            static UINT8 const lcdEnable = 0x04;
            static UINT8 const lcdReadWrite = 0x02;
            static UINT8 const lcdRegisterSelect = 0x01;

            static UINT8 const lcdClearDisplay = 0x01;
            static UINT8 const lcdReturnHome = 0x02;
            static UINT8 const lcdEntryModeSet = 0x04;
            static UINT8 const lcdDisplayControl = 0x08;
            static UINT8 const lcdCursorShift = 0x10;
            static UINT8 const lcdFunctionSet = 0x20;
            static UINT8 const lcdSetCGRAMAddress = 0x40;
            static UINT8 const lcdSetDDRAMAddress = 0x80;

            // flags for display entry mode
            static UINT8 const lcdEntryModeShiftDecrement = 0x00;
            static UINT8 const lcdEntryModeShiftIncrement = 0x01;
            static UINT8 const lcdEntryModeEntryRight = 0x00;
            static UINT8 const lcdEntryModeEntryLeft = 0x02;

            // flags for display on/off control
            static UINT8 const lcdDisplayControlOn = 0x04;
            static UINT8 const lcdDisplayControlOff = 0x00;
            static UINT8 const lcdDisplayControlCursorOn = 0x02;
            static UINT8 const lcdDisplayControlCursorOff = 0x00;
            static UINT8 const lcdDisplayControlBlinkOn = 0x01;
            static UINT8 const lcdDisplayControlBlinkOff = 0x00;

            // flags for display/cursor shift
            static UINT8 const lcdCursorShiftDisplayMove = 0x08;
            static UINT8 const lcdCursorShiftCursorMove = 0x00;
            static UINT8 const lcdCursorShiftMoveRight = 0x04;
            static UINT8 const lcdCursorShiftMoveLeft = 0x00;

            // flags for function set
            static UINT8 const lcdFunctionSet8BitMode = 0x10;
            static UINT8 const lcdFunctionSet4BitMode = 0x00;
            static UINT8 const lcdFunctionSet2LineMode = 0x80;
            static UINT8 const lcdFunctionSet1LineMode = 0x00;
            static UINT8 const lcdFunctionSet5x10Dots = 0x04;
            static UINT8 const lcdFunctionSet5x8Dots = 0x00;

			Windows::Devices::I2c::I2cDevice ^displayDevice;

            Platform::String^ LineBuf[4];
            UINT8 lcdInterfaceData = 0;
            BOOLEAN IsInitialized = FALSE;
            BOOLEAN InProgress = FALSE;

            ~I2CDisplay();
            void I2CWriteIOPort();
            void WriteNibble(boolean registerSelect, UINT8 dataNibble);
            void WriteByte(boolean registerSelect, UINT8 dataByte);
            void Backlight(boolean backlightOn);
            void ClearScreen();
            void Initialize();
            void LineSelect(unsigned int line);
            void Write(Platform::String^ str);
            void Update(unsigned int line);
        };
    }
}
