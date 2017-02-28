//#define OCV

#include <iostream>
#include "opencv2/opencv.hpp"
#include "GripPipeline.h"
#include "networktables/NetworkTable.h"
#ifndef OCV
#include "LifecamCapture.h"
#endif

#define FRCTEAM 3504
#define MAXCONTOURS 5

int main(int argc, char** argv)
{
    grip::GripPipeline pipeline;
    std::vector<std::vector<cv::Point> > *contours;
    std::shared_ptr<NetworkTable> table;
    std::vector<double> center_xs;
    std::vector<double> center_ys;
    std::vector<double> widths;
    std::vector<double> heights;

    // Open the default camera
#ifdef OCV
    cv::VideoCapture cap;
#else
    LifecamCapture cap;
#endif
    if(!cap.open(0)) {
        std::cerr << "No camera found, exiting\n";
        return 0;
    }
    // Establish connection to Network Tables server
    NetworkTable::SetClientMode();
    NetworkTable::SetTeam(FRCTEAM);
    NetworkTable::Initialize();
    table = NetworkTable::GetTable("GRIP/myContoursReport");

    pipeline = grip::GripPipeline();

    // Run the pipeline forever
    int count;
    for(count = 0; /*count < 2*/; count++)
    {
          size_t imgsize;
          void *imgdata;
	  center_xs.clear();
	  center_ys.clear();
	  widths.clear();
	  heights.clear();
#ifdef OCV
	  cv::Mat frame;
          cap >> frame;
#else
          imgdata = cap.getImage();
          cv::Mat image(240, 320, CV_8UC3, (void*)imgdata);
	  cv::Mat frame = cv::imdecode(image, 1);
	  //cv::imwrite("image.jpg", frame);
#endif
          //imshow("this is you, smile! :)", frame);
          //if( waitKey(1) == 27 ) break; // stop capturing by pressing ESC 
          if( frame.empty() ) break; // end of video stream
	  pipeline.Process(frame);
	  //cv::imwrite("imagethresh.jpg", *(pipeline.GetHsvThresholdOutput()));
	  contours = pipeline.GetFilterContoursOutput();
#ifndef OCV
          cap.releaseImage();
#endif
	  int i = 0;
	  for (std::vector<cv::Point> contour: *contours) {
	    cv::Rect bb = cv::boundingRect(contour);
	    double area = contourArea(contour);
	    std::cout << i << ": " << bb << ", area = " << area << "\n";
	    center_xs.push_back(bb.x + (bb.width / 2.0));
	    center_ys.push_back(bb.y + (bb.height / 2.0));
	    widths.push_back(bb.width);
	    heights.push_back(bb.height);
	    // If we found too many contours, just skip the rest
	    if (i++ >= MAXCONTOURS)
	      break;
	  }
	  if (i == 0) {
	    std::cout << "No contours found\n";
	  }
	  table->PutNumberArray("centerX", center_xs);
	  table->PutNumberArray("centerY", center_ys);
	  table->PutNumberArray("width", widths);
	  table->PutNumberArray("height", heights);
    }
    cap.close();
    return 0;
}
