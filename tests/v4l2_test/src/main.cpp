#include <iostream>
#include <time.h>
#include <signal.h>
#include "v4l2_capturer.h"

v4l2_capturer capturer;

void capturer_sigint_handler(int signal)
{
	capturer.stop();
	exit(1);
}

/* TODO: listen SIGINT and call capturer.stop() when it come. */
int main(int argc, char const *argv[])
{
	int frame_count=0;
    std::cout << "Hello World!" << std::endl;

	/* register signal handler function. */
	signal(SIGINT, capturer_sigint_handler);

    capturer.init();
    capturer.query_supported_format_new();
    capturer.start();

	int i=25;
	char filename[10];

	while(1){
	    frame_count=capturer.get_frame();
	    if(frame_count==25*10)break;
	}
	/*capturer.get_frame();
    capturer.save_fbdata_to_file_by_mmap("./out.jpg");*/
    capturer.stop();
    return 0;
}
