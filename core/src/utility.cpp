// Linear Algebra Library
#include <Eigen/Dense>

double const EPSILON = 0.0000001;

double get_area(double ax, double ay, double bx, double by, double cx, double cy){
    double area_of_abc = std::abs((ax*(by-cy) + bx*(cy-ay) + cx*(ay-by))/2);
	return area_of_abc;
}


bool is_point_inside_triangle(Eigen::Vector2d p, Eigen::Vector2d a, Eigen::Vector2d b, Eigen::Vector2d c) {
    double area_of_abc = get_area(a.x(), a.y(), b.x(), b.y(), c.x(), c.y());
	double area_of_pab = get_area(p.x(), p.y(), a.x(), a.y(), b.x(), b.y());
	double area_of_pbc = get_area(p.x(), p.y(), b.x(), b.y(), c.x(), c.y());
	double area_of_pac = get_area(p.x(), p.y(), a.x(), a.y(), c.x(), c.y());
	double sum_of_area = area_of_pab + area_of_pbc + area_of_pac;
	double diff = std::abs(sum_of_area - area_of_abc);
	return diff < EPSILON;
}