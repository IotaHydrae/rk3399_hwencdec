#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <linux/videodev2.h>
#include <cstring>
#define RBS_ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

unsigned char *srcBuffer;
unsigned char *dstBuffer;

#define BUFFER_WIDTH 4096
#define BUFFER_HEIGHT 2160
#define BUFFER_SIZE BUFFER_WIDTH*BUFFER_HEIGHT*4
#define NANOTIME_PER_MSECOND 1000000L


void randomData() {
    srand((unsigned)time(0));
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        srcBuffer[i] = (unsigned char)rand();
    }
}
bool checkData() {
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        if (srcBuffer[i] != dstBuffer[i]) {
            printf("[%d] src=%u, dst=%u\n", i, srcBuffer[i], dstBuffer[i]);
        }
    }
    return true;
}

static void simple_memcpy(unsigned char *src,unsigned char *dst, long long ssize)
{
	int i;
	for(i=ssize;i>0;i--){
		*dst = *src;
		src++;
		dst++;
	}
}

static unsigned long long nanoTime(void) {
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return (unsigned long long)(t.tv_sec) * 1000000000 + t.tv_nsec;
}

int main() {
    srcBuffer = new unsigned char[BUFFER_SIZE];
    dstBuffer = new unsigned char[BUFFER_SIZE];
	long long begintime,endtime;

    randomData();

    /*long long begintime = nanoTime();
    rga_copy();
    long long endtime = nanoTime() - begintime;
    printf("RGA copy time : %lld ms\n", endtime/NANOTIME_PER_MSECOND);*/
    

    begintime = nanoTime();
    memcpy(dstBuffer, srcBuffer, BUFFER_SIZE);
    endtime = nanoTime() - begintime;
	printf("Buffer size: %d\n", BUFFER_SIZE/2/1024/1024);
    printf("Memcpy copy time : %lld ms\n", endtime/NANOTIME_PER_MSECOND);

    checkData();

    delete []srcBuffer;
    delete []dstBuffer;
    return 0;
}

