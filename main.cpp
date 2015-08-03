#include "constants.h"
#include <ppltasks.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cwctype>
#include <algorithm>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::I2c;
using namespace std;

class wexception
{
public:
    explicit wexception (const wstring &msg) : msg_(msg) { }
    virtual ~wexception () { /*empty*/ }

    virtual const wchar_t *wwhat () const
    {
        return msg_.c_str();
    }

private:
    wstring msg_;
};

wostream& operator<< (wostream& os, const I2cTransferResult& result)
{
    switch (result.Status) {
    case I2cTransferStatus::FullTransfer: break;
    case I2cTransferStatus::PartialTransfer:
        os << L"Partial Transfer. Transferred " <<
            result.BytesTransferred << L" bytes\n";
        break;
    case I2cTransferStatus::SlaveAddressNotAcknowledged:
        os << L"Slave address was not acknowledged\n";
        break;
    default:
        throw wexception(L"Invalid transfer status value");
    }
    return os;
}

wistream& expect (wistream& is, wchar_t delim)
{
    wchar_t ch;
    while (is.get(ch)) {
        if (ch == delim) return is;
        if (!isspace(ch)) {
            is.clear(is.failbit);
            break;
        }
    }
    return is;
}

wistream& operator>> (wistream& is, vector<BYTE>& bytes)
{
    bytes.clear();

    if (!expect(is, L'{')) {
        wcout << L"Syntax error: expecting '{'\n";
        return is;
    }

    // get a sequence of bytes, e.g.
    //   write { 0 1 2 3 4 aa bb cc dd }
    unsigned int byte;
    while (is >> hex >> byte) {
        if (byte > 0xff) {
            wcout << L"Out of range [0, 0xff]: " << hex << byte << L"\n";
            is.clear(is.failbit);
            return is;
        }
        bytes.push_back(static_cast<BYTE>(byte));
    }

    if (bytes.empty()) {
        wcout << L"Zero-length buffers are not allowed\n";
        is.clear(is.failbit);
        return is;
    }

    is.clear();
    if (!expect(is, L'}')) {
        wcout << L"Syntax error: expecting '}'\n";
        return is;
    }
    return is;
}

wostream& operator<< (wostream& os, const Array<BYTE>^ bytes)
{
    for (auto byte : bytes)
        os << L" " << hex << byte;
    return os;
}

wostream& operator<< (wostream& os, I2cBusSpeed busSpeed)
{
    switch (busSpeed) {
    case I2cBusSpeed::StandardMode:
        return os << L"StandardMode (100Khz)";
    case I2cBusSpeed::FastMode:
        return os << L"FastMode (400kHz)";
    default:
        return os << L"[Invalid bus speed]";
    }
}

I2cDevice^ makeDevice(int slaveAddress, _In_opt_ String^ friendlyName)
{
	using namespace Windows::Devices::Enumeration;

	String^ aqs;
	if (friendlyName)
		aqs = I2cDevice::GetDeviceSelector(friendlyName);
	else
		aqs = I2cDevice::GetDeviceSelector();

	auto dis = concurrency::create_task(DeviceInformation::FindAllAsync(aqs)).get();
	if (dis->Size != 1) {
		throw wexception(L"I2C bus not found");
	}

	String^ id = dis->GetAt(0)->Id;
	auto device = concurrency::create_task(I2cDevice::FromIdAsync(
		id,
		ref new I2cConnectionSettings(slaveAddress))).get();

	if (!device) {
		wostringstream msg;
		msg << L"Slave address 0x" << hex << slaveAddress << L" on bus " << id->Data() <<
			L" is in use. Please ensure that no other applications are using I2C.";
		throw wexception(msg.str());
	}

	return device;
}

void read(I2cDevice^ device, Array<BYTE>^ readBuf)
{
	I2cTransferResult result = device->ReadPartial(readBuf);

	switch (result.Status) {
	case I2cTransferStatus::FullTransfer:
		break;
	case I2cTransferStatus::PartialTransfer:
		wcout << L"Partial Transfer. Transferred " <<
			result.BytesTransferred << L" bytes\n";
		break;
	case I2cTransferStatus::SlaveAddressNotAcknowledged:
		wcout << L"Slave address was not acknowledged\n";
		break;
	default:
		throw wexception(L"Invalid transfer status value");
	}
}

void write(vector<BYTE>& writeBuf, I2cDevice^ device)
{
	I2cTransferResult result = device->WritePartial(
		ArrayReference<BYTE>(
			writeBuf.data(),
			static_cast<unsigned int>(writeBuf.size())));

	switch (result.Status) {
	case I2cTransferStatus::FullTransfer:
		wcout << L"Full Transfer!\n";
		break;
	case I2cTransferStatus::PartialTransfer:
		wcout << L"Partial Transfer. Transferred " <<
			result.BytesTransferred << L" bytes\n";
		break;
	case I2cTransferStatus::SlaveAddressNotAcknowledged:
		wcout << L"Slave address was not acknowledged\n";
		break;
	default:
		throw wexception(L"Invalid transfer status value");
	}
}

