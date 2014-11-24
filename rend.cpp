/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

#define PI 3.14159265
#ifndef GZSCANLN
#define GZSCANLN

typedef struct{
	float *start, *end;
	float current[3];
	float slopeX, slopeZ;
	GzCoord startIntrp, endIntrp, currentIntrp, slopeIntrp;
	GzTextureIndex startTex, endTex, slopeTex, currentTex;
} GzEdge;
#endif

// Function declaration for the DDA algorithm
bool edgeInit(GzRender*,GzEdge*, float*, float*, float*, float*, float*, float*);
void edgeAdvanceDel(GzEdge*, float);
void GzScanLine(GzRender*, GzCoord*, GzCoord*, GzTextureIndex*);
void sortV(GzCoord *,GzCoord *, GzTextureIndex*);
void swapV(GzCoord *,GzCoord *, GzTextureIndex*, int, int);
void span(GzRender*, GzEdge*, GzEdge*);
void edgeAdvance(GzRender*, GzEdge*, GzEdge*);

//HW3
void unitVector(GzCoord);
float dotProduct(GzCoord, GzCoord);
void mulVector(const float, const GzCoord,GzCoord);
void matrixMul(GzMatrix, const GzMatrix, const GzMatrix);
int Xform(GzMatrix, GzCoord);

//HW4
void matrixUnitary(GzMatrix);
void shade(GzRender*, GzCoord, GzColor);

//HW5
void warpTex(GzCoord*, GzTextureIndex*);
void unWarpTex(GzCoord, GzTextureIndex);

GzColor *image[6];

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value

	float c = (float)cos( PI*degree/180.0 );
	float s = (float)sin( PI*degree/180.0 );
	mat[0][0] = 1;
	mat[1][1] = c;
	mat[1][2] = -s;
	mat[2][1] = s;
	mat[2][2] = c;
	return GZ_SUCCESS;
}

int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	float c = (float)cos( PI*degree/180.0 );
	float s = (float)sin( PI*degree/180.0 );
	mat[1][1] = 1;
	mat[0][0] = c;
	mat[2][0] = -s;
	mat[0][2] = s;
	mat[2][2] = c;
	return GZ_SUCCESS;
}

int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
	float c = (float)cos( PI*degree/180.0 );
	float s = (float)sin( PI*degree/180.0 );
	mat[2][2] = 1;
	mat[0][0] = c;
	mat[0][1] = -s;
	mat[1][0] = s;
	mat[1][1] = c;

	return GZ_SUCCESS;
}

int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	mat[0][3] = translate[X];
	mat[1][3] = translate[Y];
	mat[2][3] = translate[Z];
	return GZ_SUCCESS;
}

int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value
	mat[0][0] = scale[X];
	mat[1][1] = scale[Y];
	mat[2][2] = scale[Z];
	return GZ_SUCCESS;
}

//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- setup Xsp and anything only done once 
- save the pointer to display 
- init default camera 
*/ 

	*render = (GzRender*)malloc(sizeof(GzRender));
	(*render)->display = display;
	
	(*render)->matlevel=0;

	memset((*render)->Xsp,0,sizeof(GzMatrix));
	memset((*render)->camera.Xiw,0,sizeof(GzMatrix));
	memset((*render)->camera.Xpi,0,sizeof(GzMatrix));
	memset((*render)->camera.Xwi,0,sizeof(GzMatrix));

	for(int i=0;i<4;i++){
		(*render)->Xsp[i][i]=1;
		(*render)->camera.Xiw[i][i]=1;
		(*render)->camera.Xwi[i][i]=1;
		(*render)->camera.Xpi[i][i]=1;
	}

	(*render)->Xsp[0][0] = (float)(*render)->display->xres/2;
	(*render)->Xsp[1][1] = (float)-(*render)->display->yres/2;
	(*render)->Xsp[2][2] = (float)MAXINT;
	(*render)->Xsp[0][3] = (float)(*render)->display->xres/2;
	(*render)->Xsp[1][3] = (float)(*render)->display->yres/2;

	// setup default camera
	(*render)->camera.lookat[X]=0;
	(*render)->camera.lookat[Y]=0;
	(*render)->camera.lookat[Z]=0;
	(*render)->camera.position[X]=DEFAULT_IM_X;
	(*render)->camera.position[Y]=DEFAULT_IM_Y;
	(*render)->camera.position[Z]=DEFAULT_IM_Z;
	(*render)->camera.worldup[X]=0;
	(*render)->camera.worldup[Y]=1;
	(*render)->camera.worldup[Z]=0;
	(*render)->camera.FOV = (float)DEFAULT_FOV;

	// setup default light
	GzColor defaultKa = DEFAULT_AMBIENT;
	GzColor defaultKs = DEFAULT_SPECULAR;
	GzColor defaultKd = DEFAULT_DIFFUSE;
	memcpy((*render)->Ka,defaultKa,sizeof(defaultKa));
	memcpy((*render)->Ks,defaultKs,sizeof(defaultKs));
	memcpy((*render)->Kd,defaultKd,sizeof(defaultKd));
	(*render)->spec = DEFAULT_SPEC;
	(*render)->numlights = 0;
	memset(&((*render)->ambientlight),0,sizeof((*render)->ambientlight));

	(*render)->tex_fun = NULL;

	(*render)->dx = 0;
	(*render)->dy = 0;

	return GZ_SUCCESS;

}

