#include "LifecamCapture.h"
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

static void process_image(const void *imgdata, int i)
{
  char fname[] = "image0.jpg";

  cv::Mat image(480, 640, CV_8UC3, (void*)imgdata);
  cv::Mat frame = cv::imdecode(image, 1);
  fname[5] = '0' + i;
  printf("Filename: %s\n", fname);
  cv::imwrite(fname, frame);
}


int main(int argc, char **argv)
{
  int i;
  size_t imgsize;
  void *imgdata;
  int default_devnum = 0;
  int devnum = default_devnum;

  if (argc > 2) {
    fprintf(stderr, "Usage: %s [video-device-number]\n  Defaults to %d\n", 
            argv[0], default_devnum);
    return 1;
  }
  if (argc == 2) {
    devnum = atoi(argv[1]);
  }

  LifecamCapture cap(devnum);

  for (i = 0; i < 5; i++) {
    imgdata = cap.getImage();
    process_image(imgdata, i);
    cap.releaseImage();
  }

  cap.release();

  return 0;
}

