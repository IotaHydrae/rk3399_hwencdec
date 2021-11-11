
#include "v4l2_capturer.h"

v4l2_capturer::v4l2_capturer(): m_fd{-1}
{
    LOG_DEBUG("v4l2_capturer created.");
}

v4l2_capturer::~v4l2_capturer()
{
    LOG_DEBUG("v4l2_capturer destoryed.");
    for(int i = 0; i < DEFAULT_BUFFER_COUNT; i++) {
        if(m_video_buffers[i].start) {
            munmap(m_video_buffers[i].start, m_video_buff_size);
        }
    }
    if(m_video_buffers)
    { free(m_video_buffers); }
    if(m_fd != -1) {
        close(m_fd);
    }
}

int v4l2_capturer::init()
{
    int ret = -1;
    m_fd = open(DEFAULT_CAMERA_PATH, O_RDWR);
    if(m_fd < 0) {
        perror("open failed.");
        return -1;
    }
    memset(&m_cap, 0, sizeof(m_cap));
    if(ioctl(m_fd, VIDIOC_QUERYCAP, &m_cap) < 0) {
        perror("ioctl failed");
        return -1;
    }
    if(m_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        printf("V4L2_CAP_VIDEO_CAPTURE supportted\n");
    }
    if(m_cap.capabilities & V4L2_CAP_STREAMING) {
        printf("V4L2_CAP_STREAMING supportted\n");
    }
    if(m_cap.capabilities & V4L2_CAP_READWRITE) {
        printf("V4L2_CAP_READWRITE supportted\n");
    }
    /* set format */
    memset(&m_fmt, 0x0, sizeof(m_fmt));
    m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_fmt.fmt.pix.width = 640;
    m_fmt.fmt.pix.height = 480;
    m_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    m_fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    if(ioctl(m_fd, VIDIOC_S_FMT, &m_fmt) < 0) {
        perror("ioctl VIDIOC_S_FMT failed!");
        return -1;
    }
    /* request framebuffer */
    memset(&m_rb, 0x0, sizeof(m_rb));
    m_rb.count = DEFAULT_BUFFER_COUNT;
    m_rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_rb.memory = V4L2_MEMORY_MMAP;
    if(ioctl(m_fd, VIDIOC_REQBUFS, &m_rb) < 0) {
        perror("ioctl VIDIOC_REQBUFS failed!");
        return -1;
    }
    m_video_buffers = (struct video_buffer *)calloc(m_rb.count, sizeof(struct video_buffer));
    /* map the framebuffer to userspace */
    for(int index_buf = 0; index_buf < m_rb.count; index_buf++) {
        /* calloc the userspace buffer */
        memset(&m_buf, 0x0, sizeof(m_buf));
        m_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_buf.memory = V4L2_MEMORY_MMAP;
        m_buf.index = index_buf;
        if(ioctl(m_fd, VIDIOC_QUERYBUF, &m_buf) < 0) {
            perror("ioctl VIDIOC_QUERYBUF failed!");
            return -1;
        }
        m_video_buff_size = m_buf.length;
        /* map to userspace  */
        m_video_buffers[index_buf].start = (unsigned char *)mmap(NULL,
                                           m_buf.length,
                                           PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, m_buf.m.offset);
        if(m_video_buffers[index_buf].start == MAP_FAILED) {
            perror("map video buf failed!");
            return -1;
        }
		//memset(m_video_buffers[index_buf].start, 0x0, m_buf.length);
        /* queue the buffer */
        memset(&m_buf, 0x0, sizeof(m_buf));
        m_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_buf.memory = V4L2_MEMORY_MMAP;
        m_buf.index = index_buf;
        if(ioctl(m_fd, VIDIOC_QBUF, &m_buf) < 0) {
            perror("ioctl VIDIOC_QBUF failed!");
            return -1;
        }
    }
	printf("v4l2_buf size: %d\n", m_buf.length);
    return 0;
}

int v4l2_capturer::start()
{
    LOG_DEBUG("***start stream.");
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
        perror("ioctl VIDIOC_STREAMON failed!");
        return -1;
    }
    return 0;
}

int v4l2_capturer::stop()
{
    LOG_DEBUG("***stop stream.");
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(m_fd, VIDIOC_STREAMOFF, &type) < 0) {
        perror("ioctl VIDIOC_STREAMOFF failed!");
        return -1;
    }
    return 0;
}

