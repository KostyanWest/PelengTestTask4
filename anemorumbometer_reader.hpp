#pragma once

#include "com_port_reader.hpp"

#include <iostream>



class AnemorumbometerReader
{
public:
	AnemorumbometerReader( ComPort& port ) noexcept : port( port ) {};

	AnemorumbometerReader( const AnemorumbometerReader& ) = delete;
	AnemorumbometerReader& operator= ( const AnemorumbometerReader& ) = delete;

	void ReadSomeData( void (*callback)(int direction, int packetNumber, const int* begin, const int* end) )
	{
		if (!isReadingMessage)
			LookForMessageStart();

		if (isReadingMessage)
		{
			int dataRead = ReceiveMessage();
			if (dataRead > 0)
			{
				callback( direction, packetNumber, data, data + dataRead );
			}
		}
	}

protected:
	void LookForMessageStart()
	{
		// if the buffer is empty
		if (begin == end)
		{
			size_t bytesRead = port.Read( buffer, sizeof( buffer ) );
			begin = buffer;
			end = buffer + bytesRead;
		}

		// look for the message start character
		std::cout << "Look: " << end - begin << std::endl;
		while (begin != end)
		{
			if (*begin++ == 0x20)
			{
				std::memmove( buffer, begin, end - begin );
				end = buffer + (end - begin);
				begin = buffer;
				isReadingMessage = true;
				return;
			}
		}
		isReadingMessage = false;
	}

	int ReceiveMessage()
	{
		// if the buffer isn't full
		if (end != buffer + sizeof( buffer ))
		{
			size_t bytesRead = port.Read( end, (buffer + sizeof( buffer )) - end );
			end += bytesRead;
		}

		while (begin != end)
		{
			// if need to read the header
			if (begin == buffer)
			{
				// try read the header
				if (end - buffer >= 6)
				{
					char* ptr = buffer;
					while (ptr != buffer + 6 && (*ptr++ & 0xF0) == 0x30);
					if (ptr == buffer + 6)
					{

					}

					int direction = (buffer[1] << 8) | buffer[0];
					switch (direction)
					{
					case 0x3133:
						0;
						break;
					case 0x3234:
						1;
						break;
					case 0x3331:
						2;
						break;
					case 0x3432:
						3;
						break;
					default:
						break;
					}

					auto fgh = (long)buffer[0] << 8;

					// the check byte
					buffer[0] == 0x32;
					// read the header
					std::cout << (unsigned int)(unsigned char)(*begin++) << '\n'
						<< (unsigned int)(unsigned char)(*begin++) << (unsigned int)(unsigned char)(*begin++) << '\n'
						<< (unsigned int)(unsigned char)(*begin++) << (unsigned int)(unsigned char)(*begin++)
						<< (unsigned int)(unsigned char)(*begin++) << std::endl;
				}
			}
			else
			{
				if (end < buffer + sizeof( buffer ) - 1)
				{
					std::cout << "Read: " << end - begin << std::endl;
					begin = end;
				}
				else
				{
					std::cout << "Read: " << buffer + sizeof( buffer ) - 1 - begin << std::endl;
					begin = buffer + sizeof( buffer ) - 1;
					isReadingMessage = false;
					return 0;
				}
			}
		}
		isReadingMessage = true;
		return 0;
	}

private:
	char buffer[6178] {};
	int data[2056] {};
	ComPort& port;
	char* begin = buffer;
	char* end = buffer;
	int direction {};
	int packetNumber {};
	bool isReadingMessage = false;
};
