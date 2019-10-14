#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define  GETERRNO(ret) do{ ret = -errno; if (ret == 0) {ret = -1;}}while(0)
#define  DEBUG(...)  do{fprintf(stdout,"[%s:%d] ",__FILE__,__LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); fflush(stdout);} while(0)

#define  U_BOOT_SIZE    0x20000

int parse_get_write(char* rbuf,int rsize, unsigned char* wbuf,int wsize)
{
	int wi=0,ri=0,lastwi=0;
	int commented=0;
	unsigned int crc;

	memset(wbuf, 0, wsize);
	wi = 4;

	while(ri < rsize) {
		commented = 0;
		lastwi = wi;
		while(ri < rsize) {
			if (rbuf[ri] == '\r' || rbuf[ri] == '#' || 
				rbuf[ri] == '\n') {
				commented = 1;
			}

			if (rbuf[ri] == '\n') {
				ri ++;
				break;
			}

			if (commented == 0) {
				if (wi >= wsize) {
					fprintf(stderr, "parse overflow \n");
					return -1;
				}
				wbuf[wi] = rbuf[ri];
				wi ++;
			}
			ri ++;
		}

		if (lastwi != wi) {
			wbuf[wi] = '\0';
			wi ++;
		}
	}
	/*now to check*/
	crc = crc32(0,&(wbuf[4]),(wsize - 4));
	memcpy(wbuf,&crc,sizeof(crc));
	return 0;
}


int main(int argc,char* argv[])
{
	int rfd = -1;
	int wfd = -1;
	int ret = 0;
	off_t size;
	char* rbuf=NULL;
	char* wbuf=NULL;
	int wsize = 0;
	if (argc != 3) {
		printf("This program generate the u-boot env partition from config file\n");
		printf("Usage: %s input_config_file out_put_bin_file\n", argv[0]);
		return 0;
	}

	rfd = open(argv[1], O_RDONLY);
	if (rfd < 0) {
		GETERRNO(ret);
		fprintf(stderr, "open [%s] for error[%d]\n", argv[1], ret);
		goto out;
	}

	wfd = open(argv[2], O_CREAT|O_ASYNC|O_RDWR,0x1e4);
	if (wfd < 0) {
		GETERRNO(ret);
		fprintf(stderr, "open [%s] for error[%d]\n", argv[2], ret);
		goto out;
	}

	size = lseek(rfd,0,SEEK_END);
	if (size == -1) {
		GETERRNO(ret);
		fprintf(stderr, "seek [%s] error[%d]\n", argv[1], ret);
		goto out;
	}
	lseek(rfd,0,SEEK_SET);

	rbuf = (char*) malloc(size);
	if (rbuf == NULL) {
		GETERRNO(ret);
		goto out;
	}

	ret = read(rfd,rbuf,size);
	if (ret != size) {
		GETERRNO(ret);
		fprintf(stderr, "read [%s] error[%d]\n", argv[1], ret);
		goto out;
	}


	wsize = U_BOOT_SIZE;
	wbuf = (char*) malloc(wsize);
	if (wbuf == NULL) {
		GETERRNO(ret);
		goto out;
	}

	ret = parse_get_write(rbuf,size, wbuf,wsize);
	if (ret < 0) {
		GETERRNO(ret);
		goto out;
	}

	ret = write(wfd,wbuf,wsize);
	if (ret != wsize) {
		GETERRNO(ret);
		fprintf(stderr, "can not write [%s] size [%d]\n", argv[2], ret);
		goto out;
	}

	ret = 0;
out:
	if (wbuf) {
		free(wbuf);
	}
	wbuf = NULL;
	if (rbuf) {
		free(rbuf);
	}
	rbuf = NULL;
	if (rfd >= 0) {
		close(rfd);
	}
	rfd = -1;
	if (wfd >= 0) {
		close(wfd);
	}
	wfd = -1;
	return ret;
}


