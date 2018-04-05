#ifndef ENGINE_H
#define ENGINE_H
#include <string>
 
//====== Macros and Defines =========

#define FRAME_WIDE 1000
#define FRAME_HIGH 600
#define ROUND(x) ((int)(x+0.5))
#define TMIN 0 // For parametric line equations
#define TMAX 1 // For parametric line equations
#define PERSPECTIVE 500
#define MaxNumObjs 20
#define MaxNumPts 20//600
#define MaxNumPolys 20//900
#define NumSidesPoly 20 //600

//====== Structs & typedefs =========
typedef unsigned char BYTE;
struct point_2d {
	int x, y;
	BYTE r, g, b;
};
struct point_3d {
	int x, y, z;
	BYTE r, g, b;
};

struct polygon_2d {
	struct point_2d points[NumSidesPoly];
	int vertex_count;
};

struct polygon_3d {
	int vertices[NumSidesPoly]; // Indices/offsets for vertices in the VJS array
	int vertex_count;
};

struct object_attribs {
	struct point_3d center, scale;
};

struct object {
	struct object_attribs properties;
	int vertex_count, poly_count;
	struct polygon_3d object_polys[MaxNumPolys];
	struct point_3d object_points[MaxNumPts];
};

//====== Forward Declarations=========
void sort_vertices(struct point_2d **triangle);
void draw_line(struct point_2d p1, struct point_2d p2, BYTE *fBuffer);
bool inside(struct point_2d **tri, struct point_2d *pt);
bool same_side(struct point_2d a, struct point_2d b, struct point_2d l1, struct point_2d l2);
void fill_poly(struct polygon_2d *poly, BYTE *fBuffer);
void fill_tri(struct point_2d **triangle, BYTE *fBuffer);
bool concave(struct point_2d a, struct point_2d b, struct point_2d c);
bool points_inside(struct point_2d **tri, struct polygon_2d *poly, int vertex_count);
void clip_line(struct point_2d p1, struct point_2d p2, BYTE *fBuffer);
struct point_2d rand_point();
void draw_poly(struct point_2d **poly, int vertex_count, BYTE *fBuffer);
bool convex(point_2d p2, point_2d p1, point_2d p0);
void SetPixel3D(struct point_3d point, BYTE *fBuffer);
struct point_2d project_point(struct point_3d p3d);
void draw_object_3d(struct object *obj, BYTE *fBuffer);
void project_polygon(struct object *obj, struct polygon_2d *p2d);
void SetPixel(struct point_2d point, BYTE *fBuffer);
void draw_wireframe_3d(struct object *obj, BYTE *fBuffer);
void load_vjs(std::string fpath, struct object *obj);

#endif
