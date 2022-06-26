#pragma once

#include "com_port_reader.hpp"
#include "structs.hpp"



namespace Task4
{


class AnemorumbometerReader
{
public:
	class ReadError : public std::exception
	{
	public:
		ReadError( WindData::Direction direction, short packetNumber, int offset, char byte ) noexcept
			: direction( direction ), packetNumber( packetNumber ), offset( offset ), byte( byte )
		{
		}

		const char* what() const noexcept override
		{
			return "Byte doesn't match pattern.";
		}

		const WindData::Direction direction;
		const short packetNumber;
		const int offset;
		const char byte;
	};


	explicit AnemorumbometerReader( ComPort& port ) noexcept : port( port ) {};

	AnemorumbometerReader( const AnemorumbometerReader& ) = delete;
	AnemorumbometerReader& operator= ( const AnemorumbometerReader& ) = delete;

	void Setup();
	WindData ReadSomeData();

private:
	void LookForMessageStart();
	void ReceiveMessage();

	void ExtractHeader();
	void ExtractValues() noexcept;
	void ExtractFooter() noexcept;

private:
	Buffer<char, 6178> readBuffer{};
	Buffer<short, 2056> dataBuffer{};
	WindData::Direction direction{};
	short packetNumber{};

	ComPort& port;
	bool isReadingMessage = false;
};


} // namespace Task4
