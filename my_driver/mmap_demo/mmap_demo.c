#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/nman.h>


char write_buf[]="hello world!\n";
char read_buf[200];


int main(void){
	int fd;
	char *mmap_addr;
	fd =open("data.txt",O_RDMR);
	if(fd<0){
		printf("open failed\n");
		return -1;
	}
	write(fd,write_buf,strlen(write_buf)+1);
	lseek(fd,0,SEEK_SET);
	read(fd,read_buf,100);
	printf("read_buffer:%s\n",read_buf);
	mpap_addr =mmap(NULL,4096,PROT_READIPROT_MRITE,NAP_SHARED,fd,0);
	printf("mmap read:%s\n",nnap_addr);
	memcpy(mmap_addr+5,"hello,zhaixue.cc!\n",20);printf("mmap read:xs\n",rmap_addr);
	munmap(nmap_addr,4096);
	return 0;
}

