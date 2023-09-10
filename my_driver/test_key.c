#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <signal.h>

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

		len = read(fd,tmp,5); //block read 
		
		if( len > 0)
		{
			printf("[test][read]tmp = %s\n",tmp);		
			printf("[test][read]len = %d\n",len);
			
		}
		else
		{
			perror("read");
			
		}


	struct pollfd pfd[2];
	pfd[0].fd = fd;  //key device
	pfd[0].events = POLLIN;

	pfd[1].fd = 0;  //stdin
	pfd[1].events = POLLIN;



	
	sleep(3);
	while(1)
	{


	int ret = poll(pfd,2,-1);
	printf("poll ret %d\n",ret);
	if(ret > 0)
	{
		// at least one fd can read
		for(int i=0;i<2;i++)
		{
			if(pfd[i].revents & POLLIN)
			{
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
			}
		}
		
	}else{
		perror("poll ");
		
		return -1;
	}



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
