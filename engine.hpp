#ifndef ENGINE_H
#define ENGINE_H
#include <string>
#include <vector>
 
//====== Macros and Defines =========

#define FRAME_WIDE 1000
#define FRAME_HIGH 600
#define ROUND(x) ((int)(x+0.5))
#define TMIN 0 // For parametric line equations
#define TMAX 1 // For parametric line equations
#define X_OFFSET FRAME_WIDE / 2
#define Y_OFFSET FRAME_HIGH / 2
#define PERSPECTIVE 500
#define MaxNumObjs 20
#define MaxNumPts 20//600
#define MaxNumPolys 20//900
#define NumSidesPoly 20 //600

//====== Structs & typedefs =========
typedef unsigned char BYTE;

typedef struct point_2d {
	int x, y;
	BYTE r, g, b;
} Point_2d;

typedef struct point_3d {
	int x, y, z;
	BYTE r, g, b;
} Point_3d;

typedef std::vector<Point_2d> Polygon_2d;
typedef std::vector<int> Polygon_3d;

typedef struct object_attribs {
	Point_3d center, scale;
} Object_attribs;

typedef struct object {
	Object_attribs properties;
	int vertex_count, poly_count;
	std::vector<Polygon_3d> polys;
	std::vector<Point_3d> vertices;
} Object;

//====== Forward Declarations=========


#endif
void draw_pixel_2d(const Point_2d &point, BYTE *fBuffer);
void draw_pixel_3d(const Point_3d &p, BYTE *fBuffer);
Point_2d project_point(const Point_3d &p3d);
void project_polygon(const struct object &obj, const Polygon_2d &p2d);
void draw_line(Point_2d p1, Point_2d p2, BYTE *fBuffer);
bool clip_test(double p, double q, double &u1, double &u2);
void clip_line(Point_2d p1, Point_2d p2, BYTE *fBuffer);
void draw_tri(const Polygon_2d &tri, BYTE *fBuffer);
void draw_poly(const Polygon_2d &poly, BYTE *fBuffer);
void draw_object_3d(const struct object &obj, BYTE *fBuffer);
void draw_wireframe_3d(const struct object &obj, BYTE *fBuffer);
bool point_cmp(const Point_2d &a, const Point_2d &b);
int find_point(const Polygon_2d &neighbours, const Point_2d &point);
void fill_poly(const Polygon_2d &poly, BYTE *fBuffer);
bool convex(const Point_2d &p2, const Point_2d &p1, const Point_2d &p0);
bool points_inside(const Polygon_2d &tri, const Polygon_2d &poly);
bool inside(const Polygon_2d &tri, const Point_2d &pt);
bool same_side(const Point_2d &a, const Point_2d &b, const Point_2d &l1, const Point_2d &l2);
bool collinear(const Polygon_2d &tri);
void fill_tri(Polygon_2d &triangle, BYTE *fBuffer);
Point_2d rand_point();
void sort_vertices(Polygon_2d &triangle);
void load_vjs(std::string fpath, Object &obj);