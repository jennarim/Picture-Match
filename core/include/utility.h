#ifndef TEXTURE_H
#define TEXTURE_H

#include <Eigen/Dense>

double get_area(double ax, double ay, double bx, double by, double cx, double cy);

bool is_point_inside_triangle(Eigen::Vector2d p, Eigen::Vector2d a, Eigen::Vector2d b, Eigen::Vector2d c);

#endif

