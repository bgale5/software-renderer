#include <cstdlib>			//- for exit()
#include <cstdio>			//- for sprintf()
#include <cstring>			//- for memset()
#include <ctime>			//- for random seed
#include <unistd.h>			//- for sleep()
#include <cstring>
#include "engine.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

/*======================= Global Variables  ================================= */

std::vector<Object> world_objects;
int zBuffer[FRAME_WIDE * FRAME_HIGH];

/*======================= Drawing Functions ================================= */

/**
 * draw_pixel_2d(const Point &point, BYTE *fBuffer) 
 * -- Description:
 * 	Maps a set of x and y coordinates to an index in the pixel buffer.
 * Arguments:
 * 	A reference to a Point struct containing x and y coordinates
 * -- Side effects:
 * 	This function directly modifies the frame buffer
 */
void draw_pixel_2d(const Point &point, BYTE *fBuffer)
{
	int x  =  ROUND(point.x);
	int y  =  ROUND(point.y);
	BYTE r = ROUND(point.r);
	BYTE g = ROUND(point.g);
	BYTE b = ROUND(point.b);
	int z  =  ROUND(point.z);
	if (z > get_zbuff(x, y)) // another pixel is in front; don't draw
		return;
	fBuffer[3 * (y * FRAME_WIDE + x)] = r;		// R
	fBuffer[3 * (y * FRAME_WIDE + x) + 1] = g; 	// G
	fBuffer[3 * (y * FRAME_WIDE + x) + 2] = b; 	// B
	set_zbuff(x, y, z);
}

void set_zbuff(int x, int y, int z_val)
{
	zBuffer[y * FRAME_WIDE + x] = z_val;
}

int get_zbuff(int x, int y)
{
	return zBuffer[y * FRAME_WIDE + x];
}

void init_zbuff()
{
	std::fill(zBuffer, zBuffer + FRAME_HIGH * FRAME_WIDE, INT_MAX);
}

void draw_pixel_3d(Point p, BYTE *fBuffer)
{
	Point point = project_point(p);
	draw_pixel_2d(point, fBuffer);
}

/**
 * project_point(const Point &p3d)
 * -- Description:
 * Projects a set of 3D coordinates onto 2D viewport
 * -- Arguments:
 * A Point struct containing 3D coorinates
 * -- Return value:
 * Returns a projected 2D point, which retains its original Z value for later use in z-buffering
 */	
Point project_point(const Point &p3d)
{
	Point p2d = {
		((p3d.x - X_OFFSET) * PERSPECTIVE / (double)(p3d.z + PERSPECTIVE)) + X_OFFSET,
		((p3d.y - Y_OFFSET) * PERSPECTIVE / (double)(p3d.z + PERSPECTIVE)) + Y_OFFSET,
		p3d.r,
		p3d.g,
		p3d.b,
		p3d.z
	};
	return p2d;
}

void project_polygon(const Object &obj, std::vector<Polygon> &projected_polys)
{
	for (int i = 0; i < obj.poly_count; i++) { // For every polygon in the object...
		Polygon_ref current_poly = obj.polys[i];
		Polygon projected_poly;
		for (int j = 0; j < current_poly.size(); j++) { // For every point in the current polygon
			projected_poly.push_back(project_point(obj.vertices[current_poly[j]]));
		}
		projected_polys.push_back(projected_poly);
	}
}

/**
 * draw_line(Point p1, Point p2, BYTE *fBuffer)
 * -- Description
 * Uses the DDA line drawing algorithm to draw a straight line between two points on the screen
 * -- Arguments
 * Two Point structs containging x and y coordinates
 * -- Side effects
 * Calls functions that modfiy the frame buffer
 */
