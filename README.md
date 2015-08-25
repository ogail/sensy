Sensy is an educational embedded systems project which explores different aspects of embedded and kerne; level development starting from construcitng and communicating with I2C devices in user mode to developing KMDF Windows driver and using Simple Peripheral Bus (SPB) which is accessible by user mode applications.

# Breakdown
### src/winrt
contains a user mode application that uses WinRT APIs to communicate with the i2c devices

### src/kmdf/sys
KMDF driver that's capable of communicating with multiple i2c devices. Mainly reading temperature from sensor device and then display it on crystial display.

### src/kmdf/exe
contains a user mode application that communicates with the sensy driver via symbolic links and DeviceIoControl

# Build
Open src/sensy.sln and build all

# Deploy
Copy the driver files from src\ARM\Debug\driver over to the target machine and from that directory execute this cmd
```
devcon update Sensy.inf ACPI\Sensy
```

# Test
Copy the src\ARM\Debug\SensyCLI.exe to the target machine and then execute this `magic` cmd

# Wiring
![alt text](https://github.com/ogail/sensy/blob/master/tools/wiring.JPG "Wiring")
