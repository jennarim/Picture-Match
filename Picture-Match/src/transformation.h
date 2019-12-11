#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <Eigen/Dense>

void translate_rect(int index_of_rect, Eigen::Vector2f dis);
void rotate_selected_rect(double degree, bool clockwise);

#endif