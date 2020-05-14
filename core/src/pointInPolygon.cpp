#include "../include/pointInPolygon.h"

// OpenGL Helpers to reduce the clutter
#include "../include/helpers.h"

double const EPSILON = 0.0000001;

// Returns area of triangle formed by 3 vertices
double get_area(double ax, double ay, double bx, double by, double cx, double cy) {
	double area_of_abc = std::abs((ax*(by-cy) + bx*(cy-ay) + cx*(ay-by))/2);
	return area_of_abc;
}

// Returns whether point p is inside vertices a, b, c
bool is_point_inside_triangle(Eigen::Vector2d p, Eigen::Vector2d a, Eigen::Vector2d b, Eigen::Vector2d c) {
	double area_of_abc = get_area(a.x(), a.y(), b.x(), b.y(), c.x(), c.y());
	double area_of_pab = get_area(p.x(), p.y(), a.x(), a.y(), b.x(), b.y());
	double area_of_pbc = get_area(p.x(), p.y(), b.x(), b.y(), c.x(), c.y());
	double area_of_pac = get_area(p.x(), p.y(), a.x(), a.y(), c.x(), c.y());
	double sum_of_area = area_of_pab + area_of_pbc + area_of_pac;
	double diff = std::abs(sum_of_area - area_of_abc);
	return diff < EPSILON;
}

bool is_point_inside_circle(Eigen::Vector2f p, Eigen::Vector2f c, double r) {
	double dist = std::sqrt( (p.x()-c.x())*(p.x()-c.x()) + (p.y()-c.y())*(p.y()-c.y()));
	return (dist < r);
}

bool is_selected_rect_inside_base() {
	// the hitbox of the base face is defined as a circle.
	// if area of rect is completely inside a circle.
	// = if each corner of rect is inside a circle
    extern bool rect_is_selected;
    extern int index_of_selected_rect;
    
	assert(rect_is_selected && index_of_selected_rect > 5);

    extern Eigen::MatrixXf V;

	Eigen::Vector2f center;
	center << 0,0;
	double radius = 0.5;
	for (int v=0; v < 6; v++) {
		Eigen::Vector2f vertex = V.col(index_of_selected_rect+v);
		if (!is_point_inside_circle(vertex, center, radius)) {
			return false;
		}
	}
	return true;
}