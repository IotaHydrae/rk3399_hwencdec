#include <iostream>
#include <time.h>
#include "v4l2_capturer.h"

int main(int argc, char const *argv[])
{
    std::cout<< "Hello World!" << std::endl;

	v4l2_capturer capturer;
	//capturer.query_supported_format();
	capturer.init();
	capturer.start();
	capturer.get_frame();
	capturer.save_fbdata_to_file("./out.jpg");
	
	//capturer.frameSaveImage("./out2.jpg");
	capturer.stop();
	
    return 0;
}