int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	for(int i=0;i<6;i++){
		if(image[i]!=NULL){
			free(image[i]);
			image[i]=NULL;
		}
	}
	free(render);
	return GZ_SUCCESS;
}

int GzBeginRender(GzRender *render)
{
/*  
- setup for start of each frame - init frame buffer color,alpha,z
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms when needed 
*/ 
	GzInitDisplay(render->display);

	render->camera.Xpi[2][2] = (float)tan(PI*(render->camera.FOV/2)/180.0);
	render->camera.Xpi[3][2] = render->camera.Xpi[2][2]; 

	GzCoord vX,vY,vZ,up;
	vZ[X] = render->camera.lookat[X]-render->camera.position[X];
	vZ[Y] = render->camera.lookat[Y]-render->camera.position[Y];
	vZ[Z] = render->camera.lookat[Z]-render->camera.position[Z];
	unitVector(vZ);
	up[X] = render->camera.worldup[X];
	up[Y] = render->camera.worldup[Y];
	up[Z] = render->camera.worldup[Z];

	float dot = dotProduct(vZ,up);
	
	vY[X] = up[X]-dot*vZ[X];
	vY[Y] = up[Y]-dot*vZ[Y];
	vY[Z] = up[Z]-dot*vZ[Z];
	unitVector(vY);

	vX[X] = vY[Y]*vZ[Z]-vY[Z]*vZ[Y];
	vX[Y] = vY[Z]*vZ[X]-vY[X]*vZ[Z];
	vX[Z] = vY[X]*vZ[Y]-vY[Y]*vZ[X];
	//cross product by unit vector, vX will be unit vector

	render->camera.Xiw[0][X] = vX[X];
	render->camera.Xiw[0][Y] = vX[Y];
	render->camera.Xiw[0][Z] = vX[Z];
	render->camera.Xiw[0][3] = -1.0f*dotProduct(vX,render->camera.position);

	render->camera.Xiw[1][X] = vY[X];
	render->camera.Xiw[1][Y] = vY[Y];
	render->camera.Xiw[1][Z] = vY[Z];
	render->camera.Xiw[1][3] = -1.0f*dotProduct(vY,render->camera.position);

	render->camera.Xiw[2][X] = vZ[X];
	render->camera.Xiw[2][Y] = vZ[Y];
	render->camera.Xiw[2][Z] = vZ[Z];
	render->camera.Xiw[2][3] = -1.0f*dotProduct(vZ,render->camera.position);

	render->camera.Xiw[3][X] = 0;
	render->camera.Xiw[3][Y] = 0;
	render->camera.Xiw[3][Z] = 0;
	render->camera.Xiw[3][3] = 1;

	render->camera.Xwi[X][0] = vX[X];
	render->camera.Xwi[Y][0] = vX[Y];
	render->camera.Xwi[Z][0] = vX[Z];
	render->camera.Xwi[3][0] = 0;

	render->camera.Xwi[X][1] = vY[X];
	render->camera.Xwi[Y][1] = vY[Y];
	render->camera.Xwi[Z][1] = vY[Z];
	render->camera.Xwi[3][1] = 0;

	render->camera.Xwi[X][2] = vZ[X];
	render->camera.Xwi[Y][2] = vZ[Y];
	render->camera.Xwi[Z][2] = vZ[Z];
	render->camera.Xwi[3][2] = 0;

	render->camera.Xwi[X][3] = 0;
	render->camera.Xwi[Y][3] = 0;
	render->camera.Xwi[Z][3] = 0;
	render->camera.Xwi[3][3] = 1;

	GzPushMatrix(render,render->Xsp);
	GzPushMatrix(render,render->camera.Xpi);
	GzPushMatrix(render,render->camera.Xiw);


	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	memcpy(&render->camera,camera,sizeof(GzCamera));
	memset(render->camera.Xiw,0,sizeof(GzMatrix));
	memset(render->camera.Xwi,0,sizeof(GzMatrix));
	memset(render->camera.Xpi,0,sizeof(GzMatrix));

	for(int i=0;i<4;i++){
		render->camera.Xwi[i][i]=1;
		render->camera.Xiw[i][i]=1;
		render->camera.Xpi[i][i]=1;
	}
	return GZ_SUCCESS;
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	GzMatrix identityM = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	if(render->matlevel >= MATLEVELS )
		return GZ_FAILURE;
	else if(render->matlevel == 0){
		memcpy(render->Ximage[0],matrix,sizeof(GzMatrix));//Xsp
		memcpy(render->Xnorm[0],identityM,sizeof(GzMatrix));//Xsp-norm
	}else{
		matrixMul(render->Ximage[render->matlevel],render->Ximage[render->matlevel-1],matrix);//Xpi
		if(render->matlevel == 1){
			memcpy(render->Xnorm[1],identityM,sizeof(GzMatrix));//Xpi-norm
		}else{
			GzMatrix unitaryM;
			memcpy(unitaryM, matrix, sizeof(GzMatrix));
			matrixUnitary(unitaryM);
			unitaryM[0][3] = 0;
			unitaryM[1][3] = 0;
			unitaryM[2][3] = 0;
			unitaryM[3][0] = 0;
			unitaryM[3][1] = 0;
			unitaryM[3][2] = 0;
			unitaryM[3][3] = 1;
			matrixMul(render->Xnorm[render->matlevel],render->Xnorm[render->matlevel-1],unitaryM);
		}
	}
	render->matlevel++;
	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if(render->matlevel==0) return GZ_FAILURE;
	render->matlevel--;
	return GZ_SUCCESS;
}

