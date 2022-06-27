#pragma once

#include "structs.hpp"

#include <opencv2/core/core.hpp>



namespace Task4
{


template<int Width, int Height, int Count>
class FrameBuffer
{
public:
	static constexpr size_t width = Width;
	static constexpr int height = Height;
	static constexpr int count = Count;

	constexpr FrameBuffer() noexcept
	{
		for (int i = 0; i < count; i++)
		{
			for (int j = 0; j < width; j++)
			{
				plots[i][j] = cv::Point( j, 0 );
			}
		}
	}

	constexpr void FillWithData( const WindData& data ) noexcept
	{
		const int plotIndex = static_cast<short>(data.direction) - 1;
		for (int i = 0; i < data.size; i++)
		{
			//plots[plotIndex][data.offset + i].y = height - data.begin[i] / 4;
			plots[plotIndex][data.offset + i].y = data.begin[i] / 4;
		}
	}

	constexpr cv::Point* operator[] ( int i ) noexcept
	{
		return plots[i];
	}

	constexpr const cv::Point* operator[] ( int i ) const noexcept
	{
		return plots[i];
	}

private:
	cv::Point plots[count][width];
};


} // namespace Task4
