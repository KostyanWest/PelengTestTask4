#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

static void barclb( int state, void* userdata )
{
    std::cout << state << std::endl;
}


int main()
{
    int value = 0;
    cv::Mat image = cv::Mat::zeros( cv::Size( 500, 500 ), CV_8UC1 );
    cv::namedWindow( "test", cv::WINDOW_NORMAL );
    cv::createTrackbar( "bar", "test", &value, 1, barclb, nullptr );
    cv::createTrackbar( "bar2", "test", &value, 1, barclb, nullptr );
    cv::imshow( "test", image );
    std::cout << cv::waitKeyEx( 0 ) << std::endl;
    std::cout << cv::waitKeyEx( 1 ) << std::endl;
    std::cout << "Hello World!\n";
    cv::waitKey( 0 );
}
