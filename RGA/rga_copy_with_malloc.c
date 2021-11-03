#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <rockchip_rga/rockchip_rga.h>
#define RBS_ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

#define BUFFER_WIDTH 4096
#define BUFFER_HEIGHT 2160
#define BUFFER_SIZE BUFFER_WIDTH*BUFFER_HEIGHT*4

unsigned char *srcBuffer;
unsigned char *dstBuffer;

#define NANOTIME_PER_MSECOND 1000000L


