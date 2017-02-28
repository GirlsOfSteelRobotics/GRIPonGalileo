
#ifndef __LIFECAMCAPTURE_H_INCLUDED__
#define __LIFECAMCAPTURE_H_INCLUDED__

#include <sys/types.h>  // for size_t

class LifecamCapture {

public:
  LifecamCapture();
  LifecamCapture(int devNumber);
  bool open(int devNumber);
  bool isOpened(void);
  void *getImage(void);
  void releaseImage(void);
  void close(void);
private:
  bool    openDevice(const char *devName);
  void   *buffer_start;
  size_t  buffer_length;
  int     fd;
  bool    opened;

};
#endif
