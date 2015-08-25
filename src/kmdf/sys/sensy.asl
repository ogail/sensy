// Steps to update the SSDT:
// 0a. Copy ASL.exe to your development board 
// 0b. Open a command prompt as administrator and navigate to the directory where you have copied ASL.exe
// 1. Compile sensy.asl ("asl.exe sensy.asl")
// 5. Copy the compiled file under Windows/System32 folder
// 6. Restart your development board

DefinitionBlock ("ACPITABL.dat", "SSDT", 0x1, "Ogail", "SPBT", 0x1)
{
    Scope (\_SB)
    {
        Device(SPBT)
        {
            Name(_HID, "Sensy")
            Name(_UID, 1)
            Method(_CRS, 0x0, NotSerialized)
            {
                Name (RBUF, ResourceTemplate ()
                {
                    I2CSerialBus(0x27, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1", , )
                    I2CSerialBus(0x48, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1", , )
                })
                Return(RBUF)
            }
        }
    }
}