int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	for(int i=0; i<numAttributes; i++){
		switch(nameList[i]){
		case GZ_RGB_COLOR:
			render->flatcolor[0] = ((float*)valueList[i])[0];
			render->flatcolor[1] = ((float*)valueList[i])[1];
			render->flatcolor[2] = ((float*)valueList[i])[2];
			break;
		case GZ_INTERPOLATE:
			render->interp_mode = *(int*)valueList[i];
			break;
		case GZ_DIRECTIONAL_LIGHT:
			if (render->numlights >= MAX_LIGHTS) return GZ_FAILURE;
			memcpy(&(render->lights[render->numlights++]),(GzLight*)(valueList[i]),sizeof(GzLight));
			break;
		case GZ_AMBIENT_LIGHT:
			memcpy(&(render->ambientlight),(GzLight*)(valueList[i]),sizeof(GzLight));
			break;
		case GZ_AMBIENT_COEFFICIENT:
			memcpy(&(render->Ka),valueList[i],sizeof(GzColor));
			break;
		case GZ_DIFFUSE_COEFFICIENT:
			memcpy(&(render->Kd),valueList[i],sizeof(GzColor));
			break;
		case GZ_SPECULAR_COEFFICIENT:
			memcpy(&(render->Ks),valueList[i],sizeof(GzColor));
			break;
		case GZ_DISTRIBUTION_COEFFICIENT:
			render->spec = *(float*)valueList[i];
			break;
		case GZ_TEXTURE_MAP:
			render->tex_fun = (GzTexture)valueList[i];
			break;
		case GZ_AASHIFTX:
			render->dx = *(float*)valueList[i];
			break;
		case GZ_AASHIFTY:
			render->dy = *(float*)valueList[i];
			break;
		case GZ_CAMERAPHI:
			render->phi = *(float*)valueList[i];
			break;
		case GZ_CAMERAXITA:
			render->xita = *(float*)valueList[i];
			break;
		default:
			return GZ_FAILURE;
		}
	}
	
	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts using matrix on top of stack 
- Clip - just discard any triangle with any vert(s) behind view plane 
       - optional: test for triangles with all three verts off-screen (trivial frustum cull)
- invoke triangle rasterizer  
*/	
	GzCoord vertexList[3];
	GzCoord normList[3];
	GzTextureIndex texureList[3];
	int status = 0;
	for(int i=0; i<numParts; i++){
		switch(nameList[i]){
		case GZ_NULL_TOKEN:
			break;
		case GZ_NORMAL:
			// Transform the normal vector from model to image space
			memcpy(normList,valueList[i],sizeof(GzCoord)*3);
			Xform(render->Xnorm[render->matlevel-1],normList[0]);
			Xform(render->Xnorm[render->matlevel-1],normList[1]);
			Xform(render->Xnorm[render->matlevel-1],normList[2]);
			break;
		case GZ_POSITION:
			memcpy(vertexList,valueList[i],sizeof(GzCoord)*3);
			if(render->interp_mode == GZ_FLAT){
				//Decide the color in the GZ_FLAT mode for each triangle
				GzCoord lCross,rCross;
				lCross[X] = vertexList[0][X]-vertexList[1][X];
				lCross[Y] = vertexList[0][Y]-vertexList[1][Y];
				lCross[Z] = vertexList[0][Z]-vertexList[1][Z];
				rCross[X] = vertexList[1][X]-vertexList[2][X];
				rCross[Y] = vertexList[1][Y]-vertexList[2][Y];
				rCross[Z] = vertexList[1][Z]-vertexList[2][Z];
				GzCoord planN;
				planN[X] = lCross[Y]*rCross[Z]-lCross[Z]*rCross[Y];
				planN[Y] = lCross[Z]*rCross[X]-lCross[X]*rCross[Z];
				planN[Z] = lCross[X]*rCross[Y]-lCross[Y]*rCross[X];
				Xform(render->Xnorm[render->matlevel-1],planN);
				unitVector(planN);
				shade(render,planN,render->flatcolor);
			}
			status |=Xform(render->Ximage[render->matlevel-1],vertexList[0]);
			status |=Xform(render->Ximage[render->matlevel-1],vertexList[1]);
			status |=Xform(render->Ximage[render->matlevel-1],vertexList[2]);
			
			
			// Sub-sample Offset default=0
			for(int j=0;j<3;j++){
				vertexList[j][X]-=render->dx;
				vertexList[j][Y]-=render->dy;
			}
			
			// skip the triangle which three vertices are all located at the outside of the same edge
			if((vertexList[0][X]>render->display->xres && vertexList[1][X]>render->display->xres && vertexList[2][X]>render->display->xres) ||
				(vertexList[0][Y]>render->display->yres && vertexList[1][Y]>render->display->yres && vertexList[2][Y]>render->display->yres) ||
				(vertexList[0][X]<0 && vertexList[1][X]<0 && vertexList[2][X]<0) ||
				(vertexList[0][Y]<0 && vertexList[1][Y]<0 && vertexList[2][Y]<0))
				status |= GZ_FAILURE;
			break;
		case GZ_TEXTURE_INDEX:
			memcpy(texureList,valueList[i],sizeof(GzTextureIndex)*3);
			break;
		default:
			return GZ_FAILURE;
		}
	}
	if (status) 
		return GZ_SUCCESS;
	else{
		GzScanLine(render, vertexList, normList, texureList);
	}
	return GZ_SUCCESS;
}


