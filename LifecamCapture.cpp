#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include "LifecamCapture.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

void perror_dev(const char *dev, const char *s)
{
  fprintf(stderr, "Device %s: %s: %s\n", dev, s, strerror(errno));
}


int xioctl(int fh, int request, void *arg)
{
  int r;
  
  do {
    r = ioctl(fh, request, arg);
  } while (r == -1 && errno == EINTR);

  return r;
}

bool LifecamCapture::openDevice(const char *devName)
{
  struct v4l2_capability cap;
  struct v4l2_format fmt;
  struct v4l2_control ctl;
  struct v4l2_ext_controls ctls;
  struct v4l2_requestbuffers req;
  struct v4l2_buffer buf;

  fd = ::open(devName, O_RDWR /* required */ | O_NONBLOCK, 0);
  if (fd == -1) {
    perror_dev(devName, "Failed to open");
    return false;
  }

  if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
    if (errno == EINVAL || errno == ENOTTY)
      perror_dev(devName, "Not a V4L2 device");
    else
      perror("VIDIOC_QUERYCAP");
    return false;
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    errno = EINVAL;
    perror_dev(devName, "Does not support video capture");
    return false;
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    errno = EINVAL;
    perror_dev(devName, "Does not support streaming I/O");
    return false;
  }

  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = 320;
  fmt.fmt.pix.height      = 240;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  fmt.fmt.pix.field       = V4L2_FIELD_NONE;

  if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
    perror("VIDIOC_S_FMT");
    return false;
  }

  struct v4l2_ext_control ctl_array_user[] = {
    /* Structure members are: id, payload size, reserved, value */
    // Turn off auto white balance
    {V4L2_CID_AUTO_WHITE_BALANCE, 0, 0, 0},
    // Set white balance temperature (2800-10000, default 4500)
    //{V4L2_CID_WHITE_BALANCE_TEMPERATURE, 0, 0, 4500},
    // Turn off backlight compensation
    {V4L2_CID_BACKLIGHT_COMPENSATION, 0, 0, 0},
    // Brightness range is 30 - 255 with a default of 133
    {V4L2_CID_BRIGHTNESS, 0, 0, 133},
  };
  CLEAR(ctls);
  ctls.ctrl_class = V4L2_CTRL_CLASS_USER;
  ctls.count = sizeof(ctl_array_user)/sizeof(struct v4l2_ext_control);
  ctls.controls = ctl_array_user;
  if (xioctl(fd, VIDIOC_S_EXT_CTRLS, &ctls) == -1) {
    if (errno == EINVAL)
      perror_dev(devName, "Does not support camera user controls (brightness, etc.)");
    else
      perror("VIDIOC_S_EXT_CTRLS");
    return false;
  }

  struct v4l2_ext_control ctl_array_camera[] = {
    /* Structure members are: id, payload size, reserved, value */
    /* Turn off auto exposure */
    {V4L2_CID_EXPOSURE_AUTO, 0, 0, V4L2_EXPOSURE_MANUAL},
    /* Exposure range is 5 - 20,000 in units of 100 microsecs
     * Note that the Microsoft Lifecam doesn't implement a smooth range.
     * Instead, 5 - 10 is a small range of very low exposures,
     * and anything above 10 is a constant, very high exposure.
     */
    {V4L2_CID_EXPOSURE_ABSOLUTE, 0, 0, 19},
  };
  CLEAR(ctls);
  ctls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
  ctls.count = sizeof(ctl_array_camera)/sizeof(struct v4l2_ext_control);
  ctls.controls = ctl_array_camera;
  if (xioctl(fd, VIDIOC_S_EXT_CTRLS, &ctls) == -1) {
    if (errno == EINVAL)
      perror_dev(devName, "Does not support camera exposure controls");
    else
      perror("VIDIOC_S_EXT_CTRLS");
    return false;
  }

  CLEAR(req);
  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (xioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
    if (EINVAL == errno)
      perror_dev(devName, "Does not support memory mapped video capture");
    else
      perror("VIDIOC_REQBUFS");
    return false;
  }

  if (req.count < 1)
    perror_dev(devName, "Insufficient buffer memory for video capture");

  CLEAR(buf);
  buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory      = V4L2_MEMORY_MMAP;
  buf.index       = 0;

  if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
    perror("VIDIOC_QUERYBUF");
    return false;
  }

  buffer_length = buf.length;
  buffer_start = mmap(NULL /* start anywhere */,
		      buf.length,
		      PROT_READ | PROT_WRITE /* required */,
		      MAP_SHARED /* recommended */,
		      fd, buf.m.offset);

  if (buffer_start == MAP_FAILED) {
    perror("mmap");
    return false;
  }

  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;

  if (xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
    perror("VIDIOC_QBUF");
    return false;
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(fd, VIDIOC_STREAMON, &type) == -1) {
    perror("VIDIOC_STREAMON");
    return false;
  }

  return true;
}


LifecamCapture::LifecamCapture()
{
  fd = -1;
  opened = false;
}


LifecamCapture::LifecamCapture(int devNumber)
{
  // Space for "/dev/video" followed by enough digits for MAXINT written in decimal format
  char devName [100];
  int retval = snprintf(devName, sizeof(devName), "/dev/video%d", devNumber);
  if (devNumber < 0 || retval < 0 || retval > sizeof(devName)) {
    opened = false;
    return;
  }
  opened = openDevice(devName);
}

bool LifecamCapture::open(int devNumber)
{
  // Space for "/dev/video" followed by enough digits for MAXINT written in decimal format
  char devName [100];
  int retval = snprintf(devName, sizeof(devName), "/dev/video%d", devNumber);
  if (devNumber < 0 || retval < 0 || retval > sizeof(devName)) {
    return false;
  }
  if (opened) {
    LifecamCapture::release();
  }
  opened = openDevice(devName);
  return opened;
}


bool LifecamCapture::isOpened(void)
{
  return opened;
}


void *LifecamCapture::getImage(void)
{
  fd_set fds;
  struct timeval tv;
  int retval;
  struct v4l2_buffer buf;

  if (!opened) {
    return NULL;
  }

  do {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    /* Timeout. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    retval = select(fd + 1, &fds, NULL, NULL, &tv);
  } while ((retval == -1) && (errno == EINTR));

  if (retval == -1) {
    perror("select");
    return NULL;
  }

  if (retval == 0) {
    fprintf(stderr, "select timeout\n");
    return NULL;
  }

  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  do {
    retval = xioctl(fd, VIDIOC_DQBUF, &buf);
  } while (retval == -1 && errno == EAGAIN);

  if (retval == -1) {
    perror("VIDIOC_DQBUF");
    return NULL;
  }

  //Byte count of image is in buf.bytesused;
  return buffer_start;
}


void LifecamCapture::releaseImage(void)
{
  struct v4l2_buffer buf;

  if (!opened) {
    return;
  }

  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;

  if (xioctl(fd, VIDIOC_QBUF, &buf) == -1)
    perror("VIDIOC_QBUF");
}


void LifecamCapture::release(void)
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (!opened) {
    return;
  }

  if (xioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
    perror("VIDIOC_STREAMOFF");

  if (munmap(buffer_start, buffer_length) == -1)
    perror("munmap");

  if (close(fd) == -1)
    perror("close");

  fd = -1;
  opened = false;
}
