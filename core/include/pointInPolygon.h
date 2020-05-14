#ifndef PIP_H
#define PIP_H

#include "helpers.h"

bool is_point_inside_triangle(Eigen::Vector2d p, Eigen::Vector2d a, Eigen::Vector2d b, Eigen::Vector2d c);
bool is_point_inside_circle(Eigen::Vector2f p, Eigen::Vector2f c, double r);
bool is_selected_rect_inside_base();

#endif