I2cDevice^ create(int address, String^ friendlyName)
{
	// Create the sensor device object
	auto device = makeDevice(address, friendlyName);

	// Display connection info
	int slaveAddress = device->ConnectionSettings->SlaveAddress;
	I2cBusSpeed busSpeed = device->ConnectionSettings->BusSpeed;
	wcout << L"       DeviceId: " << device->DeviceId->Data() << "\n";
	wcout << L"  Slave address: 0x" << hex << slaveAddress << L"\n";
	wcout << L"      Bus Speed: " << busSpeed << L"\n";

	return device;
}


double readTempreture()
{
	String^ friendlyName;
	auto tmpDevice = create(0x48, friendlyName);
	auto readBuf = ref new Array<BYTE>(2);
	read(tmpDevice, readBuf);
	BYTE MSB = readBuf[0];
	BYTE LSB = readBuf[1];
	int tmp = ((MSB << 8) | LSB) >> 4;
	return tmp * 0.0625;
}

BYTE _addr;
BYTE _displayfunction;
BYTE _displaycontrol;
BYTE _displaymode;
BYTE _cols;
BYTE _rows;
BYTE _charsize;
BYTE _backlightval;
I2cDevice^ lcd;

void expanderWrite(BYTE _data) {
	int temp = _data | _backlightval;
	vector<BYTE> bytes;
	BYTE mask = 0xFF;
	do {
		bytes.push_back(temp & mask);
		temp >>= 8;
	} while (temp);
	reverse(bytes.begin(), bytes.end());
	write(bytes, lcd);
}

static LARGE_INTEGER HpcFreq;
static double HpcMicroNumTicks;
static double HpcPerdiodNs;

void delay(unsigned us) {
	Sleep(us);
}

void delayMicroseconds(unsigned us)
{
	LARGE_INTEGER t0, t1, dTicks;

	dTicks.QuadPart = (LONGLONG)((double)us * HpcMicroNumTicks);

	QueryPerformanceCounter(&t0);

	for (;;)
	{
		QueryPerformanceCounter(&t1);
		if (t1.QuadPart - t0.QuadPart > dTicks.QuadPart)
		{
			return;
		}
	}
}

void pulseEnable(BYTE _data) {
	expanderWrite(_data | En);	// En high
	delayMicroseconds(1);		// enable pulse must be >450ns

	expanderWrite(_data & ~En);	// En low
	delayMicroseconds(50);		// commands need > 37us to settle
}

void write4bits(BYTE value) {
	expanderWrite(value);
	pulseEnable(value);
}

void send(BYTE value, BYTE mode) {
	BYTE highnib = value & 0xf0;
	BYTE lownib = (value << 4) & 0xf0;
	write4bits((highnib) | mode);
	write4bits((lownib) | mode);
}

inline void command(BYTE value) {
	send(value, 0);
}

inline void write(BYTE value) {
	send(value, Rs);
}

void display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void clear() {
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void home() {
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void init() {
	_backlightval = LCD_BACKLIGHT;
	_addr = (BYTE)lcd->ConnectionSettings->SlaveAddress;
	_displaycontrol = 0;
	_displaymode = 0;
	_cols = 16;
	_rows = 2;
	_charsize = 0;
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (_rows > 1) {
		_displayfunction |= LCD_2LINE;
	}

	// for some 1 line displays you can select a 10 pixel high font
	if ((_charsize != 0) && (_rows == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delay(50);

	// Now we pull both RS and R/W low to begin commands
	expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	delay(1000);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms

							 // second try
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms

							 // third go!
	write4bits(0x03 << 4);
	delayMicroseconds(150);

	// finally, set to 4-bit interface
	write4bits(0x02 << 4);

	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	// clear it off
	clear();

	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

	home();
}

void setCursor(BYTE col, BYTE row) {
	BYTE row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > _rows) {
		row = _rows - 1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void backlight() {
	_backlightval = LCD_BACKLIGHT;
	expanderWrite(0);
}

void noBacklight() {
	_backlightval = LCD_NOBACKLIGHT;
	expanderWrite(0);
}

BYTE newChar[8] = {
	0B00000,
	0B01110,
	0B10001,
	0B10001,
	0B10001,
	0B01010,
	0B11011,
	0B00000
};

void createChar(BYTE location, BYTE charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i = 0; i < 8; i++) {
		write(charmap[i]);
	}
}

void display(wstring data)
{
	//wcout << L"Temperature is " << data << L" Hello" << L"\n";
	String^ friendlyName;
	lcd = create(0x27, friendlyName);
	init();
	createChar(0, newChar);
	for (int i = 0; i < 3; i++)
	{
		backlight();
		delay(250);
		noBacklight();
		delay(250);
	}
	setCursor(0, 0);
	backlight();
	write(100);
	write(101);
	write(102);
	write(103);
}

int main(Array<String^>^ args)
{
	(void)QueryPerformanceFrequency(&HpcFreq);

	HpcPerdiodNs = 1000000000.0 / HpcFreq.QuadPart;
	HpcMicroNumTicks = (double)HpcFreq.QuadPart / 1000000.0;

    try {
		double celsius = readTempreture();
		display(to_wstring((int)celsius));
    } catch (const wexception& ex) {
        wcerr << L"Error: " << ex.wwhat() << L"\n";
        return 1;
    } catch (Exception^ ex) {
        wcerr << L"Error: " << ex->Message->Data() << L"\n";
        return 1;
    }

    return 0;
}
