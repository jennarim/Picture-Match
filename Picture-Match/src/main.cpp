// This example is heavily based on the tutorial at https://open.gl

////////////////////////////////////////////////////////////////////////////////
// OpenGL Helpers to reduce the clutter
#include "helpers.h"
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
// Linear Algebra Library
#include <Eigen/Dense>
// Timer
#include <chrono>

#include <iostream>

// #include "texture.h"
// STB Image Loading Library
// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

#include "texture.h"
#include "pointInPolygon.h"
////////////////////////////////////////////////////////////////////////////////

int WIDTH = 640;
int HEIGHT = 640;

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_tex;
VertexBufferObject VBO_groups;

uint const NUM_OF_RECTS = 8;
uint unfinalized_rects = NUM_OF_RECTS;

// Contains the vertex positions
Eigen::MatrixXf V(2,6*NUM_OF_RECTS);
Eigen::MatrixXf TC(2,6*NUM_OF_RECTS);
Eigen::MatrixXf G(1,6*NUM_OF_RECTS);

Eigen::Vector2d click;

// Stores the previous position of the cursor
double last_cursor_pos_x = 0.;
double last_cursor_pos_y = 0.;

bool rect_is_selected = false;
uint index_of_selected_rect;

// Converts screen coordinates to world coordinates
void screen_to_world(GLFWwindow * window, double xpos, double ypos, double &xworld, double &yworld) {
	// Get viewport size (canvas in number of pixels)
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// Get the size of the window (may be different than the canvas size on retina displays)
	int width_window, height_window;
	glfwGetWindowSize(window, &width_window, &height_window);

	// Deduce position of the mouse in the viewport
	double highdpi = (double) width / (double) width_window;
	xpos *= highdpi;
	ypos *= highdpi;

	// Convert screen position to world coordinates
	xworld = ((xpos/double(width))*2)-1;
	yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw
}

// Translates the given index of rect by the given displacement
void translate_rect(int index_of_rect, Eigen::Vector2f dis) {
	for (int i=index_of_rect; i < index_of_rect+6; i++) {
		V.col(i) = V.col(i) + dis;
	}
}

// Finds the index of the first vertex of the rect the user selects
void find_selected_rect() {
	assert((V.cols() % 6 == 0) && (rect_is_selected == false));
	// assert(V.cols() == 18);
	int i = V.cols() - 6; // start checking from the last rect
	for (int t=0; t < NUM_OF_RECTS-1; t++) { // the first rect, the base, can never be selected
		// First triangle
		Eigen::Vector2d a = V.col(i).cast<double>();
		Eigen::Vector2d b = V.col(i+1).cast<double>();
		Eigen::Vector2d c = V.col(i+2).cast<double>();
		// Second triangle
		Eigen::Vector2d d = V.col(i+3).cast<double>();
		Eigen::Vector2d e = V.col(i+4).cast<double>();
		Eigen::Vector2d f = V.col(i+5).cast<double>();

		if (is_point_inside_triangle(click, a,b,c) || is_point_inside_triangle(click, d,e,f)) {
			rect_is_selected = true;
			std::cout<<"inside " << i/6 <<std::endl;
			index_of_selected_rect = i;
			return;
		}
		i = i - 6;
	}
	rect_is_selected = false;
	std::cout<<"outside"<<std::endl;
}

void remove_then_append_columns(Eigen::MatrixXf &matrix, uint first_col_to_remove, int num_of_cols_to_remove) {
	uint col_to_remove = first_col_to_remove;
	uint col_replace = first_col_to_remove + num_of_cols_to_remove;

	Eigen::MatrixXf removed_cols(matrix.rows(), num_of_cols_to_remove);
	removed_cols = matrix.block(0, first_col_to_remove, matrix.rows(), num_of_cols_to_remove);

	while (col_replace < matrix.cols()) {
		matrix.col(col_to_remove++) = matrix.col(col_replace++);
	}

	matrix.rightCols(num_of_cols_to_remove) = removed_cols;
}

void test() {
	Eigen::MatrixXf matrix(2,12);
	matrix << 0,1,2,3,4,5,6,7,8,9,10,11,
			  0,1,2,3,4,5,6,7,8,9,10,11;
	// Eigen::MatrixXf removed_cols(2, 6);
	// removed_cols = matrix.block(0, 3, 1, 6);
	// std::cout<< removed_cols << std::endl;

	remove_then_append_columns(matrix, 3, 6);
	std::cout << matrix << std::endl;

}

