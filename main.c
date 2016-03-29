#include <stdlib.h>		// standard library utils
#include <stdio.h>		// standard I/O

#include <linux/fb.h>	// framebuffer

//#include <sys/stat.h>	//
#include <fcntl.h>		// file opening
#include <sys/mman.h>	// memory management
#include <unistd.h>		// close()
#include <sys/ioctl.h>
#include <sys/kd.h>		//KDSETMODE; set mode of vt

int main(int argc, int **argv){
	int sf;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	char *fbp = 0;
	unsigned long *kb_mode; // save the keyboard mode of the tty
	long int location = 0;

	int fd = 1;

	//ioctl(fd, KDSETMODE, KD_GRAPHICS);
	ioctl(fd, KDGKBMODE, kb_mode);
	//ioctl(fd, KDSKBMODE, K_RAW);

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

    printf("Screen size: %d\n", screensize);

    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, sf, 0);

    if(fbp == NULL)
    	perror("Error mapping the framebuffer device to memory");

    //int bytespp = (vinfo.bits_per_pixel / 8);
    int bytespp = 1;
    
    printf("FBP:0x%x xoffset:%d yoffset:%d bytespp:%d line_length:%d\n", &fbp, vinfo.xoffset, vinfo.yoffset, bytespp, finfo.line_length);

    for(int y = 0; y < vinfo.yres; y++)
    	for(int x = 0; x < vinfo.xres; x++){
    		location = (x + vinfo.xoffset) * bytespp + (y + vinfo.yoffset) * (finfo.line_length);
    		//*(fbp + location + 0) = 100;
    		printf("(x=%4d,y=%4d) Location: %lu BGRA: (%u,%u,%u,%u)\n", x, y, location, (u_char)*(&fbp+location+0), (u_char)*(&fbp+location+1), (u_char)*(&fbp+location+2), (u_char)*(&fbp+location+3));
    		//printf("(x=%d,y=%d) Location: %lu BGRA: (%d,%d,%d)\n", x, y, location, *(&fbp+location+0), *(&fbp+location+1), *(&fbp+location+2), *(&fbp+location+3));
    	}

    munmap(fbp, screensize);
    //ioctl(fd, KDSETMODE, KD_TEXT);
    //ioctl(fd, KDSKBMODE, kb_mode);
    close(sf);
	return 0;
}