void draw_line(Point p1, Point p2, BYTE *fBuffer)
{
	Point buffer = p1;
	Point grad = point_gradient(p2, p1);
	double steps = abs(grad.x) > abs(grad.y) ? abs(grad.x) : abs(grad.y);
	draw_pixel_2d(buffer, fBuffer);
	for (int i = 0; i < steps; i++) {
		buffer.x += grad.x / steps;
		buffer.y += grad.y / steps;
		buffer.r += grad.r / steps;
		buffer.g += grad.g / steps;
		buffer.b += grad.b / steps;
		buffer.z += grad.z / steps;
		draw_pixel_2d(buffer, fBuffer);
	}
}

bool clip_test(double p, double q, double &u1, double &u2)
{
	double r = q / p;
	if (p < 0.0) { // outside -> inside
		if (r > u2)
			return false;
		else if (r > u1)
			u1 = r;
	} else if (p > 0.0) { // inside -> outside
		if (r < u1)
			return false;
		else if (r < u2)
			u2 = r;
	} else if (q < 0.0) {
		return false;
	}
	return true;
}

/**
 * clip_line(Point p1, Point p2, BYTE *fBuffer)
 * -- Description
 * Tests whether parts of the line between p1 and p2 fall out of bounds.
 * If out of bounds, calculates new endpoints at the boundaries.
 * -- Arguments
 * Two Point structs representing endpoints of a line
 * -- Side Effects
 * Calls functions that modify the frame buffer
 */
void clip_line(Point p1, Point p2, BYTE *fBuffer)
{
	//double dx = p2.x - p1.x;
	Point d = point_gradient(p2, p1);
	
	//double dy;
	double u1 = 0.0;
	double u2 = 1.0;
	int min_x = 0;
	int max_x = FRAME_WIDE - 1;
	int min_y = 0;
	int max_y = FRAME_HIGH - 1;
	if (clip_test(-d.x, p1.x - min_x, u1, u2)) {
		if (clip_test(d.x, max_x - p1.x, u1, u2)) {
			//dy = p2.y - p1.y;
			if (clip_test(-d.y, p1.y - min_y, u1, u2)) {
				if (clip_test(d.y, max_y - p1.y, u1, u2)) {
					if (u2 < 1) {
						p2.x = p1.x + u2 * d.x;
						p2.y = p1.y + u2 * d.y;
						p2.r = p1.r + u2 * d.r;
						p2.g = p1.g + u2 * d.g;
						p2.b = p1.b + u2 * d.b;
					}
					if (u1 > 0) {
						p1.x += u1 * d.x;
						p1.y += u1 * d.y;
						p1.r += u1 * d.r;
						p1.g += u1 * d.g;
						p1.b += u1 * d.b;
					}
					draw_line(p1, p2, fBuffer);
				}
			}
		}
	}
}

void draw_tri(const Polygon &tri, BYTE *fBuffer)
{
	// If tri contains more than 3 points, the excess will be ignored
	clip_line(tri[0], tri[1], fBuffer);
	clip_line(tri[0], tri[2], fBuffer);
	clip_line(tri[1], tri[2], fBuffer);
}

void draw_poly(const Polygon &poly, BYTE *fBuffer)
{
	int vc = poly.size();
	if (vc == 1) {
		draw_pixel_2d(poly[0], fBuffer);
		return;
	}
	for (int a = 0; a + 1 < vc; a++) {
		clip_line(poly[a], poly[a + 1], fBuffer);
	}
	clip_line(poly[vc - 1], poly[0], fBuffer);
}

Object rel_to_abs(const Object &rel)
{
	Object absolute = rel;
	double scale = rel.properties.scale;
	Point centre = rel.properties.centre;
	for (int i = 0; i < absolute.vertex_count; i++) {
		absolute.vertices[i].x = absolute.vertices[i].x * scale + centre.x;
		absolute.vertices[i].y = absolute.vertices[i].y * scale + centre.y;
		absolute.vertices[i].z = absolute.vertices[i].z * scale + centre.z;
	}
	return absolute;
}

/**
 * draw_object_3d(const Object &obj, BYTE *fBuffer)
 * -- Description
 * Breaks an object into separate polygons to be filled individually
 * If the surface normal of a face is positive, it will be skipped
 * -- Arguments:
 * An Object struct to be drawn
 * -- Side Effects
 * Calls functions that modify the frame buffer
 */
