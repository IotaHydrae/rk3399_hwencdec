

#ifndef __V4L2_CAPTURER_HXX
#define __V4L2_CAPTURER_HXX

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/ioctl.h>

#include <linux/videodev2.h>

using namespace std;

#define 

class V4l2Capturer {

public:
    V4l2Capturer();
    ~V4l2Capturer();

    int Init();
    int GetFrame();

private:


};

#endif