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


/*==================== Data Structure Initialisation and Cleanup ==================== */

/*======================= Drawing Functions ================================= */
void draw_pixel_2d(const Point_2d &point, BYTE *fBuffer)
{
	if (point.x < 0 || point.x > FRAME_WIDE - 1 || point.y < 0 || point.y > FRAME_HIGH - 1) {
		printf("Warning: point falls out of bounds!\n");
		return;
	}
	fBuffer[3 * (point.y * FRAME_WIDE + point.x)] = point.r; // R
	fBuffer[3 * (point.y * FRAME_WIDE + point.x) + 1] = point.g; // G
	fBuffer[3 * (point.y * FRAME_WIDE + point.x) + 2] = point.b; // B
}

void draw_pixel_3d(const Point_3d &p, BYTE *fBuffer)
{
	Point_2d point = {(int)(p.x * X_OFFSET / (double)(p.z + X_OFFSET)),
				(int)(p.y * Y_OFFSET / (double)(p.z + Y_OFFSET)),
				p.r, p.g, p.b};
	draw_pixel_2d(point, fBuffer);
}

Point_2d project_point(const Point_3d &p3d)
{
	Point_2d p2d = {
		(int)((p3d.x - X_OFFSET) * PERSPECTIVE / (double)(p3d.z + PERSPECTIVE)) + X_OFFSET,
		(int)((p3d.y - Y_OFFSET) * PERSPECTIVE / (double)(p3d.z + PERSPECTIVE)) + Y_OFFSET,
		p3d.r,
		p3d.g,
		p3d.b
	};
	return p2d;
}

void project_polygon(const Object &obj, std::vector<Polygon_2d> &projected_polys)
{
	for (int i = 0; i < obj.poly_count; i++) { // For every polygon in the object...
		Polygon_3d current_poly = obj.polys[i];
		Polygon_2d projected_poly;
		for (int j = 0; j < current_poly.size(); j++) { // For every point in the current polygon
			projected_poly.push_back(project_point(obj.vertices[current_poly[j]]));
		}
		projected_polys.push_back(projected_poly);
	}
}