int Xform(GzMatrix mat, GzCoord vec){
	float after[4];
	for(int i=0;i<4;i++){
		after[i] = mat[i][X]*vec[X] + 
					mat[i][Y]*vec[Y] +
					mat[i][Z]*vec[Z] +
					mat[i][3]*1;
	}

	if(after[3] != 0){
		vec[X] = after[X]/after[3];
		vec[Y] = after[Y]/after[3];
		vec[Z] = after[Z]/after[3];
	}else{
		vec[X] = after[X];
		vec[Y] = after[Y];
		vec[Z] = after[Z];
	}
	if(after[2]<0)
		return GZ_FAILURE;
	return GZ_SUCCESS;
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

void GzScanLine(GzRender *render, GzCoord *vertexList, GzCoord *normList, GzTextureIndex* textureList){
	
	sortV(vertexList, normList, textureList);
	warpTex(vertexList, textureList);
	bool upperTri=true, lowerTri=true;

	/* setup each edge */
	GzEdge e12,e13,e23;
	upperTri = edgeInit(render,&e12,vertexList[0],vertexList[1],normList[0],normList[1],textureList[0],textureList[1]);
	lowerTri = edgeInit(render,&e23,vertexList[1],vertexList[2],normList[1],normList[2],textureList[1],textureList[2]);
	edgeInit(render,&e13,vertexList[0],vertexList[2],normList[0],normList[2],textureList[0],textureList[2]);
	
	GzEdge *Le, *Re;
	float delY_L, delY_R;
	bool cond;
	if(upperTri){
		/* distinguish the left edge(Le) and the right edge(Re) */
		if(e12.slopeX < e13.slopeX){ //L(1-2-3) R(1-3)
			Le = &e12;
			Re = &e13;
			cond = true;
		}else{ //L(1-3) R(1-2-3)
			Le = &e13;
			Re = &e12;
			cond = false;
		}

		/*--------------------------------------------------------------------------
		  Move the current point to the first scanline on each edge
		----------------------------------------------------------------------------*/
		delY_L = ceil(Le->current[Y])-Le->current[Y];
		edgeAdvanceDel(Le, delY_L);
		delY_R = ceil(Re->current[Y])-Re->current[Y];
		edgeAdvanceDel(Re, delY_R);

		/* Advance toward the lower end of first two edges */
		edgeAdvance(render, Le, Re);
	}

	/*--------------------------------------------------------------------------
	  change the left edge or right edge to the edge 23 and 
	  initial the current position of the edge 23 to the current scanline
	----------------------------------------------------------------------------*/
	if(lowerTri){
		if(!upperTri){
			/* if the edge 12 is horizontal and edge 23 is not, 
				move current position of the edge 13 to the first scan line*/
			if(e23.current[X] > e13.current[X]){
				cond = false;
				Le = &e13;
				delY_L = ceil(Le->current[Y])-Le->current[Y];
				edgeAdvanceDel(Le, delY_L);
			}else{
				cond = true;
				Re = &e13;
				delY_R = ceil(Re->current[Y])-Re->current[Y];
				edgeAdvanceDel(Re, delY_R);
			}
		}
		if(cond){  //L(1-2-3) R(1-3)
			Le = &e23;
			delY_L = Re->current[Y] - Le->current[Y];
			edgeAdvanceDel(Le, delY_L);
		}else{  //L(1-3) R(1-2-3)
			Re = &e23;
			delY_R = Le->current[Y] - Re->current[Y];
			edgeAdvanceDel(Re, delY_R);
		}

		/* Advance toward the end of the edges, vertex 3 */
		edgeAdvance(render, Le, Re);
	}
}

/* 
	advance two edges toward the end vertext of two edges
	and span each scanline in the between
*/
void edgeAdvance(GzRender *render, GzEdge *Le, GzEdge *Re){

	while((Le->current[Y] < Le->end[Y]) && (Re->current[Y] < Re->end[Y])){
		/*span the scanline*/
		span(render, Le, Re);
		/* next scan line */
		edgeAdvanceDel(Le, 1);
		edgeAdvanceDel(Re, 1);
	}
}

void edgeAdvanceDel(GzEdge *e, float delY){
	e->current[Y] += delY;

	e->current[X] += e->slopeX*delY;
	e->current[Z] += e->slopeZ*delY;
	e->currentIntrp[X] += e->slopeIntrp[X]*delY;
	e->currentIntrp[Y] += e->slopeIntrp[Y]*delY;
	e->currentIntrp[Z] += e->slopeIntrp[Z]*delY;
	e->currentTex[U] += e->slopeTex[U]*delY;
	e->currentTex[V] += e->slopeTex[V]*delY;
}

void span(GzRender *render, GzEdge *le, GzEdge *re){
	/*
	- Run from left to right for the scan line
	- Call GzPutDisplay to set-up every pixel
	*/

	//assert( le->current[X]<=re->current[X]);
	GzCoord start,end,current,currentIntrp;
	GzTextureIndex currentTex;
	GzDepth fbuf_z;
	GzIntensity fbuf_data; // trash data

	start[X] = le->current[X];
	start[Z] = le->current[Z];
	end[X] = re->current[X];
	end[Z] = re->current[Z];
	memcpy(current,le->current,sizeof(GzCoord));
	memcpy(currentIntrp,le->currentIntrp,sizeof(GzCoord));
	memcpy(currentTex,le->currentTex,sizeof(GzTextureIndex));

	if(re->current[X] == le->current[X] && le->current[X] == ceil(le->current[X])){// special case
		if(GzGetDisplay(render->display, (int)current[X], (int)le->current[Y],
			&fbuf_data, &fbuf_data, &fbuf_data, &fbuf_data, &fbuf_z) == GZ_SUCCESS){
			/* if the current Z smaller than the Z in the frame buffer, then replace the pixel to the new one */
			if(current[Z] < fbuf_z){
				GzPutDisplay(render->display, (int)current[X],(int)current[Y],
					ctoi( render->flatcolor[RED]),
					ctoi( render->flatcolor[GREEN]),
					ctoi( render->flatcolor[BLUE]),
					0,
					(GzDepth)current[Z]);
			}
		}	
		return;
	}

	float detX = re->current[X]-le->current[X];
	float slopeZ = (re->current[Z]-le->current[Z])/detX;
	GzCoord slopeIntrp;
	slopeIntrp[X] = (re->currentIntrp[X]-le->currentIntrp[X])/detX;
	slopeIntrp[Y] = (re->currentIntrp[Y]-le->currentIntrp[Y])/detX;
	slopeIntrp[Z] = (re->currentIntrp[Z]-le->currentIntrp[Z])/detX;
	GzTextureIndex slopeTex;
	slopeTex[U] = (re->currentTex[U]-le->currentTex[U])/detX;
	slopeTex[V] = (re->currentTex[V]-le->currentTex[V])/detX;

	float delX = ceil(start[X])- start[X];
	if(current[X]<0)
		delX = -current[X];

	/* move the current point to the first pixel on the scanline */
	current[X] += delX;
	current[Z] += delX*slopeZ;
	currentIntrp[X] += delX*slopeIntrp[X];
	currentIntrp[Y] += delX*slopeIntrp[Y];
	currentIntrp[Z] += delX*slopeIntrp[Z];
	currentTex[U] += delX*slopeTex[U];
	currentTex[V] += delX*slopeTex[V];
	
	/* make sure the current point still locate between two edges */
	while(current[X] < end[X] && current[X] < render->display->xres){
		/* To fetch the Z from the current display frame buffer */
		if(GzGetDisplay(render->display, (int)current[X], (int)le->current[Y],
			&fbuf_data, &fbuf_data, &fbuf_data, &fbuf_data, &fbuf_z) == GZ_SUCCESS){
			/* if the current Z smaller than the Z in the frame buffer, then replace the pixel to the new one */
			if(current[Z] < fbuf_z){
				GzTextureIndex unWarpedTex;

				GzCoord E = {0,0,1};
					GzCoord R={0,0,0},tmp={0,0,0};
					int face = 0;
					float u,v;
					int xs = render->display->xres;
					int ys = render->display->yres;
					GzColor cubeCol[6]={{0,100,0},{0,128,0},{154,204,255},{51,204,51},{0,255,0}
					,{102,102,255}};
					int texU,texV;
				switch(render->interp_mode){
				case GZ_FLAT:
					GzPutDisplay(render->display, (int)current[X],(int)current[Y],
						ctoi( render->flatcolor[RED]),
						ctoi( render->flatcolor[GREEN]),
						ctoi( render->flatcolor[BLUE]),
						0,
						(GzDepth)current[Z]);
					break;
				case GZ_NORMAL: //Phong
					GzColor col;
					unitVector(currentIntrp);
					if(render->tex_fun!=NULL){
						memcpy(unWarpedTex,currentTex,sizeof(GzTextureIndex));
						unWarpTex(current,unWarpedTex);
						render->tex_fun(unWarpedTex[U],unWarpedTex[V],col);
						memcpy(render->Kd,col,sizeof(GzColor));
						memcpy(render->Ka,col,sizeof(GzColor));
					}

					/********************************************
						    CubeMap Lookup - Reflected face
					*********************************************/
					float NE;
					GzCoord N;
					
					memcpy(N,currentIntrp,sizeof(GzCoord));
					
					NE = dotProduct(E,N);
					if(NE > 0){
						mulVector(-1,N,N);
						NE = -NE;
					}
					mulVector(2*NE,N,tmp);
					R[X] = E[X]+tmp[X];
					R[Y] = E[Y]+tmp[Y];
					R[Z] = E[Z]+tmp[Z];
					
					Xform(render->camera.Xwi,R); // transform the vector R to World space
					
					float absX,absY,absZ;
					absX = abs(R[X]);
					absY = abs(R[Y]);
					absZ = abs(R[Z]);
					if(absX>absY){
						if(absX>absZ){
							face = X;
							u = (R[Y]+absX)/2/absX;
							v = (R[Z]+absX)/2/absX;
							
							if(R[X]<0){
								face+=3;
								u = abs(1-u);
								v = abs(1-v);
							}else{
								v = abs(1-v);
							}
							
						}else{
							face = Z;
							u = (R[X]+absZ)/2/absZ;
							v = (R[Y]+absZ)/2/absZ;
							
							if(R[Z]<0){
								face+=3;
							}else{
								v = abs(1-v);						
							}
							
						}
					}else{
						if(absY>absZ){
							face = Y;
							u = (R[X]+absY)/2/absY;
							v = (R[Z]+absY)/2/absY;
							
							if(R[Y]<0){
								face+=3;
								v = abs(1-v);						
							}else{
								v = abs(1-v);
							}
							
						}else{
							face = Z;
							u = (R[X]+absZ)/2/absZ;
							v = (R[Y]+absZ)/2/absZ;
							
							if(R[Z]<0){
								face+=3;
							}else{
								v = abs(1-v);						
							}
						}
					}
					//if(R[face]<0) face+=3;
					/********************************************
					  CubeMap Lookup - Initialize the cube map
					*********************************************/
					if(image[0] == NULL){
						for(int i=0;i<6;i++){
							FILE *fd;
							char filein[20];
							sprintf(filein,"cubemap/sky%d.ppm",i);
							fd = fopen(filein,"rb");
							if (fd == NULL) {
								fprintf (stderr, "texture file not found\n");
								exit(-1);
							}
							char foo[10];
							unsigned char pixel[3];
							unsigned char dummy;
							int dum;
							fscanf (fd, "%s %d %d %c", foo, &dum, &dum, &dummy);
							image[i] = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
							if(image[i]==NULL){
								fprintf (stderr, "malloc for texture image failed\n");
								exit(-1);
							}
							for (int x = 0; x < xs*ys; x++) {	/* create array of GzColor values */
								fread(pixel, sizeof(pixel), 1, fd);
								image[i][x][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);//(float)((int)cubeCol[i][RED]) * (1.0 / 255.0);//
								image[i][x][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);//(float)((int)cubeCol[i][GREEN]) * (1.0 / 255.0);//
								image[i][x][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);//(float)((int)cubeCol[i][BLUE]) * (1.0 / 255.0);//
							}
							fclose(fd);
						}
					}
					/********************************************
					     CubeMap Lookup - Texture lookup
					*********************************************/
					GzTextureIndex tex;
					float s, t;
					tex[U] = u*(xs-1);
					tex[V] = v*(ys-1);
					s = tex[U] - (int)tex[U];
					t = tex[V] - (int)tex[V];
					texU = (int)tex[U];
					texV = (int)tex[V];

					for (int i=0; i<3; i++){
						col[i] = 
							image[face][ texU    + texV   *xs][i] * (1-s)*(1-t) +
							image[face][(texU+1) + texV   *xs][i] *    s *(1-t) +
							image[face][(texU+1) +(texV+1)*xs][i] *    s * t    +
							image[face][ texU    +(texV+1)*xs][i] * (1-s)* t;
						
					}
					/*******************************************/
					//shade(render,currentIntrp,col);
					
					GzPutDisplay(render->display, (int)current[X],(int)current[Y],
						ctoi( col[RED]),
						ctoi( col[GREEN]),
						ctoi( col[BLUE]),
						0,
						(GzDepth)current[Z]);
					break;
				case GZ_COLOR: //Gouraud
					GzColor intrpedCol;
					if(render->tex_fun!=NULL){
						GzColor Kt;
						memcpy(unWarpedTex,currentTex,sizeof(GzTextureIndex));
						unWarpTex(current,unWarpedTex);
						render->tex_fun(unWarpedTex[U],unWarpedTex[V],Kt);
						intrpedCol[RED] = Kt[RED]*currentIntrp[RED];
						intrpedCol[GREEN] = Kt[GREEN]*currentIntrp[GREEN];
						intrpedCol[BLUE] = Kt[BLUE]*currentIntrp[BLUE];
						if(intrpedCol[RED]>1.0f) intrpedCol[RED] = 1.0f;
						if(intrpedCol[GREEN]>1.0f) intrpedCol[GREEN] = 1.0f;
						if(intrpedCol[BLUE]>1.0f) intrpedCol[BLUE] = 1.0f;
					}else{
						memcpy(intrpedCol,currentIntrp,sizeof(GzColor));
					}
					GzPutDisplay(render->display, (int)current[X],(int)current[Y],
						ctoi( intrpedCol[RED]),
						ctoi( intrpedCol[GREEN]),
						ctoi( intrpedCol[BLUE]),
						0,
						(GzDepth)current[Z]);
					break;
				default:
					break;
				}
			}
		}
		current[X] += 1;
		current[Z] += slopeZ;
		currentIntrp[X] += slopeIntrp[X];
		currentIntrp[Y] += slopeIntrp[Y];
		currentIntrp[Z] += slopeIntrp[Z];
		currentTex[U] += slopeTex[U];
		currentTex[V] += slopeTex[V];
	}

}

/* Initialize the GzEdge for the use of scanline algorithm */
bool edgeInit(GzRender *render, GzEdge *e, float *start, float *end, float *startN, float *endN, float *startT, float *endT){
	if(end[Y] == start[Y])
		return false;
	e->start = start;
	e->end = end;
	memcpy(e->current,e->start,sizeof(GzCoord));
	float delY = e->end[Y] - e->start[Y];
	e->slopeX = (e->end[X] - e->start[X])/delY;
	e->slopeZ = (e->end[Z] - e->start[Z])/delY;
	
	memcpy(e->startTex,startT,sizeof(GzTextureIndex));
	memcpy(e->endTex,endT,sizeof(GzTextureIndex));
	memcpy(e->currentTex,e->startTex,sizeof(GzTextureIndex));
	e->slopeTex[U] = (e->endTex[U] - e->startTex[U])/delY;
	e->slopeTex[V] = (e->endTex[V] - e->startTex[V])/delY;
	
	switch(render->interp_mode){
	case GZ_FLAT:
		break;
	case GZ_NORMAL:
		//Interpolate Normal
		memcpy(e->startIntrp,startN,sizeof(GzCoord));
		memcpy(e->endIntrp,endN,sizeof(GzCoord));
		memcpy(e->currentIntrp,e->startIntrp,sizeof(GzCoord));
		e->slopeIntrp[X] = (e->endIntrp[X] - e->startIntrp[X])/delY;
		e->slopeIntrp[Y] = (e->endIntrp[Y] - e->startIntrp[Y])/delY;
		e->slopeIntrp[Z] = (e->endIntrp[Z] - e->startIntrp[Z])/delY;
		//set-up for interpolate normal vector for drawing
		break;
	case GZ_COLOR:
		//shade for two vertext for the color interpolation
		//Interpolate Color
		shade(render,startN,e->startIntrp);
		shade(render,endN,e->endIntrp);
		memcpy(e->currentIntrp,e->startIntrp,sizeof(GzCoord));
		e->slopeIntrp[X] = (e->endIntrp[X] - e->startIntrp[X])/delY;
		e->slopeIntrp[Y] = (e->endIntrp[Y] - e->startIntrp[Y])/delY;
		e->slopeIntrp[Z] = (e->endIntrp[Z] - e->startIntrp[Z])/delY;
		break;
	default:
		break;
	}
	return true;
}

/* Sort the verticies by Y and then by X */
void sortV(GzCoord *vertexList, GzCoord *normList, GzTextureIndex* textureList){
	swapV(vertexList,normList, textureList, 0, 1);
	swapV(vertexList,normList, textureList, 1, 2);
	swapV(vertexList,normList, textureList, 0, 1);
}

void swapV(GzCoord *vertexList,GzCoord *normList, GzTextureIndex* textureList, int a, int b){
	float tmp[3];
	if(vertexList[a][Y]>vertexList[b][Y] ||
		(vertexList[a][Y]==vertexList[b][Y] && vertexList[a][X]<vertexList[b][X])){
		memcpy(tmp, vertexList[a], sizeof(GzCoord));
		memcpy(vertexList[a], vertexList[b], sizeof(GzCoord));
		memcpy(vertexList[b], tmp, sizeof(GzCoord));
		memcpy(tmp, normList[a], sizeof(GzCoord));
		memcpy(normList[a], normList[b], sizeof(GzCoord));
		memcpy(normList[b], tmp, sizeof(GzCoord));
		memcpy(tmp, textureList[a], sizeof(GzTextureIndex));
		memcpy(textureList[a], textureList[b], sizeof(GzTextureIndex));
		memcpy(textureList[b], tmp, sizeof(GzTextureIndex));
	}
}

void unitVector(GzCoord v){
	float det = sqrt(v[X]*v[X]+v[Y]*v[Y]+v[Z]*v[Z]);
	v[X] /= det;
	v[Y] /= det;
	v[Z] /= det;
}

float dotProduct(GzCoord v1, GzCoord v2){
	return v1[X]*v2[X]+v1[Y]*v2[Y]+v1[Z]*v2[Z];
}

void mulVector(const float a, const GzCoord v,GzCoord tmp){
	tmp[X] = v[X] * a;
	tmp[Y] = v[Y] * a;
	tmp[Z] = v[Z] * a;
}

void matrixMul(GzMatrix dst, const GzMatrix src1, const GzMatrix src2){
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			dst[i][j] = 0;
			for(int k=0;k<4;k++)
				dst[i][j] += src1[i][k]*src2[k][j];
		}
	}
}

