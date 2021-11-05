#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#define DEFAULT_CAMERA_PATH "/dev/video1"

int query_list[] = {
    // VIDIOC_REQBUFS   ：分配内存
    // VIDIOC_QUERYBUF  ：把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址
    VIDIOC_QUERYCAP, //：查询驱动功能
    VIDIOC_ENUM_FMT, //：获取当前驱动支持的视频格式
    VIDIOC_S_FMT,    //：设置当前驱动的频捕获格式
    VIDIOC_G_FMT,    //：读取当前驱动的频捕获格式
    VIDIOC_TRY_FMT,  //：验证当前驱动的显示格式
    VIDIOC_CROPCAP,  //：查询驱动的修剪能力
    // VIDIOC_S_CROP    ,//：设置视频信号的边框
    // VIDIOC_G_CROP    ,//：读取视频信号的边框
    // VIDIOC_QBUF      ,//：把数据从缓存中读取出来
    // VIDIOC_DQBUF     ,//：把数据放回缓存队列
    // VIDIOC_STREAMON  ,//：开始视频显示函数
    // VIDIOC_STREAMOFF ,//：结束视频显示函数
    VIDIOC_QUERYSTD, //：检查当前视频设备支持的标准，例如PAL或NTSC。
};

int query_video_info(int fd, int request)
{
    int ret;
    char dummy[256];

    ret = ioctl(fd, request, dummy);
    if (ret < 0)
    {
        perror("error on ioctl!");
        return -errno;
    }
    printf("%s\n", dummy);
}

int main(int argc, char **argv)
{
    int fd;
    int ret;
    struct v4l2_capability v4l2_cap;

    fd = open(DEFAULT_CAMERA_PATH, O_RDONLY);
    if (fd < 0)
    {
        perror("error on open cam!");
        return -errno;
    }

    // for (int i = 0; i < sizeof(query_list) / sizeof(int); i++)
    // {
    //     query_video_info(fd, query_list[i]);
    // }
    ret = ioctl(fd, VIDIOC_QUERYCAP, &v4l2_cap);
    if (ret < 0)
    {
        perror("error on ioctl!");
        return -errno;
    }
    printf("%s\n", v4l2_cap.driver);
    printf("%s\n", v4l2_cap.card);
    printf("%s\n", v4l2_cap.bus_info);

    printf("%d\n", v4l2_cap.version);
    printf("%d\n", v4l2_cap.capabilities);
    printf("%d\n", v4l2_cap.device_caps);
    
    return 0;
}