void draw_line(Point_2d p1, Point_2d p2, BYTE *fBuffer)
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
	draw_pixel_2d(p1, fBuffer);
	for (int i = 0; i < steps; i++) {
		x += x_inc;
		y += y_inc;
		r += step_r;
		g += step_g;
		b += step_b;
		p1 = {ROUND(x), ROUND(y), (BYTE)r, (BYTE)g, (BYTE)b};
		draw_pixel_2d(p1, fBuffer);
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

void clip_line(Point_2d p1, Point_2d p2, BYTE *fBuffer)
{
	double dx = p2.x - p1.x;
	double dy;
	double u1 = 0.0;
	double u2 = 1.0;
	int min_x = 0;
	int max_x = FRAME_WIDE - 1;
	int min_y = 0;
	int max_y = FRAME_HIGH - 1;
	if (clip_test(-dx, p1.x - min_x, u1, u2)) {
		if (clip_test(dx, max_x - p1.x, u1, u2)) {
			dy = p2.y - p1.y;
			if (clip_test(-dy, p1.y - min_y, u1, u2)) {
				if (clip_test(dy, max_y - p1.y, u1, u2)) {
					if (u2 < 1) {
						p2.x = p1.x + u2 * dx;
						p2.y = p1.y + u2 * dy;
					}
					if (u1 > 0) {
						p1.x += u1 * dx;
						p1.y += u1 * dy;
					}
					draw_line(p1, p2, fBuffer);
				}
			}
		}
	}
}


// If tri contains more than 3 points, the excess will be ignored
void draw_tri(const Polygon_2d &tri, BYTE *fBuffer)
{
	clip_line(tri[0], tri[1], fBuffer);
	clip_line(tri[0], tri[2], fBuffer);
	clip_line(tri[1], tri[2], fBuffer);
}

void draw_poly(const Polygon_2d &poly, BYTE *fBuffer)
{
	int vc = poly.size();
	if (vc == 1) {
		draw_pixel_2d(poly[0], fBuffer);
		return;
	}
	for (int a = 0, b = 1; b < vc; a++, b++) {
		clip_line(poly[a], poly[b], fBuffer);
	}
	clip_line(poly[vc - 1], poly[0], fBuffer);
}

void draw_object_3d(const Object &obj, BYTE *fBuffer)
{
	std::vector<Polygon_2d> projected_polys;
	project_polygon(obj, projected_polys); // Populates projected_polys
	for (int i = 0; i < projected_polys.size(); i++) {
		fill_poly(projected_polys[i], fBuffer);
	}
}

void draw_wireframe_3d(const Object &obj, BYTE *fBuffer)
{
	std::vector<Polygon_2d> polys;
	project_polygon(obj, polys);
	for (int i = 0; i < obj.poly_count; i++) {
		for (int j = 0; j < polys[i].size(); j++) {
			polys[i][j].r = 255;
			polys[i][j].g = 255;
			polys[i][j].b = 255;
		}
		draw_poly(polys[i], fBuffer);
	}
}

// Check whether two points are identical in location and colour
bool point_cmp(const Point_2d &a, const Point_2d &b)
{
	return (
		   a.x == b.x
		&& a.y == b.y
		&& a.r == b.r
		&& a.g == b.g
		&& a.b == b.b
	);
}

int find_point(const Polygon_2d &neighbours, const Point_2d &point) // NOTE: Assumes that points are unique
{
	for (int i = 0; i < neighbours.size(); i++) {
		if (point_cmp(point, neighbours[i]))
			return i;
	}
	return -1;
}

void fill_poly(const Polygon_2d &poly, BYTE *fBuffer)
{
	Polygon_2d neighbours = poly;
	int vc = poly.size();
	for (int i = 0; i < vc; i++) {
		int current = find_point(neighbours, poly[i]);
		int next_adjacent = current == neighbours.size() - 1 ? 0 : current + 1;
		int prev_adjacent = current == 0 ? neighbours.size() - 1 : current - 1;
		Polygon_2d tri = {
			neighbours[current],
			neighbours[next_adjacent],
			neighbours[prev_adjacent]
		};
		if (!points_inside(tri, poly)
				&& convex(neighbours[current], neighbours[prev_adjacent], neighbours[next_adjacent])) {
			fill_tri(tri, fBuffer);
			//draw_tri(tri, fBuffer);
			neighbours.erase(neighbours.begin()+current);
		}
	}
	//if (neighbours.size() >= 3) // fixes missing triangle on the big complex poly
	//	fill_poly(neighbours, fBuffer);
}

// Test to see if p0 is on the left/right side of p2 --> p1 edge.
bool convex(const Point_2d &p2, const Point_2d &p1, const Point_2d &p0)
{
    return ((p2.x - p1.x) * (p0.y - p1.y) - (p2.y - p1.y) * (p0.x - p1.x)) < 0;
}

bool points_inside(const Polygon_2d &tri, const Polygon_2d &poly)
{
	for (int i = 0; i < poly.size(); i++) {
		if (inside(tri, poly[i]))
			return true;
	}
	return false;
}

bool inside(const Polygon_2d &tri, const Point_2d &pt)
{
	return same_side(pt, tri[0], tri[1], tri[2])
			&& same_side(pt, tri[1], tri[0], tri[2])
			&& same_side(pt, tri[2], tri[0], tri[1]);
}

// l1 and l2 are the ends of the line
// returns true if a & b are on the same side of line
bool same_side(const Point_2d &a, const Point_2d &b, const Point_2d &l1, const Point_2d &l2)
{
	double apt = (a.x - l1.x) * (l2.y - l1.y) - (l2.x - l1.x) * (a.y - l1.y);
	double bpt = (b.x - l1.x) * (l2.y - l1.y) - (l2.x - l1.x) * (b.y - l1.y);
	return ((apt * bpt) > 0);
}

bool collinear(const Polygon_2d &tri)
{
	double grad = (double)(tri[1].y - tri[0].y) / (double)(tri[1].x - tri[0].x);
	if ((double)(tri[2].y - tri[1].y) / (double)(tri[2].x - tri[1].x) != grad)
		return false;
	if ((double)(tri[2].y - tri[0].y) / (double)(tri[2].x - tri[0].x) != grad)
		return false;
	return true;
}

//TODO: Refactor, get rid of all the local variables?
//TODO: Fix colour overflow bug
void fill_tri(Polygon_2d &triangle, BYTE *fBuffer)
{
	sort_vertices(triangle);
	if (collinear(triangle)) {
		clip_line(triangle[0], triangle[1], fBuffer);
		clip_line(triangle[1], triangle[2], fBuffer);
		return;
	}
	// Placeholder variables for maintaining current value after truncation
	double fromx = (double)triangle[0].x;
	double tox   = (double)triangle[0].x;
	double fromr = (double)triangle[0].r;
	double fromg = (double)triangle[0].g;
	double fromb = (double)triangle[0].b;
	double tor   = (double)triangle[0].r;
	double tog   = (double)triangle[0].g;
	double tob   = (double)triangle[0].b;
	
	// Gradients
	int a, b;
	if (triangle[0].y == triangle[1].y) { // Handle flat-topped triangles
		a = 2;
		b = 1;
		fromx = double(triangle[1].x);
	} else {
		a = 1;
		b = 0;	}
	int from_dx = (triangle[a].x - triangle[b].x);
	int from_dy = (triangle[a].y - triangle[b].y);
	int from_dr = (triangle[a].r - triangle[b].r);
	int from_dg = (triangle[a].g - triangle[b].g);
	int from_db = (triangle[a].b - triangle[b].b);

	int to_dx = (triangle[2].x - triangle[0].x);
	int to_dy = (triangle[2].y - triangle[0].y);
	int to_dr = (triangle[2].r - triangle[0].r);
	int to_dg = (triangle[2].g - triangle[0].g);
	int to_db = (triangle[2].b - triangle[0].b);

	// Step (increment) values
	double from_inc = from_dx / (double)from_dy;
	double to_inc = to_dx / (double)to_dy;
	double from_r_inc = from_dr / (double)from_dy;
	double from_g_inc = from_dg / (double)from_dy;
	double from_b_inc = from_db / (double)from_dy;
	double to_r_inc = to_dr / (double)to_dy;
	double to_g_inc = to_dg / (double)to_dy;
	double to_b_inc = to_db / (double)to_dy; //TODO: Refactor

	for (int y = triangle[0].y; y < triangle[2].y; y++) {
		if (triangle[1].y == y) {
			from_dx = triangle[2].x - triangle[1].x;
			from_dy = triangle[2].y - triangle[1].y;
			from_inc = from_dx / (double)from_dy;

			from_dr = triangle[2].r - triangle[1].r;
			from_dg = triangle[2].g - triangle[1].g;
			from_db = triangle[2].b - triangle[1].b;
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
		//draw_line({ROUND(fromx), y, (BYTE)fromr, (BYTE)(fromg), (BYTE)(fromb)}, {ROUND(tox), y, (BYTE)(tor), (BYTE)(tog), (BYTE)(tob)}, fBuffer);
		clip_line({ROUND(fromx), y, (BYTE)fromr, (BYTE)(fromg), (BYTE)(fromb)}, {ROUND(tox), y, (BYTE)(tor), (BYTE)(tog), (BYTE)(tob)}, fBuffer);
	}
}

Point_2d rand_point()
{
	Point_2d point;
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
void sort_vertices(Polygon_2d &triangle)
{
	Point_2d *temp = NULL;
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

Point_3d toks_to_p3d(std::vector<std::string> &toks)
{
	Point_3d p3d;
	p3d.x = atoi(toks[0].c_str());
	p3d.y = atoi(toks[1].c_str());
	p3d.z = atoi(toks[2].c_str());
	p3d.r = atoi(toks[3].c_str());
	p3d.g = atoi(toks[4].c_str());
	p3d.b = atoi(toks[5].c_str());
	return p3d;
}

Polygon_3d toks_to_poly3d(std::vector<std::string> &toks)
{
	Polygon_3d poly;
	for (int i = 1; i < toks.size(); i++) {
		poly.push_back(atoi(toks[i].c_str()));
	}
	return poly;
}

void load_vjs(std::string fpath, Object &obj)
{
	std::ifstream infile(fpath);
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
		Point_3d point = toks_to_p3d(toks);
		obj.vertices.push_back(point);
	}
	for (int i = 0; i < poly_count; i++) {
		std::getline(infile, linebuffer);
		toks = tokenize(linebuffer, ' ');
		Polygon_3d poly = toks_to_poly3d(toks);
		obj.polys.push_back(poly);
	}
}