int v4l2_capturer::get_frame()
{
    int ret;
	static int frame_count=1;
    struct v4l2_buffer v4lbuffer;
	   /* blocked here */
    /*struct pollfd my_pfds[1];
    LOG_DEBUG("***get frame.");
    my_pfds[0].fd = m_fd;
    my_pfds[0].events = POLLIN;
    my_pfds[0].revents = POLLIN;
 
    ret = poll(my_pfds, 1, -1);
    if(ret < 0) {
        perror("poll failed!");
        return ret;
    }*/

    /* pop fb from queue */
    memset(&v4lbuffer, 0x0, sizeof(v4lbuffer));
    v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuffer.memory = V4L2_MEMORY_MMAP;
    if(ioctl(m_fd, VIDIOC_DQBUF, &v4lbuffer) < 0) {
        perror("ioctl VIDIOC_DQBUF failed!");
        return ret;
    }
    m_rb_current = v4lbuffer.index;
    m_total_bytes = v4lbuffer.bytesused;


	int ffd;
    unsigned char *file_base;
	char jpeg_fname[50];
	sprintf(jpeg_fname, "./abc_%04d.jpeg", frame_count);
//    struct v4l2_buffer v4lbuffer;
    LOG_DEBUG("***save_fbdata_to_file by mmap.");
    ffd = open(jpeg_fname, O_RDWR | O_CREAT | O_TRUNC, 0755);
    if(ffd < 0) {
        perror("open path failed!");
        return ffd;
    }
    /* make file length */
    lseek(ffd, m_total_bytes - 1, SEEK_END);
    write(ffd, "", 1);   /* because of COW? */
    printf("total_bytes:%d\n", m_total_bytes);
    printf("current:%d\n", m_rb_current);
    /* write operation here */
    file_base = (unsigned char *)mmap(NULL, m_total_bytes,
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      ffd, 0);
    if(file_base == MAP_FAILED) {
        perror("mmap file failed!");
        return -1;
    }
    LOG_DEBUG("memcpying...");
    memcpy(file_base, m_video_buffers[m_rb_current].start, m_total_bytes);
    /* release the resource */
    munmap(file_base, m_total_bytes);
    close(ffd);
	frame_count++;
    /* push back fb to queue */
    LOG_DEBUG("push back fb to queue");
    memset(&v4lbuffer, 0x0, sizeof(v4lbuffer));
    v4lbuffer.index = m_rb_current;
    v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuffer.memory = V4L2_MEMORY_MMAP;
    if(ioctl(m_fd, VIDIOC_QBUF, &v4lbuffer) < 0) {
        perror("ioctl VIDIOC_QBUF failed!");
        return -1;
    }
	//if(frame_count > 24)exit(1);
    return frame_count;
}

int v4l2_capturer::save_fbdata_to_file(const char *path)
{
    int fd;
    struct v4l2_buffer v4lbuffer;
    LOG_DEBUG("***save_fbdata_to_file.");
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0755);
    if(fd < 0) {
        perror("open path failed!");
        return fd;
    }
    printf("total_bytes:%d\n", m_total_bytes);
    printf("current:%d\n", m_rb_current);
    write(fd, m_video_buffers[m_rb_current].start, m_total_bytes);
    close(fd);
    /* push back fb to queue */
    /* queue the buffer */
    memset(&v4lbuffer, 0x0, sizeof(v4lbuffer));
    v4lbuffer.index = m_rb_current;
    v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuffer.memory = V4L2_MEMORY_MMAP;
    if(ioctl(m_fd, VIDIOC_QBUF, &v4lbuffer) < 0) {
        perror("ioctl VIDIOC_QBUF failed!");
        return -1;
    }

  
    return 0;
}

int v4l2_capturer::save_fbdata_to_file_by_mmap(const char *path)
{
    int fd;
    unsigned char *file_base;
    struct v4l2_buffer v4lbuffer;
    LOG_DEBUG("***save_fbdata_to_file by mmap.");
    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0755);
    if(fd < 0) {
        perror("open path failed!");
        return fd;
    }
    /* make file length */
    lseek(fd, m_total_bytes - 1, SEEK_END);
    write(fd, "", 1);   /* because of COW? */
    printf("total_bytes:%d\n", m_total_bytes);
    printf("current:%d\n", m_rb_current);
    /* write operation here */
    file_base = (unsigned char *)mmap(NULL, m_total_bytes,
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      fd, 0);
    if(file_base == MAP_FAILED) {
        perror("mmap file failed!");
        return -1;
    }
    LOG_DEBUG("memcpying...");
    memcpy(file_base, m_video_buffers[m_rb_current].start, m_total_bytes);
    /* release the resource */
    munmap(file_base, m_total_bytes);
    close(fd);
    /* push back fb to queue */
    LOG_DEBUG("push back fb to queue");
    memset(&v4lbuffer, 0x0, sizeof(v4lbuffer));
    v4lbuffer.index = m_rb_current;
    v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuffer.memory = V4L2_MEMORY_MMAP;
    if(ioctl(m_fd, VIDIOC_QBUF, &v4lbuffer) < 0) {
        perror("ioctl VIDIOC_QBUF failed!");
        return -1;
    }
    return 0;
}

