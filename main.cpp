#include <cstdlib>			//- for exit()
#include <cstdio>			//- for sprintf()
#include <cstring>			//- for memset()
#include <ctime>			//- for random seed
#include <unistd.h>			//- for sleep()
#include <cstring>
#include "engine.hpp"
#include <cmath>

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
	case 27 : exit(0); break;
	case 'j': translate_3d(DOWN, TRANSLATION_FACTOR); break;
	case 'k': translate_3d(UP, TRANSLATION_FACTOR); break;
	case 'h': translate_3d(LEFT, TRANSLATION_FACTOR); break;
	case 'l': translate_3d(RIGHT, TRANSLATION_FACTOR); break;
	case 'i': translate_3d(IN, TRANSLATION_FACTOR); break;
	case 'o': translate_3d(OUT, TRANSLATION_FACTOR); break;
	case '=': translate_3d(SCALE_UP, TRANSLATION_FACTOR / 100.0); break;
	case '-': translate_3d(SCALE_DOWN, TRANSLATION_FACTOR / 100.0); break;
	case 'c': centre_3d(); break;
	}
	//PlaySoundEffect("Whoosh.wav"); 
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
bool loaded = false;
Object temp;
int counter = 0;
Polygon_2d triangle;
Point_2d centre;
void BuildFrame(BYTE *pFrame, int view)
{
	//  ------ CLIP LINE TEST ----- //

	// Point_2d p0 = {100, 100, 255, 0, 0};
	// Point_2d p1 = {400, 300, 0, 255, 0};
	// clip_line(p0, p1, pFrame);

	// ----- FILL TRIANGLE TEST ----- //
	// if (!loaded) {
	// 	Point_2d p0 = {500, 200, 255, 0, 0};
	// 	Point_2d p2 = {400, 400, 0, 255, 0};
	// 	Point_2d p1 = {600, 400, 0, 0, 255};
	// 	Point_2d centre;
	// 	centre = {500, 300, 0, 0, 0};
	// 	triangle.push_back(p0);
	// 	triangle.push_back(p1);
	// 	triangle.push_back(p2);
	// 	loaded = true;
	// }
	// double angle = (2 * M_PI * (counter / 360.0));
	// //fill_tri(triangle, pFrame);
	// draw_tri(triangle, pFrame);
	// rotate_2d(triangle, centre, angle);
	// counter = (counter + 1) % 360;
	// draw_pixel_2d(centre, pFrame);

	// -------  POLY TEST ----- //

	// // Point_2d p0 = {100, 100, 255, 255, 255};
	// // Point_2d p1 = {100, 500, 255, 0, 0};
	// // Point_2d pa = {400, 250, 0, 255, 255};
	// // Point_2d p2 = {500, 500, 0, 255, 0};
	// // Point_2d p3 = {500, 150, 0, 0, 255};
	// // Point_2d p4 = {300, 300, 255, 255, 0};
	// // Polygon_2d poly;
	// // poly.push_back(p1);
	// // poly.push_back(pa);
	// // poly.push_back(p2);
	// // poly.push_back(p3);
	// // poly.push_back(p4);
	// // poly.push_back(p0);
	// Polygon_2d poly = rand_polygon({FRAME_WIDE / 2, FRAME_HIGH / 2, 0, 0, 0}, M_PI / 3);
	// fill_poly(poly, pFrame);
	// //draw_poly(poly, pFrame);
	// //draw_pixel_2d(poly[counter = (counter + 1) % poly.size()], pFrame);
	// //draw_poly(poly, pFrame);

	// ----------- VJS LOAD TEST --------//

	if (!loaded) {
		Object_attribs temp_properties;
		temp_properties.scale = 1;
		temp_properties.centre = {300, 350, 100, 0, 0, 0};
		load_vjs("cube.vjs", temp, temp_properties);
		loaded = true;
		translatable.push_back(&temp);
	}
	printf("Loaded object:\n");
	printf("Vertices:\n");
	for (int i = 0; i < temp.vertex_count; i++) {
		Point_3d vert = temp.vertices[i];
		printf("(%d, %d, %d, %d, %d, %d)\n", (int)vert.x, (int)vert.y, (int)vert.z, (BYTE)vert.r, (BYTE)vert.g, (BYTE)vert.b);
	}
	printf("Polygons:\n");
	for (int i = 0; i < temp.poly_count; i++) {
		Polygon_3d polygon = temp.polys[i];
		printf("(%d, %d, %d, %d)\n", polygon[0], polygon[1], polygon[2], polygon[3]);
	}

	draw_object_3d(temp, pFrame);
	draw_wireframe_3d(temp, pFrame);
	
	//sleep(2);
}