void draw_object_3d(const Object &obj, BYTE *fBuffer)
{
	std::vector<Polygon> projected_polys;
	Object absolute = rel_to_abs(obj);
	project_polygon(absolute, projected_polys); // Populates projected_polys
	//Object_norms norms = compute_surface_normals(obj);
	for (int i = 0; i < projected_polys.size(); i++) {
		//long test = norms[i];
		//if (norms[i] > 0) // Enable Back-face culling
			fill_poly(projected_polys[i], fBuffer);
	}
}

void draw_wireframe_3d(const Object &obj, BYTE *fBuffer)
{
	std::vector<Polygon> polys;
	Object absolute = rel_to_abs(obj);
	project_polygon(absolute, polys);
	for (int i = 0; i < obj.poly_count; i++) {
		for (int j = 0; j < polys[i].size(); j++) {
			polys[i][j].r = 255;
			polys[i][j].g = 255;
			polys[i][j].b = 255;
		}
		draw_poly(polys[i], fBuffer);
	}
}

bool point_cmp(const Point &a, const Point &b)
{
	return (
		   a.x == b.x
		&& a.y == b.y
		&& a.r == b.r
		&& a.g == b.g
		&& a.b == b.b
		&& a.z == b.z
	);
}

int find_point(const Polygon &neighbours, const Point &point)
{
	// NOTE: Assumes that points are unique
	for (int i = 0; i < neighbours.size(); i++) {
		if (point_cmp(point, neighbours[i]))
			return i;
	}
	return -1;
}

/**
 * fill_poly(Polygon poly, BYTE *fBuffer)
 * -- Description
 * Performs triangle decomposition on the given polygon and passes each triangle to fill_tri()
 * Stops when n-2 triangles have been generated.
 * Supports both concave and convex polygons.
 * -- Arguments:
 * A Polygon to be filled
 * -- Side Effects
 * Calls functions that modify the frame buffer
 */ 
void fill_poly(Polygon poly, BYTE *fBuffer)
{
	Polygon neighbours = poly;
	Polygon original = poly;
	int tri_count = 0;
	for (int i = 0; tri_count < poly.size() - 2; i++) {
		if (i % poly.size() == 0 && i > 0) {
			if (poly.size() == neighbours.size())
				break; // All sides are concave; the plane is facing away from the screen
			poly = neighbours; // Dump the triangles that have already been drawn
		}
		int current = find_point(neighbours, poly[i % poly.size()]); // Wrap to beginning
		int next_adjacent = current == neighbours.size() - 1 ? 0 : current + 1;
		int prev_adjacent = current == 0 ? neighbours.size() - 1 : current - 1;
		Polygon tri = {neighbours[current], neighbours[next_adjacent], neighbours[prev_adjacent]};
		if (points_inside(tri, original) || concave(neighbours[current], neighbours[prev_adjacent], neighbours[next_adjacent]))
			continue;
		fill_tri(tri, fBuffer);
		neighbours.erase(neighbours.begin()+current);
		tri_count++;
	}
}

bool concave(const Point &p2, const Point &p1, const Point &p0)
{
	// Test to see if p0 is on the left/right side of p2 --> p1 edge.
    return ((p2.x - p1.x) * (p0.y - p1.y) - (p2.y - p1.y) * (p0.x - p1.x)) < 0;
}

bool points_inside(const Polygon &tri, const Polygon &poly)
{
	for (int i = 0; i < poly.size(); i++) {
		if (inside(tri, poly[i]))
			return true;
	}
	return false;
}

bool inside(const Polygon &tri, const Point &pt)
{
	return same_side(pt, tri[0], tri[1], tri[2])
			&& same_side(pt, tri[1], tri[0], tri[2])
			&& same_side(pt, tri[2], tri[0], tri[1]);
}

