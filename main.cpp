#include "constants.h"
#include <ppltasks.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cwctype>
#include <algorithm>
#include "I2CDisplay.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::I2c;
using namespace DefaultApp::Data;
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

void write(BYTE data, I2cDevice^ device) {
	vector<BYTE> v = { data };
	write(v, device);
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

I2cDevice^ lcd;

static LARGE_INTEGER HpcFreq;
static double HpcMicroNumTicks;
static double HpcPerdiodNs;
static I2CDisplay ^d;

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

void show(wstring data)
{
	d = ref new I2CDisplay();
	Platform::String ^dataString = ref new Platform::String(data.c_str());
	d->Line0 = dataString;
}

int main(Array<String^>^ args)
{
	(void)QueryPerformanceFrequency(&HpcFreq);
	HpcPerdiodNs = 1000000000.0 / HpcFreq.QuadPart;
	HpcMicroNumTicks = (double)HpcFreq.QuadPart / 1000000.0;

    try {
		double celsius = readTempreture();
		wcout << L"Temperature is " << celsius << L"\n";
		show(to_wstring((int)celsius));
    } catch (const wexception& ex) {
        wcerr << L"Error: " << ex.wwhat() << L"\n";
        return 1;
    } catch (Exception^ ex) {
        wcerr << L"Error: " << ex->Message->Data() << L"\n";
        return 1;
    }

    return 0;
}
