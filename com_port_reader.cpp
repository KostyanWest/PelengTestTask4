#include "com_port_reader.hpp"

#include <Windows.h>
#include <stdexcept>
#include <string>



ComPort::ComPort( unsigned char portNumber )
{
	std::wstring portName = std::wstring( L"\\\\.\\COM" ) + std::to_wstring( portNumber );
	nativePortHandle = ::CreateFile( portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );


	if (nativePortHandle == INVALID_HANDLE_VALUE)
	{
		if (::GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			throw std::runtime_error( "Serial port does not exist." );
		}
		throw std::runtime_error( "Some other error occurred." );
	}
}

void ComPort::Setup( int baudRate )
{
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof( dcbSerialParams );
	if (!::GetCommState( nativePortHandle, &dcbSerialParams ))
	{
		throw std::runtime_error( "Getting state error." );
	}
	dcbSerialParams.BaudRate = baudRate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!::SetCommState( nativePortHandle, &dcbSerialParams ))
	{
		throw std::runtime_error( "Error setting serial port state." );
	}
}

char ComPort::ReadByte()
{
	DWORD bytesRead;
	char buffer;

	if (::ReadFile( nativePortHandle, &buffer, 1, &bytesRead, NULL ) && bytesRead > 0)
	{
		return buffer;
	}
	else
	{
		throw std::runtime_error( "Port read error." );
	}
}

size_t ComPort::Read( char* buffer, size_t bufferSize )
{
	DWORD bytesRead = 0;
	if (::ReadFile( nativePortHandle, buffer, (DWORD)bufferSize, &bytesRead, NULL ))
	{
		return bytesRead;
	}
	else
	{
		throw std::runtime_error( "Port read error." );
	}
}

size_t ComPort::Write( char* buffer, size_t bufferSize )
{
	DWORD bytesWritten = 0;
	if (::WriteFile( nativePortHandle, buffer, (DWORD)bufferSize, &bytesWritten, NULL ))
	{
		return bytesWritten;
	}
	else
	{
		throw std::runtime_error( "Port write error." );
	}
}

ComPort::~ComPort()
{
	if (nativePortHandle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle( nativePortHandle );
	}
}