// l1 and l2 are the ends of the line
// returns true if a & b are on the same side of line
bool same_side(const Point &a, const Point &b, const Point &l1, const Point &l2)
{
	double apt = (a.x - l1.x) * (l2.y - l1.y) - (l2.x - l1.x) * (a.y - l1.y);
	double bpt = (b.x - l1.x) * (l2.y - l1.y) - (l2.x - l1.x) * (b.y - l1.y);
	return ((apt * bpt) > 0);
}

bool collinear(const Polygon &tri)
{
	// Checks whether all the gradients are equal
	double grad = (double)(tri[1].y - tri[0].y) / (double)(tri[1].x - tri[0].x);
	if ((double)(tri[2].y - tri[1].y) / (double)(tri[2].x - tri[1].x) != grad)
		return false;
	if ((double)(tri[2].y - tri[0].y) / (double)(tri[2].x - tri[0].x) != grad)
		return false;
	return true;
}

Point point_gradient(const Point &p1, const Point &p2)
{
	Point gradient = {
		(double)ROUND(p1.x) - (double)ROUND(p2.x),
		(double)ROUND(p1.y) - (double)ROUND(p2.y),
		(double)ROUND(p1.r) - (double)ROUND(p2.r),
		(double)ROUND(p1.g) - (double)ROUND(p2.g),
		(double)ROUND(p1.b) - (double)ROUND(p2.b),
		(double)ROUND(p1.z) - (double)ROUND(p2.z)
	};
	return gradient;
}

void round_vertices(Polygon &poly)
{
	for (int i = 0; i < poly.size(); i++) {
		poly[i].x = (double)ROUND(poly[i].x);
		poly[i].y = (double)ROUND(poly[i].y);
		poly[i].r = (double)ROUND(poly[i].r);
		poly[i].g = (double)ROUND(poly[i].g);
		poly[i].b = (double)ROUND(poly[i].b);
		poly[i].z = (double)ROUND(poly[i].z);
	}
}

/**
 * fill_tri(Polygon triangle, BYTE *fBuffer)
 * -- Description
 * Applies smoothshading to the given triangle using the scanline method
 * Supports special cases: collinear and flat-topped triangles
 * -- Arguments 
 * A Polygon containing 3 points representing a triangle
 * -- Side effects
 * Calls functions that modify the frame buffer
 */
void fill_tri(Polygon triangle, BYTE *fBuffer)
{
	round_vertices(triangle);
	sort_vertices(triangle);
	if (collinear(triangle)) { // handle collinear case
		clip_line(triangle[0], triangle[1], fBuffer);
		clip_line(triangle[1], triangle[2], fBuffer);
		return;
	}
	Point start, end;
	end = triangle[0];
	start = triangle[0].y == triangle[1].y ? triangle[1] : triangle[0]; // handle flat-top case
	Point start_gradient, end_gradient;
	start_gradient = point_gradient(triangle[1], triangle[0]);
	end_gradient = point_gradient(triangle[2], triangle[0]);
	for (int y = triangle[0].y; y < triangle[2].y; y++, start.y++, end.y++) {
		if (y == triangle[1].y)
			start_gradient = point_gradient(triangle[2], triangle[1]);
		clip_line(start, end, fBuffer);
		start.x += start_gradient.x / start_gradient.y;
		start.r += start_gradient.r / start_gradient.y;
		start.g += start_gradient.g / start_gradient.y;
		start.b += start_gradient.b / start_gradient.y;
		start.z += start_gradient.z / start_gradient.y; // For Z-Buffering
		end.x   += end_gradient.x   / end_gradient.y;
		end.r   += end_gradient.r   / end_gradient.y;
		end.g   += end_gradient.g   / end_gradient.y;
		end.b   += end_gradient.b   / end_gradient.y;
		end.z   += end_gradient.z   / end_gradient.y; // For Z-Buffering
	}
	draw_tri(triangle, fBuffer); // Hide any rounding artifacts
}

Point rand_point()
{
	Point point;
	point.x = rand() % FRAME_WIDE;
	point.y = rand() % FRAME_HIGH;
	point.r = rand() % 255;
	point.g = rand() % 255;
	point.b = rand() % 255;
	return point;
}

