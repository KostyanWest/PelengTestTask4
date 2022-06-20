#pragma once

#include <utility>



class ComPort
{
public:
	ComPort( unsigned char portNumber );

	ComPort( const ComPort& ) = delete;

	ComPort( ComPort&& other ) noexcept
		: nativePortHandle( other.nativePortHandle )
	{
		other.nativePortHandle = nullptr;
	}

	ComPort& operator= ( const ComPort& ) = delete;

	ComPort& operator= ( ComPort&& other ) noexcept
	{
		ComPort move( std::move( other ) );
		std::swap( nativePortHandle, move.nativePortHandle );
		return *this;
	}

	void Setup( int baudRate );
	char ReadByte();
	size_t Read( char* buffer, size_t bufferSize );
	size_t Write( char* buffer, size_t bufferSize );

	~ComPort();

private:
	void* nativePortHandle = nullptr;
};
