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
int plot[frameWidth];
char frame[frameWidth * frameHeight];
//char portBuffer[6178];

//void REad( int direction, int packetNumber, const int* begin, size_t count )
//{
//	cv::Mat image( frameHeight, frameWidth, CV_8UC1 );
//	cv::rectangle( image, cv::Rect( 0, 0, count, frameHeight ), cv::Scalar( 0.5 ), CV_FILLED );
//	std::cout << "\nDir: " << direction << "\nNum: " << packetNumber << "\nSiz: " << end - begin << '\n';
//	while (begin != end)
//	{
//		std::cout << *begin++ << ' ';
//	}
//	std::cout << std::endl;
//}


} // namespace


int main()
{
	int portNumber = 0;
	cv::Mat image = cv::Mat::zeros( cv::Size( 500, 500 ), CV_8UC1 );
	cv::namedWindow( "Plot", cv::WINDOW_NORMAL );
	cv::imshow( "Plot", image );
	do
	{
		std::cout << "Select port number:" << std::endl;
		std::cin >> portNumber;
		try
		{
			ComPort port( portNumber );
			std::cout << std::hex << "Port opened" << std::endl;

			AnemorumbometerReader reader( port );
			reader.Setup();
			std::cout << "Anemorumbometer setuped" << std::endl;

			// wait ESC key
			while (cv::waitKey( 20 ) != 27)
			{
				try
				{
					AnemorumbometerReader::Data data = reader.ReadSomeData();
				}
				catch (const AnemorumbometerReader::ReadError& ex)
				{
					std::cerr << "Packet skipped because of error: " << ex.what()
						<< "\nat offset: " << ex.offset << std::endl;
					reader.Setup();
					std::cout << "Anemorumbometer resetuped" << std::endl;
				}
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Fatal error: " << ex.what() << std::endl;
		}
	}
	while (portNumber > 0);

	/*int value = 0;
	cv::Mat image = cv::Mat::zeros( cv::Size( 500, 500 ), CV_8UC1 );
	cv::namedWindow( "Plot", cv::WINDOW_NORMAL );
	cv::resizeWindow( "Plot", 1028, 512 );

	cv::createTrackbar( "Frame", "Plot", &value, 15, barclb, nullptr );
	cv::imshow( "Plot", image );
	std::cout << cv::waitKeyEx( 0 ) << std::endl;
	std::cout << cv::waitKeyEx( 1 ) << std::endl;
	std::cout << "Hello World!\n";
	cv::waitKey( 0 );
	cv::setTrackbarPos( "Frame", "test", 1 );
	cv::waitKey( 0 );*/
}
