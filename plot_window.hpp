#pragma once

#include "plots.hpp"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>



namespace Task4
{


class FrameWindow
{
	using PlotsT = FrameBuffer<2056, 1024, 4>;

public:
	FrameWindow( const char* windowName )
		: name( windowName )
	{
		cv::namedWindow( name, cv::WINDOW_NORMAL );
		cv::resizeWindow( name, PlotsT::width, PlotsT::height );
	}

	FrameWindow( const FrameWindow& ) = delete;
	FrameWindow& operator= ( const FrameWindow& ) = delete;

	void Draw( const PlotsT& plots )
	{
		cv::rectangle( img, cv::Rect( 0, 0, PlotsT::width, PlotsT::height ), CV_RGB( 0, 0, 0 ), CV_FILLED );
		for (int i = 0; i < PlotsT::count; i++)
		{
			if (isVisable[i])
			{
				const cv::Point* subPlot = plots[i];
				int count = PlotsT::width;
				cv::polylines( img, &subPlot, &count, 1, false, colors[i] );
			}
		}
		//cv::Mat newFrame( PlotsT::width / 2, PlotsT::height / 2, CV_8UC3 );
		//cv::resize( img, newFrame, newFrame.size(), 0.0, 0.0, cv::INTER_CUBIC );
		cv::imshow( name, img );
	}

	int PressKey( int delay = 0 )
	{
		int keyCode = cv::waitKey( delay );
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
		case 'z':
		case 'Z':
			ToggleZoom();
			break;
		default:
			break;
		}
		return keyCode;
	}

	~FrameWindow() noexcept(false)
	{
		cv::destroyWindow( name );
	}

protected:
	void ToggleZoom()
	{
		isZoomed = !isZoomed;
		if (isZoomed)
			cv::resizeWindow( name, PlotsT::width, PlotsT::height );
		else
			cv::resizeWindow( name, PlotsT::width / 2, PlotsT::height / 2 );
	}

	void ToggleVisability( int plotIndex )
	{
		isVisable[plotIndex] = !isVisable[plotIndex];
		//DrawPlotRange( 0, PlotsT::width );
	}

	/*void DrawPlotRange( PlotsT& plots, int start, int length )
	{
		cv::Mat image( PlotsT::height, PlotsT::width, CV_8UC3, frame );
		cv::rectangle( image, cv::Rect( start, 0, length, PlotsT::height ), CV_RGB( 0, 0, 0 ), CV_FILLED );
		for (int i = 0; i < 4; i++)
		{
			if (isVisable[i])
			{
				cv::Point* subPlot = plots[i] + start - 1;
				int count = length + 1;
				if (start == 0)
				{
					subPlot++;
					count--;
				}
				cv::polylines( image, &subPlot, &count, 1, false, colors[i], 1 );
			}
		}
		cv::Mat newFrame( PlotsT::width / 2, PlotsT::height / 2, CV_8UC3 );
		cv::resize( image, newFrame, newFrame.size(), 0.0, 0.0, cv::INTER_CUBIC );
		cv::imshow( name, newFrame );
	}*/

private:
	cv::Mat img{ PlotsT::height, PlotsT::width, CV_8UC3 };
	//unsigned char frame[PlotsT::width * PlotsT::height * 3]{};
	cv::String name;
	cv::Scalar colors[PlotsT::count] = {
		CV_RGB( 255, 0, 0 ),
		CV_RGB( 0, 255, 0 ),
		CV_RGB( 0, 0, 255 ),
		CV_RGB( 255, 255, 255 )
	};
	bool isVisable[PlotsT::count] = { true, true, true, true };
	bool isZoomed = true;
};


} // namespace Task4
