#pragma once

#include "structs.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>



namespace Task4
{


class WindDataDumper
{
public:
	WindDataDumper() = default;

	WindDataDumper( const WindDataDumper& ) = delete;
	WindDataDumper& operator= ( const WindDataDumper& ) = delete;

	void Dump( const WindData& data )
	{
		if (!file.is_open())
		{
			if (data.direction == WindData::Direction::_1_3)
			{
				std::time_t time = std::time( nullptr );
				std::stringstream ss{};
#pragma warning(suppress : 4996) // std::gmtime unsafe warning suppress
				ss << "rogatka " << std::put_time( std::gmtime( &time ), "%Y-%m-%d %H.%M.%S" ) << ".bin";
				file.open( ss.str(), std::ios_base::binary | std::ios_base::out );
				recordsRemain = 40;
			}
		}

		if (file.is_open())
		{
			if (data.offset == 0)
				file << data.packetNumber << static_cast<int>(data.direction);

			for (int i = 0; i < data.size; i++)
				file << data.begin[i];

			if (recordsRemain-- == 0)
				file.close();
		}
	}

private:
	std::ofstream file{};
	int recordsRemain = 0;
};


} // namespace Task4
