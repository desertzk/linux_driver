#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include<stdio.h>

char write_buf[]="hello world!\n";
char read_buf[200];


int main(void){
	int fd;
	char *mmap_addr;
	fd =open("/dev/zkkindlemem",O_RDWR);
	if(fd<0){
		perror("open failed\n");
		return -1;
	}
	printf("read_buffer:%s\n",read_buf);
	mmap_addr = mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	printf("mmap mmap_addr %lx read:%s\n",mmap_addr,mmap_addr);
	memcpy(mmap_addr,"hello,dududu.cc!\n",20);
	read(fd,read_buf,100);
	printf("read_buffer addr %lx read_buffer:%s\n",read_buf,read_buf);
printf("mmap read:%s\n",mmap_addr);
while(1)
{
	sleep(1);
}

	munmap(mmap_addr,4096);
	return 0;
}

