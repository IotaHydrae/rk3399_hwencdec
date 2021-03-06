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

int                        m_fd;
int                        m_rb_count;
int                        m_rb_current;
int                        m_total_bytes;
struct v4l2_capability     m_cap;
struct v4l2_format         m_fmt;
struct v4l2_fmtdesc        m_desc_fmt;
struct v4l2_requestbuffers m_rb;
struct v4l2_buffer         m_buf;
struct v4l2_frmsizeenum    m_frmsize;
int                        m_max_vide_buff_size;

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

	if(v4l2_cap.capabilities & V4L2_CAP_STREAMING)
	{
		fprintf(stderr, "support streaming i/o\n");
	}

	if(v4l2_cap.capabilities & V4L2_CAP_READWRITE) 
	{
		fprintf(stderr, "support read i/o\n");
	}
    
    //support format
	memset(&m_desc_fmt, 0, sizeof(m_desc_fmt));
	m_desc_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	 ioctl(m_fd, VIDIOC_ENUM_FMT, &m_desc_fmt);
	printf("pixelformat: 0x%x\n", m_desc_fmt.pixelformat);

	while(0 <= (ret = ioctl(m_fd, VIDIOC_ENUM_FMT, &m_desc_fmt))) 
	{
		printf("pixelformat: 0x%x\n", m_desc_fmt.pixelformat);

		if(V4L2_PIX_FMT_MJPEG == m_desc_fmt.pixelformat)
		{
			printf("support pixelformat MJPEG\n");
		}

		if(V4L2_PIX_FMT_RGB565 == m_desc_fmt.pixelformat)
		{
			printf("support pixelformat RGB565\n");
		}

		if(V4L2_PIX_FMT_YUYV == m_desc_fmt.pixelformat)
		{
			printf("support pixelformat YUYV\n");
		}

		m_frmsize.pixel_format = m_desc_fmt.pixelformat;
		m_frmsize.index = 0;
		while(0 <= ioctl(m_fd, VIDIOC_ENUM_FRAMESIZES, &m_frmsize))
		{
			if(V4L2_FRMSIZE_TYPE_DISCRETE == m_frmsize.type ||
				V4L2_FRMSIZE_TYPE_STEPWISE == m_frmsize.type)
			{
				printf("support frmsize:%d-%d\n",
						m_frmsize.discrete.width, m_frmsize.discrete.height);
			}
			m_frmsize.index++;
		}

		m_desc_fmt.index++;
	}

    return 0;
}