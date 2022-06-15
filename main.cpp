#include "com_port_reader.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>



namespace
{


constexpr size_t frameWidth = 2056;
constexpr size_t frameHeight = 1024;
constexpr size_t maxPlotsCount = 16;
int plots[maxPlotsCount][frameWidth];
char frame[frameWidth * frameHeight];
char portBuffer[6178];


void SetupAnemorumbometer( ComPort& port )
{
	port.Setup( 1200 );

	// wait for any message from the anemorumbometer
	while (port.Read( portBuffer, sizeof( portBuffer ) ) <= 0);

	// responce to setup the anemorumbometer
	char response[] = { 0x20, 0x32, 0x30, 0x32, 0x33, 0x31, 0x33, 0x30, 0x30, 0x30, 0x4B, 0x4C, 0x0D };
	for (int i = 0; i < 5; i++) {
		char* current = response;
		while (current != response + sizeof( response ))
		{
			size_t bytesWritten = port.Write( current, response + sizeof( response ) - current );
			current += bytesWritten;
			std::cout << "Write: " << bytesWritten << std::endl;
		}
	}

	port.Setup( 115200 );
}

bool LookForMessageStart( ComPort& port, char*& current, char*& end )
{
	// if portBuffer is empty
	if (current == end)
	{
		size_t bytesRead = port.Read( portBuffer, sizeof( portBuffer ) );
		current = portBuffer;
		end = portBuffer + bytesRead;
	}

	// look for message start character
	std::cout << "Look: " << end - current << std::endl;
	while (current != end)
	{
		if (*current++ == 0x20)
		{
			std::memmove( portBuffer, current, end - current );
			end = portBuffer + (end - current);
			current = portBuffer;
			return true;
		}
	}
	current = end = portBuffer;
	return false;
}

bool ReceiveMessage( ComPort& port, char*& current, char*& end)
{
	// if portBuffer isn't full
	if (end != portBuffer + sizeof( portBuffer ))
	{
		size_t bytesRead = port.Read( end, portBuffer + sizeof( portBuffer ) - end );
		end = end + bytesRead;
	}

	while (current != end)
	{
		if (current == portBuffer)
		{
			// try read header
			if (end - portBuffer >= 6)
			{
				//read header
				std::cout << (unsigned int)(unsigned char)(*current++) << '\n'
					<< (unsigned int)(unsigned char)(*current++) << (unsigned int)(unsigned char)(*current++) << '\n'
					<< (unsigned int)(unsigned char)(*current++) << (unsigned int)(unsigned char)(*current++) << (unsigned int)(unsigned char)(*current++) << std::endl;
			}
		}
		else
		{
			if (end < portBuffer + sizeof( portBuffer ) - 1)
			{
				std::cout << "Read: " << end - current << std::endl;
				current = end;
			}
			else
			{
				std::cout << "Read: " << portBuffer + sizeof( portBuffer ) - 1 - current << std::endl;
				current = portBuffer + sizeof( portBuffer ) - 1;
				return false;
			}
		}
	}
	return true;
}

void barclb( int state, void* userdata )
{
	std::cout << state << std::endl;
}


} // namespace


int main()
{
	std::cout << "Select port number:" << std::endl;
	int portNumber = 0;
	cv::Mat image = cv::Mat::zeros( cv::Size( 500, 500 ), CV_8UC1 );
	cv::namedWindow( "Plot", cv::WINDOW_NORMAL );
	cv::imshow( "Plot", image );
	do
	{
		std::cin >> portNumber;
		try
		{
			ComPort port( portNumber );
			std::cout << std::hex << "Port opened" << std::endl;
			SetupAnemorumbometer( port );
			std::cout << "Anemorumbometer setuped" << std::endl;

			char* current = portBuffer;
			char* end = current;
			bool isReadingMessage = true;
			while (cv::waitKey( 1 ) != 27)
			{
				if (!isReadingMessage)
				{
					isReadingMessage = LookForMessageStart( port, current, end );
				}
				else
				{
					isReadingMessage = ReceiveMessage( port, current, end );
				}
			}
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
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
