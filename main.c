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

static uint8_t *previous_buffer;
static uint8_t *fbp;

static long int location = 0;
long int screensize = 0;

int xoff;
int yoff;

int d;

int bytespp;

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

    if(fbp == NULL)
        perror("Error mapping the framebuffer device to memory");

    bytespp = (vinfo.bits_per_pixel / 8);
    //int bytespp = 1;
    
    printf("fbp:0x%x Prev:0x%x xoffset:%d yoffset:%d bytespp:%d line_length:%d\n", fbp, previous_buffer, vinfo.xoffset, vinfo.yoffset, bytespp, finfo.line_length);
    printf("Color Offsets (rgb): (%hhu, %hhu, %hhu)\n", vinfo.red.offset, vinfo.green.offset, vinfo.blue.offset);

    void *previous_buffer;

    int i = 0;

    xoff = vinfo.xoffset;
    yoff = vinfo.yoffset;

    while(1){

        clock_t tic = clock();

        get_screenshot(&vinfo, &finfo);

        clock_t toc = clock();

        printf("Screenshot #%2d differences: %d took %fs\n", i, d, (double)(toc-tic)/CLOCKS_PER_SEC);

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
    for(int x = 0; x < vinfo->xres; x+=bytespp)
        for(int y = 0; y < vinfo->yres; y+=bytespp){
            location = (x + xoff) + (y + yoff) * finfo->line_length;
                memcpy((previous_buffer+location),(fbp+location), sizeof(uint32_t));
             //if(__builtin_expect(memcmp(*(fbp+location), *(previous_buffer+location)/* || *((uint8_t*)(fbp+location+1)) ^ *((uint8_t*)(previous_buffer+location+1)) ^ *((uint8_t*)(fbp+location+2)) != *((uint8_t*)(previous_buffer+location+2))*/,uint8_t) != 0, 0)){
                //printf("Different @ %ld!\n", location);
            //printf("Location: %ld\n", location);
            //*(fbp + location + 0) = 100;
            //printf("(x=%4d,y=%4d) Location: %ld BGR: (%hhu,%hhu,%hhu : %hhu,%hhu,%hhu)\n", x, y, location, *((uint8_t*)(fbp+location+0)), *((uint8_t*)(fbp+location+1)), *((uint8_t*)(fbp+location+2)) , *((uint8_t*)(previous_buffer+location+0)), *((uint8_t*)(previous_buffer+location+1)), *((uint8_t*)(previous_buffer+location+2)) );
            //printf("(x=%4d,y=%4d) BGR: (%u)\n", x, y, *((uint32_t*)(fbp+location)));
            //printf("(x=%d,y=%d) Location: %lu BGRA: (%d,%d,%d)\n", x, y, location, *(&fbp+location+0), *(&fbp+location+1), *(&fbp+location+2), *(&fbp+location+3));
        }
}