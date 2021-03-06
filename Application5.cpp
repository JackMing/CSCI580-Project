// Application5.cpp: implementation of the Application5 class.
//
//////////////////////////////////////////////////////////////////////

/*
 * application test code for homework assignment #5
*/

#include "stdafx.h"
#include "CS580HW.h"
#include "Application5.h"
#include "Gz.h"
#include "disp.h"
#include "rend.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define INFILE  "teapot.asc"
#define SKYMAP  "sphere.asc"
#define OUTFILE "Demo7-"


extern int tex_fun(float u, float v, GzColor color, int bgface); /* image texture function */
extern int ptex_fun(float u, float v, GzColor color); /* procedural texture function */

void shade(GzCoord norm, GzCoord color);

//float AAFilter[6][3] =		/* X-shift, Y-shift, weight */
//{
//-0.52, 0.38, 0.128, 		0.41, 0.56, 0.119,		0.27, 0.08, 0.294,
//-0.17, -0.29, 0.249,		0.58, -0.55, 0.104,		-0.31, -0.71, 0.106
//};
float AAFilter[6][3] = {
0, 0, 1, 		0.41, 0.56, 0.119,		0.27, 0.08, 0.294,
-0.17, -0.29, 0.249,		0.58, -0.55, 0.104,		-0.31, -0.71, 0.106
};

int fcount=199;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Application5::Application5()
{

}

Application5::~Application5()
{
	Clean();
}

