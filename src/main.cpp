/*`
 *   Copyright 2017 Toby Liu
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <cmath>
#include <thread>
#include <chrono>

typedef struct {
    float pos[3];
    float color[3];
} tVertex;

char *readShader(const char *filename, int &bufSize)
{
    char *buf = NULL;

    std::ifstream infile(filename, std::ifstream::ate);
    if (infile)
    {
        bufSize = infile.tellg();

        buf = new char[bufSize + 1];
        infile.clear();
        infile.seekg(0, std::ifstream::beg);
        infile.read(buf, bufSize);
        buf[bufSize] = '\0';

        infile.close();
    }
    return buf;
}

GLFWwindow* initWindow(int w, int h, const char *title)
{
    GLFWwindow* window;

    if (!glfwInit())
        return NULL;

    window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    return window;
}

GLuint loadAndCompileShader(const char *filename, GLenum shaderType)
{
    char *shaderBuf = NULL;
    int shaderSize = 0;
    GLuint shader;
    int isCompiled;

    shaderBuf = readShader(filename, shaderSize);
    if (shaderBuf == NULL || shaderSize == 0)
    {
        printf("[Error] Cannot load shader file %s !\n", filename);
        return 0;
    }

    shader = glCreateShader(shaderType);
    if (shader == 0)
    {
        printf("[Error] Cannot create shader %d !\n", (int)shaderType);
        return 0;
    }

    glShaderSource(shader, 1, (const GLchar**)&shaderBuf, 0);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint logLength;
        char *infoLog;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        infoLog = (char *)malloc(logLength + 1);
        glGetShaderInfoLog(shader, logLength, &logLength, infoLog);

        printf("[Error] Shader log: %s\n", infoLog);

        free(infoLog);
        return 0;
    }

    return shader;
}

GLuint initOpenGLShaderProgram(void)
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint shaderProg;
    GLint isLinked;

    vertexShader = loadAndCompileShader("./src/simple.vert", GL_VERTEX_SHADER);
    if (vertexShader == 0)
        return 0;

    fragmentShader = loadAndCompileShader("./src/simple.frag", GL_FRAGMENT_SHADER);
    if (fragmentShader == 0)
        return 0;

    shaderProg = glCreateProgram();
    if (shaderProg == 0)
    {
        printf("[Error] Cannot create shader program !\n");
        return 0;
    }

    glAttachShader(shaderProg, vertexShader);
    glAttachShader(shaderProg, fragmentShader);
    glLinkProgram(shaderProg);

    glGetProgramiv(shaderProg, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint logLength;
        char *infoLog;

        glGetProgramiv(shaderProg, GL_INFO_LOG_LENGTH, &logLength);
        infoLog = (char *)malloc(logLength + 1);
        glGetProgramInfoLog(shaderProg, logLength, &logLength, infoLog);

        printf("[Error] Program log: %s\n", infoLog);

        free(infoLog);
        return 0;
    }

    return shaderProg;
}

GLuint setOpenGLVertexBuffer(GLuint shaderProg, const tVertex *vertexArray, int arraySize)
{
    GLuint vertexBuffer;
    GLint pos_loc, color_loc;

    glGenBuffers(1, &vertexBuffer);
    if (vertexBuffer == GL_INVALID_VALUE)
    {
        printf("[Error] Cannot generate vertex buffer !\n");
        return GL_INVALID_VALUE;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, arraySize, vertexArray, GL_STATIC_DRAW);

    pos_loc = glGetAttribLocation(shaderProg, "position");
    if (pos_loc == -1 || pos_loc == GL_INVALID_OPERATION)
    {
        printf("[Error] Cannot get attribute %s location !\n", "position");
        return GL_INVALID_VALUE;
    }
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*) 0);

    color_loc = glGetAttribLocation(shaderProg, "color");
    if (color_loc == -1 || color_loc == GL_INVALID_OPERATION)
    {
        printf("[Error] Cannot get attribute %s location !\n", "color");
        return GL_INVALID_VALUE;
    }
    glEnableVertexAttribArray(color_loc);
    glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*) (sizeof(float) * 3));

    return vertexBuffer;
}

glm::mat4 genMVPMatrix(int frameWidth, int frameHeight,
                       const glm::vec3 &cameraPos, const glm::vec3 &modelTrans,
                       float modelRotRad, const glm::vec3 &modelRotAxis)
{
    glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), (float)frameWidth / (float)frameHeight, 1.0f, 100.0f);

    glm::mat4 viewMatrix = glm::lookAt(cameraPos, glm::vec3(0,0,0), glm::vec3(0,1,0));

    glm::mat4 modelMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), modelTrans), modelRotRad, modelRotAxis);

    glm::mat4 MVPMatrix = projMatrix * viewMatrix * modelMatrix;

    return MVPMatrix;
}

void run(GLFWwindow* window, GLuint shaderProg)
{
    tVertex vertexArray1[] = {
        {{ 1.0,  1.0, 0.0}, {1.0, 0.0, 0.0}},
        {{-1.0,  1.0, 0.0}, {0.0, 1.0, 0.0}},
        {{ 1.0, -1.0, 0.0}, {0.0, 0.0, 1.0}},
        {{-1.0, -1.0, 0.0}, {1.0, 0.0, 1.0}}
    };

    tVertex vertexArray2[] = {
        {{ 1.0,  0.0, 0.0}, {1.0, 0.0, 0.0}},
        {{ 0.0,  1.0, 0.0}, {0.0, 1.0, 0.0}},
        {{ 0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}},
        {{-1.0,  0.0, 0.0}, {1.0, 0.0, 1.0}}
    };

    bool useFirstVertexArray = true;
    float angle = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        int frameWidth, frameHeight;
        glfwGetFramebufferSize(window, &frameWidth, &frameHeight);

        glClearColor(0.0, 0.0, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, frameWidth, frameHeight);

        if (angle == 0.0)
        {
            setOpenGLVertexBuffer(shaderProg,
                                  (useFirstVertexArray)? vertexArray1: vertexArray2,
                                  (useFirstVertexArray)? sizeof(vertexArray1): sizeof(vertexArray2));
        }

        const glm::vec3 cameraPos = glm::vec3(0, 0, 5.0);
        const glm::vec3 modelRotAxis = glm::vec3(0, 1.0, 0);
        glm::vec3 modelTrans = glm::vec3(0.5 * std::cos(glm::radians(angle)),
                                         0.5 * std::sin(glm::radians(angle)),
                                         0.0);

        glm::mat4 MVPMatrix = genMVPMatrix(frameWidth, frameHeight,
                                           cameraPos, modelTrans,
                                           glm::radians(angle), modelRotAxis);

        GLuint mvp_loc = glGetUniformLocation(shaderProg, "MVPMatrix");
        if (mvp_loc == -1 || mvp_loc == GL_INVALID_OPERATION)
        {
            printf("[Error] Cannot get uniform %s location !\n", "MVPMatrix");
            break;
        }
        glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(MVPMatrix));

        glUseProgram(shaderProg);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);

        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        angle += 10.0;
        if (angle > 360.0)
        {
            useFirstVertexArray = !useFirstVertexArray;
            angle = 0.0;
        }
    }
}

void closeWindow()
{
    glfwTerminate();
}

int main(void)
{
    GLFWwindow* window;

    window = initWindow(640, 480, "First OpenGL Project");

    if (window != NULL)
    {
        GLuint shaderProg = initOpenGLShaderProgram();
        if (shaderProg)
        {
            run(window, shaderProg);
        }

        closeWindow();
    }
    return 0;
}