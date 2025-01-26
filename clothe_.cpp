// clothe.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <GL/GL.h>
#include	<ocidl.h>
#include	<stdio.h>
#include	<direct.h>


int done,SelectOn;
	float Gravity;
HDC hDC;
unsigned int SelectX,SelectY;


typedef struct
{
float	vert[4];
float   *linked[16];
float   OrigLen[16];
int		nlink;
int		fixed;
}springed_shape;



springed_shape	Shape[32][32];

float			verts [32*32][4];
float			untest[32*32][4];
float			temp[32*32][4];

float			vnorms[32*32][4];
unsigned int	tri_index[32*32*2][3];

int				intersec[32][32];
int				tricol[32*32*2];

float			move[32][32][4];
//float			norms[32][32][4];




//|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|  
LRESULT CALLBACK WndProc(HWND	hWnd, UINT	uMsg,WPARAM	wParam, LPARAM	lParam)	
//=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|  
{



	if(uMsg==WM_LBUTTONDOWN)
	{
		SelectOn=1;
	}
	if(uMsg==WM_LBUTTONUP)
	{
		SelectOn=0;
		Shape[SelectY][SelectX].vert[2]=0;
	}

	if(uMsg==WM_DESTROY)done=1;
	if(uMsg==WM_KEYDOWN)done=1;
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
static	PIXELFORMATDESCRIPTOR pfd=			// pfd Tells Windows How We Want Things To Be
{
sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
1,											// Version Number
PFD_DRAW_TO_WINDOW |						// Format Must Support Window
PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
PFD_DOUBLEBUFFER,							// Must Support Double Buffering
PFD_TYPE_RGBA,								// Request An RGBA Format
24,											// Select Our Color Depth
0, 0, 0, 0, 0, 0,							// Color Bits Ignored
0,											// No Alpha Buffer
0,											// Shift Bit Ignored
0,											// No Accumulation Buffer
0, 0, 0, 0,									// Accumulation Bits Ignored
16,											// 16Bit Z-Buffer (Depth Buffer)  
0,											// No Stencil Buffer
0,											// No Auxiliary Buffer
PFD_MAIN_PLANE,								// Main Drawing Layer
0,											// Reserved
0, 0, 0										// Layer Masks Ignored
};






/*
vertice  vert,vert2;
vertice  cloth_springs[32];
vertice  cloth_springs_t[32];

*/

float fac=0.1;

void calc_spring(springed_shape *shape,float *vv,float Gravity,float *out_vert)
{
	int		i;
	float	m1,ext1,xv,yv,zv;
	float	vert[4];

	xv=0;
	yv=Gravity;
	zv=0;

	for(i=0;i<shape->nlink;i++)
	{
		vert[0]		=	shape->linked[i][0] - vv[0];
		vert[1]		=	shape->linked[i][1] - vv[1];
		vert[2]		=	shape->linked[i][2] - vv[2];
		
		m1			=	sqrtf(vert[0]*vert[0]+vert[1]*vert[1]+vert[2]*vert[2]);
		ext1		=	m1 - shape->OrigLen[i];

		xv			+=	vert[0]/m1 * ext1;
		yv			+=	vert[1]/m1 * ext1;
		zv			+=	vert[2]/m1 * ext1;
	}

	/*
	out_vert[0] = shape->vert[0] + (xv * fac);
	out_vert[1] = shape->vert[1] + (yv * fac);
	out_vert[2] = shape->vert[2] + (zv * fac);
	*/
	out_vert[0]=(xv * fac);
	out_vert[1]=(yv * fac);
	out_vert[2]=(zv * fac);
}

void cross(float *out,float *v1,float *v2)
{
	out[0]=v1[1]*v2[2]-v2[1]*v1[2];
	out[1]=v1[0]*v2[2]-v2[0]*v1[2];
	out[2]=v1[0]*v2[1]-v2[0]*v1[1];

}

void add_link   (springed_shape *shape,float *link)
{
	shape->linked[shape->nlink]		=link;
	shape->OrigLen[shape->nlink]	=sqrtf(	(shape->vert[0]-link[0])*(shape->vert[0]-link[0])+
											(shape->vert[1]-link[1])*(shape->vert[1]-link[1])+
											(shape->vert[2]-link[2])*(shape->vert[2]-link[2]));
	shape->nlink++;
}


//=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|  
void gluPickMatrix(GLdouble x, GLdouble y,GLdouble width, GLdouble height, GLint viewport[4])
//=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|  
{
   GLfloat m[16];
   GLfloat sx, sy;
   GLfloat tx, ty;

   sx = viewport[2] / width;
   sy = viewport[3] / height;
   tx = (viewport[2] + 2.0 * (viewport[0] - x)) / width;
   ty = (viewport[3] + 2.0 * (viewport[1] - y)) / height;

#define M(row,col)  m[col*4+row]
   M(0, 0) = sx;
   M(0, 1) = 0.0;
   M(0, 2) = 0.0;
   M(0, 3) = tx;
   M(1, 0) = 0.0;
   M(1, 1) = sy;
   M(1, 2) = 0.0;
   M(1, 3) = ty;
   M(2, 0) = 0.0;
   M(2, 1) = 0.0;
   M(2, 2) = 1.0;
   M(2, 3) = 0.0;
   M(3, 0) = 0.0;
   M(3, 1) = 0.0;
   M(3, 2) = 0.0;
   M(3, 3) = 1.0;
#undef M

   glMultMatrixf(m);
}

unsigned int SelectMouse(unsigned int x,unsigned int y)
{
	unsigned int	selectBuf[10000];
	int				hits;
	unsigned int	i,j,n;
	int				vp[4];
	unsigned int	ret;

	glGetIntegerv( GL_VIEWPORT,vp);

	glSelectBuffer	(100, selectBuf);
	glRenderMode	(GL_SELECT);
	glInitNames		();
	glPushName		(~0);

	glMatrixMode	(GL_PROJECTION);
	glPushMatrix	();
	
	glLoadIdentity	();
	gluPickMatrix	(x, vp[3]-y, 4, 4, vp);
	glFrustum		(-0.1,0.1,-0.1,0.1,0.1,100);

	glMatrixMode	(GL_MODELVIEW);
	glLoadIdentity	();
	glTranslatef(0,0,-30);

	int ntri=0;


		
	for(j=0;j<31;j++)
	{
		for(i=0;i<31;i++)
		{
			int ind1,ind2,ind3;

			ind1=tri_index[ntri][0];
			ind2=tri_index[ntri][1];
			ind3=tri_index[ntri][2];
			ntri++;

			n=(j<<16)|(i);
			glLoadName(n);

			glBegin(GL_TRIANGLES);

			glVertex3fv	(verts[ind1]);
			glVertex3fv	(verts[ind2]);
			glVertex3fv	(verts[ind3]);

			ind1=tri_index[ntri][0];
			ind2=tri_index[ntri][1];
			ind3=tri_index[ntri][2];
			ntri++;

			glVertex3fv	(verts[ind1]);
			glVertex3fv	(verts[ind2]);
			glVertex3fv	(verts[ind3]);

			glEnd();
		}
	}
		

/*

	for(j=0;j<31;j++)
	{
		for(i=0;i<31;i++)
		{
			n=(j<<16)|(i);
			glLoadName(n);
			
			glBegin(GL_QUADS);

			glVertex3fv	(verts[j][i]);
			glVertex3fv	(verts[j][i+1]);
			glVertex3fv	(verts[j+1][i+1]);
			glVertex3fv	(verts[j+1][i]);
			glEnd();
		}
	}
*/
	


	hits= glRenderMode(GL_RENDER);

	glMatrixMode	(GL_PROJECTION);
	glPopMatrix		();

	if(hits<=0)
		return 0xFFFFFFFF;
	else
		return selectBuf[3];

}

typedef struct
{
int				nWidth;
int				nHeight;
unsigned char	*pixelPtr;					//adresse de base de l'image
}OleImg;
#define WINOLECTLAPI        EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE

const UUID IID_IPicture= {0x7BF80980,0xBF32,0x101A,{0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB}};
WINOLECTLAPI OleLoadPicture(LPSTREAM lpstream, LONG lSize, BOOL fRunmode,REFIID riid, LPVOID FAR* lplpvObj);

#define	HIMETRIC_INCH	2540
#define	RES_OK			true 
#define	RES_FAIL		false

unsigned int LoadTex(char *File,OleImg *Img)
{

	unsigned int	glid;
	BITMAPINFO		 bmi;
	BITMAPINFOHEADER *bmih=&bmi.bmiHeader;
	HGLOBAL 		hGlobal;
	LPSTREAM 		pictureStreamPtr;
	LPPICTURE 		picture;
	long 			pictureWidth ,  pictureHeight ;
	HRESULT 		hr;
	FILE			*s;
	char			*Data,*dataPtr;
	int				DataLen;
	unsigned char	*TpixelPtr;
	HBITMAP			memBitmap , oldBitmap ;		//bitmap memoire qui va nous servir de buffer
	HDC				memDC	;					//	HDC memoire
	BITMAP bmp;

	s=fopen(File,"rb");
	fseek(s,0,SEEK_END);
	DataLen=ftell(s);
	Data=(char *)calloc(DataLen+1,1);
	rewind(s);
	fread(Data,DataLen,1,s);
	fclose(s);

	memset( &bmi , 0  , sizeof(BITMAPINFO) ) ;
	
	hGlobal					= GlobalAlloc(GMEM_MOVEABLE, DataLen);
	dataPtr					= (char*) GlobalLock(hGlobal);	//our data points to the buffer prepared to receive the file
	memcpy					(dataPtr,Data,DataLen);
	
	hr						= CreateStreamOnHGlobal	(hGlobal, TRUE, &pictureStreamPtr);
	hr						= OleLoadPicture		(pictureStreamPtr, DataLen, FALSE, IID_IPicture , (LPVOID *)&picture);

	memDC				= CreateCompatibleDC	(NULL) ;		//on cree un DC compatible bidon

	picture->get_Width (&pictureWidth);
	picture->get_Height(&pictureHeight);
	
	// convert himetric to pixels
	Img->nWidth				= MulDiv(	pictureWidth	, GetDeviceCaps(memDC, LOGPIXELSX), HIMETRIC_INCH) ;
	Img->nHeight			= MulDiv(	pictureHeight	, GetDeviceCaps(memDC, LOGPIXELSY), HIMETRIC_INCH) ;
	
	bmih->biSize			=	sizeof (BITMAPINFOHEADER ) ;
	bmih->biWidth			=	Img->nWidth ;
	bmih->biHeight			=	Img->nHeight ;
	bmih->biPlanes			=	1 ;
	bmih->biBitCount		=	32 ;  //16 = 16 bits  24 = 24 bits et 32 = 32 bits
	bmih->biSizeImage		=	0	;
	bmih->biClrUsed			=	0 ;
	bmih->biClrImportant	=	0 ;
	bmih->biCompression		=	BI_RGB ;
	bmih->biXPelsPerMeter	=	0 ;
	bmih->biYPelsPerMeter	=	0 ;
		
	memBitmap			=	CreateDIBSection			(memDC ,&bmi ,DIB_RGB_COLORS , (void **)&TpixelPtr ,NULL ,0) ;
	oldBitmap			=	( HBITMAP)SelectObject		(memDC ,memBitmap ) ;	// selectionne la bitmap sur le hdc ( important !!)

	picture->Render		(memDC,0,0,Img->nWidth	,Img->nHeight,0,pictureHeight,pictureWidth	,-pictureHeight,NULL  )	;

	glGenTextures(1,&glid);

	for(int i=0;i<Img->nWidth*Img->nHeight*4;i+=4) 
	{
		char c;
		c=TpixelPtr[i];
		TpixelPtr[i]=TpixelPtr[i+2];
		TpixelPtr[i+2]=c;
	}

	glBindTexture (GL_TEXTURE_2D, glid);
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, Img->nWidth	,Img->nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, TpixelPtr);



	return glid;
}