// Sets the center of the given rect
void get_center(uint index_of_rect, float &center_x, float &center_y) {
	int minx = V.col(index_of_rect).x();
	int maxx = V.col(index_of_rect+1).x();
	float width = (float) (maxx-minx);

	int miny = V.col(index_of_rect).y();
	int maxy = V.col(index_of_rect+2).y();

	float height = (float) (maxy - miny);

	center_x = minx + width/2.0f;
	center_y = miny + height/2.0f;
}

// Translates the given triangle directly
void translate_triangle(uint index_of_triangle, float dis_x, float dis_y) {
	// Directly editing positions of vertices CPU side
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

void rotate_triangle(int index_of_first_vertex, int angle, bool clockwise) {
	double radians;
	if (clockwise)
		radians = (2*3.1415926535) - (angle * 3.1415926535 / 180.0);
	else
		radians = angle * 3.1415926535 / 180.0;
	
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

void rotate_selected_rect(int degree, bool clockwise) {
	
	uint index_of_left_selected_triangle = index_of_selected_rect;
	uint index_of_right_selected_triangle = index_of_selected_rect+3;

	// Find displacement from barycenter to origin
	float dis_x;
	float dis_y;

	get_center(index_of_selected_rect, dis_x, dis_y);

	/* Rotate Left Triangle*/
	// Translate each vertex to origin
	translate_triangle(index_of_left_selected_triangle, dis_x, dis_y);
	// Rotate 10 degrees
	rotate_triangle(index_of_left_selected_triangle, 10, true);
	// translate, rotate, translate left triangle
	translate_triangle(index_of_left_selected_triangle, -dis_x, -dis_y);

	/* Rotate Right Triangle*/
	// Translate each vertex to origin
	translate_triangle(index_of_right_selected_triangle, dis_x, dis_y);
	// Rotate 10 degrees
	rotate_triangle(index_of_right_selected_triangle, 10, true);
	// translate, rotate, translate right triangle
	translate_triangle(index_of_right_selected_triangle, -dis_x, -dis_y);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// Get viewport size (canvas in number of pixels)
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// Get the size of the window (may be different than the canvas size on retina displays)
	int width_window, height_window;
	glfwGetWindowSize(window, &width_window, &height_window);

	// Get the position of the mouse in the window
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Deduce position of the mouse in the viewport
	double highdpi = (double) width / (double) width_window;
	xpos *= highdpi;
	ypos *= highdpi;

	// Convert screen position to world coordinates
	double xworld = ((xpos/double(width))*2)-1;
	double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

	// Update the position of the first vertex if the left button is pressed
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// mouse_is_down = true;
		click.x() = xworld;
		click.y() = yworld;

		// If there is already a selected rectangle, then deselect it
		if (rect_is_selected) {
			if (is_selected_rect_inside_base()) {
				std::cout << "inside base" << std::endl;
				remove_then_append_columns(V, index_of_selected_rect, 6);
				remove_then_append_columns(TC, index_of_selected_rect, 6);
				remove_then_append_columns(G, index_of_selected_rect, 6);

				unfinalized_rects--;
			}
			rect_is_selected = false; // what if user selects another rect, while another rect was selected?
		} else {
			find_selected_rect();
		}
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		// mouse_is_down = false;
	}

	// Upload the change to the GPU
	VBO.update(V);
	VBO_tex.update(TC);
	VBO_groups.update(G);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Update the position of the first vertex if the keys 1,2, or 3 are pressed
	switch (key) {
		case GLFW_KEY_J:
			/*
				rotate selected triangle left
			*/
			if (rect_is_selected) {
				rotate_selected_rect(10, true);
			}
			break;
	}

	// Upload the change to the GPU
	VBO.update(V);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	double xpos_world, ypos_world;
	screen_to_world(window, xpos, ypos, xpos_world, ypos_world);

	Eigen::Vector2f dis;
	dis << (float) xpos_world - (float) last_cursor_pos_x, (float) ypos_world - (float) last_cursor_pos_y;

	last_cursor_pos_x = xpos_world;
	last_cursor_pos_y = ypos_world;

	if (rect_is_selected) {
		// std::cout << "selected rect: " << index_of_selected_rect/6 << std::endl;
		translate_rect(index_of_selected_rect, dis);
	}

	VBO.update(V);
}

int main(void) {
	// Initialize the GLFW library
	if (!glfwInit()) {
		return -1;
	}

	// Activate supersampling
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Ensure that we get at least a 3.2 context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create a windowed mode window and its OpenGL context
	GLFWwindow * window = glfwCreateWindow(WIDTH, HEIGHT, "[Float] Hello World", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Load OpenGL and its extensions
	if (!gladLoadGL()) {
		printf("Failed to load OpenGL and its extensions");
		return(-1);
	}
	printf("OpenGL Version %d.%d loaded", GLVersion.major, GLVersion.minor);

	int major, minor, rev;
	major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
	printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
	printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
	printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Initialize the VAO
	// A Vertex Array Object (or VAO) is an object that describes how the vertex
	// attributes are stored in a Vertex Buffer Object (or VBO). This means that
	// the VAO is not the actual object storing the vertex data,
	// but the descriptor of the vertex data.
	VertexArrayObject VAO;
	VAO.init();
	VAO.bind();

	check_gl_error();

	// Initialize the VBO with the vertices data
	// A VBO is a data container that lives in the GPU memory
	VBO.init();
	VBO_tex.init();
	VBO_groups.init();

	load_all_textures();

	// Initialize the OpenGL Program
	// A program controls the OpenGL pipeline and it must contains
	// at least a vertex shader and a fragment shader to be valid
	Program program;
	const GLchar* vertex_shader = R"(
		#version 150 core

		in vec2 position;
		in vec2 texcoord;

		in float group_num;
		out float Group_num;

		out vec2 Texcoord;

		void main() {
			gl_Position = vec4(position, 0.0, 1.0);
			Texcoord = texcoord;
			Group_num = group_num;
		}
	)";

	const GLchar* fragment_shader = R"(
		#version 150 core

		in vec2 Texcoord;
		in float Group_num;

		out vec4 outColor;

		uniform sampler2D tex0;
		uniform sampler2D tex1;
		uniform sampler2D tex2;
		uniform sampler2D tex3;
		uniform sampler2D tex4;
		uniform sampler2D tex5;
		uniform sampler2D tex6;
		uniform sampler2D tex7;

		void main()
		{
			int tex_group = int(Group_num);

			switch (tex_group) {
				case 0:
					outColor = texture(tex0, Texcoord);
					break;
				case 1:
					outColor = texture(tex1, Texcoord);
					break;
				case 2:
					outColor = texture(tex2, Texcoord);
					break;
				case 3:
					outColor = texture(tex3, Texcoord);
					break;
				case 4:
					outColor = texture(tex4, Texcoord);
					break;
				case 5:
					outColor = texture(tex5, Texcoord);
					break;
				case 6:
					outColor = texture(tex6, Texcoord);
					break;
				case 7:
					outColor = texture(tex7, Texcoord);
					break;
			}
		}
	)";

	// Compile the two shaders and upload the binary to the GPU
	// Note that we have to explicitly specify that the output "slot" called outColor
	// is the one that we want in the fragment buffer (and thus on screen)
	program.init(vertex_shader, fragment_shader, "outColor");
	program.bind();

	// The vertex shader wants the position of the vertices as an input.
	// The following line connects the VBO we defined above with the position "slot"
	// in the vertex shader
	program.bindVertexAttribArray("position", VBO);
	program.bindVertexAttribArray("texcoord", VBO_tex);
	program.bindVertexAttribArray("group_num", VBO_groups);

	// Register the keyboard callback
	glfwSetKeyCallback(window, key_callback);

	// Register the mouse callback
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Register the cursor callback
	glfwSetCursorPosCallback(window, cursor_position_callback);

	// Set uniforms
	glUniform1i(program.uniform("tex0"), 0);
	glUniform1i(program.uniform("tex1"), 1);
	glUniform1i(program.uniform("tex2"), 2);
	glUniform1i(program.uniform("tex3"), 3);
	glUniform1i(program.uniform("tex4"), 4);
	glUniform1i(program.uniform("tex5"), 5);
	glUniform1i(program.uniform("tex6"), 6);
	glUniform1i(program.uniform("tex7"), 7);
	

// test();
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {
		// Set the size of the viewport (canvas) to the size of the application window (framebuffer)
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// Bind your VAO (not necessary if you have only one)
		VAO.bind();

		// Bind your program
		program.bind();

		// Clear the framebuffer
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// test();


		bool face_finished = (unfinalized_rects == 1);
		if (face_finished) {
			std::cout << "FINISHED" << std::endl;
			glDrawArrays(GL_TRIANGLES, 0, 6*NUM_OF_RECTS);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, 6*unfinalized_rects);
		}		

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	// Deallocate opengl memory
	program.free();
	VAO.free();
	VBO.free();
	VBO_tex.free();
	VBO_groups.free();

	// Deallocate glfw internals
	glfwTerminate();
	return 0;
}