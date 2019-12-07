#include "texture.h"

// OpenGL Helpers to reduce the clutter
#include "helpers.h"

// STB Image Loading Library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
extern int WIDTH;
extern int HEIGHT;

extern Eigen::MatrixXf V;
extern Eigen::MatrixXf TC;
extern Eigen::MatrixXf G;

void intialize_base_values() {
	Eigen::MatrixXf V_0(2,6);
	Eigen::MatrixXf TC_0(2,6);
	Eigen::MatrixXf G_0(1,6);

	V_0 << -0.5, 0.5, -0.5, 0.5, 0.5, -0.5,
	       -0.5, -0.5, 0.5, -0.5, 0.5, 0.5;
	TC_0 << 0, 1, 0, 1, 1, 0,
		    0, 0, 1, 0, 1, 1;
	G_0 << 0, 0, 0, 0, 0, 0;

	V.block(0, 0, 2, 6) = V_0;
	TC.block(0, 0, 2, 6) = TC_0;
	G.block(0, 0, 1, 6) = G_0;
}

int generate_random_number(int left_bot, int left_top, int right_bot, int right_top) {
	int left_range_length = left_top - left_bot;
	int right_range_length = right_top - right_bot;
	int total_length_of_range = left_range_length + right_range_length;
	int random_num = std::rand() % total_length_of_range;
	if (random_num < left_range_length) {
		return left_bot + random_num;
	} else {
		return right_bot + (random_num - left_range_length);
	}
}

void append_texture_vertices(int width, int height, int index_of_start_col) {
	Eigen::MatrixXf to_append(2,6);
	double ar_width = ((double) width/WIDTH)/2;
	double ar_height = ((double) height/HEIGHT)/2;
	double offset_x = generate_random_number(-9, -4, 4, 9)/10.0;
	double offset_y = generate_random_number(-9, -4, 4, 9)/10.0;
	to_append << 0 + offset_x, ar_width + offset_x,         0 + offset_x, ar_width + offset_x,  ar_width + offset_x,         0 + offset_x,
				 0 + offset_y,        0 + offset_y, ar_height + offset_y, 	     0 + offset_y, ar_height + offset_y, ar_height + offset_y;
	V.block(0,index_of_start_col, 2, 6) = to_append;
}

void append_texture_coordinates(int index_of_start_col) {
	Eigen::MatrixXf to_append(2,6);
	to_append << 0, 1, 0, 1, 1, 0,
					0, 0, 1, 0, 1, 1;
	TC.block(0,index_of_start_col, 2, 6) = to_append;
}

void append_groups(int texture_num, int index_of_start_col) {
	Eigen::MatrixXf to_append(1,6);
	to_append << texture_num, texture_num, texture_num, texture_num, texture_num, texture_num;
	G.block(0,index_of_start_col, 1, 6) = to_append;
}

void load_all_textures() {
	extern VertexBufferObject VBO;
	extern VertexBufferObject VBO_tex;
	extern VertexBufferObject VBO_groups;
	extern uint unfinalized_rects;

	const uint NUM_OF_RECTS = unfinalized_rects;

	std::vector<char *> img_filenames = { (char*)"images/base.png", 
		(char*)"images/mouth.png", 
		(char*)"images/left_eye.png",
		(char*)"images/right_eye.png",
		(char*)"images/mustache.png",
		(char*)"images/nose.png",
		(char*)"images/left_eyebrow.png",
		(char*)"images/right_eyebrow.png"
	};
	std::vector<GLenum> texture_array = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2,
		GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7
	};

	// Flip images when loaded
	stbi_set_flip_vertically_on_load(true);

	/* Load images into GPU memory */
	GLuint tex[NUM_OF_RECTS];
	glGenTextures(NUM_OF_RECTS, tex);
	int width, height, nrChannels;
	unsigned char *data;

	/* Base image */
	glActiveTexture(texture_array[0]);
	// Enable Transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	// Texture Wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Load to CPU memory
	data = stbi_load(img_filenames[0], &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	intialize_base_values();

	/* Rest of the images */
	for (int t=1; t < NUM_OF_RECTS; t++) {
		// Activate texture unit before binding texture
		glActiveTexture(texture_array[t]);
		
		// Enable Transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Bind texture to apply subsequent operations
		glBindTexture(GL_TEXTURE_2D, tex[t]);

		// Texture Wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Load to CPU memory
		data = stbi_load(img_filenames[t], &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		int index_of_start_col = t*6;

		// Append to V
		append_texture_vertices(width, height, index_of_start_col);

		// Append to TC
		append_texture_coordinates(index_of_start_col);

		// Append to G
		append_groups(t, index_of_start_col);
	}

	VBO.update(V);
	VBO_tex.update(TC);
	VBO_groups.update(G);
}