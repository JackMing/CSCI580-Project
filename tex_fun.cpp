/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"

GzColor	*image=NULL;
int xs, ys;
int reset = 1;
float N=1;

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }

/* bounds-test u,v to make sure nothing will overflow image array bounds */
/* determine texture cell corner values and perform bilinear interpolation */
/* set color to interpolated GzColor value and return */
/* bounds-test u,v to make sure nothing will overflow image array bounds */
	if (u>1.0) u = u - floor(u); 
	else if (u<0) u = 1 - ceil(u) + u;
	if (v>1.0) v -= v - floor(v); 
	else if (v<0) v = 1 - ceil(v) + v;

	GzTextureIndex tex;
	float s, t;
	tex[U] = u*(xs-1);
	tex[V] = v*(ys-1);
	s = tex[U] - (int)tex[U];
	t = tex[V] - (int)tex[V];
	int texU = (int)tex[U];
	int texV = (int)tex[V];


	for (int i=0; i<3; i++){
		color[i] = 
			image[ texU    + texV   *xs][i] * (1-s)*(1-t) +
			image[(texU+1) + texV   *xs][i] *    s *(1-t) +
			image[(texU+1) +(texV+1)*xs][i] *    s * t    +
			image[ texU    +(texV+1)*xs][i] * (1-s)* t;
	}
  
  return GZ_SUCCESS;
}

void LutColor(float len, GzColor col){
	GzColor LUT[21]={{0,100,0},{0,128,0},{154,204,255},{51,204,51},{0,255,0}
					,{102,102,255},{153,255,102},{153,255,153},{0,0,204},{204,255,255}
					,{255,255,255}
					,{204,236,255},{204,204,255},{0,153,0},{51,153,255},{102,153,255}
					,{102,255,102},{51,102,255},{51,51,255},{204,255,204},{0,0,102}};
	if (len>2.0) len = len - floor(len); 
	else if (len<0) len = 2 - ceil(len) + len;
	for(int i=0;i<3;i++){
		int a = floor(len/0.1);
		int b = ceil(len/0.1);
		col[i]=
			((LUT[b][i]-LUT[a][i])*(len-a*0.1)/0.1+LUT[a][i])/255;
	}

}


void JuliaSet(float *Xr, float *Xi, float cr, float ci,int N){
	float xr=*Xr, xi=*Xi;
	float xrr=xr, xii=xi;
	int i=0;
	for(i=0;i<N;i++){
		xrr=xr;
		xii=xi;
		xr = xr*xr-xi*xi+cr;
		xi = 2*xr*xi+ci;
		if(abs(xr)>2)
			break;
	}
	*Xr = xrr;
	*Xi = xii;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{
	bool x=false,y=false;
	//u+= 0.3; v+=0.7;
	u-=0.5;v-=0.5;
	u=abs(u);v=abs(v);
	memset(color,0,sizeof(GzColor));
	if (u>1.0) u = u - floor(u); 
	else if (u<0) u = 1 - ceil(u) + u;
	if (v>1.0) v -= v - floor(v); 
	else if (v<0) v = 1 - ceil(v) + v;
	float length = 0;
	//JuliaSet(&u,&v,0.638,0.316,300);

	if((int)u==(int)v){
		length = 0.2;
	}
	if(abs(u-v)<0.1){
		u+=abs(u-v)*3;
		v+=abs(u-v)*3;
	}
	length += sqrt(u*u+v*v);
	LutColor(length,color);
  return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture()
{
	if(image!=NULL)
		free(image);
	return GZ_SUCCESS;
}

