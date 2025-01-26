// clothe.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <GL/GL.h>

int done;
HDC hDC;
//|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|  
LRESULT CALLBACK WndProc(HWND	hWnd, UINT	uMsg,WPARAM	wParam, LPARAM	lParam)	
//=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|  
{
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







typedef struct
{
float	vert[4];
float   *linked[8];
float   OrigLen[8];
int		nlink;
}springed_shape;



springed_shape	Shape[32][32];
float			verts[32][32][4];
float			rands[32][32][4];
/*
vertice  vert,vert2;
vertice  cloth_springs[32];
vertice  cloth_springs_t[32];

*/

float fac=0.01;

void cross(float *out,float *v1,float *v2)
{
	out[0]=v1[1]*v2[2]-v2[1]*v1[2];
	out[1]=v1[0]*v2[2]-v2[0]*v1[2];
	out[2]=v1[0]*v2[1]-v2[0]*v1[1];

}

void calc_spring(springed_shape *shape,float Gravity,float *out_vert)
{
	int		i;
	float	m1,ext1,xv,yv,zv;
	float	vert[4];

	xv=0;
	yv=Gravity;
	zv=0;

	for(i=0;i<shape->nlink;i++)
	{
		vert[0]		= shape->linked[i][0] - shape->vert[0];
		vert[1]		= shape->linked[i][1] - shape->vert[1];
		vert[2]		= shape->linked[i][2] - shape->vert[2];
		
		m1			= sqrtf(vert[0]*vert[0]+vert[1]*vert[1]+vert[2]*vert[2]);
		ext1		= m1 - shape->OrigLen[i];

		xv+=vert[0]/m1 * ext1;
		yv+=vert[1]/m1 * ext1;
		zv+=vert[2]/m1 * ext1;
	}

	out_vert[0] = shape->vert[0] + (xv * fac);
	out_vert[1] = shape->vert[1] + (yv * fac);
	out_vert[2] = shape->vert[2] + (zv * fac);


	


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
	unsigned int	i,j;
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
	glFrustum		(-1,1,-1,1,0.1,100);

	glMatrixMode	(GL_MODELVIEW);
	glLoadIdentity	();

	glBegin(GL_QUADS);

	for(j=0;j<32;j++)
	{
		for(i=0;i<32;i++)
		{
			glLoadName((j<<16)|(i));

			glVertex3fv	(verts[j][i]);
			glVertex3fv	(verts[j][i+1]);
			glVertex3fv	(verts[j+1][i+1]);
			glVertex3fv	(verts[j+1][i]);
		}
	}
		
	glEnd();


	hits= glRenderMode(GL_RENDER);

	glMatrixMode	(GL_PROJECTION);
	glPopMatrix		();

	if(hits<=0)
		return 0xFFFFFFFF;
	else
		return selectBuf[3];



}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	float Gravity;
	float tt;
	float light_pos[4]= {0.7,-0.3,0.6,0};
	float col1[4]={1,1,1,1};
	float col2[4]={1,1,1,1};

	unsigned int			i,j;
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


	srand(time(0));

	for(j=0;j<32;j++)
	{
		for(i=0;i<32;i++)
		{
			rands[j][i][0]=((float)(rand()%1000))/1000.0f;
			rands[j][i][1]=((float)(rand()%1000))/1000.0f;
			rands[j][i][2]=((float)(rand()%1000))/1000.0f;
		}
	}
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

	add_link(&Shape[31][0],Shape[31][1].vert);
	add_link(&Shape[31][0],Shape[30][0].vert);
	
	for(i=1;i<31;i++)
	{
		add_link(&Shape[0][i],Shape[0][i-1].vert);
		add_link(&Shape[0][i],Shape[0][i+1].vert);
		add_link(&Shape[0][i],Shape[1][i].vert);


		add_link(&Shape[31][i],Shape[31][i-1].vert);
		add_link(&Shape[31][i],Shape[31][i+1].vert);
		add_link(&Shape[31][i],Shape[30][i].vert);

	
		add_link(&Shape[i][0],Shape[i-1][0].vert);
		add_link(&Shape[i][0],Shape[i+1][0].vert);
		add_link(&Shape[i][0],Shape[i][1].vert);


		add_link(&Shape[i][31],Shape[i-1][31].vert);
		add_link(&Shape[i][31],Shape[i+1][31].vert);
		add_link(&Shape[i][31],Shape[i][30].vert);
	
	}

	for(j=1;j<31;j++)
	{
		for(i=1;i<31;i++)
		{
			add_link(&Shape[j][i],Shape[j][i-1].vert);
			add_link(&Shape[j][i],Shape[j][i+1].vert);
			add_link(&Shape[j][i],Shape[j+1][i].vert);
			add_link(&Shape[j][i],Shape[j-1][i].vert);
		}
	}



	tt=0;
	Gravity=-0.05;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_POSITION,light_pos);


	
	while(!done)
	{

		Shape[31][31].vert[0]=16;
		Shape[31][31].vert[1]=16;
		Shape[31][31].vert[2]=0;
		

		Shape[31][0].vert[0]=-16;
		Shape[31][0].vert[1]=16;
		Shape[31][0].vert[2]=0;
		
		tt=tt+0.005;


		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				calc_spring(&Shape[j][i],Gravity,verts[j][i]);

			}
		}

		for(j=0;j<32;j++)
		{
			for(i=0;i<32;i++)
			{
				memcpy(Shape[j][i].vert,verts[j][i],sizeof(float)*4);		
			}
		}
//		unsigned int hit,tot;
//		POINT Cursor;
		//GetCursorPos(&Cursor);
		//hit=SelectMouse(Cursor.x,Cursor.y);

		glClear			(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glMatrixMode	(GL_PROJECTION);
		glLoadIdentity	();
		glFrustum		(-1,1,-1,1,0.1,100);

		glMatrixMode	(GL_MODELVIEW);
		glLoadIdentity	();
		glTranslatef	(0,0,-3);

		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,col2);
		
		glBegin(GL_QUADS);
		
		for(j=0;j<31;j++)
		{
			for(i=0;i<31;i++)
			{

				float v1[3];
				float v2[3];
				float norm[3];
/*
				tot=(j<<16)|i;
				if(tot==hit)
				{
					glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,col1);
				}
				else
				{
					glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,col2);
				}
*/		
				glColor4f(1,1,1,1);
				v1[0]=verts[j][i+1][0]-verts[j][i][0];
				v1[1]=verts[j][i+1][1]-verts[j][i][1];
				v1[2]=verts[j][i+1][2]-verts[j][i][2];

				v2[0]=verts[j+1][i][0]-verts[j][i][0];
				v2[1]=verts[j+1][i][1]-verts[j][i][1];
				v2[2]=verts[j+1][i][2]-verts[j][i][2];

				cross		(norm,v1,v2);
				
				glNormal3fv	(norm);

				glVertex3fv	(verts[j][i]);
				glVertex3fv	(verts[j][i+1]);
				glVertex3fv	(verts[j+1][i+1]);
				glVertex3fv	(verts[j+1][i]);
			}
		}
		
		glEnd();

		SleepEx(100,1);


		SwapBuffers(hDC);
		if(PeekMessage(&Msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

	}
	
	
	return 0;
}



