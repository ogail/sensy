// Steps to update the SSDT:
// 0a. Copy ASL.exe to your development board 
// 0b. Open a command prompt as administrator and navigate to the directory where you have copied ASL.exe
// 1. Compile spbtesttool.asl ("asl.exe spbtesttool.asl")
// 5. Copy the compiled file under Windows/System32 folder
// 6. Restart your development board

DefinitionBlock ("ACPITABL.dat", "SSDT", 1, "MSFT", "SPBT", 1)
{
    Scope (\_SB)
    {
		Device(SPBT)
		{
			Name(_HID, "SpbTestTool")
			Name(_UID, 1)
			Method(_CRS, 0x0, NotSerialized)
			{
				Name (RBUF, ResourceTemplate ()
				{
					//
					// Sample I2C resources. Modify to match your
					// platform's underlying controllers and connections.
					// \_SB.I2C is path to predefined I2C 
					// and GPIO controller instance. 
					//
					I2CSerialBus(0x48, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C", , )
					//I2CSerialBus(0x27, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C", , )
				})
				Return(RBUF)
			}
		}
    }
}