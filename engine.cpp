#include <cstdlib>			//- for exit()
#include <cstdio>			//- for sprintf()
#include <cstring>			//- for memset()
#include <ctime>			//- for random seed
#include <unistd.h>			//- for sleep()
#include <cstring>
#include "engine.hpp"

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

bool clip_test(double p, double q, double *u1, double *u2)
{
	double r = q / p;
	if (p < 0.0) { // outside -> inside
		if (r > *u2)
			return false;
		else if (r > *u1)
			*u1 = r;
	} else if (p > 0.0) { // inside -> outside
		if (r < *u1)
			return false;
		else if (r < *u2)
			*u2 = r;
	} else if (q < 0.0) {
		return false;
	}
	return true;
}

void clip_line(struct POINT2D p1, struct POINT2D p2, BYTE *fBuffer)
{
	double dx = p2.x - p1.x;
	double dy;
	double u1 = 0.0;
	double u2 = 1.0;
	int min_x = 0;
	int max_x = FRAME_WIDE - 1;
	int min_y = 0;
	int max_y = FRAME_HIGH - 1;
	if (clip_test(-dx, p1.x - min_x, &u1, &u2)) {
		if (clip_test(dx, max_x - p1.x, &u1, &u2)) {
			dy = p2.y - p1.y;
			if (clip_test(-dy, p1.y - min_y, &u1, &u2)) {
				if (clip_test(dy, max_y - p1.y, &u1, &u2)) {
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

void draw_tri(struct POINT2D p1, struct POINT2D p2, struct POINT2D p3, BYTE *fBuffer)
{
	clip_line(p1, p2, fBuffer);
	clip_line(p1, p3, fBuffer);
	clip_line(p2, p3, fBuffer);
}


void draw_poly(struct POINT2D **poly, int vertex_count, BYTE *fBuffer)
{
	if (vertex_count == 1) {
		SetPixel(*(poly[0]), fBuffer);
		return;
	}
	for (int a = 0, b = 1; b < vertex_count; a++, b++) {
		clip_line(*(poly[a]), *(poly[b]), fBuffer);
	}
	clip_line(*(poly[vertex_count - 1]), *(poly[0]), fBuffer);
}

struct POINT2D *x_min(struct POINT2D **arr, int len)
{
	struct POINT2D *min = arr[0];
	if (len == 1)
		return min;
	for (int i = 1; i < len; i++) {
		if (arr[i]->x < min->x)
			min = arr[i];
	}
	return min;
}

int compar(const void *p1, const void *p2) // Used for the quicksort
{
	return (*(struct POINT2D **)p1)->x - (*(struct POINT2D **)p2)->x;
}

void remove_point(struct POINT2D **arr, struct POINT2D **current, int *n)
{
	int index = current - arr;
	if (current != arr + (*n - 1))
		memcpy(current, current + 1, sizeof(struct POINT2D*) * (*n - (1 + index)));
	(*n)--;
}

struct POINT2D **find_point(struct POINT2D **neighbours, int neighbour_count, struct POINT2D *point)
{
	for (int i = 0; i < neighbour_count; i++) {
		if (point == neighbours[i])
			return neighbours + i;
	}
	return NULL;
}

void fill_poly(struct POINT2D **poly, int vertex_count, BYTE *fBuffer)
{
	struct POINT2D **neighbours = (struct POINT2D**)calloc(vertex_count, sizeof(struct POINT2D *));
	int neighbour_count = vertex_count;
	memcpy(neighbours, poly, sizeof(struct POINT2D*) * vertex_count);
	for (int i = 0; i < vertex_count; i++) {
		struct POINT2D **current = find_point(neighbours, neighbour_count, poly[i]);
		struct POINT2D **prev_adjacent = (*current == neighbours[0]) ? neighbours + neighbour_count - 1 : current - 1; // Wrap to end
		struct POINT2D **next_adjacent = (*current == neighbours[vertex_count - 1]) ? neighbours : current + 1; // Wrap to start
		struct POINT2D *tri[3] = {*current, *next_adjacent, *prev_adjacent};
		printf("%d: Current = (%d, %d)\n", i, (*current)->x, (*current)->y);
		if (!points_inside(tri, poly, vertex_count)
				&& convex(**current, **prev_adjacent, **next_adjacent)) {
			printf("%d: Drawing tri (%d, %d), (%d, %d), (%d, %d)\n",
					i, (*current)->x,
					(*current)->y,
					(*next_adjacent)->x,
					(*next_adjacent)->y,
					(*prev_adjacent)->x,
					(*prev_adjacent)->y);
			//draw_tri(*tri[0], *tri[1], *tri[2], fBuffer);
			fill_tri(tri, fBuffer);
			printf("%d: Removing node: (%d, %d) from neighbours\n", i, (*current)->x, (*current)->y);
			remove_point(neighbours, current, &neighbour_count);
		} else {
			printf("%d: Not drawing: inside=%d, concave=%d\n", i, points_inside(tri, poly, vertex_count), !convex(**current, **prev_adjacent, **next_adjacent));
		}
		printf("%d: Neighbours:\n", i);
		for (int j = 0; j < neighbour_count; j++) {
			printf("%d: (%d, %d)\n", j, neighbours[j]->x, neighbours[j]->y);
		}
	}
	//if (neighbour_count >= 3) // fixes missing triangle on the big complex poly
	//	fill_poly(neighbours, neighbour_count, fBuffer);
	free(neighbours);
	neighbours = NULL;
}

// Test to see if p0 is on the left/right side of p2 --> p1 edge.
bool convex(POINT2D p2, POINT2D p1, POINT2D p0)
{
    return ((p2.x - p1.x) * (p0.y - p1.y) - (p2.y - p1.y) * (p0.x - p1.x)) < 0;
}

void test_points_inside(BYTE *pFrame)
{
	struct POINT2D t1 = rand_point();
	struct POINT2D t2 = rand_point();
	struct POINT2D t3 = rand_point();
	struct POINT2D p  = rand_point();
	struct POINT2D *tri[3] = {&t1, &t2, &t3};
	//t1 = {100, 100, 255, 255, 255};
	//t2 = {200, 110, 255, 255, 255};
	//t3 = {200, 200, 255, 255, 255};
	p = {p.x, p.y, (BYTE)0, (BYTE)0, (BYTE)0}; //inside
	if (inside(tri, &p))
		p.g = (BYTE)255;
	else
		p.r = (BYTE)255;
	draw_tri(t1, t2, t3, pFrame);
	printf("(%d, %d): %d\n", p.x, p.y, inside(tri, &p));
	SetPixel(p, pFrame);
}

void test_same_side(BYTE *pFrame)
{
	struct POINT2D l1 = rand_point();
	struct POINT2D l2 = rand_point();
	struct POINT2D p1 = rand_point();
	struct POINT2D p2 = rand_point();
	p1 = {p1.x, p1.y, 0, 0, 0};
	p2 = {p2.x, p2.y, 0, 0, 0};
	if (same_side(p1, p2, l1, l2)) {
		p1.g = (BYTE)255;
		p2.g = (BYTE)255;
	} else {
		p1.r = (BYTE)255;
		p2.r = (BYTE)255;
	}
	clip_line(l1, l2, pFrame);
	SetPixel(p1, pFrame);
	SetPixel(p2, pFrame);
	printf("p1(%d, %d), p2(%d, %d), l1(%d, %d), l2(%d, %d)\n",
			p1.x, p1.y, p2.x, p2.y, l1.x, l1.y, l2.x, l2.y);
	printf(same_side(p1, p2, l1, l2) ? " green\n" : " red\n");
}

bool points_inside(struct POINT2D **tri, struct POINT2D **poly, int vertex_count)
{
	for (int i = 0; i < vertex_count; i++) {
		if (inside(tri, poly[i]))
			return true;
	}
	return false;
}

bool inside(struct POINT2D **tri, struct POINT2D *pt)
{
	return same_side(*pt, *tri[0], *tri[1], *tri[2])
			&& same_side(*pt, *tri[1], *tri[0], *tri[2])
			&& same_side(*pt, *tri[2], *tri[0], *tri[1]);
}

// Return true if points a and b are on the same side of line l1-l2
bool same_side(struct POINT2D a, struct POINT2D b, struct POINT2D l1, struct POINT2D l2)
{
	// l1 and l2 are the ends of the line
	// returns true if a & b are on the same side of line
	double apt = (a.x - l1.x) * (l2.y - l1.y) - (l2.x - l1.x) * (a.y - l1.y);
	double bpt = (b.x - l1.x) * (l2.y - l1.y) - (l2.x - l1.x) * (b.y - l1.y);
	return ((apt * bpt) > 0);
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
		//draw_line(*(triangle[0]), *(triangle[1]), fBuffer);
		//draw_line(*(triangle[1]), *(triangle[2]), fBuffer);
		clip_line(*(triangle[0]), *(triangle[1]), fBuffer);
		clip_line(*(triangle[1]), *(triangle[2]), fBuffer);
		return;
	}
	// Placeholder variables for maintaining current value after truncation
	double fromx = (double)triangle[0]->x;
	double tox   = (double)triangle[0]->x;
	double fromr = (double)triangle[0]->r;
	double fromg = (double)triangle[0]->g;
	double fromb = (double)triangle[0]->b;
	double tor   = (double)triangle[0]->r;
	double tog   = (double)triangle[0]->g;
	double tob   = (double)triangle[0]->b;
	
	// Gradients
	int a, b;
	if (triangle[0]->y == triangle[1]->y) { // Handle flat-topped triangles
		a = 2;
		b = 1;
		fromx = double(triangle[1]->x);
	} else {
		a = 1;
		b = 0;	}
	int from_dx = (triangle[a]->x - triangle[b]->x);
	int from_dy = (triangle[a]->y - triangle[b]->y);
	int from_dr = (triangle[a]->r - triangle[b]->r);
	int from_dg = (triangle[a]->g - triangle[b]->g);
	int from_db = (triangle[a]->b - triangle[b]->b);

	int to_dx = (triangle[2]->x - triangle[0]->x);
	int to_dy = (triangle[2]->y - triangle[0]->y);
	int to_dr = (triangle[2]->r - triangle[0]->r);
	int to_dg = (triangle[2]->g - triangle[0]->g);
	int to_db = (triangle[2]->b - triangle[0]->b);

	// Step (increment) values
	double from_inc = from_dx / (double)from_dy;
	double to_inc = to_dx / (double)to_dy;
	double from_r_inc = from_dr / (double)from_dy;
	double from_g_inc = from_dg / (double)from_dy;
	double from_b_inc = from_db / (double)from_dy;
	double to_r_inc = to_dr / (double)to_dy;
	double to_g_inc = to_dg / (double)to_dy;
	double to_b_inc = to_db / (double)to_dy; //TODO: Refactor

	for (int y = triangle[0]->y; y < triangle[2]->y; y++) {
		if (triangle[1]->y == y) {
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
		//draw_line({ROUND(fromx), y, (BYTE)fromr, (BYTE)(fromg), (BYTE)(fromb)}, {ROUND(tox), y, (BYTE)(tor), (BYTE)(tog), (BYTE)(tob)}, fBuffer);
		clip_line({ROUND(fromx), y, (BYTE)fromr, (BYTE)(fromg), (BYTE)(fromb)}, {ROUND(tox), y, (BYTE)(tor), (BYTE)(tog), (BYTE)(tob)}, fBuffer);
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
