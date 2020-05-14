// Based on the tutorial at https://open.gl
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
// OpenGL Helpers to reduce the clutter
#include <helpers.h>
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
// Linear Algebra Library
#include <Eigen/Dense>
// Timer
#include <chrono>
// Loading Textures
#include "../include/texture.h"
// Determine if a point is inside a triangle/rectangle
#include "../include/pointInPolygon.h"
// Translate/rotate rectangles
#include "../include/transformation.h"
////////////////////////////////////////////////////////////////////////////////

int WIDTH = 640;
int HEIGHT = 640;

// Viewport transformation/resize
Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_tex;
VertexBufferObject VBO_groups;

// Info about number of textures
uint const NUM_OF_RECTS = 9;
uint const NUM_OF_FACE_PIECES = 7;
uint unfinalized_faces = NUM_OF_FACE_PIECES;

// Contains the vertex positions, texture coordinates, and which texture # the texture belongs 
Eigen::MatrixXf V(2,6*NUM_OF_RECTS);
Eigen::MatrixXf TC(2,6*NUM_OF_RECTS);
Eigen::MatrixXf G(1,6*NUM_OF_RECTS);

// Selection
bool rect_is_selected = false;
uint index_of_selected_rect;
Eigen::Vector2d click;

// Stores the previous position of the cursor
double last_cursor_pos_x = 0.;
double last_cursor_pos_y = 0.;

// Game status
int game_status;
uint const INSTRUCTIONS_SCREEN = 0;
uint const STARTED = 1;
uint const FINISHED = 2;

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
	Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
	Eigen::Vector4f p_world = view.inverse()*p_canonical;

	xworld = (double) p_world[0];
	yworld = (double) p_world[1];

	// xworld = ((xpos/double(width))*2)-1;
	// yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw
}

// Finds the index of the first vertex of the rect the user selects
void find_selected_rect() {
	assert((V.cols() % 6 == 0) && (rect_is_selected == false));
	
	int index_of_last_unfinalized_face = 6*unfinalized_faces;
	int i = index_of_last_unfinalized_face; // start from the last NONFINALIZED rect

	// Determine selected rect by checking intersection for each triangle
	for (int t=0; t < unfinalized_faces; t++) {
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
			index_of_selected_rect = i;
			return;
		}
		i = i - 6;
	}

	rect_is_selected = false;
}

// Removes columns from given matrix to be appended at the end
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

void reset_game() {
	game_status = STARTED;
	unfinalized_faces = NUM_OF_FACE_PIECES;
	rect_is_selected = false;
	destroy_all_textures();
	load_all_textures();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (game_status == FINISHED) return;
	
	// Get the position of the mouse in the window
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	double xworld;
	double yworld;
	screen_to_world(window, xpos, ypos, xworld, yworld);

	// Update the position of the first vertex if the left button is pressed
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (game_status == INSTRUCTIONS_SCREEN) {
			game_status = STARTED;
			return;
		}

		click.x() = xworld;
		click.y() = yworld;

		if (rect_is_selected) {
			// The user finalized the selected piece
			if (is_selected_rect_inside_base()) {
				remove_then_append_columns(V, index_of_selected_rect, 6);
				remove_then_append_columns(TC, index_of_selected_rect, 6);
				remove_then_append_columns(G, index_of_selected_rect, 6);

				unfinalized_faces--;
			}
			// The user clicked outside any face
			rect_is_selected = false;
		} else {
			find_selected_rect();
		}
	}

	// Upload the change to the GPU
	VBO.update(V);
	VBO_tex.update(TC);
	VBO_groups.update(G);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {	// Avoid updating twice, as function is called 
	switch (key) {
		case GLFW_KEY_J:
			if (rect_is_selected)
				rotate_selected_rect(20, true);
			break;
		case GLFW_KEY_K:
			if (rect_is_selected)
				rotate_selected_rect(20, false);
			break;
		case GLFW_KEY_R:
			reset_game();
			break;
	}
	}

	// Upload the change to the GPU
	VBO.update(V);
	VBO_tex.update(TC);
	VBO_groups.update(G);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	double xpos_world, ypos_world;
	screen_to_world(window, xpos, ypos, xpos_world, ypos_world);

	if (rect_is_selected) {
		// Bind selected piece to cursor
		Eigen::Vector2f dis;
		dis << (float) xpos_world - (float) last_cursor_pos_x, (float) ypos_world - (float) last_cursor_pos_y;
		translate_rect(index_of_selected_rect, dis);
	}

	last_cursor_pos_x = xpos_world;
	last_cursor_pos_y = ypos_world;

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
	GLFWwindow * window = glfwCreateWindow(WIDTH, HEIGHT, "Picture Match", NULL, NULL);
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

		uniform mat4 view;

		void main() {
			gl_Position = view * vec4(position, 0.0, 1.0);
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
		uniform sampler2D tex8;

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
				case 8:
					outColor = texture(tex8, Texcoord);
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
	glUniform1i(program.uniform("tex8"), 8);
	
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {
		// Set the size of the viewport (canvas) to the size of the application window (framebuffer)
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// Get the size of the window (may be different than the canvas size on retina displays)
		int width_window, height_window;
		glfwGetWindowSize(window, &width_window, &height_window);

		WIDTH = width_window;
		HEIGHT = height_window;

		// Bind your VAO (not necessary if you have only one)
		VAO.bind();

		// Bind your program
		program.bind();

		// Clear the framebuffer
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Find the adjusted aspect ratio of the window
		float ar = (float) height_window / width_window;
		view << ar, 0, 0, 0,
				 0, 1, 0, 0,
				 0, 0, 1, 0,
				 0, 0, 0, 1;
		glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, view.data());

		switch (game_status) {
			case INSTRUCTIONS_SCREEN: {
				int index_of_instr = 6*(NUM_OF_FACE_PIECES+1); // instr is always the rect after all the face pieces
				glDrawArrays(GL_TRIANGLES, index_of_instr, 6);
				break;
			} case STARTED: {
				if (unfinalized_faces == 0) {
					game_status = FINISHED;
				} else {
					glDrawArrays(GL_TRIANGLES, 0, 6*(unfinalized_faces+1)); // add 1, to draw the base face
				}
				break;
			} case FINISHED: {
				// Display base face
				glDrawArrays(GL_TRIANGLES, 0, 6);

				// Display finalized face pieces
				int index_of_instr = 6; // when all has been finalized, instructions page is the second texture
				int index_of_first_face = index_of_instr + 6;
				glDrawArrays(GL_TRIANGLES, index_of_first_face, 6*(NUM_OF_FACE_PIECES));
				break;
			}
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