int intersect_RayTriangle( float *P0,float *dir, float *T[3], float* out )
{
    float     u[4] , v[4],n[4];             // triangle vectors
    float     w0[4], w[4];          // ray vectors
    float     r, a, b;             // params to calc ray-plane intersect

/*
    u = T.V1 - T.V0;
    v = T.V2 - T.V0;
    n = u * v;             // cross product
*/

	u[0]=T[1][0]-T[0][0];
	u[1]=T[1][1]-T[0][1];
	u[2]=T[1][2]-T[0][2];

	v[0]=T[2][0]-T[0][0];
	v[1]=T[2][1]-T[0][1];
	v[2]=T[2][2]-T[0][2];
	cross(n,u,v);

	/*
    if ((n[0] == 0)&            // triangle is degenerate
		(n[1] == 0)&
		(n[2] == 0))
        return -1;                 // do not deal with this case
    */

//    w0 = R.P0 - T.V0;
    
	w0[0] = P0[0] - T[0][0];
	w0[1] = P0[1] - T[0][1];
	w0[2] = P0[2] - T[0][2];

	//a = -dot(n,w0);

	a=-(n[0]*w0[0]+n[1]*w0[1]+n[2]*w0[2]);

	//b = dot(n,dir);
	b=(n[0]*dir[0]+n[1]*dir[1]+n[2]*dir[2]);

    if (fabs(b) < 0.1) {     // ray is parallel to triangle plane
        
		if (a == 0)                // ray lies in triangle plane
            return 2;
        else 
			return 0;             // ray disjoint from plane
    }
	
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect

    out[0] = P0[0] + r * dir[0];           // intersect point of ray and plane
	out[1] = P0[1] + r * dir[1];           // intersect point of ray and plane
	out[2] = P0[2] + r * dir[2];           // intersect point of ray and plane

    float    uu, uv, vv, wu, wv, D;
    uu = (u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
    uv = (u[0]*v[0]+u[1]*v[1]+u[2]*v[2]);
    vv = (v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    
	w[0] = out[0] - T[0][0];
	w[1] = out[1] - T[0][1];
	w[2] = out[2] - T[0][2];

    wu = (w[0]*u[0]+w[1]*u[1]+w[2]*u[2]);
    wv = (w[0]*v[0]+w[1]*v[1]+w[2]*v[2]);
    
	D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
}

/*
int intersect_RayTriangle( Ray R, Triangle T, Point* I )
{
    Vector    u, v, n;             // triangle vectors
    Vector    dir, w0, w;          // ray vectors
    float     r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = T.V1 - T.V0;
    v = T.V2 - T.V0;
    n = u * v;             // cross product
    if (n == (Vector)0)            // triangle is degenerate
        return -1;                 // do not deal with this case

    dir = R.P1 - R.P0;             // ray direction vector
    w0 = R.P0 - T.V0;
    a = -dot(n,w0);
    b = dot(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
        if (a == 0)                // ray lies in triangle plane
            return 2;
        else return 0;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect

    *I = R.P0 + r * dir;           // intersect point of ray and plane

    // is I inside T?
    float    uu, uv, vv, wu, wv, D;
    uu = dot(u,u);
    uv = dot(u,v);
    vv = dot(v,v);
    w = *I - T.V0;
    wu = dot(w,u);
    wv = dot(w,v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
}
*/
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	float tt;
	float light_pos[4] = {0.7  , 0.3  , 0.8  ,0};
	float light_pos2[4]= {0.0  ,-0.6f ,-0.8  ,0};
	
	float col1[4]={1,0,0,1};
	float col2[4]={1,1,1,1};
	float col3[4]={1,0,1,1};
	POINT			Cursor;
	unsigned int	hit,tot;
	OleImg			MyImg;


	int			i,j,n;
	HFONT		font;
	WNDCLASS	gwc;
	MSG			Msg;

	gwc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window->
	gwc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	gwc.cbClsExtra		= 0;									// No Extra Window Data
	gwc.cbWndExtra		= 0;									// No Extra Window Data
	gwc.hInstance		= GetModuleHandle(NULL);				// Set The Instance
	gwc.hIcon			= NULL;		// Load The Default Icon
	gwc.hCursor			= LoadCursor(NULL,IDC_ARROW);			// Load The Arrow Pointer
	gwc.hbrBackground	= NULL;//NULL;							// No Background Required For GL
	gwc.lpszMenuName	= NULL;									// We Don't Want A Menu
	gwc.lpszClassName	= "OpenGL";								// Set The Class Name

	RegisterClass		(&gwc);	
	HWND GlWin			= CreateWindowEx(0,"OpenGL","",WS_CLIPSIBLINGS |WS_CLIPCHILDREN|WS_POPUP,0, 0,1024,768,NULL,NULL,gwc.hInstance,NULL);
	ShowWindow(GlWin,1);

	hDC				=	GetDC				(GlWin);
	int PixelFormat		=	ChoosePixelFormat	(hDC,&pfd);
	SetPixelFormat								(hDC,PixelFormat,&pfd);
	HGLRC hRC			=	wglCreateContext	(hDC);
	wglMakeCurrent								(hDC,hRC);
	glViewport		(0,0,1024,768);	

	glMatrixMode	(GL_PROJECTION);
	glLoadIdentity	();
	glFrustum		(-0.1,0.1,-0.1,0.1,0.1,100);

	srand(time(0));

	memset(Shape,0,32*sizeof(springed_shape));

	for(j=0;j<32;j++)
	{
		for(i=0;i<32;i++)
		{
			Shape[j][i].vert[0]	=i-16;
			Shape[j][i].vert[1]	=j-16;
			Shape[j][i].vert[2]	=0;
		}
	}


	add_link(&Shape[0][0],Shape[0][1].vert);
	add_link(&Shape[0][0],Shape[1][0].vert);
	add_link(&Shape[0][0],Shape[1][1].vert);
	
	add_link(&Shape[0][0],Shape[0][2].vert);
	add_link(&Shape[0][0],Shape[2][0].vert);


	add_link(&Shape[31][0],Shape[31][1].vert);
	add_link(&Shape[31][0],Shape[30][0].vert);
	add_link(&Shape[31][0],Shape[30][1].vert);

	add_link(&Shape[31][0],Shape[29][0].vert);
	add_link(&Shape[31][0],Shape[31][2].vert);
	
	
	
	add_link(&Shape[0][31],Shape[1][31].vert);
	add_link(&Shape[0][31],Shape[0][30].vert);
	add_link(&Shape[0][31],Shape[1][30].vert);

	add_link(&Shape[0][31],Shape[0][29].vert);
	add_link(&Shape[0][31],Shape[2][31].vert);


	add_link(&Shape[31][31],Shape[31][30].vert);
	add_link(&Shape[31][31],Shape[30][30].vert);
	add_link(&Shape[31][31],Shape[30][31].vert);

	add_link(&Shape[31][31],Shape[29][31].vert);
	add_link(&Shape[31][31],Shape[31][29].vert);

	
	for(i=1;i<31;i++)
	{
		add_link(&Shape[0][i],Shape[0][i-1].vert);
		add_link(&Shape[0][i],Shape[0][i+1].vert);
		add_link(&Shape[0][i],Shape[1][i].vert);

		add_link(&Shape[0][i],Shape[1][i+1].vert);
		add_link(&Shape[0][i],Shape[1][i-1].vert);

		
		add_link(&Shape[0][i],Shape[2][i].vert);
		if(i<30)add_link(&Shape[0][i],Shape[0][i+2].vert);
		if(i>1)	add_link(&Shape[0][i],Shape[0][i-2].vert);
		

		
		add_link(&Shape[31][i],Shape[31][i-1].vert);
		add_link(&Shape[31][i],Shape[31][i+1].vert);
		add_link(&Shape[31][i],Shape[30][i].vert);
		
		add_link(&Shape[31][i],Shape[30][i-1].vert);
		add_link(&Shape[31][i],Shape[30][i+1].vert);

		add_link(&Shape[31][i],Shape[29][i].vert);
		if(i<30)add_link(&Shape[31][i],Shape[31][i+2].vert);
		if(i>1)	add_link(&Shape[31][i],Shape[31][i-2].vert);

	
		add_link(&Shape[i][0],Shape[i-1][0].vert);
		add_link(&Shape[i][0],Shape[i+1][0].vert);
		add_link(&Shape[i][0],Shape[i][1].vert);

		add_link(&Shape[i][0],Shape[i-1][1].vert);
		add_link(&Shape[i][0],Shape[i+1][1].vert);

		add_link(&Shape[i][0],Shape[i][2].vert);
		if(i<30)add_link(&Shape[i][0],Shape[i+2][0].vert);
		if(i>1)	add_link(&Shape[i][0],Shape[i-2][0].vert);


		add_link(&Shape[i][31],Shape[i-1][31].vert);
		add_link(&Shape[i][31],Shape[i+1][31].vert);
		add_link(&Shape[i][31],Shape[i][30].vert);

		add_link(&Shape[i][31],Shape[i-1][30].vert);
		add_link(&Shape[i][31],Shape[i+1][30].vert);

		add_link(&Shape[i][31],Shape[i][29].vert);
		if(i<30)add_link(&Shape[i][31],Shape[i+2][31].vert);
		if(i>1)	add_link(&Shape[i][31],Shape[i-2][31].vert);

	
	}

	for(j=1;j<31;j++)
	{
		for(i=1;i<31;i++)
		{
			add_link(&Shape[j][i],Shape[j][i-1].vert);
			add_link(&Shape[j][i],Shape[j][i+1].vert);
			add_link(&Shape[j][i],Shape[j+1][i].vert);
			add_link(&Shape[j][i],Shape[j-1][i].vert);


			add_link(&Shape[j][i],Shape[j+1][i-1].vert);
			add_link(&Shape[j][i],Shape[j+1][i+1].vert);

		
			add_link(&Shape[j][i],Shape[j-1][i-1].vert);
			add_link(&Shape[j][i],Shape[j-1][i+1].vert);

			if(i>1)
				add_link(&Shape[j][i],Shape[j][i-2].vert);
			
			if(i<30)
				add_link(&Shape[j][i],Shape[j][i+2].vert);
			
			if(j<30)
				add_link(&Shape[j][i],Shape[j+2][i].vert);
			

			if(j>1)
				add_link(&Shape[j][i],Shape[j-2][i].vert);

		}
	}



	tt=0;
	SelectOn=0;
	Gravity=-0.3;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);
	
	unsigned int glid=LoadTex("test.jpg",&MyImg);
	
	glEnable(GL_LIGHTING);
	
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_POSITION,light_pos);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,col2);

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1,GL_POSITION,light_pos2);

	glLightfv(GL_LIGHT1,GL_DIFFUSE,col2);

	int ntri=0,ntritotal=0;

	for(j=0;j<31;j++)
	{
		for(i=0;i<31;i++)
		{
			tri_index[ntritotal][0]=32*j+i;
			tri_index[ntritotal][1]=32*j+i+1;
			tri_index[ntritotal][2]=32*(j+1)+i+1;
			ntritotal++;

			tri_index[ntritotal][0]=32*j+i;
			tri_index[ntritotal][1]=32*(j+1)+i+1;
			tri_index[ntritotal][2]=32*(j+1)+i;
			ntritotal++;
		}
	}



	Shape[31][31].fixed=1;
	Shape[31][30].fixed=1;

	Shape[31][0].fixed=1;
	Shape[31][1].fixed=1;
		
	while(!done)
	{

		glClear			(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
/*
		Shape[31][31].fixed=1;
		Shape[31][31].vert[0]=16;
		Shape[31][31].vert[1]=16;
		Shape[31][31].vert[2]=0;
		
		Shape[31][30].fixed=1;
		Shape[31][30].vert[0]=15;
		Shape[31][30].vert[1]=16;
		Shape[31][30].vert[2]=0;

		Shape[31][30].fixed=1;
		Shape[31][0].vert[0]=-16;
		Shape[31][0].vert[1]=16;
		Shape[31][0].vert[2]=0;
	
		Shape[31][1].fixed=1;
		Shape[31][1].vert[0]=-15;
		Shape[31][1].vert[1]=16;
		Shape[31][1].vert[2]=0;
*/

		/*
		GetCursorPos(&Cursor);
		if((hit!=0xFFFFFFFF)&&(SelectOn))
		{
			Shape[SelectY][SelectX].vert[0]=(((float)(Cursor.x-512))/512.0f)*64.0f;
			Shape[SelectY][SelectX].vert[1]=(((float)((768-Cursor.y)-384))/384.0f)*64.0f;
			Shape[SelectY][SelectX].vert[2]=5;
		}
		*/
		
		tt=tt+0.005;

		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				if(!Shape[j][i].fixed)
					calc_spring(&Shape[j][i],Shape[j][i].vert,Gravity,move[j][i]);
				else
					memset(move[j][i],0,16);
			}
		}
	
		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				Shape[j][i].vert[0]=Shape[j][i].vert[0]+move[j][i][0];
				Shape[j][i].vert[1]=Shape[j][i].vert[1]+move[j][i][1];
				Shape[j][i].vert[2]=Shape[j][i].vert[2]+move[j][i][2];
			}
		}


		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				temp[32*j+i][0]=Shape[j][i].vert[0];
				temp[32*j+i][1]=Shape[j][i].vert[1];
				temp[32*j+i][2]=Shape[j][i].vert[2];

				untest[32*j+i][0]=Shape[j][i].vert[0];
				untest[32*j+i][1]=Shape[j][i].vert[1];
				untest[32*j+i][2]=Shape[j][i].vert[2];
			
			}
		}

		for(int ttt=0;ttt<2;ttt++)
		{
			for(j=0;j<32;j++)
			{
				for(i=0;i<32;i++)
				{
					if(!Shape[j][i].fixed)
					{
						calc_spring(&Shape[j][i],Shape[j][i].vert,0,move[j][i]);
						untest[32*j+i][0]+=move[j][i][0];
						untest[32*j+i][1]+=move[j][i][1];
						untest[32*j+i][2]+=move[j][i][2];
					}
				}
			}

			for(j=0;j<32;j++)
			{
				for(i=0;i<32;i++)
				{
					memcpy(Shape[j][i].vert,untest[32*j+i],16);
					tricol[(32*j+i)*2+0]=0;
					tricol[(32*j+i)*2+1]=0;
				}
			}
		}


		/*
		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				float tv[4];
				float tm[4];
				
				intersec[j][i]=0;
				memcpy(tv,temp[32*j+i]	,16);
				memcpy(tm,move[j][i]	,16);

				
				for(int ni=0;ni<ntritotal;ni++)
				{
					float *T[3];
					float out[4],didi;

					didi=0;
					T[0]=untest[tri_index[ni][0]];
					T[1]=untest[tri_index[ni][1]];
					T[2]=untest[tri_index[ni][2]];
					if(intersect_RayTriangle(tv,tm,T,out)==1)
					{
						intersec[j][i]	=1;
						tricol[ni]		=1;
						memcpy(Shape[j][i].vert,temp[32*j+i],16);
						break;
					}
				}
				if(intersec[j][i]==0)
					memcpy(Shape[j][i].vert,untest[32*j+i],16);
			}
		}
		
		*/

		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				memcpy(verts[32*j+i],Shape[j][i].vert,sizeof(float)*4);		
			}
		}
		
		/*

		if(SelectOn==0)
		{
			hit=SelectMouse(Cursor.x,Cursor.y);
			if(hit!=0xFFFFFFFF)
			{
				SelectX=hit&0xFFFF;
				SelectY=hit>>16;
			}
			else
			{
				SelectX=0;
				SelectY=0;
			}
		}
		*/
		memset(vnorms,0,sizeof(float)*4*32*32);

		for(i=0;i<ntritotal;i++)
		{
				float	v1[3];
				float	v2[3];
				float	norms[3];
				int		ind1,ind2,ind3;

				ind1=tri_index[i][0];
				ind2=tri_index[i][1];
				ind3=tri_index[i][2];


				v1[0]	=verts[ind2][0]-verts[ind1][0];
				v1[1]	=verts[ind2][1]-verts[ind1][1];
				v1[2]	=verts[ind2][2]-verts[ind1][2];

				v2[0]	=verts[ind3][0]-verts[ind1][0];
				v2[1]	=verts[ind3][1]-verts[ind1][1];
				v2[2]	=verts[ind3][2]-verts[ind1][2];

				cross		(norms,v1,v2);

				vnorms[ind1][0]	 +=norms[0];
				vnorms[ind1][1]	 +=norms[1];
				vnorms[ind1][2]	 +=norms[2];

				vnorms[ind2][0]  +=norms[0];
				vnorms[ind2][1]  +=norms[1];
				vnorms[ind2][2]  +=norms[2];

				vnorms[ind3][0]  +=norms[0];
				vnorms[ind3][1]  +=norms[1];
				vnorms[ind3][2]  +=norms[2];
		}

		/*
		for(j=0;j<31;j++)
		{
			for(i=0;i<31;i++)
			{
				float v1[3];
				float v2[3];
				float norms[3];

				v1[0]	=verts[32*j+i+1][0]-verts[32*j+i][0];
				v1[1]	=verts[32*j+i+1][1]-verts[32*j+i][1];
				v1[2]	=verts[32*j+i+1][2]-verts[32*j+i][2];

				v2[0]	=verts[32*(j+1)+i][0]-verts[32*j+i][0];
				v2[1]	=verts[32*(j+1)+i][1]-verts[32*j+i][1];
				v2[2]	=verts[32*(j+1)+i][2]-verts[32*j+i][2];


				cross		(norms,v1,v2);

				vnorms[32*j+i][0]	 +=norms[0];
				vnorms[32*j+i][1]	 +=norms[1];
				vnorms[32*j+i][2]	 +=norms[2];

				vnorms[32*j+i+1][0]  +=norms[0];
				vnorms[32*j+i+1][1]  +=norms[1];
				vnorms[32*j+i+1][2]  +=norms[2];

				vnorms[32*(j+1)+i][0]+=norms[0];
				vnorms[32*(j+1)+i][1]+=norms[1];
				vnorms[32*(j+1)+i][2]+=norms[2];

			}
		}
		*/

		glClear			(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glMatrixMode	(GL_MODELVIEW);
		glLoadIdentity	();

		glEnable(GL_NORMALIZE);

		glTranslatef(0,0,-30);
		
		ntri=0;

		glBegin(GL_TRIANGLES);
		
		for(j=0;j<31;j++)
		{
			for(i=0;i<31;i++)
			{
				int ind1,ind2,ind3;
/*
				if(tricol[ntri]!=0)
				{
					glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,col3);
				}
				else if((i==SelectX)&&(j==SelectY))
				{
					glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,col1);
				}
				else
				{
					glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,col2);
				}
*/
				ind1=tri_index[ntri][0];
				ind2=tri_index[ntri][1];
				ind3=tri_index[ntri][2];
				ntri++;


				glNormal3fv	(vnorms[ind1]);
				glTexCoord2f((float)(i)/31.0f,(float)(j)/31.0f);
				glVertex3fv	(verts[ind1]);
				
				glNormal3fv	(vnorms[ind2]);
				glTexCoord2f((float)(i+1)/31.0f,(float)(j)/31.0f);
				glVertex3fv	(verts[ind2]);

				glNormal3fv	(vnorms[ind3]);
				glTexCoord2f((float)(i+1)/31.0f,(float)(j+1)/31.0f);
				glVertex3fv	(verts[ind3]);

				if(tricol[ntri]!=0)
				{
					glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,col3);
				}

				ind1=tri_index[ntri][0];
				ind2=tri_index[ntri][1];
				ind3=tri_index[ntri][2];
				ntri++;


				glNormal3fv	(vnorms[ind1]);
				glTexCoord2f((float)(i)/31.0f,(float)(j)/31.0f);
				glVertex3fv	(verts[ind1]);

				glNormal3fv	(vnorms[ind2]);
				glTexCoord2f((float)(i+1)/31.0f,(float)(j+1)/31.0f);
				glVertex3fv	(verts[ind2]);

				glNormal3fv	(vnorms[ind3]);
				glTexCoord2f((float)(i)/31.0f,(float)(j+1)/31.0f);
				glVertex3fv	(verts[ind3]);
			}
		}
		
		glEnd();

		//SleepEx(100,1);


		SwapBuffers(hDC);
		if(PeekMessage(&Msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

	}
	
	
	return 0;
}



		/*
		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				verts[32*j+i][0]=Shape[j][i].vert[0]+move[j][i][0];
				verts[32*j+i][1]=Shape[j][i].vert[1]+move[j][i][1];
				verts[32*j+i][2]=Shape[j][i].vert[2]+move[j][i][2];
			}
		}
			
		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				memcpy(Shape[j][i].vert,verts[32*j+i],sizeof(float)*4);		
			}
		}
		*/