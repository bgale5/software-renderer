#ifndef ENGINE_H
#define ENGINE_H
 
//====== Macros and Defines =========

#define FRAME_WIDE 1000
#define FRAME_HIGH 600
#define ROUND(x) ((int)(x+0.5))
#define TMIN 0 // For parametric line equations
#define TMAX 1 // For parametric line equations

//====== Structs & typedefs =========
typedef unsigned char BYTE;
struct POINT2D {
	int x, y;
	BYTE r, g, b;
};

//====== Forward Declaration s=========
void sort_vertices(struct POINT2D **triangle);
void draw_line(struct POINT2D p1, struct POINT2D p2, BYTE *fBuffer);
bool inside(struct POINT2D **tri, struct POINT2D *pt);
bool same_side(struct POINT2D a, struct POINT2D b, struct POINT2D l1, struct POINT2D l2);
void fill_poly(struct POINT2D **poly, int vertex_count, BYTE *fBuffer);
void fill_tri(struct POINT2D **triangle, BYTE *fBuffer);
bool concave(struct POINT2D a, struct POINT2D b, struct POINT2D c);
bool points_inside(struct POINT2D **tri, struct POINT2D **poly, int vertex_count);
void clip_line(struct POINT2D p1, struct POINT2D p2, BYTE *fBuffer);
void test_points_inside(BYTE *pFrame);
struct POINT2D rand_point();
void test_same_side(BYTE *pFrame);
void draw_poly(struct POINT2D **poly, int vertex_count, BYTE *fBuffer);
bool convex(POINT2D p2, POINT2D p1, POINT2D p0);

#endif
