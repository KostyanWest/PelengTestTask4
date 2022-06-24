#include "anemorumbometer_reader.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>



namespace
{


constexpr size_t frameWidth = 2056;
constexpr size_t frameHeight = 1024;
constexpr size_t maxPlotsCount = 16;

bool isVisable[4] = { true, true, true, true };
cv::Point plots[4][frameWidth];
unsigned char frame[frameWidth * frameHeight * 3];

constexpr void InitPlots() noexcept
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < frameWidth; j++)
		{
			plots[i][j].x = j;
		}
	}
}

void ShowData( const AnemorumbometerReader::Data& data ) noexcept
{
	const int plotIndex = static_cast<int>(data.direction) - 1;
	for (int i = 0; i < data.dataSize; i++)
	{
		plots[plotIndex][data.dataOffset + i].y = data.data[i] / 4;
	}

	if (isVisable[plotIndex])
	{
		cv::Mat image( frameHeight, frameWidth, CV_8UC3, frame );
		cv::rectangle( image, cv::Rect( data.dataOffset, 0, data.dataSize, frameHeight ), CV_RGB( 0, 0, 0 ), CV_FILLED );
		for (int i = 0; i < 4; i++)
		{
			if (isVisable[i])
			{
				cv::Scalar color;
				switch (i)
				{
				case 0:
					color = CV_RGB( 255, 0, 0 );
					break;
				case 1:
					color = CV_RGB( 0, 255, 0 );
					break;
				case 2:
					color = CV_RGB( 0, 0, 255 );
					break;
				default:
					color = CV_RGB( 255, 255, 255 );
					break;
				}
				cv::Point* subPlot = plots[i] + data.dataOffset - 1;
				int count = data.dataSize + 1;
				if (data.dataOffset == 0)
				{
					subPlot++;
					count--;
				}
				cv::polylines( image, &subPlot, &count, 1, false, color, 1 );
			}
		}
		//cv::Mat newFrame( frameWidth / 2, frameHeight / 2, CV_8UC3 );
		//cv::resize( image, newFrame, newFrame.size(), 0.0, 0.0, cv::INTER_CUBIC );
		cv::imshow( "Plot", image );
	}
}


void TestSin()
{
	cv::namedWindow( "Plot", cv::WINDOW_NORMAL );
	cv::resizeWindow( "Plot", frameWidth / 2, frameHeight / 2 );
	int off = 0;

	// wait ESC key
	while (cv::waitKey( 20 ) != 27)
	{
		int val = 0x7FF + std::sin( off / 20.f ) * 0x7FE;
		AnemorumbometerReader::Data data{
			AnemorumbometerReader::Data::Direction::_1_3,
			0,
			&val,
			off,
			1
		};
		ShowData( data );
		if (off == 2055)
			off = 0;
		else
			off++;
	}
}


} // namespace


int main()
{
	int portNumber = 0;
	InitPlots();

	do
	{
		std::cout << "Select port number (0 to exit):" << std::endl;
		portNumber = 0;
		std::cin >> portNumber;
		try
		{
			ComPort port( portNumber );
			std::cout << std::hex << "Port opened" << std::endl;

			AnemorumbometerReader reader( port );
			reader.Setup();
			std::cout << "Anemorumbometer setuped" << std::endl;

			cv::namedWindow( "Plot", cv::WINDOW_NORMAL );
			cv::resizeWindow( "Plot", frameWidth, frameHeight );

			// wait ESC key
			while (cv::waitKey( 20 ) != 27)
			{
				try
				{
					AnemorumbometerReader::Data data = reader.ReadSomeData();
					ShowData( data );
				}
				catch (const AnemorumbometerReader::ReadError& ex)
				{
					std::cerr << "Packet skipped because of error: " << ex.what()
						<< "\nbyte: " << (unsigned int)(unsigned char)ex.byte
						<< "\nat offset: " << ex.offset << std::endl;
					reader.Setup();
					std::cout << "Anemorumbometer resetuped" << std::endl;
				}
			}
			cv::destroyWindow( "Plot" );
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Error: " << ex.what() << std::endl;
		}
	}
	while (portNumber > 0);
}
