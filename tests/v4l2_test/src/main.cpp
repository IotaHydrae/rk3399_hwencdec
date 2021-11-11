#include <iostream>
#include <time.h>
#include "v4l2_capturer.h"

int main(int argc, char const *argv[])
{
    std::cout << "Hello World!" << std::endl;
    v4l2_capturer capturer;
    capturer.init();
    capturer.query_supported_format_new();
    capturer.start();

	int i=25;
	char filename[10];

	while(1){
	    capturer.get_frame();
	}
	/*capturer.get_frame();
    capturer.save_fbdata_to_file_by_mmap("./out.jpg");*/
    capturer.stop();
    return 0;
}
