// three vertices for a triangle
float vertices[] = {
	-0.5f, -0.5f, 0.0f, // z at 0 to make it look 2D (depth remains same)
	0.5f, -0.5f, 0.0f,
	0.0f, 0.5f, 0.0f
};

unsigned int VBO{};
glGenBuffers(1, &VBO);