int Application5::Initialize()
{
	GzCamera	camera;  
	int		    xRes, yRes;	/* display parameters */ 

	GzToken		nameListShader[9]; 	    /* shader attribute names */
	GzPointer   valueListShader[9];		/* shader attribute pointers */
	GzToken     nameListLights[10];		/* light info */
	GzPointer   valueListLights[10];
	GzToken		nameListSample[2];
	GzPointer	valueListSample[2];
	int			shaderType, interpStyle;
	float		specpower;
	int		status; 
 
	status = 0; 



/* Translation matrix */
GzMatrix	scale = 
{ 
	0.5,	0.0,	0.0,	0.0, 
	0.0,	0.5,	0.0,	0.0, 
	0.0,	0.0,	0.5,	0.0, 
	0.0,	0.0,	0.0,	1.0 
}; 
 
GzMatrix	rotateX = 
{ 
	1.0,	0.0,	0.0,	0.0, 
	0.0,	.7071,	.7071,	0.0, 
	0.0,	-.7071,	.7071,	0.0, 
	0.0,	0.0,	0.0,	1.0 
}; 
 
GzMatrix	rotateY = 
{ 
	.866,	0.0,	-0.5,	0.0, 
	0.0,	1.0,	0.0,	0.0, 
	0.5,	0.0,	.866,	0.0, 
	0.0,	0.0,	0.0,	1.0 
}; 

	/* 
	 * Allocate memory for user input
	 */
	m_pUserInput = new GzInput;

	/* 
	 * initialize the display and the renderer 
	 */ 
 	m_nWidth = 768;		// frame buffer and display width
	m_nHeight = 768;    // frame buffer and display height

	
	/* set up a app-defined camera */
    camera.position[X] = -0.7;
    camera.position[Y] = 0;
    camera.position[Z] = 0;

    camera.lookat[X] = 0;
    camera.lookat[Y] = 0;
    camera.lookat[Z] = 0;

    camera.worldup[X] = 0.0;
    camera.worldup[Y] = 0.0;
    camera.worldup[Z] = 1.0;

    camera.FOV = 63.7; /* degree */

	/* Light */
	GzLight	light1 = { {-0.7071, 0.7071, 0}, {0.5, 0.5, 0.9} };
	GzLight	light2 = { {0, -0.7071, -0.7071}, {0.9, 0.2, 0.3} };
	GzLight	light3 = { {0.7071, 0.0, -0.7071}, {0.2, 0.7, 0.3} };
	GzLight	ambientlight = { {0, 0, 0}, {0.3, 0.3, 0.3} };

	/* Material property */
	GzColor specularCoefficient = { 0.3, 0.3, 0.3 };
	GzColor ambientCoefficient = { 0.1, 0.1, 0.1 };
	GzColor diffuseCoefficient = {0.7, 0.7, 0.7};

/* 
  renderer is ready for frame --- define lights and shader at start of frame 
*/

	/*
	 * Tokens associated with light parameters
	 */
	nameListLights[0] = GZ_DIRECTIONAL_LIGHT;
	valueListLights[0] = (GzPointer)&light1;
	nameListLights[1] = GZ_DIRECTIONAL_LIGHT;
	valueListLights[1] = (GzPointer)&light2;
	nameListLights[2] = GZ_DIRECTIONAL_LIGHT;
	valueListLights[2] = (GzPointer)&light3;
	nameListLights[3] = GZ_AMBIENT_LIGHT;
	valueListLights[3] = (GzPointer)&ambientlight;

	/*
	 * Tokens associated with shading 
	 */
	nameListShader[0]  = GZ_DIFFUSE_COEFFICIENT;
	valueListShader[0] = (GzPointer)diffuseCoefficient;
	/* 
	* Select either GZ_COLOR or GZ_NORMALS as interpolation mode  
	*/
	nameListShader[1]  = GZ_INTERPOLATE;
	interpStyle = GZ_NORMALS;         /* Phong shading */
	valueListShader[1] = (GzPointer)&interpStyle;
	nameListShader[2]  = GZ_AMBIENT_COEFFICIENT;
	valueListShader[2] = (GzPointer)ambientCoefficient;
	nameListShader[3]  = GZ_SPECULAR_COEFFICIENT;
	valueListShader[3] = (GzPointer)specularCoefficient;
	nameListShader[4]  = GZ_DISTRIBUTION_COEFFICIENT;
	specpower = 32;
	valueListShader[4] = (GzPointer)&specpower;
	nameListShader[5]  = GZ_TEXTURE_MAP;
#if 0   /* set up null texture function or valid pointer */
	valueListShader[5] = (GzPointer)0;
#else
	valueListShader[5] = (GzPointer)(tex_fun);	/* or use ptex_fun */
#endif



	status |= GzNewFrameBuffer(&m_pFrameBuffer, m_nWidth, m_nHeight);

	for(int sampleNum = 0; sampleNum < AAKERNEL_SIZE; sampleNum++){		

		status |= GzNewDisplay(&m_pDisplay[sampleNum], m_nWidth, m_nHeight);
		status |= GzGetDisplayParams(m_pDisplay[sampleNum], &xRes, &yRes); 
		status |= GzNewRender(&m_pRender[sampleNum], m_pDisplay[sampleNum]); 

#if 1 	/* set up app-defined camera if desired, else use camera defaults */
		status |= GzPutCamera(m_pRender[sampleNum], &camera); 
#endif 
		/* Start Renderer */
		status |= GzBeginRender(m_pRender[sampleNum]);

		nameListSample[0] = GZ_AASHIFTX;
		valueListSample[0] = (GzPointer)&AAFilter[sampleNum][0];
		nameListSample[1] = GZ_AASHIFTY;
		valueListSample[1] = (GzPointer)&AAFilter[sampleNum][1];

		status |= GzPutAttribute(m_pRender[sampleNum], 4, nameListLights, valueListLights);
		status |= GzPutAttribute(m_pRender[sampleNum], 6, nameListShader, valueListShader);
		status |= GzPutAttribute(m_pRender[sampleNum], 2, nameListSample, valueListSample);

		//status |= GzPushMatrix(m_pRender[sampleNum], scale);  
		//status |= GzPushMatrix(m_pRender[sampleNum], rotateY); 
		//status |= GzPushMatrix(m_pRender[sampleNum], rotateX); 
	}

	if (status) exit(GZ_FAILURE); 

	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application5::Render() 
{
	GzToken		nameListTriangle[4]; 	/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 	/* vertex attribute pointers */
	GzToken		nameListShader[2]; 	/* vertex attribute names */
	GzPointer	valueListShader[2]; 	/* vertex attribute pointers */
	GzCoord		vertexList[3];	/* vertex position coordinates */ 
	GzCoord		normalList[3];	/* vertex normals */ 
	GzTextureIndex  	uvList[3];		/* vertex texture map indices */ 
	char		dummy[1000]; 
	int			status; 
	int			face;
	// 0.6 for sphere.asc, 0.01 for teapot.asc
	GzMatrix	scale_obj = 
	{ 
		0.01,	0.0,	0.0,	0.0, 
		0.0,	0.01,	0.0,	0.0, 
		0.0,	0.0,	0.01,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	}; 
	GzMatrix	rotateY_obj = 
	{ 
		-1.0,	0.0,	0.0,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.0,	0.0,	-1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	}; 
	GzMatrix	rotateZ_obj = 
	{ 
		0.0,	1.0,	0.0,	0.0, 
		-1.0,	0.0,	0.0,	0.0, 
		0.0,	0.0,	1.0,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	}; 
	GzMatrix	scale_cube = 
	{ 
		5,	0.0,	0.0,	0.0, 
		0.0,	5,	0.0,	0.0, 
		0.0,	0.0,	5,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	}; 	/* Initialize Display */
	for(int i=0;i<AAKERNEL_SIZE;i++)
		status |= GzInitDisplay(m_pDisplay[i]); 
	
	/* 
	* Tokens associated with triangle vertex values 
	*/ 
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_FACE;
	// I/O File open
	FILE *infile;
	FILE *outfile;
	char ofilename[70];

	sprintf(ofilename,"D:\\JackMing\\Desktop\\csci580-capture\\%s%03d.ppm",OUTFILE,fcount++);

	if( (outfile  = fopen( ofilename , "wb" )) == NULL )
	{
         //AfxMessageBox( "The output file was not opened\n" );
		 //return GZ_FAILURE;
	}

	/* 
	* Walk through the list of triangles, set color 
	* and render each triangle 
	*/
	bool skybox;
	int interpStyle;
	for(int idx=0;idx<2;idx++){
		switch(idx){
		case 0:
			if( (infile  = fopen( SKYMAP , "r" )) == NULL )
			{
				 AfxMessageBox( "The input file was not opened\n" );
				 return GZ_FAILURE;
			}
			//skybox=true;
			//interpStyle = GZ_SKYBOX;
			skybox=false;
			interpStyle = GZ_SKYBOX;
			nameListShader[0]  = GZ_INTERPOLATE;
			valueListShader[0] = (GzPointer)&interpStyle;
			nameListShader[1]  = GZ_TEXTURE_MAP;
			valueListShader[1] = (GzPointer)tex_fun;
			for(int i=0;i<AAKERNEL_SIZE;i++){
				GzPutAttribute(m_pRender[i], 2, nameListShader, valueListShader);
				GzPushMatrix(m_pRender[i], scale_cube);
			}
			break;
		case 1:
			if( (infile  = fopen( INFILE , "r" )) == NULL )
			{
				 AfxMessageBox( "The input file was not opened\n" );
				 return GZ_FAILURE;
			}
			skybox=false;
			interpStyle = GZ_NORMALS;
			nameListShader[0]  = GZ_INTERPOLATE;
			valueListShader[0] = (GzPointer)&interpStyle;
			nameListShader[1]  = GZ_TEXTURE_MAP;
			valueListShader[1] = (GzPointer)0;
			for(int i=0;i<AAKERNEL_SIZE;i++){
				GzPutAttribute(m_pRender[i], 2, nameListShader, valueListShader);
				GzPushMatrix(m_pRender[i], scale_obj); 
				GzPushMatrix(m_pRender[i], rotateY_obj);
				GzPushMatrix(m_pRender[i], rotateZ_obj);
			}
			break;
		}
		while( fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
			LoadFile(infile,vertexList,normalList,uvList,&face,skybox);
			/* 
			 * Set the value pointers to the first vertex of the 	
			 * triangle, then feed it to the renderer 
			 * NOTE: this sequence matches the nameList token sequence
			 */ 
			 valueListTriangle[0] = (GzPointer)vertexList; 
			 valueListTriangle[1] = (GzPointer)normalList; 
			 valueListTriangle[2] = (GzPointer)uvList; 
			 if(skybox){
				valueListTriangle[3] = (GzPointer)&face; 
			}
			for(int i=0;i<AAKERNEL_SIZE;i++)
				 GzPutTriangle(m_pRender[i], (skybox)?4:3, nameListTriangle, valueListTriangle); 
		}
		for(int i=0;i<AAKERNEL_SIZE;i++){
			//if(idx==1){
				//GzPopMatrix(m_pRender[i]);
				//GzPopMatrix(m_pRender[i]);
			//}
			GzPopMatrix(m_pRender[i]);
		}
		if( fclose( infile ) )
			AfxMessageBox( "The input file was not closed\n" );
	}


	for(int j=0;j<m_pDisplay[0]->xres*m_pDisplay[0]->yres;j++){
		m_pDisplay[0]->fbuf[j].red *= AAFilter[0][2];
		m_pDisplay[0]->fbuf[j].green *= AAFilter[0][2];
		m_pDisplay[0]->fbuf[j].blue *= AAFilter[0][2];
		m_pDisplay[0]->fbuf[j].alpha *= AAFilter[0][2];
		for(int i=1;i<AAKERNEL_SIZE;i++){
			m_pDisplay[0]->fbuf[j].red += m_pDisplay[i]->fbuf[j].red*AAFilter[i][2];
			m_pDisplay[0]->fbuf[j].green += m_pDisplay[i]->fbuf[j].green*AAFilter[i][2];
			m_pDisplay[0]->fbuf[j].blue += m_pDisplay[i]->fbuf[j].blue*AAFilter[i][2];
			m_pDisplay[0]->fbuf[j].alpha += m_pDisplay[i]->fbuf[j].alpha*AAFilter[i][2];
		}
	}

	GzFlushDisplay2File(outfile, m_pDisplay[0]); 	/* write out or update display to file*/
	GzFlushDisplay2FrameBuffer(m_pFrameBuffer, m_pDisplay[0]);	// write out or update display to frame buffer
	//}
	/* 
	 * Close file
	 */ 

	if( fclose( outfile ) )
      AfxMessageBox( "The output file was not closed\n" );
 
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application5::Clean()
{
	/* 
	 * Clean up and exit 
	 */ 
	int	status = 0; 
	for(int i=0;i<AAKERNEL_SIZE;i++){
		status |= GzFreeRender(m_pRender[i]); 
		status |= GzFreeDisplay(m_pDisplay[i]);
	}
	status |= GzFreeTexture();
	
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS);
}


void Application5::LoadFile(FILE * infile, GzCoord *vertexList, GzCoord *normalList, GzTextureIndex *uvList, int *face, bool skybox){

	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[0][0]), &(vertexList[0][1]),  
		&(vertexList[0][2]), 
		&(normalList[0][0]), &(normalList[0][1]), 	
		&(normalList[0][2]), 
		&(uvList[0][0]), &(uvList[0][1]) ); 
		if(skybox){
			fscanf(infile,"%d",face);
		}
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[1][0]), &(vertexList[1][1]), 	
		&(vertexList[1][2]), 
		&(normalList[1][0]), &(normalList[1][1]), 	
		&(normalList[1][2]), 
		&(uvList[1][0]), &(uvList[1][1]) ); 
		if(skybox){
			fscanf(infile,"%d",face);
		}
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[2][0]), &(vertexList[2][1]), 	
		&(vertexList[2][2]), 
		&(normalList[2][0]), &(normalList[2][1]), 	
		&(normalList[2][2]), 
		&(uvList[2][0]), &(uvList[2][1]) ); 
		if(skybox){
			fscanf(infile,"%d",face);
		}
}
