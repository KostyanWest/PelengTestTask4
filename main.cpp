#include "anemorumbometer_reader.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>



namespace /*<unnamed>*/
{


constexpr const char* winName = "Plot (ESC to close)";
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


void DrawRange( int start, int length ) noexcept
{
	cv::Mat image( frameHeight, frameWidth, CV_8UC3, frame );
	cv::rectangle( image, cv::Rect( start, 0, length, frameHeight ), CV_RGB( 0, 0, 0 ), CV_FILLED );
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
			cv::Point* subPlot = plots[i] + start - 1;
			int count = length + 1;
			if (start == 0)
			{
				subPlot++;
				count--;
			}
			cv::polylines( image, &subPlot, &count, 1, false, color, 1 );
		}
	}
	//cv::Mat newFrame( frameWidth / 2, frameHeight / 2, CV_8UC3 );
	//cv::resize( image, newFrame, newFrame.size(), 0.0, 0.0, cv::INTER_CUBIC );
	cv::imshow( winName, image );
}


void UpdateData( const AnemorumbometerReader::Data& data ) noexcept
{
	const int plotIndex = static_cast<int>(data.direction) - 1;
	for (int i = 0; i < data.dataSize; i++)
	{
		plots[plotIndex][data.dataOffset + i].y = data.data[i] / 4;
	}

	if (isVisable[plotIndex])
	{
		DrawRange( data.dataOffset, data.dataSize );
	}
}


void ToggleVisability( int plotIndex ) noexcept
{
	isVisable[plotIndex] = !isVisable[plotIndex];
	DrawRange( 0, frameWidth );
}


void KeyPressed( int keyCode ) noexcept
{
	switch (keyCode)
	{
	case '1':
		ToggleVisability( 0 );
		break;
	case '2':
		ToggleVisability( 1 );
		break;
	case '3':
		ToggleVisability( 2 );
		break;
	case '4':
		ToggleVisability( 3 );
		break;
	default:
		break;
	}
}


void TestSin()
{
	cv::namedWindow( winName, cv::WINDOW_NORMAL );
	cv::resizeWindow( winName, frameWidth / 2, frameHeight / 2 );
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
		UpdateData( data );
		if (off == 2055)
			off = 0;
		else
			off++;
	}
}


} // namespace /*<unnamed>*/


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

			cv::namedWindow( winName, cv::WINDOW_NORMAL );
			cv::resizeWindow( winName, frameWidth, frameHeight );

			// wait ESC key
			int keyCode;
			while ((keyCode = cv::waitKey( 20 )) != 27)
			{
				KeyPressed( keyCode );
				try
				{
					AnemorumbometerReader::Data data = reader.ReadSomeData();
					UpdateData( data );
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
			cv::destroyWindow( winName );
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Error: " << ex.what() << std::endl;
		}
	}
	while (portNumber > 0);
}
