#include <cstdlib>			//- for exit()
#include <cstdio>			//- for sprintf()
#include <cstring>			//- for memset()
#include <ctime>			//- for random seed
#include <unistd.h>			//- for sleep()
#include <cstring>
#include "engine.hpp"

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

//====== Global Variables ==========
BYTE	pFrameL[FRAME_WIDE * FRAME_HIGH * 3];
BYTE	pFrameR[FRAME_WIDE * FRAME_HIGH * 3];
int		shade = 0;
point_2d	xypos = {0,0};
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
    glPixelZoom( 1, -1 );
    glRasterPos2i(0, FRAME_HIGH-1);
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
	ClearScreen();
	
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

////////////////////////////////////////////////////////
// Drawing Funcion
////////////////////////////////////////////////////////

void BuildFrame(BYTE *pFrame, int view)
{
	struct point_2d p0 = {100, 250, 255, 0, 0};
	struct point_2d p1 = {250, 400, 0, 255, 0};
	struct point_2d p2 = {400, 300, 0, 0, 255};
	struct point_2d p3 = {500, 350, 100, 100, 100};
	struct point_2d p4 = {650, 150, 200, 200, 200};
	struct point_2d p5 = {500, 200, 255, 255, 255};
	struct point_2d p6 = {450, 100, 255, 255, 255};
	struct point_2d p7 = {350, 250, 255, 255, 255};
	struct point_2d p8 = {200, 200, 255, 255, 255};
	struct point_2d p9 = {250, 100, 255, 255, 255};
	struct polygon_2d pol;
	pol.vertex_count = 10;
	pol.points[0] = p0;
	pol.points[1] = p1;
	pol.points[2] = p2;
	pol.points[3] = p3;
	pol.points[4] = p4;
	pol.points[5] = p5;
	pol.points[6] = p6;
	pol.points[7] = p7;
	pol.points[8] = p8;
	pol.points[9] = p9;
	fill_poly(&pol, pFrame);
	
	//struct polygon_3d p0;
	//struct polygon_3d p1;
	//struct polygon_3d p2;
	//struct polygon_3d p3;
	//struct polygon_3d p4;
	//struct polygon_3d p5;
	////p0.vertices = {3, 2, 1, 0};
	//p0.vertices[0] = 3;
	//p0.vertices[1] = 2;
	//p0.vertices[2] = 1;
	//p0.vertices[3] = 0;
	////p1.vertices = {0, 7, 4, 3};
	//p1.vertices[0] = 0;
	//p1.vertices[1] = 7;
	//p1.vertices[2] = 4;
	//p1.vertices[3] = 3;
	////p2.vertices = {5, 4, 3, 2};
	//p2.vertices[0] = 5;
	//p2.vertices[1] = 4;
	//p2.vertices[2] = 3;
	//p2.vertices[3] = 2;
	////p3.vertices = {1, 2, 5, 6};
	//p3.vertices[0] = 1;
	//p3.vertices[1] = 2;
	//p3.vertices[2] = 5;
	//p3.vertices[3] = 6;
	////p4.vertices = {0, 1, 6, 7};
	//p4.vertices[0] = 0;
	//p4.vertices[1] = 1;
	//p4.vertices[2] = 6;
	//p4.vertices[3] = 7;
	////p5.vertices = {4, 5, 6, 7};
	//p5.vertices[0] = 4;
	//p5.vertices[1] = 5;
	//p5.vertices[2] = 6;
	//p5.vertices[3] = 7;
//
	//struct object obj;
	//obj.poly_count = 6;
	//obj.vertex_count = 8;
	//obj.object_polys[0] = p0;
	//obj.object_polys[1] = p1;
	//obj.object_polys[2] = p2;
	//obj.object_polys[3] = p3;
	//obj.object_polys[4] = p4;
	//obj.object_polys[5] = p5;
	//obj.object_points[0] = {200, 200, 1,   255, 0, 128};
	//obj.object_points[1] = {200, 400, 1,   128, 0, 0};
	//obj.object_points[2] = {400, 400, 1,   255, 255, 255};
	//obj.object_points[3] = {400, 200, 1,   0, 0, 255};
	//obj.object_points[4] = {200, 200, 200, 255, 255, 0};
	//obj.object_points[5] = {200, 400, 200, 128, 128, 0};
	//obj.object_points[6] = {400, 400, 200, 255, 0, 128};
	//obj.object_points[7] = {400, 200, 200, 0, 255, 255};
	//draw_3d_object(&obj, pFrame);

	sleep(1);
}
