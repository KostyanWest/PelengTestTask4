#pragma once

#include <algorithm>



namespace Task4
{


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
	bool IsFull() const noexcept { return cend() == std::cend( buffer ); }

	void Clear() noexcept { beginPtr = endPtr = std::begin( buffer ); }
	bool Cut( const T* beginIter ) noexcept
	{
		if (beginIter >= cbegin() && beginIter <= cend())
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
		if (Offset() > 0)
		{
			std::copy( cbegin(), cend(), std::begin( buffer ) );
			endPtr = std::begin( buffer ) + Size();
			beginPtr = std::begin( buffer );
		}
	}

	template <typename WriteFunc>
	size_t Append( WriteFunc writeFunc )
	{
		size_t bytesRead = writeFunc( end(), std::cend( buffer ) - cend() );
		endPtr += bytesRead;
		return bytesRead;
	}

private:
	T buffer[N]{};
	T* beginPtr = std::begin( buffer );
	T* endPtr = std::begin( buffer );
};


struct WindData
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
	const int* begin;
	size_t offset;
	size_t size;
};


} // namespace Task4