Polygon rand_polygon(const Point &centre, double angle_increment)
{
	Polygon poly;
	double mag, x, y, r, g, b;
	for(double theta = 0; theta < 2 * M_PI; theta += angle_increment) {
		mag = 10 + rand() % 190; // Arbitrary max and min values
		x = mag * cos(theta);
		y = mag * sin(theta);
		r = rand() % 255;
		g = rand() % 255;
		b = rand() % 255;
		Point point = {centre.x + x, centre.y + y, r, g, b};
		poly.push_back(point);
	}
	return poly;
}

void sort_vertices(Polygon &triangle)
{
	Point *temp = NULL;
	if (triangle[0].y > triangle[2].y) {
		std::swap(triangle[0], triangle[2]);
	}
	if (triangle[0].y > triangle[1].y) {
		std::swap(triangle[0], triangle[1]);
	}
	if (triangle[1].y > triangle[2].y) {
		std::swap(triangle[1], triangle[2]);
	}
}

std::vector<std::string> tokenize(std::string str, char sep=' ')
{
	std::vector<std::string> ret;
	std::istringstream stm(str);
	std::string token;
	while (std::getline(stm, token, sep))
		ret.push_back(token);
	return ret;
}


Point toks_to_p3d(std::vector<std::string> &toks)
{
	Point p3d;
	p3d.x = atoi(toks[0].c_str());
	p3d.y = atoi(toks[1].c_str());
	p3d.z = atoi(toks[2].c_str());
	p3d.r = atoi(toks[3].c_str());
	p3d.g = atoi(toks[4].c_str());
	p3d.b = atoi(toks[5].c_str());
	return p3d;
}

Polygon_ref toks_to_poly3d(std::vector<std::string> &toks)
{
	Polygon_ref poly;
	for (int i = 1; i < toks.size(); i++) {
		poly.push_back(atoi(toks[i].c_str()));
	}
	return poly;
}

/**
 * load_vjs(std::string fpath, Object &obj, const Object_attribs &properties)
 * -- Description
 * Loads object in given VJS file
 * -- Arguments
 * String containing VJS file path, and an object + its attributes struct in which to load the data
 * -- Side effects
 * Populates the given Object (through reference)
 */
void load_vjs(std::string fpath, Object &obj, const Object_attribs &properties)
{
	std::ifstream infile(fpath);
	if (!infile) {
		printf("The file doesn't exist\n");
		exit(1);
	}
	std::string linebuffer;
	std::getline(infile, linebuffer);
	std::vector<std::string> toks = tokenize(linebuffer, ',');
	int vert_count = atoi(toks[0].c_str());
	int poly_count = atoi(toks[1].c_str());
	obj.vertex_count = vert_count;
	obj.poly_count = poly_count;
	for (int i = 0; i < vert_count; i++) {
		std::getline(infile, linebuffer);
		toks = tokenize(linebuffer, ' ');
		Point point = toks_to_p3d(toks);
		obj.vertices.push_back(point);
	}
	for (int i = 0; i < poly_count; i++) {
		std::getline(infile, linebuffer);
		toks = tokenize(linebuffer, ' ');
		Polygon_ref poly = toks_to_poly3d(toks);
		obj.polys.push_back(poly);
	}
	obj.properties = properties;
}

void translate_3d(Object &obj, Point &offsets) 
{
	obj.properties.centre.x += offsets.x;
	obj.properties.centre.y += offsets.y;
	obj.properties.centre.z += offsets.z;
}

void scale_3d(Object &obj, double offset)
{
	obj.properties.scale += offset;
}

void translate_2d(Polygon &poly, const Point &offset)
{
	for (int i = 0; i < poly.size(); i++) {
		poly[i].x += offset.x;
		poly[i].y += offset.y;
	}
}

void rotate_object(Object &obj, Rotation_offsets offset)
{
	for (int i = 0; i < obj.vertices.size(); i++) {
		rotate_point(obj.vertices[i], offset);
	}
}

