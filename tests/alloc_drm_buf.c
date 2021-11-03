#include <asm/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/stddef.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <time.h>
#include "drmrga.h"

#include "rockchip_rga/rockchip_rga.h"


int main(int argc, char **argv)
{
	struct drm_mode_create_dumb cd;
	__s32 ret;
	__u32 bpp;
	__u32 vHeight;
	__s32 drmFd;

	drmFd = open("/dev/dri/card0", O_RDWR, 0);
	if(drmFd < 0) 
		rga_err(rga, "Open drm device failed\n");

	memset(&cd, 0, sizeof(cd));
	cd.bpp = 8;
	cd.width = RGA_ALIGN(1080, 16);
	cd.height = RGA_ALIGN(1920, 16);
	ret = ioctl(drmFd, DRM_IOCTL_MODE_CREATE_DUMB, &cd);
	if(ret < 0) {
        rga_err(rga, "Allocate drm buffer failed, return %d\n", ret);
    }
	
	
	
}

