#include "anemorumbometer_reader.hpp"
#include "wind_data_dumper.hpp"

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


void UpdateData( const Task4::WindData& data ) noexcept
{
	const int plotIndex = static_cast<short>(data.direction) - 1;
	for (int i = 0; i < data.size; i++)
	{
		plots[plotIndex][data.offset + i].y = data.begin[i] / 4;
	}

	if (isVisable[plotIndex])
	{
		DrawRange( data.offset, data.size );
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


void TestRead()
{
	char name[] = "rogatka 2022-06-24 12.43.17.bin";
	std::ifstream f( name, std::ios_base::binary | std::ios_base::in );
	for (int i = 0; i < 2; i++)
	{
		int iii;
		f >> iii;
		std::cout << iii << ' ';
	}
	for (int i = 0; i < 1000; i++)
	{
		short sss;
		f >> sss;
		std::cout << sss << ' ';
	}
	std::cout << std::endl;
}


class FakeAnem
{
public:
	void Setup() const noexcept {}

	Task4::WindData ReadSomeData() noexcept
	{
		if (dataPart == 0)
		{
			x = 0;
			k = static_cast<float>(std::rand()) / RAND_MAX * 40.0f + 10.0f;
		}

		for (short& value : data)
		{
			value = static_cast<short>(0x7FF + std::sin( x++ / k ) * 0x7FE);
		}

		Task4::WindData result{
			direction,
			packetNumber,
			data,
			dataPart * 257,
			257
		};

		if (++dataPart == 8)
		{
			dataPart = 0;
			if (direction == Task4::WindData::Direction::_4_2)
			{
				direction = Task4::WindData::Direction::_1_3;
				packetNumber++;
			}
			else
			{
				direction = static_cast<Task4::WindData::Direction>(static_cast<short>(direction) + 1);
			}
		}

		return result;
	}

private:
	short data[257];
	short packetNumber = 0;
	Task4::WindData::Direction direction = Task4::WindData::Direction::_1_3;

	size_t dataPart = 0;
	int x = 0;
	float k = 20.f;
};


} // namespace /*<unnamed>*/


int main()
{
	int portNumber = 0;
	InitPlots();

	//std::cout << std::hex;
	//TestRead();
	//return 0;

	do
	{
		std::cout << "Select port number (0 to exit):" << std::endl;
		portNumber = 0;
		std::cin >> portNumber;
		try
		{
			//ComPort port( portNumber );
			std::cout << std::hex << "Port opened" << std::endl;

			//Task4::AnemorumbometerReader reader( port );
			FakeAnem reader{};
			reader.Setup();
			std::cout << "Anemorumbometer setuped" << std::endl;

			cv::namedWindow( winName, cv::WINDOW_NORMAL );
			cv::resizeWindow( winName, frameWidth / 2, frameHeight / 2 );

			Task4::WindDataDumper dumper{};

			// wait ESC key
			int keyCode;
			while ((keyCode = cv::waitKey( 20 )) != 27)
			{
				KeyPressed( keyCode );
				try
				{
					Task4::WindData data = reader.ReadSomeData();
					UpdateData( data );
					dumper.Dump( data );
				}
				catch (const Task4::AnemorumbometerReader::ReadError& ex)
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
