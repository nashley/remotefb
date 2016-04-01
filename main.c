#include <stdlib.h>     // standard library utils
#include <stdio.h>      // standard I/O

#include <linux/fb.h>   // framebuffer

//#include <sys/stat.h> //
#include <fcntl.h>      // file opening
#include <sys/mman.h>   // memory management
#include <unistd.h>     // close()
#include <sys/ioctl.h>
#include <sys/kd.h>     // KDSETMODE; set mode of vt
#include <stdint.h>     // uint8_t
#include <time.h>       // time
#include <string.h>     // memcmp

void get_screenshot(struct fb_var_screeninfo *vinfo, struct fb_fix_screeninfo *finfo);
void write_screenshot(struct fb_var_screeninfo *vinfo, struct fb_fix_screeninfo *finfo);

static uint8_t *previous_buffer;
static uint8_t *fbp;

static long int location = 0;
long int screensize = 0;

int off;

int d;
int d1;

int bytespp;

int xmax;
int ymax;

int main(int argc, int **argv){
    int sf;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    unsigned long *kb_mode; // save the keyboard mode of the tty

    int fd = 1;

    //ioctl(fd, KDSETMODE, KD_GRAPHICS);
    ioctl(fd, KDGKBMODE, kb_mode);
    //ioctl(fd, KDSKBMODE, K_RAW);
    printf("\n");
    sf = open("/dev/fb0",O_RDWR); // open framebuffer device as readonly
    if(sf == -1)
        perror("There was an error opening the framebuffer device. Ensure that you have the required permissions (e.g., video group)");
    
    if (ioctl(sf, FBIOGET_FSCREENINFO, &finfo) == -1)
        perror("There was an error getting the fixed video information");

    if (ioctl(sf, FBIOGET_VSCREENINFO, &vinfo) == -1)
        perror("There was an error getting the variable video information");

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel); // Framebuffer resolution and bits per pixel

    // Figure out the size of the screen in bytes
    //screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    screensize = finfo.smem_len;
    //screensize = vinfo.yres_virtual * finfo.line_length;

    printf("Screen size: %d\n", screensize);

    fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, sf, (off_t)0);
    previous_buffer = malloc(screensize);
    memset(previous_buffer, 0, screensize);

    if(fbp == NULL)
        perror("Error mapping the framebuffer device to memory");

    bytespp = (vinfo.bits_per_pixel / 8);
    //int bytespp = 1;
    
    printf("fbp:0x%x Prev:0x%x xoffset:%d yoffset:%d bytespp:%d line_length:%d\n", fbp, previous_buffer, vinfo.xoffset, vinfo.yoffset, bytespp, finfo.line_length);
    printf("Color Offsets (rgb): (%hhu, %hhu, %hhu)\n", vinfo.red.offset, vinfo.green.offset, vinfo.blue.offset);

    void *previous_buffer;

    int i = 0;

    int xoff = vinfo.xoffset * bytespp;
    int yoff = vinfo.yoffset * finfo.line_length;
    off = xoff + yoff;

    xmax = vinfo.xres * bytespp;
    ymax = vinfo.yres * finfo.line_length;
    while(1){

        clock_t tic = clock();

        get_screenshot(&vinfo, &finfo);

        clock_t toc = clock();

        clock_t tac = clock();

        write_screenshot(&vinfo, &finfo);

        clock_t tuc = clock();

        //sleep(3);

        printf("Screenshot #%2d differences: %d took %fs\n", i, d, (double)(toc-tic)/CLOCKS_PER_SEC);
        printf("Write #%2d differences: %d took %fs\n", i, d1, (double)(toc-tic)/CLOCKS_PER_SEC);

        i++;
    }

    munmap(fbp, screensize);
    //ioctl(fd, KDSETMODE, KD_TEXT);
    //ioctl(fd, KDSKBMODE, kb_mode);
    close(sf);
    return 0;
}

void get_screenshot(struct fb_var_screeninfo *vinfo, struct fb_fix_screeninfo *finfo){
    d = 0;
    printf("xres:%d yres:%d xd:%d yd: %d\n", vinfo->xres, vinfo->yres, bytespp * finfo->line_length);
    for(int x = 0; x < xmax; x+=bytespp)
        for(int y = 0; y < ymax; y+=finfo->line_length){
            //printf("xoff: %d yoff:%d x:%d y:%d\n", xoff, yoff, x, y);
            location = x + y + off;
            //printf("Location: %lu\n", location);
            if(*((uint32_t*)(fbp+location)) != *((uint32_t*)(previous_buffer+location))){
                memcpy((previous_buffer+location),(fbp+location), 4); // sizeof(uint32_t)
                d++;
            }
            //printf("Location0: %lu\n", location);
        }
    printf("Location1: %lu\n", location);
}
void write_screenshot(struct fb_var_screeninfo *vinfo, struct fb_fix_screeninfo *finfo){
    d1 = 0;
    //memset(fbp, 0, screensize);
    //sleep(1);
    for(int x = 0; x < vinfo->xres * bytespp; x+=bytespp)
        for(int y = 0; y < vinfo->yres * finfo->line_length; y+=finfo->line_length){
            //printf("xoff: %d yoff:%d x:%d y:%d\n", xoff, yoff, x, y);
            location = x + y + off;
            //printf("Location: %lu\n", location);
            if(*((uint32_t*)(fbp+location)) != *((uint32_t*)(previous_buffer+location))){
                memcpy((fbp+location), (previous_buffer+location), 4); // sizeof(uint32_t)
                d1++;
            }
        }
    //sleep(1);
}
