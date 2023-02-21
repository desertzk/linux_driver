#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
//arm-linux-gnueabi-gcc test.c -o test
int main(int argc,char **argv)
{
	int fd = 0;
	int len=0;
	char buf[]="11";
	char tmp[64]={0};
	
	//1.打开myled设备
	
	fd = open("/dev/zkleddevice",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open");
		
		return -1;
		
	}
	
	sleep(3);
	while(1)
	{
		//向myled设备写入数据
		len = write(fd,buf,strlen(buf));
		
		if( len > 0)
		{
			printf("[test][write]len = %d\n",len);
			
		}
		else
		{
			perror("write");
			
		}
		
		sleep(3);	
		
		//从myled设备读取数据
		len = read(fd,tmp,5);
		
		if( len > 0)
		{
			printf("[test][read]tmp = %s\n",tmp);		
			printf("[test][read]len = %d\n",len);
			
		}
		else
		{
			perror("read");
			
		}


				//向myled设备写入数据
		len = write(fd,"12",strlen("12"));
		
		if( len > 0)
		{
			printf("[test][write]len = %d\n",len);
			
		}
		else
		{
			perror("write");
			
		}
		sleep(3);
	}
	
	
	sleep(3);		
	

	close(fd);

	return 0;
}