
#include <stdlib.h>			//- for exit()
#include <stdio.h>			//- for sprintf()
#include <string.h>			//- for memset()

#ifdef _WIN32
	#include "libs/glut.h"
	#include <windows.h>
	#pragma comment(lib, "winmm.lib")			//- not required but have it in anyway
	#pragma comment(lib, "libs/glut32.lib")
#elif __APPLE__
	#include <GLUT/glut.h>
#elif __unix__		// all unices including  __linux__
	#include <GL/glut.h>
#endif

//====== Macros and Defines =========

#define FRAME_WIDE	1000
#define FRAME_HIGH	600
#define ROUND(x) ((int)(x+0.5))

//====== Structs & typedefs =========
typedef unsigned char BYTE;
struct POINT2D {
	int x, y;
	BYTE r, g, b;
};

//====== Global Variables ==========
BYTE	pFrameL[FRAME_WIDE * FRAME_HIGH * 3];
BYTE	pFrameR[FRAME_WIDE * FRAME_HIGH * 3];
int		shade = 0;
POINT2D	xypos = {0,0};
int		stereo = 0;
int		eyes = 10;

//===== Forward Declarations ========
void ClearScreen();
void DrawFrame();
void Interlace(BYTE* pL, BYTE* pR);
void PlaySoundEffect(char * filename);
void BuildFrame(BYTE *pFrame, int view);
void OnIdle(void);
void OnDisplay(void);
void reshape(int w, int h);
void OnMouse(int button, int state, int x, int y);
void OnKeypress(unsigned char key, int x, int y);

////////////////////////////////////////////////////////
// Program Entry Poinr
////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	//-- setup GLUT --
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	//GLUT_3_2_CORE_PROFILE |
	glutInitWindowSize(FRAME_WIDE, FRAME_HIGH);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
/*
#ifdef WIN32
	//- eliminate flashing --
	typedef void (APIENTRY * PFNWGLEXTSWAPCONTROLPROC) (int i);
	PFNWGLEXTSWAPCONTROLPROC wglSwapControl = NULL;
	wglSwapControl = (PFNWGLEXTSWAPCONTROLPROC) wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapControl != NULL) wglSwapControl(1); 
#endif
*/

	//--- set openGL state --
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//-- register call back functions --
	glutIdleFunc(OnIdle);
	glutDisplayFunc(OnDisplay);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(OnKeypress);
	glutMouseFunc(OnMouse);

	//-- run the program
	glutMainLoop();
	return 0;
}


////////////////////////////////////////////////////////
// Event Handers
////////////////////////////////////////////////////////
	  
void OnIdle(void)
{
	DrawFrame();
	glutPostRedisplay();
}


void OnDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2i(0, 0);
	glDrawPixels(FRAME_WIDE, FRAME_HIGH, GL_RGB,GL_UNSIGNED_BYTE, (GLubyte*)pFrameR);
	glutSwapBuffers();
	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void OnMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		PlaySoundEffect("Laser.wav"); 
		if (++shade > 16) shade = 0;	
	}
}

void OnKeypress(unsigned char key, int x, int y)
{
	switch (key) 
	{ 
	case ' ': xypos.x = xypos.y = 0; break;
	case 's': stereo ^= 1, eyes = 10;break;
	case ']': eyes++;	break;
	case '[': eyes--;	break;
	case 27 : exit(0);
	}
	PlaySoundEffect("Whoosh.wav"); 
}


////////////////////////////////////////////////////////
// Utility Functions
////////////////////////////////////////////////////////


void ClearScreen()
{
	memset(pFrameL, 0, FRAME_WIDE * FRAME_HIGH * 3);
	memset(pFrameR, 0, FRAME_WIDE * FRAME_HIGH * 3);
}


void Interlace(BYTE* pL, BYTE* pR)
{
	int rowlen = 3 * FRAME_WIDE;
	for (int y = 0; y < FRAME_HIGH; y+=2)
	{
		for (int x = 0; x < rowlen; x++) *pR++ = *pL++;
		pL += rowlen;
		pR += rowlen;
	}
}


void DrawFrame()
{
	//ClearScreen();
	
	if (!stereo) BuildFrame(pFrameR, 0);
	else {
		BuildFrame(pFrameL, -eyes);
		BuildFrame(pFrameR, +eyes);
		Interlace((BYTE*)pFrameL, (BYTE*)pFrameR);
	}
}