/**
 *  rotate_point(Point &point, Rotation_offsets offset) 
 * -- Description
 * Applies 3D rotation 3 axes in the order X, Y, Z.
 * -- Arguments
 * Point struct containing 3D coordinates to be rotated,
 * Rotation offset containing the rotation deltas in radians
 * -- Side effects
 * Modifies the values in the point struct (through indirection)
 */
void rotate_point(Point &point, Rotation_offsets offset)
{
	double x = 0, y = 0, z = 0;
	// Rotate About X
	if (offset.x) {
		y = point.y * cos(offset.x) - point.z * sin(offset.x);
		z = point.y * sin(offset.x) + point.z * cos(offset.x);
		point.y = y;
		point.z = z;
	}
	// Rotate About Y
	if (offset.y) {
		z = point.z * cos(offset.y) - point.x * sin(offset.y);
		x = point.z * sin(offset.y) + point.x * cos(offset.y);
		point.z = z;
		point.x = x;
	}
	// Rotate About Z
	if (offset.z) {
		x = point.x * cos(offset.z) - point.y * sin(offset.z);
		y = point.x * sin(offset.z) + point.y * cos(offset.z);
		point.x = x;
		point.y = y;
	}
}

void centre_3d(Object &obj)
{
		obj.properties.centre.x = FRAME_WIDE / 2;
		obj.properties.centre.y = FRAME_HIGH / 2;
		obj.properties.centre.z = 0;
		obj.properties.scale = 1;
}

void apply_translations(Point offset, std::vector<Object> &objects)
{
	for (int i = 0; i < objects.size(); i++)
	{
		if (!objects[i].properties.visible || objects[i].properties.fixed_location)
			continue;
		translate_3d(objects[i], offset);
	}
}

void apply_rotations(Rotation_offsets offset, std::vector<Object> &objects)
{
	for (int i = 0; i < objects.size(); i++) {
		if (!objects[i].properties.visible || objects[i].properties.fixed_orientation)
			continue;
		rotate_object(objects[i], offset);
	}
}

void apply_scale(double offset, std::vector<Object> &objects)
{
	for (int i = 0; i < objects.size(); i++) {
		if (!objects[i].properties.visible || objects[i].properties.fixed_scale)
			continue;
		scale_3d(objects[i], offset);
	}
}

void apply_centre(std::vector<Object> &objects)
{
	for (int i = 0; i < objects.size(); i++) {
		if (!objects[i].properties.visible || objects[i].properties.fixed_location)
			continue;
		centre_3d(objects[i]);
	}
}

void draw_objects(BYTE *fBuffer, std::vector<Object> &objects)
{
	for (int i = 0; i < objects.size(); i++) {
		if (!objects[i].properties.visible)
			continue;
		draw_object_3d(objects[i], fBuffer);
	}
}

/**
 * compute_surface_normals(const Object &obj) 
 * -- Description
 * Computes the areas of each surface in the given Object using the 'shoelace method'
 * The sign of the area is used to detect the orientation of a plane for backface culling
 * -- Arguments
 * An Object for which to calculate the normals
 * -- Return values
 * An vector containing surface normals for each polygon in the object
 */
Object_norms compute_surface_normals(const Object &obj) // For backface culling
{
	std::vector<Polygon> surfaces;
	project_polygon(obj, surfaces);
	Object_norms surface_normals;
	for (int i = 0; i < surfaces.size(); i++) {
		double N = polygon_area(surfaces[i]);
		surface_normals.push_back(N);
	}
	return surface_normals;
}

long polygon_area(Polygon poly) // shoelace method
{
	long  area = 0;
	for (int i = 0; i < poly.size(); i++) {
		if (i == poly.size() -1) {
			area += poly[i].x * poly[0].y;
			area -= poly[i].y * poly[0].x;
		}
		else {
			area += poly[i].x * poly[i + 1].y;
			area -= poly[i].y * poly[i + 1].x;
		}
	}
	return area;
}