#pragma once

#include "com_port_reader.hpp"

#include <algorithm>
#include <iostream>



class AnemorumbometerReader
{
private:
	template<typename T, size_t N>
	class Buffer
	{
	public:
		Buffer() = default;
		Buffer( const Buffer& ) = delete;
		Buffer& operator= ( const Buffer& ) = delete;

		T* begin() noexcept { return beginPtr; }
		T* end() noexcept { return endPtr; }

		const T* cbegin() const noexcept { return beginPtr; }
		const T* cend() const noexcept { return endPtr; }

		size_t Size() const noexcept { return cend() - cbegin(); }
		constexpr size_t MaxSize() const noexcept { return std::cend( buffer ) - std::cbegin( buffer ); }
		size_t Offset() const noexcept { return cbegin() - std::cbegin( buffer ); }

		bool IsEmpty() const noexcept { return cbegin() == cend(); }
		bool IsFull() const noexcept { return cend() == std::end( buffer ); }

		void Clear() noexcept { beginPtr = endPtr = std::begin( buffer ); }
		bool Cut( const T* beginIter ) noexcept
		{
			if (beginIter >= begin() && beginIter <= end())
			{
				beginPtr = const_cast<T*>(beginIter);
				return true;
			}
			else
			{
				return false;
			}
		}

		void AlignBegin() noexcept
		{
			std::copy( cbegin(), cend(), std::begin( buffer ) );
			endPtr = std::begin( buffer ) + Size();
			beginPtr = std::begin( buffer );
		}

		template <typename WriteFunc>
		size_t Append( WriteFunc writeFunc )
		{
			size_t bytesRead = writeFunc( begin(), std::cend( buffer ) - cend() );
			endPtr += bytesRead;
			return bytesRead;
		}

	private:
		T buffer[N] {};
		T* beginPtr = std::begin( buffer );
		T* endPtr = std::begin( buffer );
	};

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
		constexpr size_t MaxSize() const noexcept { return std::cend( buffer ) - std::cbegin( buffer ); }
		size_t Offset() const noexcept { return cbegin() - std::cbegin( buffer ); }

		bool IsEmpty() const noexcept { return cbegin() == cend(); }
		bool IsFull() const noexcept { return cend() == std::end( buffer ); }

		void Clear() noexcept { beginPtr = endPtr = std::begin( buffer ); }
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
		size_t Append( WriteFunc writeFunc )
		{
			size_t bytesRead = writeFunc( begin(), std::cend( buffer ) - cend() );
			endPtr += bytesRead;
			return bytesRead;
		}

	private:
		char buffer[6178] {};
		char* beginPtr = std::begin( buffer );
		char* endPtr = std::begin( buffer );
	};



public:
	struct Data
	{
		enum class Direction : int
		{
			Invalid,
			_1_3,
			_2_4,
			_3_1,
			_4_2,
		};

		Direction direction;
		int packetNumber;
		const int* data;
		size_t dataOffset;
		size_t dataSize;
	};


	class ReadError : public std::exception
	{
	public:
		ReadError( Data::Direction direction, int packetNumber, int offset, char byte ) noexcept
			: direction( direction ), packetNumber( packetNumber ), offset( offset ), byte( byte )
		{
		}

		const char* what() const noexcept override
		{
			return "Byte doesn't match pattern.";
		}

		const Data::Direction direction;
		const int packetNumber;
		const int offset;
		const char byte;
	};


	AnemorumbometerReader( ComPort& port ) noexcept : port( port ) {};

	AnemorumbometerReader( const AnemorumbometerReader& ) = delete;
	AnemorumbometerReader& operator= ( const AnemorumbometerReader& ) = delete;

	void Setup()
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
				std::cout << "Write: " << bytesWritten << std::endl;
			}
		}

		port.Setup( 115200 );
		readBuffer.Clear();
		isReadingMessage = false;
	}

	Data ReadSomeData()
	{
		if (!isReadingMessage)
			LookForMessageStart();
		else
			ReceiveMessage();

		if (dataBuffer.Size() > 0)
		{
			Data data { direction, packetNumber, dataBuffer.cbegin(), dataBuffer.Offset(), dataBuffer.Size() };
			dataBuffer.Cut( dataBuffer.cend() );
			return data;
		}
		else
		{
			return Data();
		}
	}

private:
	void LookForMessageStart()
	{
		if (readBuffer.IsEmpty())
		{
			readBuffer.Clear();
			std::cout << "Look: " << readBuffer.Append( [this] ( char* buf, size_t size ) { return port.Read( buf, size ); } ) << std::endl;
			/*for (const char ch : readBuffer)
			{
				std::cout << (unsigned int)(unsigned char)ch << ' ';
			}
			std::cout << std::endl;*/
		}

		const char* iter = std::find( readBuffer.cbegin(), readBuffer.cend(), 0x20 );
		if (iter != readBuffer.cend())
		{
			readBuffer.Cut( iter + 1 );
			readBuffer.AlignBegin();
			isReadingMessage = true;
		}
		else
		{
			readBuffer.Clear();
		}
	}

	void ReceiveMessage()
	{
		if (!readBuffer.IsFull())
		{
			readBuffer.Append(
				[this] ( char* buf, size_t size )
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

	void ExtractHeader()
	{
		if (readBuffer.Size() < 6)
			return;

		const char* iter = readBuffer.cbegin();
		if (iter[0] != 0x32)
			throw ReadError( Data::Direction::Invalid, 0, 1, iter[0] );

		int intDirection = (iter[1] << 8) | iter[2];
		switch (intDirection)
		{
		case 0x3133:
			direction = Data::Direction::_1_3;
			break;
		case 0x3234:
			direction = Data::Direction::_2_4;
			break;
		case 0x3331:
			direction = Data::Direction::_3_1;
			break;
		case 0x3432:
			direction = Data::Direction::_4_2;
			break;
		default:
			throw ReadError( Data::Direction::Invalid, 0, 1 + 1, 0 );
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

	void ExtractValues() noexcept
	{
		dataBuffer.Append(
			[this] ( int* const dataBufBegin, const size_t dataBufLimit ) noexcept
			{
				const size_t limit = std::min( readBuffer.Size() / 3, dataBufLimit );
				int* dataBufIter = dataBufBegin;
				char* readBufIter = readBuffer.begin();
				for (int i = 0; i < limit; i++)
				{
					int value = 0;
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

	void ExtractFooter() noexcept
	{
		if (readBuffer.Size() >= 3)
		{
			if (readBuffer.cbegin()[2] != 0x0D)
				std::cerr << "Bad last byte: " << (unsigned int)(unsigned char)readBuffer.cbegin()[2] << std::endl;

			readBuffer.Cut( readBuffer.cbegin() + 3 );
		}
	}

private:
	Buffer<char, 6178> readBuffer {};
	Buffer<int, 2056> dataBuffer {};
	Data::Direction direction {};
	int packetNumber {};
	ComPort& port;
	bool isReadingMessage = false;
};
