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

#include "rockchip_rga/rockchip_rga.h"

#define BUF_WIDTH 4096
#define BUF_HEIGHT 2160

#define BUF_SIZE (BUF_WIDTH*BUF_HEIGHT*4)

uint8_t *srcBuffer;
uint8_t *dstBuffer;

int get_elapse_in_ms(struct timeval *tv)
{
	struct timeval this_tv;
	
	gettimeofday(&this_tv, NULL);
	uint64_t diff_sec = this_tv.tv_sec - tv->tv_sec;
	int elapse_in_ms;


	if(diff_sec==0){
		elapse_in_ms = (this_tv.tv_usec - tv->tv_usec)/1000.0;
	}else{
		elapse_in_ms = ((--diff_sec) * 1000)+ ((1000000 - tv->tv_usec)+this_tv.tv_usec)/1000.0;
	}
	return elapse_in_ms;
}

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
	for(int i=0;i<BUF_SIZE;i++){
		if(srcBuffer[i]!=dstBuffer[i]){
			printf("[DIFF at pos:%d] src: [%d] dst: [%d]\n", i, srcBuffer[i], dstBuffer[i]);
		}
	}
}

void rga_copy()
{
	RockchipRga *mRga = RgaCreate();
	if(!mRga){
		printf("create rga failed!\n");
		return;
	}
	mRga->ops->initCtx(mRga);
	mRga->ops->setSrcFormat(mRga, V4L2_PIX_FMT_ABGR32, BUF_WIDTH, BUF_HEIGHT);
	mRga->ops->setDstFormat(mRga, V4L2_PIX_FMT_YUYV, BUF_WIDTH, BUF_HEIGHT);
	
	

	mRga->ops->setSrcBufferPtr(mRga, srcBuffer);
	mRga->ops->setDstBufferPtr(mRga, dstBuffer);

	mRga->ops->go(mRga);
}

int main(int argc, char **argv)
{
	struct timeval tv;
	srcBuffer = (unsigned char *)malloc(BUF_SIZE);
	dstBuffer = (unsigned char *)malloc(BUF_SIZE);
	
	make_random_data();
	
	gettimeofday(&tv, NULL);
	memcpy(dstBuffer, srcBuffer, BUF_SIZE);
	//printf("%d\n", srcBuffer[20]);
	printf("[memcpy] elapse time: %d ms\n",get_elapse_in_ms(&tv));
	check_data();

	gettimeofday(&tv, NULL);
	rga_copy();
	printf("[rgacpy] elapse time: %d ms\n",get_elapse_in_ms(&tv));
	check_data();

	free(srcBuffer);
	free(dstBuffer);
	
	return 0;
}
