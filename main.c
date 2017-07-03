#include <stdio.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <errno.h>

#include "android_keyboard.h"

int main(int argc, char **argv)
{
	int ret;
	int key_fd;
	struct android_keyboard_info key_info;
	char input[255];
	
	key_info.mode = KEY_WINDOWS;
	key_info.input_string = "Hello World!";
	key_info.str_len = sizeof("Hello World!");
	
	key_fd = open("/dev/android_keyboard",O_RDWR);
	if(key_fd < 0){
		printf("errno:%d	strerror:%s\n",errno,strerror(errno));
		return 0;
	}
	
	ret = write(key_fd,&key_info,sizeof(struct android_keyboard_info));
	
	memset(input, 0 , 255);
	fgets(input, 254, stdin);
	
	printf("模拟键盘：%s\n",input);
	close(key_fd);
	
	return 0;	
}

