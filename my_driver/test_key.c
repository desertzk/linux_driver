#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define CMD_KEY_ALL		_IOR('K',0,unsigned int)

//arm-linux-gnueabi-gcc test_key.c -o test_key
int main(int argc,char **argv)
{
	int fd = 0;
	int len=0;
	char buf[]="11";
	char tmp[64]={0};
	unsigned int key_val=0;
	
	//1.打开myled设备
	
	fd = open("/dev/zkkeydevice",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open");
		
		return -1;
		
	}
	
	sleep(3);
	while(1)
	{
		int rt=ioctl(fd,CMD_KEY_ALL,&key_val);
		printf("key_val %d\n",key_val);
		if(rt < 0)
		{
			perror("ioctl key");
			continue;
		}
		
		// sleep(1);	
		
		// //从myled设备读取数据
		// len = read(fd,tmp,5);
		
		// if( len > 0)
		// {
		// 	printf("[test][read]tmp = %s\n",tmp);		
		// 	printf("[test][read]len = %d\n",len);
			
		// }
		// else
		// {
		// 	perror("read");
			
		// }


		// 		//向myled设备写入数据
		// len = write(fd,"12",strlen("12"));
		
		// if( len > 0)
		// {
		// 	printf("[test][write]len = %d\n",len);
			
		// }
		// else
		// {
		// 	perror("write");
			
		// }
		// sleep(3);
	}
	
	
	sleep(3);		
	

	close(fd);

	return 0;
}