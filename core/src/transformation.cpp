#include "../include/transformation.h"
#include <iostream>

extern Eigen::MatrixXf V;

// Finds the center of the given rect
void get_center(uint index_of_rect, float &center_x, float &center_y) {
	float p1_x = V.col(index_of_rect).x();
	float p1_y = V.col(index_of_rect).y();

	float p2_x = V.col(index_of_rect+1).x();
	float p2_y = V.col(index_of_rect+1).y();

	float p3_x = V.col(index_of_rect+2).x();
	float p3_y = V.col(index_of_rect+2).y();
	
	float p4_x = V.col(index_of_rect+4).x();
	float p4_y = V.col(index_of_rect+4).y();

	center_x = (p1_x + p2_x + p3_x + p4_x) /4.0f;
	center_y = (p1_y + p2_y + p3_y + p4_y) /4.0f;
}

// Translates the given triangle
void translate_triangle(uint index_of_triangle, float dis_x, float dis_y) {
	uint v_1 = index_of_triangle;
	uint v_2 = index_of_triangle+1;
	uint v_3 = index_of_triangle+2;

	V.col(v_1).x() = V.col(v_1).x() - dis_x;
	V.col(v_1).y() = V.col(v_1).y() - dis_y;

	V.col(v_2).x() = V.col(v_2).x() - dis_x;
	V.col(v_2).y() = V.col(v_2).y() - dis_y;

	V.col(v_3).x() = V.col(v_3).x() - dis_x;
	V.col(v_3).y() = V.col(v_3).y() - dis_y;
}

// Rotates the given triangle
void rotate_triangle(int index_of_first_vertex, double angle, bool clockwise) {
	// Convert to radians
	double radians;
	if (clockwise)
		radians = (2*3.1415926535) - ((angle * 3.1415926535) / 180.0);
	else
		radians = (angle * 3.1415926535) / 180.0;
	
	// Directly editing positions of vertices CPU side
	uint v_1 = index_of_first_vertex;
	uint v_2 = index_of_first_vertex+1;
	uint v_3 = index_of_first_vertex+2;
	
	Eigen::MatrixXf R(2,2);
	R << (float) cos(radians), (float) -sin(radians),
		 (float) sin(radians), (float) cos(radians);

	V.col(v_1) = R * V.col(v_1);
	V.col(v_2) = R * V.col(v_2);
	V.col(v_3) = R * V.col(v_3);
}

// Rotates the given rectangle
void rotate_selected_rect(double degree, bool clockwise) {
    extern uint index_of_selected_rect;

	// Find displacement from barycenter to origin
	float dis_x;
	float dis_y;
	get_center(index_of_selected_rect, dis_x, dis_y);

	uint index_of_left_selected_triangle = index_of_selected_rect;
	uint index_of_right_selected_triangle = index_of_selected_rect+3;

	/* Rotate Left Triangle*/
	// Translate center of triangle to origin
	translate_triangle(index_of_left_selected_triangle, dis_x, dis_y);
    // Rotate triangle around origin
	rotate_triangle(index_of_left_selected_triangle, degree, clockwise);
    // Translate triangle back to original position
	translate_triangle(index_of_left_selected_triangle, -dis_x, -dis_y);

	/* Rotate Right Triangle*/
	// Translate each vertex to origin
	translate_triangle(index_of_right_selected_triangle, dis_x, dis_y);
    // Rotate triangle around origin
	rotate_triangle(index_of_right_selected_triangle, degree, clockwise);
    // Translate triangle back to original position
	translate_triangle(index_of_right_selected_triangle, -dis_x, -dis_y);
}

// Translates the given index of rect by the given displacement
void translate_rect(int index_of_rect, Eigen::Vector2f dis) {
	for (int i=index_of_rect; i < index_of_rect+6; i++) {
		V.col(i) = V.col(i) + dis;
	}
}