void matrixUnitary(GzMatrix mat){

	float K = 1/sqrt(mat[0][0]*mat[0][0]+mat[0][1]*mat[0][1]+mat[0][2]*mat[0][2]);

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			mat[i][j] *= K;
		}
	}
}

void shade(GzRender *render, GzCoord N, GzColor color){
	GzColor aColor;
	GzColor dColor;
	GzColor sColor;

	GzCoord R;
	GzCoord E = {0,0,-1};
	GzCoord tmp;

	memset(aColor,0,sizeof(GzColor));
	memset(dColor,0,sizeof(GzColor));
	memset(sColor,0,sizeof(GzColor));
	memset(color,0,sizeof(GzColor));
	
	aColor[RED]   = render->ambientlight.color[RED];
	aColor[GREEN] = render->ambientlight.color[GREEN];
	aColor[BLUE]  = render->ambientlight.color[BLUE];



	for(int i=0;i<render->numlights;i++){
		float NL = dotProduct(N,render->lights[i].direction);
		float NE = dotProduct(N,E);
		if(NL < 0 && NE < 0){
			mulVector(-1,N,N);
			NL = -NL;
			NE = -NE;
		}else if(NL * NE < 0){
			continue;
		}

		// R = 2*(NL)N-L
		mulVector(2*NL,N,tmp);
		R[X] = tmp[X] - render->lights[i].direction[X];
		R[Y] = tmp[Y] - render->lights[i].direction[Y];
		R[Z] = tmp[Z] - render->lights[i].direction[Z];

		//Ie*(RE)^s
		float RE = dotProduct(R,E);
		if(RE > 0){
			double REspec = pow(RE,render->spec);

			sColor[RED] += render->lights[i].color[RED]*REspec;
			sColor[GREEN] += render->lights[i].color[GREEN]*REspec;
			sColor[BLUE] += render->lights[i].color[BLUE]*REspec;
		}

		//Ie*(NL)
		dColor[RED] += render->lights[i].color[RED]*NL;
		dColor[GREEN] += render->lights[i].color[GREEN]*NL;
		dColor[BLUE] += render->lights[i].color[BLUE]*NL;
	}
	if(render->tex_fun!=NULL && render->interp_mode==GZ_COLOR){
		color[RED] = sColor[RED] + dColor[RED] + aColor[RED];
		color[GREEN] = sColor[GREEN] + dColor[GREEN] + aColor[GREEN];
		color[BLUE] = sColor[BLUE] + dColor[BLUE] + aColor[BLUE];
	}else{
		color[RED] = render->Ks[RED]*sColor[RED] + render->Kd[RED]*dColor[RED] + render->Ka[RED]*aColor[RED];
		color[GREEN] = render->Ks[GREEN]*sColor[GREEN] + render->Kd[GREEN]*dColor[GREEN] + render->Ka[GREEN]*aColor[GREEN];
		color[BLUE] = render->Ks[BLUE]*sColor[BLUE] + render->Kd[BLUE]*dColor[BLUE] + render->Ka[BLUE]*aColor[BLUE];
		if(color[RED]>1.0f) color[RED] = 1.0f;
		if(color[GREEN]>1.0f) color[GREEN] = 1.0f;
		if(color[BLUE]>1.0f) color[BLUE] = 1.0f;
	}
}

void warpTex(GzCoord *vertexList, GzTextureIndex *textureList){
	//return;
	float Vz=0;
	for(int i=0; i<3; i++){
		Vz = vertexList[i][Z] / (MAXINT - vertexList[i][Z]);
		textureList[i][U] /= (Vz+1);
		textureList[i][V] /= (Vz+1);	
	}
}

void unWarpTex(GzCoord pixel, GzTextureIndex texture){
	//return;
	float Vz=0;
	Vz = pixel[Z]/ (MAXINT - pixel[Z]);
	texture[U] *= Vz+1;
	texture[V] *= Vz+1;
}