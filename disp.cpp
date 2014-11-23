/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"

int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
 -- NOTE: this function is optional and not part of the API, but you may want to use it within the display function.
*/
	*framebuffer = (char*)malloc(sizeof(GzPixel)*width*height);
	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, int xRes, int yRes)
{
/* create a display:
  -- allocate memory for indicated resolution
  -- pass back pointer to GzDisplay object in display
*/
	*display = (GzDisplay*)malloc(sizeof(GzDisplay));
	 GzNewFrameBuffer((char**)&((*display)->fbuf),xRes,yRes);
	(*display)->xres = xRes;
	(*display)->yres = yRes;
	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	free(display->fbuf);
	free(display);
	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes)
{
/* pass back values for a display */
	*xRes = display->xres;
	*yRes = display->yres;
	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */
	//memset(display->fbuf,0,sizeof(GzPixel)*display->xres*display->yres);
	int i;
	for(i=0;i<display->xres*display->yres;i++){
		display->fbuf[i].blue=2000;
		display->fbuf[i].red=2000;
		display->fbuf[i].green=2000;
		display->fbuf[i].z=MAXINT;
	}
	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	if(i<0 || j<0 || i>display->xres-1 || j>display->yres-1) return GZ_SUCCESS;
	if(r>4095) r = 4095;
	if(g>4095) g = 4095;
	if(b>4095) b = 4095;
	if(r<0)    r = 0;
	if(g<0)    g = 0;
	if(b<0)    b = 0;
	display->fbuf[j*display->xres+i].red = r;
	display->fbuf[j*display->xres+i].green = g;
	display->fbuf[j*display->xres+i].blue = b;
	display->fbuf[j*display->xres+i].alpha = a;
	display->fbuf[j*display->xres+i].z = z;
	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	if(i<0 || j<0 || i>display->xres-1 || j>display->yres-1) return GZ_FAILURE;
	*r = display->fbuf[j*display->xres+i].red;
	*g = display->fbuf[j*display->xres+i].green;
	*b = display->fbuf[j*display->xres+i].blue;
	*a = display->fbuf[j*display->xres+i].alpha;
	*z = display->fbuf[j*display->xres+i].z;
		return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file -- "P6 %d %d 255\r" */
	fprintf(outfile,"P6 %d %d 255\r",display->xres, display->yres);
	int i;
	GzPixel *pix;
	for(i=0,pix=display->fbuf;i<display->xres*display->yres;i++,pix++){
		fprintf(outfile,"%c%c%c",pix->red>>4,pix->green>>4,pix->blue>>4);
	}
	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/
	int i=0;
	GzPixel *pix = display->fbuf;
	for(i=0;i<display->xres*display->yres*3;i+=3,pix++){
		framebuffer[i] = (char)(pix->blue>>4);
		framebuffer[i+1] = (char)(pix->green>>4);
		framebuffer[i+2] = (char)(pix->red>>4);
	}
	return GZ_SUCCESS;
}