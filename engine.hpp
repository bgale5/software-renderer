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
#define TRANSLATION_FACTOR 10
#define ROTATION_FACTOR 2 * M_PI / 360

//====== Structs & typedefs =========
typedef unsigned char BYTE;

typedef struct Point {
	double x, y;
	double r, g, b;
	double z = 0;
} Point;

typedef std::vector<Point> Polygon_2d;
typedef std::vector<int> Polygon_3d;

typedef struct object_attribs {
	Point centre;
	double scale;
} Object_attribs;

typedef struct object {
	Object_attribs properties;
	int vertex_count, poly_count;
	std::vector<Polygon_3d> polys;
	std::vector<Point> vertices;
} Object;

enum Direction {
	LEFT,
	RIGHT,
	UP,
	DOWN,
	IN,
	OUT, 
	SCALE_UP,
	SCALE_DOWN
};

extern std::vector<Object*> translatable;

//====== Forward Declarations=========
void draw_pixel_2d(const Point &point, BYTE *fBuffer);
void draw_pixel_3d(Point p, BYTE *fBuffer);
Point project_point(const Point &p3d);
void project_polygon(const struct object &obj, const Polygon_2d &p2d);
void draw_line(Point p1, Point p2, BYTE *fBuffer);
bool clip_test(double p, double q, double &u1, double &u2);
void clip_line(Point p1, Point p2, BYTE *fBuffer);
void draw_tri(const Polygon_2d &tri, BYTE *fBuffer);
void draw_poly(const Polygon_2d &poly, BYTE *fBuffer);
void draw_object_3d(const struct object &obj, BYTE *fBuffer);
void draw_wireframe_3d(const struct object &obj, BYTE *fBuffer);
bool point_cmp(const Point &a, const Point &b);
int find_point(const Polygon_2d &neighbours, const Point &point);
void fill_poly(Polygon_2d poly, BYTE *fBuffer);
bool concave(const Point &p2, const Point &p1, const Point &p0);
bool points_inside(const Polygon_2d &tri, const Polygon_2d &poly);
bool inside(const Polygon_2d &tri, const Point &pt);
bool same_side(const Point &a, const Point &b, const Point &l1, const Point &l2);
bool collinear(const Polygon_2d &tri);
void fill_tri(Polygon_2d triangle, BYTE *fBuffer);
Point rand_point();
void sort_vertices(Polygon_2d &triangle);
void load_vjs(std::string fpath, Object &obj, const Object_attribs &properties);
void translate_3d(Direction d, double offset);
void centre_3d();
Polygon_2d rand_polygon(const Point &centre, double angle_increment);
void translate_2d(Polygon_2d &poly, const Point &offset);
void round_vertices(Polygon_2d &poly);
void rotate_z(double angle);
void rotate_x(double angle);
void rotate_y(double angle);
void rotate_3d(double angle_x, double angle_y, double angle_z);
Point point_gradient(const Point &p1, const Point &p2);
void set_zbuff(int x, int y, int z_val);
int get_zbuff(int x, int y);
void init_zbuff();


#endif