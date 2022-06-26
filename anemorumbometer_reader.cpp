#include "anemorumbometer_reader.hpp"

#include <algorithm>
#include <iostream>



namespace /*<unnamed>*/
{


//


} // namespace /*<unnamed>*/



void Task4::AnemorumbometerReader::Setup()
{
	port.Setup( 1200 );

	// wait for any message from the anemorumbometer
	while (port.Read( readBuffer.begin(), readBuffer.MaxSize() ) <= 0);

	// responce to setup the anemorumbometer
	char response[] = { 0x20, 0x32, 0x30, 0x32, 0x33, 0x31, 0x33, 0x30, 0x30, 0x30, 0x4B, 0x4C, 0x0D };
	for (int i = 0; i < 5; i++)
	{
		char* current = std::begin( response );
		while (current != std::end( response ))
		{
			size_t bytesWritten = port.Write( current, response + sizeof( response ) - current );
			current += bytesWritten;
		}
	}

	port.Setup( 115200 );
	readBuffer.Clear();
	isReadingMessage = false;
}


Task4::WindData Task4::AnemorumbometerReader::ReadSomeData()
{
	if (!isReadingMessage)
		LookForMessageStart();
	if (isReadingMessage)
		ReceiveMessage();

	if (dataBuffer.Size() > 0)
	{
		WindData data { direction, packetNumber, dataBuffer.cbegin(), dataBuffer.Offset(), dataBuffer.Size() };
		dataBuffer.Cut( dataBuffer.cend() );
		return data;
	}
	else
	{
		return WindData();
	}
}


void Task4::AnemorumbometerReader::LookForMessageStart()
{
	if (readBuffer.IsEmpty())
	{
		readBuffer.Clear();
		readBuffer.Append(
			[this]( char* buf, size_t size )
			{
				return port.Read( buf, size );
			}
		);
	}

	const char* iter = std::find( readBuffer.cbegin(), readBuffer.cend(), 0x20 );
	if (iter != readBuffer.cend())
	{
		readBuffer.Cut( iter + 1 );
		readBuffer.AlignBegin();
		isReadingMessage = true;
		dataBuffer.Clear();
	}
	else
	{
		readBuffer.Clear();
	}
}


void Task4::AnemorumbometerReader::ReceiveMessage()
{
	if (!readBuffer.IsFull())
	{
		readBuffer.Append(
			[this]( char* buf, size_t size )
			{
				return port.Read( buf, size );
			}
		);
	}

	if (readBuffer.Offset() == 0)
		ExtractHeader();
	if (readBuffer.Offset() < readBuffer.MaxSize() - 4)
		ExtractValues();
	if (readBuffer.Offset() == readBuffer.MaxSize() - 4)
		ExtractFooter();
	if (readBuffer.Offset() == readBuffer.MaxSize() - 1)
		isReadingMessage = false;
}


void Task4::AnemorumbometerReader::ExtractHeader()
{
	if (readBuffer.Size() < 6)
		return;

	const char* iter = readBuffer.cbegin();
	if (iter[0] != 0x32)
		throw ReadError( WindData::Direction::Invalid, 0, 1, iter[0] );

	int intDirection = (iter[1] << 8) | iter[2];
	switch (intDirection)
	{
	case 0x3133:
		direction = WindData::Direction::_1_3;
		break;
	case 0x3234:
		direction = WindData::Direction::_2_4;
		break;
	case 0x3331:
		direction = WindData::Direction::_3_1;
		break;
	case 0x3432:
		direction = WindData::Direction::_4_2;
		break;
	default:
		throw ReadError( WindData::Direction::Invalid, 0, 1 + 1, 0 );
	}

	packetNumber = 0;
	for (int i = 0; i < 3; i++)
	{
		if ((iter[3 + i] & 0xF0) != 0x30)
			throw ReadError( direction, 0, i + 3 + 1, iter[3 + i] );
		packetNumber <<= 4;
		packetNumber |= (iter[3 + i] & 0x0F);
	}

	readBuffer.Cut( iter + 6 );
}


void Task4::AnemorumbometerReader::ExtractValues() noexcept
{
	dataBuffer.Append(
		[this]( short* const dataBufBegin, const size_t dataBufLimit ) noexcept
		{
			const size_t limit = std::min( readBuffer.Size() / 3, dataBufLimit );
			short* dataBufIter = dataBufBegin;
			char* readBufIter = readBuffer.begin();
			for (int i = 0; i < limit; i++)
			{
				short value = 0;
				for (int j = 0; j < 3; j++)
				{
					char byte = *readBufIter++;
					if ((byte & 0xF0) != 0x30)
						std::cerr << "Bad byte: " << (unsigned int)(unsigned char)byte << std::endl;
					value <<= 4;
					value |= (byte & 0x0F);
				}
				*dataBufIter++ = value;
			}
			readBuffer.Cut( readBufIter );
			return dataBufIter - dataBufBegin;
		}
	);
}


void Task4::AnemorumbometerReader::ExtractFooter() noexcept
{
	if (readBuffer.Size() >= 3)
	{
		if (readBuffer.cbegin()[2] != 0x0D)
			std::cerr << "Bad last byte: " << (unsigned int)(unsigned char)readBuffer.cbegin()[2] << std::endl;

		readBuffer.Cut( readBuffer.cbegin() + 3 );
	}
}
