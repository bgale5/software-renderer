
#include <cstdlib>			//- for exit()
#include <cstdio>			//- for sprintf()
#include <cstring>			//- for memset()
#include <ctime>			//- for random seed
#include <unistd.h>			//- for sleep()

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
void sort_vertices(struct POINT2D **triangle);
void draw_line(struct POINT2D p1, struct POINT2D p2, BYTE *fBuffer);

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

void SetPixel(struct POINT2D point, BYTE *fBuffer)
{
	fBuffer[3 * (point.y * FRAME_WIDE + point.x)] = point.r; // R
	fBuffer[3 * (point.y * FRAME_WIDE + point.x) + 1] = point.g; // G
	fBuffer[3 * (point.y * FRAME_WIDE + point.x) + 2] = point.b; // B
}

void draw_line(struct POINT2D p1, struct POINT2D p2, BYTE *fBuffer)
{
	double x = (double)(p1.x);
	double y = (double)(p1.y);
	double r = (double)(p1.r);
	double g = (double)(p1.g);
	double b = (double)(p1.b);
	int dx = p2.x - p1.x;
	int dy = p2.y - p1.y;
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
	double step_r = (p2.r - p1.r) / (double)steps;
	double step_g = (p2.g - p1.g) / (double)steps;
	double step_b = (p2.b - p1.b) / (double)steps;
	double x_inc = dx / (double)steps;
	double y_inc = dy / (double)steps;
	SetPixel(p1, fBuffer);
	for (int i = 0; i < steps; i++) {
		x += x_inc;
		y += y_inc;
		r += step_r;
		g += step_g;
		b += step_b;
		p1 = {ROUND(x), ROUND(y), (BYTE)r, (BYTE)g, (BYTE)b};
		SetPixel(p1, fBuffer);
	}
}

void draw_tri(struct POINT2D p1, struct POINT2D p2, struct POINT2D p3, BYTE *fBuffer)
{
	draw_line(p1, p2, fBuffer);
	draw_line(p1, p3, fBuffer);
	draw_line(p2, p3, fBuffer);
}

bool collinear(struct POINT2D **tri)
{
	double grad = (double)(tri[1]->y - tri[0]->y) / (double)(tri[1]->x - tri[0]->x);
	if ((double)(tri[2]->y - tri[1]->y) / (double)(tri[2]->x - tri[1]->x) != grad)
		return false;
	if ((double)(tri[2]->y - tri[0]->y) / (double)(tri[2]->x - tri[0]->x) != grad)
		return false;
	return true;
}

void fill_tri(struct POINT2D **triangle, BYTE *fBuffer)
{
	sort_vertices(triangle);
	if (collinear(triangle)) {
		draw_line(*(triangle[0]), *(triangle[1]), fBuffer);
		draw_line(*(triangle[1]), *(triangle[2]), fBuffer);
		return;
	}
	double fromx = (double)triangle[0]->x;
	double tox = (double)triangle[0]->x;
	
	int from_dx = (triangle[1]->x - triangle[0]->x);
	int from_dy = (triangle[1]->y - triangle[0]->y);
	int to_dx = (triangle[2]->x - triangle[0]->x);
	int to_dy = (triangle[2]->y - triangle[0]->y);

	double fromr = (double)triangle[0]->r;
	double fromg = (double)triangle[0]->g;
	double fromb = (double)triangle[0]->b;
	double tor = (double)triangle[0]->r;
	double tog = (double)triangle[0]->g;
	double tob = (double)triangle[0]->b;

	int from_dr = (triangle[1]->r - triangle[0]->r);
	int from_dg = (triangle[1]->g - triangle[0]->g);
	int from_db = (triangle[1]->b - triangle[0]->b);
	int to_dr = (triangle[2]->r - triangle[0]->r);
	int to_dg = (triangle[2]->g - triangle[0]->g);
	int to_db = (triangle[2]->b - triangle[0]->b);

	double from_inc = from_dx / (double)from_dy;
	double to_inc = to_dx / (double)to_dy;
	double from_r_inc = from_dr / (double)from_dy;
	double from_g_inc = from_dg / (double)from_dy;
	double from_b_inc = from_db / (double)from_dy;
	double to_r_inc = to_dr / (double)to_dy;
	double to_g_inc = to_dg / (double)to_dy;
	double to_b_inc = to_db / (double)to_dy; //TODO: Refactor

	for (int y = triangle[0]->y; y < triangle[2]->y; y++) {
		if (y == triangle[1]->y) {
			from_dx = triangle[2]->x - triangle[1]->x;
			from_dy = triangle[2]->y - triangle[1]->y;
			from_inc = from_dx / (double)from_dy;
			from_dr = triangle[2]->r - triangle[1]->r;
			from_dg = triangle[2]->g - triangle[1]->g;
			from_db = triangle[2]->b - triangle[1]->b;
			from_r_inc = from_dr / (double)from_dy;
			from_g_inc = from_dg / (double)from_dy;
			from_b_inc = from_db / (double)from_dy;
		}
		fromx += from_inc;
		tox += to_inc;
		fromr += from_r_inc;
		fromg += from_g_inc;
		fromb += from_b_inc;
		tor += to_r_inc;
		tog += to_g_inc;
		tob += to_b_inc;
		printf("%d ||%d %d %d | %d %d %d\n", y, (BYTE)fromr, (BYTE)fromg, (BYTE)fromb, (BYTE)fromg, (BYTE)tor, (BYTE)tog, (BYTE)tob);
		draw_line({ROUND(fromx), y, (BYTE)fromr, (BYTE)(fromg), (BYTE)(fromb)}, {ROUND(tox), y, (BYTE)(tor), (BYTE)(tog), (BYTE)(tob)}, fBuffer);
	}
}

struct POINT2D rand_point()
{
	struct POINT2D point;
	point.x = rand() % FRAME_WIDE;
	point.y = rand() % FRAME_HIGH;
	point.r = (BYTE)(rand() % 255);
	point.g = (BYTE)(rand() % 255);
	point.b = (BYTE)(rand() % 255);
	return point;
}

/*
 * Finds sorts the points in order of y value
 * So that top, middle, and bottom can be identified
 */
void sort_vertices(struct POINT2D **triangle)
{
	struct POINT2D *temp = NULL;
	if (triangle[0]->y > triangle[2]->y) {
		temp = triangle[0];
		triangle[0] = triangle[2];
		triangle[2] = temp;
	}
	if (triangle[0]->y > triangle[1]->y) {
		temp = triangle[0];
		triangle[0] = triangle[1];
		triangle[1] = temp;
	}
	if (triangle[1]->y > triangle[2]->y) {
		temp = triangle[1];
		triangle[1] = triangle[2];
		triangle[2] = temp;
	}
}


////////////////////////////////////////////////////////
// Drawing Funcion
////////////////////////////////////////////////////////

void BuildFrame(BYTE *pFrame, int view)
{
	struct POINT2D p1 = rand_point();
	struct POINT2D p2 = rand_point();
	struct POINT2D p3 = rand_point();
	struct POINT2D *tri[3] = {&p1, &p2, &p3};
	// p1 = {100, p1.y, 255, 0, 0};
	// p2 = {100, p2.y, 0, 255, 0};
	// p3 = {100, p3.y, 0, 0, 255};
	fill_tri(tri, pFrame);
	//draw_tri(p1, p2, p3, pFrame);
	sleep(1);
}

