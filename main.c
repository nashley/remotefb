#include <stdlib.h>		// standard library utils
#include <stdio.h>		// standard I/O

#include <linux/fb.h>	// framebuffer

//#include <sys/stat.h>	//
#include <fcntl.h>		// file opening
#include <sys/mman.h>	// memory management
#include <unistd.h>		// close()
#include <sys/ioctl.h>

int main(int argc, int **argv){
	int sf;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	char *fbp = 0;

	sf = open("/dev/fb0",O_RDONLY); // open framebuffer device as readonly
	if(sf == -1)
		perror("There was an error opening the framebuffer device. Ensure that you have the required permissions (e.g., video group)");
	
	if (ioctl(sf, FBIOGET_FSCREENINFO, &finfo) == -1)
		perror("There was an error getting the fixed video information");

	if (ioctl(sf, FBIOGET_VSCREENINFO, &vinfo) == -1)
		perror("There was an error getting the variable video information");

	printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel); // Framebuffer resolution and bits per pixel

	// Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, sf, 0);

    if(fbp == NULL)
    	perror("Error mapping the framebuffer device to memory");

    munmap(fbp, screensize);
    close(sf);
	return 0;
}