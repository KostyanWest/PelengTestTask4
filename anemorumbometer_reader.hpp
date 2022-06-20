#pragma once

#include "com_port_reader.hpp"

#include <algorithm>
#include <iostream>



class AnemorumbometerReader
{
private:
	class ReadBuffer
	{
	public:
		ReadBuffer() = default;
		ReadBuffer( const ReadBuffer& ) = delete;
		ReadBuffer& operator= ( const ReadBuffer& ) = delete;

		char* begin() noexcept { return beginPtr; }
		char* end() noexcept { return endPtr; }

		const char* cbegin() const noexcept { return beginPtr; }
		const char* cend() const noexcept { return endPtr; }

		size_t Size() const noexcept { return cend() - cbegin(); }
		constexpr size_t MaxSize() const noexcept { return std::cend(buffer) - std::cbegin(buffer); }
		size_t Offset() const noexcept { return cbegin() - std::cbegin( buffer ); }

		bool IsEmpty() const noexcept { return cbegin() == cend(); }
		bool IsFull() const noexcept { return cend() == std::end( buffer ); }

		void Flush() noexcept { beginPtr = endPtr = std::begin( buffer ); }
		bool Cut( const char* beginIter ) noexcept
		{
			if (beginIter >= begin() && beginIter <= end())
			{
				beginPtr = const_cast<char*>(beginIter);
				return true;
			}
			else
			{
				return false;
			}
		}

		void AlignBegin() noexcept
		{
			std::memmove( std::begin( buffer ), begin(), Size() );
			endPtr = std::begin( buffer ) + Size();
			beginPtr = std::begin( buffer );
		}

		template <typename WriteFunc>
		size_t Write( WriteFunc writeFunc )
		{
			size_t bytesRead = writeFunc( std::begin( buffer ), MaxSize() );
			beginPtr = std::begin( buffer );
			endPtr = std::begin( buffer ) + bytesRead;
			return bytesRead;
		}

		template <typename WriteFunc>
		size_t Append( WriteFunc writeFunc )
		{
			size_t bytesRead = writeFunc( begin(), std::cend( buffer ) - cend() );
			endPtr += bytesRead;
			return bytesRead;
		}

		
	private:
		char buffer[6178]{};
		char* beginPtr = std::begin( buffer );
		char* endPtr = std::begin( buffer );
	};

	struct DataBuffer
	{
		int data[2056]{};
		int direction{};
		int packetNumber{};
	};

public:
	class ReadError : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	AnemorumbometerReader( ComPort& port ) noexcept : port( port ) {};

	AnemorumbometerReader( const AnemorumbometerReader& ) = delete;
	AnemorumbometerReader& operator= ( const AnemorumbometerReader& ) = delete;

	void ReadSomeData( void (*callback)(int direction, int packetNumber, const int* begin, size_t count) )
	{
		if (!isReadingMessage)
			LookForMessageStart();

		if (isReadingMessage)
		{
			int dataRead = ReceiveMessage();
			if (dataRead > 0)
			{
				callback( dataBuffer.direction, dataBuffer.packetNumber, dataBuffer.data, dataRead );
			}
		}
	}

	void Reset() noexcept
	{
		readBuffer.Flush();
		isReadingMessage = false;
	}

protected:
	void LookForMessageStart()
	{
		if (readBuffer.IsEmpty())
		{
			std::cout << "Look: " << readBuffer.Write( [this]( char* buf, size_t size ) { return port.Read( buf, size ); } ) << std::endl;
			/*for (const char ch : readBuffer)
			{
				std::cout << (unsigned int)(unsigned char)ch << ' ';
			}
			std::cout << std::endl;*/
		}

		const char* iter = std::find( readBuffer.cbegin(), readBuffer.cend(), 0x20 );
		if (iter != readBuffer.end())
		{
			readBuffer.Cut( iter + 1 );
			readBuffer.AlignBegin();
			isReadingMessage = true;
		}
		else
		{
			readBuffer.Flush();
		}
	}

	int ReceiveMessage()
	{
		if (!readBuffer.IsFull())
		{
			readBuffer.Append( [this]( char* buf, size_t size ) { return port.Read( buf, size ); } );
		}

		while (!readBuffer.IsEmpty())
		{
			// if need to read the header
			if (readBuffer.Offset() == 0)
			{
				// try read the header
				if (readBuffer.Size() >= 6)
				{
					const char* iter = readBuffer.cbegin();
					if (iter[0] != 0x32)
						throw ReadError( "Invalid second byte." );

					int direction = (iter[1] << 8) | iter[2];
					switch (direction)
					{
					case 0x3133:
						dataBuffer.direction = 0;
						break;
					case 0x3234:
						dataBuffer.direction = 1;
						break;
					case 0x3331:
						dataBuffer.direction = 2;
						break;
					case 0x3432:
						dataBuffer.direction = 3;
						break;
					default:
						throw ReadError( "Invalid direction number." );
					}

					if (std::any_of( iter + 3, iter + 6, []( const char& ch ) noexcept { return (ch & 0xF0) != 0x30; } ))
						throw ReadError( "Invalid packet number." );

					dataBuffer.packetNumber =
						(iter[3] & 0x0F) << 16 |
						(iter[4] & 0x0F) << 8 |
						iter[5] & 0x0F;

					readBuffer.Cut( iter + 6 );
				}
			}
			// if need to read the values
			else if (readBuffer.Offset() < readBuffer.MaxSize() - 4)
			{
				int* dataIter = dataBuffer.data;
				while (readBuffer.Size() >= 3)
				{
					const char* iter = readBuffer.cbegin();

					if (std::any_of( iter, iter + 3, []( const char& ch ) noexcept { return (ch & 0xF0) != 0x30; } ))
						throw ReadError( "Invalid value." );

					*dataIter++ =
						(iter[0] & 0x0F) << 16 |
						(iter[1] & 0x0F) << 8 |
						iter[2] & 0x0F;
					readBuffer.Cut( iter + 3 );
				}
				return dataIter - dataBuffer.data;
			}
			// if need to read the footer
			else
			{
				// try read the footer
				if (readBuffer.Size() >= 3)
				{
					if (readBuffer.cbegin()[2] == 0x0D)
					{
						readBuffer.Cut( readBuffer.cbegin() + 3 );
						isReadingMessage = false;
					}
					else
					{
						throw ReadError( "Invalid last byte." );
					}
				}
			}
		}
		return 0;
	}

private:
	ReadBuffer readBuffer{};
	DataBuffer dataBuffer{};
	ComPort& port;
	bool isReadingMessage = false;
};