void	PlaySoundEffect(char * filename)		
{
#ifdef _WIN32
	PlaySound(filename, NULL, SND_ASYNC | SND_FILENAME ); 
#else
	char command[80];
	#ifdef __APPLE__
		sprintf(command, "afplay %s &", filename);
	#else
		sprintf(command, "play %s &", filename);
	#endif	
	system(command);
#endif
}

void SetPixel(struct POINT2D point, BYTE *fBuffer)
{
	fBuffer[3 * (point.y * FRAME_WIDE + point.x)] = point.r; // R
	fBuffer[3 * (point.y * FRAME_WIDE + point.x) + 1] = point.g; // G
	fBuffer[3 * (point.y * FRAME_WIDE + point.x) + 2] = point.b; // B
}

void drawLine(struct POINT2D p1, struct POINT2D p2, BYTE *fBuffer)
{
	int dx = p2.x - p1.x;
	int dy = p2.y - p1.y;
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
	double step_r = (p2.r - p1.r) / (double)steps;
	double step_g = (p2.g - p1.g) / (double)steps;
	double step_b = (p2.b - p1.b) / (double)steps;
	double x_inc = dx / (double)steps;
	double y_inc = dy / (double)steps;
	SetPixel(p1, fBuffer);
	double r_current = double(p1.r);
	double g_current = double(p1.g);
	double b_current = double(p1.b);
	for (int i = 0; i < steps; i++, p1.x += x_inc, p1.y += y_inc) {
		p1.x = ROUND(p1.x);
		p1.y = ROUND(p1.y);
		r_current += step_r;
		g_current += step_g;
		b_current += step_b;
		p1.r = r_current;
		p1.g = g_current;
		p1.b = b_current;
		SetPixel(p1, fBuffer);
	}
}

void triangle_edge(struct POINT2D p1, struct POINT2D p2, struct POINT2D p3, BYTE *fBuffer)
{
	drawLine(p1, p2, fBuffer);
	drawLine(p1, p3, fBuffer);
	drawLine(p2, p3, fBuffer);
}

struct POINT2D randPoint()
{
	struct POINT2D point = {
		rand() % FRAME_WIDE,
		rand() % FRAME_HIGH,
		rand() % 255,
		rand() % 255,
		rand() % 255
	};
	return point;
}

////////////////////////////////////////////////////////
// Drawing Funcion
////////////////////////////////////////////////////////

void BuildFrame(BYTE *pFrame, int view)
{
	// BYTE*	screen = (BYTE*)pFrame;		// use copy of screen pointer for safety

	// int		SBox = 250;					// size of box in pixels
	// int		channels = 3;				// number of colour channels

	// ////////////////////////////////////////////
	// // Display some animated shaded boxes - hacky method
	// ////////////////////////////////////////////

	// for (int x= xypos.x; x < xypos.x+SBox ; x++)
	// 	for (int y= xypos.y; y <xypos.y+SBox ; y++)
	// 	{
	// 		screen[channels*(x + view + y * FRAME_WIDE) + shade] = y-xypos.y;		// for 24 bit screen
	// 	}

	// if (++xypos.x +SBox  + view >= FRAME_WIDE ) xypos.x = 0;		// do not write past screen X boundary
	// if (++xypos.y +SBox >= FRAME_HIGH ) xypos.y = 0;		// do not write past screen Y boundary

	// 	//xypos.x = 200;
	// 	//xypos.y = 200;
	// //-- get rid of horizontal background edge --
	// if (xypos.y) for (int x= xypos.x; x < xypos.x+SBox ; x++)
	// {
	// 	screen[channels*(x + view + (xypos.y-1) * FRAME_WIDE) + shade] = 0;
	// }

	// //-- get rid of vertical background edge --
	// if (xypos.x) for (int y= xypos.y; y <xypos.y+SBox ; y++)
	// {
	// 	screen[channels*(view + xypos.x-1 + y * FRAME_WIDE) + shade] = 0;
	// }
	// struct POINT2D point1 = randPoint();
	// struct POINT2D point2 = randPoint();
	// struct POINT2D point3 = randPoint();
	struct POINT2D point1 = {100, 52, 255, 255, 255};
	struct POINT2D point2 = {111, 233, 255, 255, 255};
	struct POINT2D point3 = {240, 86, 255, 255, 255};
	//triangle_edge(point1, point2, point3, pFrame);
	drawLine(point1, point2, pFrame);
}

