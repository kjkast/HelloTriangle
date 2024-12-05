#include <SDL.h>
#include <iostream>
#include <glad/glad.h>

// local variables

// screen dimensions
constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

// GLSL shader code - just passes values through and typecasts it to vec4
// ** could be loaded from .txt files
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n" // init and declares vec3 aPos as input
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" // outputs position data
"}";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n" // outputs a color
"void main()\n"
"{\n"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}";

// accessible anywhere in file
static SDL_Window* window = NULL;
static SDL_GLContext context = NULL;

void Init() {
    std::cout << "Hello World!";
    SDL_Init(SDL_INIT_VIDEO); // initialize SDL

    // set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // set OpenGL profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // set OpenGL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // assign window
    window = SDL_CreateWindow("Hello Triangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        std::cout << "Failed to create SDL window" << std::endl;
        SDL_Quit();
        return;
    }

    // create context
    context = SDL_GL_CreateContext(window);

    // initialize glad
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        SDL_Quit();
        return;
    }

    // std::cout << glGetString(GL_VERSION) << std::endl; // print OPENGL version
}

void Render()
{
    // clear the screen (unless you want to keep the results from the previous frame)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Update() {
    SDL_GL_SwapWindow(window);
}

void Quit()
{
    std::cout << "Goodbye World!";
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// TODO: Connect to SDL so the function is automatically called when the window size is updated 
// otherwise, manually call it each size change
// to adjust OpenGL viewport when the size of the window is changed
void framebuffer_size_callback(SDL_Window* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char* argv[])
{
    // initialize SDL and OpenGL
    Init();

    // tell OpenGL the size of the rendering window
    glViewport(0, 0, WIDTH, HEIGHT);

    // back-face culling optimization
    glEnable(GL_CULL_FACE); // enables cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // declares what the front face is (clockwise polygons)

    // build and compile shader program

    // create a shader object referenced by an ID 
    unsigned int vertexShader{};
    vertexShader = glCreateShader(GL_VERTEX_SHADER); // shader type as arg

    // attach shader source code to shader object and compile
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // error-checking if shader compiled
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // ** coloring each fragment
    unsigned int fragmentShader{};
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // init shader program - final linked program of multiple shaders combined
    // ** puts into a single executable GPU shader program
    unsigned int shaderProgram{};
    shaderProgram = glCreateProgram();
    
    // link program to the shaders
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // link shaders to each other
    glLinkProgram(shaderProgram);

    // error-checking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // every shading and rendering call now uses shaderProgram
    glUseProgram(shaderProgram);

    // no longer need the shader objects 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // set up vertex data and buffers and configure vertex attributes

    // three vertices for a triangle
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // z at 0 to make it look 2D (depth remains same)
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    // TODO: WHY?
    // current understanding - Use VAO to draw multiple objects
    // instead of repeating configuration for new ones
    // ** in old GL you had to bind a new buffer object every time you draw the mesh
    // this stops that
    unsigned int VAO{};
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // put it in focus in GL's state machine

    // initializing vertex buffer object to send as much data as possible 
    // to the GPU memory at once for efficiency and to be used by shaders
    // ** generate empty buffer and set as current buffer
    unsigned int VBO{}; // creates positive vertex buffer object id // ** a simple array of data
    glGenBuffers(1, &VBO); // generates one buffer name and stores it in VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // binds new buffer to the GL_ARRAY_BUFFER target
    // ** ^ sits in GPU memory

    // copies vertex data into buffer's memory
    // first arg - buffer type, second - data size (in bytes)
    // third - the data, fourth - how the GPU manages the data
    // fourth arg can be:
    // GL_STREAM_DRAW (data set once, used by GPU few times)
    // GL_STATIC_DRAW (data set once, used many times)
    // GL_DYNAMIC_DRAW (data set many times, used many times)
    // The vertices will not change and are used alot, thus GL_STATIC_DRAW
    // ** copies points into currently bound buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // tells OpenGl how to interpret the vertex data in the memory
    // first arg - pass data to the vertex attribute in location 0, second - size of the vertex attribute (vec3)
    // third - type of data (float), fifth - distance to next vertex attribute (3 floats), 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // ** points to data and defines its layout
    glEnableVertexAttribArray(0); // ** enables attriubte in location 0

    // main loop
    bool running{ true };
    while (running) {
        Render(); // ** wipe surface clear

        // SDL Input Events
        SDL_Event event{};
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
                // quit window
                case SDL_QUIT:
                    running = false;
                    break;
                
                // input events
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        // quit window if escape key pressed
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                    }
                    break;
            }
        }

        // draw code
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3); // primitive type, starting index of vertex array, # vertices 

        Update(); // ** displays changes
    }
    
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // exit
    Quit();
    return 0;
}