void v4l2_capturer::query_supported_format()
{
    int ret;
    m_fd = open(DEFAULT_CAMERA_PATH, O_RDWR);
    if(m_fd < 0) {
        perror("opening camera.");
        return;
    }
    memset(&m_desc_fmt, 0, sizeof(m_desc_fmt));
    m_desc_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while(0 <= (ret = ioctl(m_fd, VIDIOC_ENUM_FMT, &m_desc_fmt))) {
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB332);                /*  8  RGB-3-3-2     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB444);                /* 16  xxxxrrrr ggggbbbb */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ARGB444);               /* 16  aaaarrrr ggggbbbb */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XRGB444);               /* 16  xxxxrrrr ggggbbbb */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGBA444);               /* 16  rrrrgggg bbbbaaaa */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGBX444);               /* 16  rrrrgggg bbbbxxxx */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ABGR444);               /* 16  aaaabbbb ggggrrrr */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XBGR444);               /* 16  xxxxbbbb ggggrrrr */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGRA444);               /* 16  bbbbgggg rrrraaaa */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGRX444);               /* 16  bbbbgggg rrrrxxxx */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB555);                /* 16  RGB-5-5-5     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ARGB555);               /* 16  ARGB-1-5-5-5  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XRGB555);               /* 16  XRGB-1-5-5-5  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGBA555);               /* 16  RGBA-5-5-5-1  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGBX555);               /* 16  RGBX-5-5-5-1  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ABGR555);               /* 16  ABGR-1-5-5-5  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XBGR555);               /* 16  XBGR-1-5-5-5  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGRA555);               /* 16  BGRA-5-5-5-1  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGRX555);               /* 16  BGRX-5-5-5-1  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB565);                /* 16  RGB-5-6-5     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB555X);               /* 16  RGB-5-5-5 BE  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ARGB555X);              /* 16  ARGB-5-5-5 BE */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XRGB555X);              /* 16  XRGB-5-5-5 BE */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB565X);               /* 16  RGB-5-6-5 BE  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGR666);                /* 18  BGR-6-6-6      */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGR24);                 /* 24  BGR-8-8-8     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB24);                 /* 24  RGB-8-8-8     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGR32);                 /* 32  BGR-8-8-8-8   */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ABGR32);                /* 32  BGRA-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XBGR32);                /* 32  BGRX-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGRA32);                /* 32  ABGR-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_BGRX32);                /* 32  XBGR-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGB32);                 /* 32  RGB-8-8-8-8   */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGBA32);                /* 32  RGBA-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_RGBX32);                /* 32  RGBX-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ARGB32);                /* 32  ARGB-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XRGB32);                /* 32  XRGB-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y4);                    /*  4  Greyscale     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y6);                    /*  6  Greyscale     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y10);                   /* 10  Greyscale     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y12);                   /* 12  Greyscale     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y14);                   /* 14  Greyscale     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y16);                   /* 16  Greyscale     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y16_BE);                /* 16  Greyscale BE  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y10BPACK);              /* 10  Greyscale bit-packed */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y10P);                  /* 10  Greyscale, MIPI RAW10 packed */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_PAL8);                  /*  8  8-bit palette */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_UV8);                   /*  8  UV 4:4 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUYV);                  /* 16  YUV 4:2:2     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YYUV);                  /* 16  YUV 4:2:2     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YVYU);                  /* 16 YVU 4:2:2 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_UYVY);                  /* 16  YUV 4:2:2     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VYUY);                  /* 16  YUV 4:2:2     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y41P);                  /* 12  YUV 4:1:1     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV444);                /* 16  xxxxyyyy uuuuvvvv */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV555);                /* 16  YUV-5-5-5     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV565);                /* 16  YUV-5-6-5     */
        //CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV24                 );  /* 24  YUV-8-8-8     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV32);                 /* 32  YUV-8-8-8-8   */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_AYUV32);                /* 32  AYUV-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XYUV32);                /* 32  XYUV-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VUYA32);                /* 32  VUYA-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VUYX32);                /* 32  VUYX-8-8-8-8  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_M420);                  /* 12  YUV 4:2:0 2 lines y, 1 line uv interleaved */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV12);                  /* 12  Y/CbCr 4:2:0  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV21);                  /* 12  Y/CrCb 4:2:0  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV16);                  /* 16  Y/CbCr 4:2:2  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV61);                  /* 16  Y/CrCb 4:2:2  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV24);                  /* 24  Y/CbCr 4:4:4  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV42);                  /* 24  Y/CrCb 4:4:4  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_HM12);                  /*  8  YUV 4:2:0 16x16 macroblocks */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV12M);                 /* 12  Y/CbCr 4:2:0  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV21M);                 /* 21  Y/CrCb 4:2:0  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV16M);                 /* 16  Y/CbCr 4:2:2  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV61M);                 /* 16  Y/CrCb 4:2:2  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV12MT);                /* 12  Y/CbCr 4:2:0 64x32 macroblocks */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_NV12MT_16X16);          /* 12  Y/CbCr 4:2:0 16x16 macroblocks */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV410);                /*  9  YUV 4:1:0     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YVU410);                /*  9  YVU 4:1:0     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV411P);               /* 12  YVU411 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV420);                /* 12  YUV 4:2:0     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YVU420);                /* 12  YVU 4:2:0     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV422P);               /* 16  YVU422 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV420M);               /* 12  YUV420 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YVU420M);               /* 12  YVU420 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV422M);               /* 16  YUV422 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YVU422M);               /* 16  YVU422 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YUV444M);               /* 24  YUV444 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_YVU444M);               /* 24  YVU444 planar */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR8);                /*  8  BGBG.. GRGR.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG8);                /*  8  GBGB.. RGRG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG8);                /*  8  GRGR.. BGBG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB8);                /*  8  RGRG.. GBGB.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR10);               /* 10  BGBG.. GRGR.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG10);               /* 10  GBGB.. RGRG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG10);               /* 10  GRGR.. BGBG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB10);               /* 10  RGRG.. GBGB.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR10P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG10P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG10P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB10P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR10ALAW8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG10ALAW8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG10ALAW8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB10ALAW8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR10DPCM8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG10DPCM8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG10DPCM8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB10DPCM8);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR12);               /* 12  BGBG.. GRGR.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG12);               /* 12  GBGB.. RGRG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG12);               /* 12  GRGR.. BGBG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB12);               /* 12  RGRG.. GBGB.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR12P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG12P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG12P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB12P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR14);               /* 14  BGBG.. GRGR.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG14);               /* 14  GBGB.. RGRG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG14);               /* 14  GRGR.. BGBG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB14);               /* 14  RGRG.. GBGB.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR14P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG14P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG14P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB14P);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SBGGR16);               /* 16  BGBG.. GRGR.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGBRG16);               /* 16  GBGB.. RGRG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SGRBG16);               /* 16  GRGR.. BGBG.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SRGGB16);               /* 16  RGRG.. GBGB.. */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_HSV24);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_HSV32);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MJPEG);                 /* Motion-JPEG   */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_JPEG);                  /* JFIF JPEG     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_DV);                    /* 1394          */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MPEG);                  /* MPEG-1/2/4 Multiplexed */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_H264);                  /* H264 with start codes */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_H264_NO_SC);            /* H264 without start codes */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_H264_MVC);              /* H264 MVC */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_H263);                  /* H263          */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MPEG1);                 /* MPEG-1 ES     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MPEG2);                 /* MPEG-2 ES     */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MPEG2_SLICE);           /* MPEG-2 parsed slice data */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MPEG4);                 /* MPEG-4 part 2 ES */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_XVID);                  /* Xvid           */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VC1_ANNEX_G);           /* SMPTE 421M Annex G compliant stream */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VC1_ANNEX_L);           /* SMPTE 421M Annex L compliant stream */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VP8);                   /* VP8 */
        //CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VP8_FRAME             );  /* VP8 parsed frame */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_VP9);                   /* VP9 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_HEVC);                  /* HEVC aka H.265 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_FWHT);                  /* Fast Walsh Hadamard Transform (vicodec) */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_FWHT_STATELESS);        /* Stateless FWHT (vicodec) */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_H264_SLICE);            /* H264 parsed slices */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_CPIA1);                 /* cpia1 YUV */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_WNVA);                  /* Winnov hw compress */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SN9C10X);               /* SN9C10x compression */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SN9C20X_I420);          /* SN9C20x YUV 4:2:0 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_PWC1);                  /* pwc older webcam */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_PWC2);                  /* pwc newer webcam */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_ET61X251);              /* ET61X251 compression */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SPCA501);               /* YUYV per line */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SPCA505);               /* YYUV per line */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SPCA508);               /* YUVY per line */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SPCA561);               /* compressed GBRG bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_PAC207);                /* compressed BGGR bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MR97310A);              /* compressed BGGR bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_JL2005BCD);             /* compressed RGGB bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SN9C2028);              /* compressed GBRG bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SQ905C);                /* compressed RGGB bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_PJPG);                  /* Pixart 73xx JPEG */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_OV511);                 /* ov511 JPEG */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_OV518);                 /* ov518 JPEG */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_STV0680);               /* stv0680 bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_TM6000);                /* tm5600/tm60x0 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_CIT_YYVYUY);            /* one line of Y then 1 line of VYUY */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_KONICA420);             /* YUV420 planar in blocks of 256 pixels */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_JPGL);                   /* JPEG-Lite */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SE401);                 /* se401 janggu compressed rgb */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_S5C_UYVY_JPG);          /* S5C73M3 interleaved UYVY/JPEG */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y8I);                   /* Greyscale 8-bit L/R interleaved */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Y12I);                  /* Greyscale 12-bit L/R interleaved */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_Z16);                   /* Depth data 16-bit */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_MT21C);                 /* Mediatek compressed block mode  */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_INZI);                  /* Intel Planar Greyscale 10-bit and Depth 16-bit */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_SUNXI_TILED_NV12);      /* Sunxi Tiled NV12 Format */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_CNF4);                  /* Intel 4-bit packed depth confidence information */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_HI240);                 /* BTTV 8-bit dithered RGB */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_IPU3_SBGGR10);           /* IPU3 packed 10-bit BGGR bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_IPU3_SGBRG10);           /* IPU3 packed 10-bit GBRG bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_IPU3_SGRBG10);           /* IPU3 packed 10-bit GRBG bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_IPU3_SRGGB10);           /* IPU3 packed 10-bit RGGB bayer */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_CU8);                   /* IQ u8 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_CU16LE);                /* IQ u16le */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_CS8);                   /* complex s8 */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_CS14LE);                /* complex s14le */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_RU12LE);                /* real u12le */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_PCU16BE);              /* planar complex u16be */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_PCU18BE);              /* planar complex u18be */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_SDR_FMT_PCU20BE);              /* planar complex u20be */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_TCH_FMT_DELTA_TD16);             /* 16-bit signed deltas */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_TCH_FMT_DELTA_TD08);             /* 8-bit signed deltas */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_TCH_FMT_TU16);                   /* 16-bit unsigned touch data */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_TCH_FMT_TU08);                   /* 8-bit unsigned touch data */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_META_FMT_VSP1_HGO);             /* R-Car VSP1 1-D Histogram */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_META_FMT_VSP1_HGT);             /* R-Car VSP1 2-D Histogram */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_META_FMT_UVC);                  /* UVC Payload Header metadata */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_META_FMT_D4XX);                 /* D4XX Payload Header metadata */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_META_FMT_VIVID);                 /* Vivid Metadata */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_META_FMT_RK_ISP1_PARAMS);       /* Rockchip ISP1 3A Parameters */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_META_FMT_RK_ISP1_STAT_3A);       /* Rockchip ISP1 3A Statistics */
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_PRIV_MAGIC);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_FLAG_PREMUL_ALPHA);
        CHECK_FMT_IF_SUPPORTED(m_desc_fmt.pixelformat, V4L2_PIX_FMT_FLAG_SET_CSC);
        m_desc_fmt.index++;
    }
}

void v4l2_capturer::query_supported_format_new()
{
	struct v4l2_fmtdesc fmt_desc;
	if(m_fd < 0){
		printf("device haven't been openned!\n");
		return;
	}

	memset(&fmt_desc, 0x0, sizeof(fmt_desc));
	fmt_desc.index=0;
	fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("Format supportted:\n");
	while(ioctl(m_fd, VIDIOC_ENUM_FMT, &fmt_desc)!=-1){
		printf("\t%d. Type: %s\n", fmt_desc.index, fmt_desc.description);
		fmt_desc.index++;
	}
}

