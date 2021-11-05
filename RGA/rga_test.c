#include <stdio.h>
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

#include <RgaApi.h>
#include <im2d.h>

#include <drm/drm.h>
#include "drm/drm_mode.h"

#define RGA_ALIGN(x, a) (((x) + (a)-1) / (a) * (a))
#define BUF_WIDTH 1280
#define BUF_HEIGHT 720

#define BUF_SIZE (BUF_WIDTH * BUF_HEIGHT * 4)

uint8_t *srcBuffer;
uint8_t *dstBuffer;

rga_info_t src;
rga_info_t dst;
rga_info_t src1;

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

void rga_copy_vir()
{
	src.virAddr = srcBuffer;
	dst.virAddr = dstBuffer;

	rga_set_rect(&src.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);
	rga_set_rect(&dst.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);

	c_RkRgaBlit(&src, &dst, NULL);
}

void rga_copy_dma_fd()
{
	int ret;
	bo_t bo_src, bo_dst;
	struct timeval tv;

	memset(&bo_src, 0x0, sizeof(bo_t));
	memset(&bo_dst, 0x0, sizeof(bo_t));

	c_RkRgaGetAllocBuffer(&bo_src, BUF_WIDTH, BUF_HEIGHT, 32);
	c_RkRgaGetAllocBuffer(&bo_dst, BUF_WIDTH, BUF_HEIGHT, 32);

	c_RkRgaGetMmap(&bo_src);
	c_RkRgaGetMmap(&bo_dst);
	memset(&src, 0x0, sizeof(rga_info_t));
	src.fd = -1;
	src.mmuFlag = 1;
	memset(&dst, 0x0, sizeof(rga_info_t));
	dst.fd = -1;
	dst.mmuFlag = 1;

	printf("bo_src ptr %p\n", bo_src.ptr);

	ret = c_RkRgaGetBufferFd(&bo_src, &src.fd);
	if (ret)
	{
		fprintf(stderr, "can't get buffer fd of src!\n");
		perror("c_RkRgaGetBufferFd src");
	}

	printf("Buffer fd of src: %d\n", src.fd);

	ret = c_RkRgaGetBufferFd(&bo_dst, &dst.fd);
	if (ret)
	{
		fprintf(stderr, "can't get buffer fd of dst!\n");
		perror("c_RkRgaGetBufferFd dst");
	}

	printf("Buffer fd of src: %d\n", dst.fd);

	rga_set_rect(&src.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);
	rga_set_rect(&dst.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);

	gettimeofday(&tv, NULL);
	c_RkRgaBlit(&src, &dst, NULL);
	printf("[c_RkRgaBlit] elapse time: %d ms\n", get_elapse_in_ms(&tv));
}

int syber_rga_copy_dma_test()
{
	bo_t bo_src, bo_dst;
	int buff_fd_src, buff_fd_dst;
	struct timeval tv;

	static const char *card = "/dev/dri/card0";
	int drm_fd;
	int flag = O_RDWR;
	drm_fd = open(card, flag);
	if (drm_fd < 0)
	{
		fprintf(stderr, "Fail to open %s: %m\n", card);
		return -errno;
	}

	/* create dumb */
	struct drm_mode_create_dumb arg;
	struct drm_mode_create_dumb arg2;
	int ret;

	memset(&arg, 0, sizeof(arg));
	arg.bpp = 32;
	arg.width = BUF_WIDTH;
	arg.height = BUF_HEIGHT;

	memset(&arg2, 0, sizeof(arg2));
	arg2.bpp = 32;
	arg2.width = BUF_WIDTH;
	arg2.height = BUF_HEIGHT;
	// arg.flags = 0;

	ret = ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
	if (ret)
	{
		fprintf(stderr, "can't alloc drm dumb buffer src!\n");
		return -errno;
	}

	ret = ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg2);
	if (ret)
	{
		fprintf(stderr, "can't alloc drm dumb buffer dst!\n");
		return -errno;
	}

	bo_src.fd = drm_fd;
	bo_src.handle = arg.handle;
	bo_src.size = arg.size;
	bo_src.pitch = arg.pitch;

	bo_dst.fd = drm_fd;
	bo_dst.handle = arg2.handle;
	bo_dst.size = arg2.size;
	bo_dst.pitch = arg2.pitch;

	printf("arg  %u %u %lu \n", arg.handle, arg.pitch, arg.size);
	printf("arg2 %u %u %lu \n", arg2.handle, arg2.pitch, arg2.size);

	c_RkRgaGetMmap(&bo_src);
	c_RkRgaGetMmap(&bo_dst);
	memset(&src, 0x0, sizeof(rga_info_t));
	memset(&dst, 0x0, sizeof(rga_info_t));
	src.fd = -1;
	dst.fd = -1;
	src.mmuFlag = 1;
	dst.mmuFlag = 1;

	ret = c_RkRgaGetBufferFd(&bo_src, &src.fd);
	if (ret)
	{
		fprintf(stderr, "can't get buffer fd of src!\n");
		perror("c_RkRgaGetBufferFd src");
		return;
	}
	printf("Buffer fd of src: %d\n", src.fd);

	c_RkRgaGetBufferFd(&bo_dst, &dst.fd);
	if (ret)
	{
		fprintf(stderr, "can't get buffer fd of dst!\n");
		perror("c_RkRgaGetBufferFd dst");
		return;
	}
	printf("Buffer fd of src: %d\n", dst.fd);

	rga_set_rect(&src.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);
	rga_set_rect(&dst.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);

	gettimeofday(&tv, NULL);
	c_RkRgaBlit(&src, &dst, NULL);
	printf("[c_RkRgaBlit] elapse time: %d ms\n", get_elapse_in_ms(&tv));
}

