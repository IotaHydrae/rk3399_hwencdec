#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "RockchipRga.h"
#include <im2d.h>

#include <drm/drm.h>
#include "drm/drm_mode.h"

#define RGA_ALIGN(x, a) (((x) + (a)-1) / (a) * (a))
#define BUF_WIDTH 1280
#define BUF_HEIGHT 720
#define BUF_BPP 32

#define BUF_SIZE (BUF_WIDTH * BUF_HEIGHT * 4)

uint8_t *srcBuffer;
uint8_t *dstBuffer;

rga_info_t mSrcInfo;
rga_info_t mDstInfo;

RockchipRga mRkRga;
bo_t mSrcBo;
bo_t mDstBo;

/**
 * @brief
 *
 * @param
 *
 * @return
 */
int get_elapse_in_ms(struct timeval *tv)
{
    struct timeval this_tv;

    gettimeofday(&this_tv, NULL);
    uint64_t diff_sec = this_tv.tv_sec - tv->tv_sec;
    int elapse_in_ms;

    if (diff_sec == 0)
    {
        elapse_in_ms = (this_tv.tv_usec - tv->tv_usec) / 1000.0;
    }
    else
    {
        elapse_in_ms = ((--diff_sec) * 1000) + ((1000000 - tv->tv_usec) + this_tv.tv_usec) / 1000.0;
    }
    return elapse_in_ms;
}

/**
 * @brief get time elapse in us
 *
 * @param timeval tv
 *
 * @return
 */
int get_elapse_in_us(struct timeval *tv)
{
    struct timeval this_tv;

    gettimeofday(&this_tv, NULL);
    uint64_t diff_sec = this_tv.tv_sec - tv->tv_sec;
    int elapse_in_us;

    if (diff_sec == 0)
    {
        elapse_in_us = (this_tv.tv_usec - tv->tv_usec);
    }
    else
    {
        elapse_in_us = ((--diff_sec) * 1000000) + ((1000000 - tv->tv_usec) + this_tv.tv_usec);
    }
    return elapse_in_us;
}

/**
 * @brief
 *
 * @param
 *
 * @return
 */
void make_random_data()
{
    int rand_fd;
    rand_fd = open("/dev/urandom", O_RDONLY);
    /*unsigned char *rand_base;

    rand_base = (unsigned char *)mmap(NULL, BUF_SIZE,PROT_READ,MAP_SHARED,rand_fd,0);
    perror("rand_base");
    printf("mmap done\n");
    memcpy(srcBuffer, rand_base, BUF_SIZE);

    munmap(rand_base, BUF_SIZE);*/
    read(rand_fd, srcBuffer, BUF_SIZE);
    close(rand_fd);
}

void check_data()
{
    for (int i = 0; i < BUF_SIZE; i++)
    {
        if (srcBuffer[i] != dstBuffer[i])
        {
            printf("[diff at pos: %d] src: [%d] dst: [%d]\n", i, srcBuffer[i], dstBuffer[i]);
        }
    }
}

int rga_copy()
{
    int ret;
    struct timeval tv;

    mRkRga.RkRgaInit();
    mRkRga.RkRgaGetAllocBuffer(&mSrcBo, BUF_WIDTH, BUF_HEIGHT, BUF_BPP);
    mRkRga.RkRgaGetAllocBuffer(&mDstBo, BUF_WIDTH, BUF_HEIGHT, BUF_BPP);

    mRkRga.RkRgaGetMmap(&mSrcBo);
    mRkRga.RkRgaGetMmap(&mDstBo);

    printf("mSrcBo .ptr %p\n", mSrcBo.ptr);

    mRkRga.RkRgaGetBufferFd(&mSrcBo, &mSrcInfo.fd);
    mRkRga.RkRgaGetBufferF d(&mDstBo, &mDstInfo.fd);

    rga_set_rect(&mSrcInfo.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);
    rga_set_rect(&mDstInfo.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);

    gettimeofday(&tv, NULL);
    mRkRga.RkRgaBlit(&mSrcInfo, &mDstInfo, NULL);
    printf("[c_RkRgaBlit] elapse time: %d ms\n", get_elapse_in_ms(&tv));
}

int main(int argc, char **argv)
{
    rga_copy();
    return 0;
}