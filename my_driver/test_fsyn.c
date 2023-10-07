#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
//arm-linux-gnueabi-gcc test.c -o test

int fd = 0;
char tmp[64]={0};


int catch_signal(int signo)
{
    int len;
    if(signo == SIGIO){
        printf("in catch_signal \n");
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
    }
}


int main(int argc,char **argv)
{
	
	int len=0;
	char buf[]="11";
	
	//1.打开myled设备
	
	fd = open("/dev/zkleddevice",O_RDWR);
	
	if(fd < 0)
	{
		
		perror("open");
		
		return -1;
		
	}

    // 1,设置信号处理方法
    signal (SIGIO, catch_signal) ;
    // 2，将当前进程设置成SIGTO的属主进程
    fcntl ( fd,F_SETOWN,getpid());
    // 3，将io模式设置成异步模式
    int flags= fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL,flags |FASYNC);

    sleep(15);


}