int rga_copy()
{
	bo_t bo_src, bo_dst;
	int buff_fd_src, buff_fd_dst;
	struct timeval tv;

	static const char *card = "/dev/dri/card0";
	int drm_fd;
	int flag = O_RDWR | O_CLOEXEC;
	drm_fd = open(card, flag);
	if (drm_fd < 0)
	{
		fprintf(stderr, "Fail to open %s: %m\n", card);
		return -errno;
	}

	ioctl(drm_fd, DRM_IOCTL_SET_MASTER, 0);

	// memset(&src, 0x0, sizeof(rga_info_t));
	// memset(&dst, 0x0, sizeof(rga_info_t));
	// src.fd = -1;
	// dst.fd = -1;
	// src.mmuFlag = 1;
	// dst.mmuFlag = 1;

	/* create dumb */
	struct drm_mode_create_dumb arg;
	struct drm_mode_create_dumb arg2;
	int ret;

	memset(&arg, 0, sizeof(arg));
	arg.bpp = 32;
	arg.width = BUF_WIDTH;
	arg.height = BUF_HEIGHT;

	memset(&arg2, 0, sizeof(arg2));
	arg2.bpp = 32;
	arg2.width = BUF_WIDTH;
	arg2.height = BUF_HEIGHT;
	// arg.flags = 0;

	ret = ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
	if (ret)
	{
		fprintf(stderr, "can't alloc drm dumb buffer src!\n");
		return -errno;
	}

	ret = ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg2);
	if (ret)
	{
		fprintf(stderr, "can't alloc drm dumb buffer dst!\n");
		return -errno;
	}

	printf("arg  %u %u %lu \n", arg.handle, arg.pitch, arg.size);
	printf("arg2 %u %u %lu \n", arg2.handle, arg2.pitch, arg2.size);

	/*
    //  create a dumb scanout buffer
    struct drm_mode_create_dumb {
        __u32 height;
        __u32 width;
        __u32 bpp;
        __u32 flags;
        //  handle, pitch, size will be returned
        __u32 handle;
        __u32 pitch;
        __u64 size;
    };
    */

	bo_src.fd = drm_fd;
	bo_src.handle = arg.handle;
	bo_src.size = arg.size;
	bo_src.pitch = arg.pitch;

	bo_dst.fd = drm_fd;
	bo_dst.handle = arg2.handle;
	bo_dst.size = arg2.size;
	bo_dst.pitch = arg2.pitch;

	/* map  dumb */
	struct drm_mode_map_dumb arg_map_src;
	struct drm_mode_map_dumb arg_map_dst;
	void *map_src;
	void *map_dst;

	memset(&arg_map_src, 0, sizeof(arg_map_src));
	memset(&arg_map_dst, 0, sizeof(arg_map_dst));

	arg_map_src.handle = bo_src.handle;
	arg_map_dst.handle = bo_dst.handle;

	ret = ioctl(bo_src.fd, DRM_IOCTL_MODE_MAP_DUMB, &arg_map_src);
	if (ret)
	{
		fprintf(stderr, "can't map drm dumb buffer src!\n");
		return -errno;
	}

	ret = ioctl(bo_dst.fd, DRM_IOCTL_MODE_MAP_DUMB, &arg_map_dst);
	if (ret)
	{
		fprintf(stderr, "can't map drm dumb buffer dst!\n");
		return -errno;
	}

	map_src = mmap(0, bo_src.size, PROT_READ | PROT_WRITE, MAP_SHARED, bo_src.fd, arg_map_src.offset);
	if (map_src == MAP_FAILED)
		return -errno;

	bo_src.ptr = map_src;
	printf("bo_src.ptr %p\n", bo_src.ptr);
	printf("bo_src.size %d\n", bo_src.size);
	printf("arg_map_src.offset %p\n", arg_map_src.offset);

	map_dst = mmap(0, bo_dst.size, PROT_READ | PROT_WRITE, MAP_SHARED, bo_dst.fd, arg_map_dst.offset);
	if (map_dst == MAP_FAILED)
		return -EINVAL;

	bo_dst.ptr = map_dst;
	printf("arg_map_dst.offset %p\n", arg_map_dst.offset);

	c_RkRgaGetBufferFd(&bo_src, &buff_fd_src);
	c_RkRgaGetBufferFd(&bo_dst, &buff_fd_dst);

	printf(" buff_fd_src %d\n", buff_fd_src);
	printf(" buff_fd_dst %d\n", buff_fd_src);

	src.fd = buff_fd_src;
	dst.fd = buff_fd_dst;

	src.phyAddr = map_src;
	dst.phyAddr = map_dst;

	rga_set_rect(&src.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);
	rga_set_rect(&dst.rect, 0, 0, BUF_WIDTH, BUF_HEIGHT, BUF_WIDTH, BUF_HEIGHT, RK_FORMAT_RGBA_8888);

	// src.sync_mode=RGA_BLIT_SYNC;
	memcpy(map_src, srcBuffer, BUF_SIZE);
	printf("%p\n", map_src);

	gettimeofday(&tv, NULL);
	c_RkRgaBlit(&src, &dst, NULL);
	printf("[c_RkRgaBlit with phyAddr] elapse time: %d us\n", get_elapse_in_us(&tv));

	// uint8_t *tmp_src_ptr = map_src;
	// uint8_t *tmp_dst_ptr = map_dst;
	// /* check data */
	// for (int i = 0; i < BUF_SIZE; i++)
	// {
	// 	if (tmp_src_ptr[i] != *tmp_dst_ptr[i])
	// 	{
	// 		printf("[diff at pos: %d] src: [%d] dst: [%d]\n", i, tmp_src_ptr[i], tmp_dst_ptr[i]);
	// 	}
	// }

	/**
	 * Destroy the dumb, did last.
	 */
	struct drm_mode_destroy_dumb destroy_arg;

	/* unmap */
	munmap(map_src, BUF_SIZE);
	munmap(map_dst, BUF_SIZE);

	memset(&destroy_arg, 0x0, sizeof(arg));
	destroy_arg.handle = bo_src.handle;
	ioctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);

	memset(&destroy_arg, 0x0, sizeof(destroy_arg));
	destroy_arg.handle = bo_dst.handle;
	ioctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);

	close(drm_fd);
}

int main(int argc, char **argv)
{
	// int buff_fd;
	struct timeval tv;

	srcBuffer = (unsigned char *)malloc(BUF_SIZE);
	dstBuffer = (unsigned char *)malloc(BUF_SIZE);

	make_random_data();
	printf("%s\n", querystring(RGA_VERSION));
	printf("%s\n", querystring(RGA_MAX_INPUT));
	printf("%s\n", querystring(RGA_INPUT_FORMAT));

	gettimeofday(&tv, NULL);
	memcpy(dstBuffer, srcBuffer, BUF_SIZE);
	//printf("%d\n", srcBuffer[20]);
	printf("[memcpy] elapse time: %d us\n", get_elapse_in_us(&tv));
	check_data();
	memset(dstBuffer, 0x0, BUF_SIZE);

	// printf("testing alloc arm!\n");
	// buff_fd = test_alloc_drm();

	// rga_copy_dma_fd();
	gettimeofday(&tv, NULL);
	rga_copy_vir();
	printf("[rgacpy with virAddr] elapse time: %d us\n",get_elapse_in_us(&tv));
	// check_data();
	rga_copy();
	printf("data haven't been checked!\n");

	free(srcBuffer);
	free(dstBuffer);

	return 0;
}
