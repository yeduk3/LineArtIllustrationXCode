//
//  main.cpp
//  LineArtIllustrationXCode
//
//  Created by 이용규 on 3/7/25.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>

#include <iostream>
#include <thread>
#include <chrono>

#include "myprogram.hpp"
#include "objreader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//
// init
//
Program normalPositionProgram, pdProgram;
ObjData obj;

GLuint normalVAO;
GLuint positionVBO, normalVBO;
GLuint vertexElement;

enum Index {
    NORMAL,
    POSITION,
    PHONG,
    EDGE
};
GLuint dataFB[4];
GLuint dataTexture[4];
GLenum dataTexDrawBuffers[4] = {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3
};
GLuint renderBufferobject;
float edgeThreshold = 0.1f;

// pd: umbilic unsolved
GLuint pdFB;
GLuint pdTexture;
GLuint pdVAO;
GLuint pdQuadVBO;
GLenum pdTexDrawBuffers[1] = {GL_COLOR_ATTACHMENT0};

// umbolic solver

Program usProgram;

GLuint usFB;
GLuint usTexture;
GLenum usTexDrawBuffers[1] = {GL_COLOR_ATTACHMENT0};

// smooth directions

const int SMOOTHING_COUNT = 6;
int sdCount = 2;
Program sdProgram;
GLuint sdFB[SMOOTHING_COUNT], angleFB;
GLuint sdTexture[2], angleTexture;
GLenum angleTexDrawBuffers[1] = {GL_COLOR_ATTACHMENT0};

// test

Program quadProgram;

// quads

GLuint quadVAO;
GLuint quadVBO;

GLuint finalTexture;

float quadVertices[] = {
    // positions   // texCoords
    -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f};

// Run Gen-Bind-Data Buffer with GL_STATIC_DRAW
// `glGenBuffers(1, buffer);`
// `glBindBuffer(target, *buffer);`
// `glBufferData(target, memSize, data, GL_STATIC_DRAW);`
void glGBDArrayBuffer(GLenum target, GLuint *buffer, GLsizeiptr memSize, const void *data)
{
    glGenBuffers(1, buffer);
    glBindBuffer(target, *buffer);
    glBufferData(target, memSize, data, GL_STATIC_DRAW);
}

// Run Gen-Bind Texture(s) and TexImage2D-TexParameteri with some default settings
// `glGenTextures(1, texture);`
// `glBindTexture(GL_TEXTURE_2D, *texture);`
// `glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);`
// `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);`
// `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);`
void glGBIPTexture2D(GLuint *texture, int w, int h)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// movement

const float PI = 3.14159265358979323846f;
float cameraTheta(0), cameraPhi(0);

namespace comparator
{
float min(const float &a, const float &b)
{
    return a > b ? b : a;
}
float max(const float &a, const float &b)
{
    return a > b ? a : b;
}
}

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    static double lastX = 0;
    static double lastY = 0;
    // when left mouse button clicked
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
    {
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        // rotate 180 degree per each width/height dragging
        cameraTheta -= dx / w * PI; // related with y-axis rotation
        cameraPhi -= dy / h * PI;   // related with x-axis rotation
        cameraPhi = comparator::max(-PI / 2 + 0.01f, comparator::min(cameraPhi, PI / 2 - 0.01f));
        // printf("%.3f %.3f\n", cameraTheta, cameraPhi);
    }
    // whenever, save current cursor position as previous one
    lastX = xpos;
    lastY = ypos;
}

//float testOffset = 0.00001;
//float testOffsetDelta = 0.00001;
bool enableCaseTest = false;
float testCloseToZero = 0.0005;
float testCloseToZeroDelta = 0.0001;

bool doSmoothing = true;
//bool isView = false;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
//    if (key == GLFW_KEY_RIGHT && action > GLFW_RELEASE)
//    {
//        testOffset = comparator::min(0.1, comparator::max(0, testOffset + testOffsetDelta));
//        std::cout << "Offset: " << testOffset << std::endl;
//    }
//    else if (key == GLFW_KEY_LEFT && action > GLFW_RELEASE)
//    {
//        testOffset = comparator::min(0.1, comparator::max(0, testOffset - testOffsetDelta));
//        std::cout << "Offset: " << testOffset << std::endl;
//    }
     if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        enableCaseTest = !enableCaseTest;
    }
    else if (key == GLFW_KEY_UP && action > GLFW_RELEASE)
    {
        testCloseToZero = comparator::min(0.1, comparator::max(0, testCloseToZero + testCloseToZeroDelta));
        std::cout << "CloseToZero: " << testCloseToZero << std::endl;
    }
    else if (key == GLFW_KEY_DOWN && action > GLFW_RELEASE)
    {
        testCloseToZero = comparator::min(0.1, comparator::max(0, testCloseToZero - testCloseToZeroDelta));
        std::cout << "CloseToZero: " << testCloseToZero << std::endl;
    }
    else if (GLFW_KEY_1 <= key && key <= GLFW_KEY_6) {
        if(action == GLFW_PRESS) {
            sdCount = key - GLFW_KEY_0;
            std::cout << "sdCount: " << sdCount << std::endl;
        }
    }
    else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        finalTexture = dataTexture[NORMAL];
        std::cout << "Normal Texture" << std::endl;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        finalTexture = dataTexture[POSITION];
        std::cout << "World Position Texture" << std::endl;
    }
    else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        finalTexture = dataTexture[PHONG];
        std::cout << "Phong Texture" << std::endl;
    }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        finalTexture = dataTexture[EDGE];
        std::cout << "Edge Detection Texture" << std::endl;
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        finalTexture = pdTexture;
        std::cout << "Principal Direction Texture" << std::endl;
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        finalTexture = sdTexture[(sdCount - 1) % 2];
        std::cout << "Smoothed Direction Texture" << std::endl;
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        finalTexture = angleTexture;
        std::cout << "Angle Texture" << std::endl;
    }


//    else if (key == GLFW_KEY_V && action == GLFW_PRESS) {
//        isView = !isView;
//        std::cout << "isView: " << isView << std::endl;
//    }
}



// TAM Texture Load //

// tone: 0 is lightest, (TONE_COUNT-1) is darkest
// mip: 0 is finest, (MIPMAP_COUNT-1) is coarsest // it is reverse order of the paper
const int TONE_COUNT = 6;
const int MIPMAP_COUNT = 4;
GLuint TAMTexture[TONE_COUNT];

void tamTexLoad()
{
    std::string filename = "TAM/tone0mip0.png";
    stbi_set_flip_vertically_on_load(1);
    glGenTextures(TONE_COUNT, TAMTexture);
    for (int tone = 0; tone < TONE_COUNT; tone++)
    {
        glBindTexture(GL_TEXTURE_2D, TAMTexture[tone]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MIPMAP_COUNT - 1);
        for (int mip = 0; mip < MIPMAP_COUNT; mip++)
        {
            filename[8] = '0' + tone;
            filename[12] = '0' + mip;
            int x, y, n;
            unsigned char *data = stbi_load(filename.c_str(), &x, &y, &n, 0);
            std::cout << filename << " Image: x = " << x << ", y = " << y << ", n = " << n << std::endl;

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void pdInit(GLFWwindow *window)
{
    // Question. Call below function at first cause the error 1282(Invalid Operation).
    // Case1. XCode. Previous project on VSCode worked fine, but XCode not.
    // Case2. Another.... Something....
    // tamTexLoad()
    
    obj.loadObject("obj", "teapot.obj");
    
    //
    // Normal & Position program
    //
    
    std::cout << "--- Normal & Position Program Init ---\n";
    
    normalPositionProgram.loadShader("normalPosition.vert", "normalPosition.frag");
    glUseProgram(normalPositionProgram.programID);
    
    glGenVertexArrays(1, &normalVAO);
    glBindVertexArray(normalVAO);
    
    glGBDArrayBuffer(GL_ARRAY_BUFFER, &positionVBO, sizeof(glm::vec3) * obj.nVertices, obj.vertices.data());
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    
    glGBDArrayBuffer(GL_ARRAY_BUFFER, &normalVBO, sizeof(glm::vec3) * obj.nSyncedNormals, obj.syncedNormals.data());
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    
    glGBDArrayBuffer(GL_ELEMENT_ARRAY_BUFFER, &vertexElement, sizeof(glm::u16vec3) * obj.nElements3, obj.elements3.data());
    
    // framebuffer
    
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    
    glGenRenderbuffers(1, &renderBufferobject);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferobject);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    
    for(int i = 0; i < 4; i++) {
        glGenFramebuffers(1, &dataFB[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, dataFB[i]);
        
        glGBIPTexture2D(&dataTexture[i], w, h);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               dataTexDrawBuffers[0],
                               GL_TEXTURE_2D,
                               dataFB[i],
                               0);
        // `glDrawBuffers` set the buffer list to be drawn
        glDrawBuffer(dataTexDrawBuffers[0]);
        
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferobject);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    
    //
    // umbilic unsolved principal direction
    //
    
    std::cout << "--- Principal Direction Program Init ---\n";
    
    pdProgram.loadShader("pd.vert", "pd.frag");
    glUseProgram(pdProgram.programID);
    
    glGenFramebuffers(1, &pdFB);
    glBindFramebuffer(GL_FRAMEBUFFER, pdFB);
    
    glGBIPTexture2D(&pdTexture, w, h);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pdTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    
    //
    // try to solve umbilic points - uncompleted
    //
    std::cout << "--- Umbilic Solver Program Init ---\n";
    
    usProgram.loadShader("umbolicSolver.vert", "umbolicSolver.frag");
    glUseProgram(usProgram.programID);
    
    glGenFramebuffers(1, &usFB);
    glBindFramebuffer(GL_FRAMEBUFFER, usFB);
    
    glGBIPTexture2D(&usTexture, w, h);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, usTexDrawBuffers[0], GL_TEXTURE_2D, usTexture, 0);
    glDrawBuffers(1, usTexDrawBuffers);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    
    //
    // smooth direction
    //
    std::cout << "--- Smooth Direction Program Init ---\n";
    
    sdProgram.loadShader("smoothingPD.vert", "smoothingPD.frag");
    glUseProgram(sdProgram.programID);
    
    glGenFramebuffers(SMOOTHING_COUNT, sdFB);
//    GLint maxAttach;
//    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
//    std::cout << "Max Color Attachments: " << maxAttach << std::endl;
    
    // bind framebuffer[i] - texture[i%2]
    glGBIPTexture2D(&sdTexture[0], w, h);
    glGBIPTexture2D(&sdTexture[1], w, h);
    for (int i = 0; i < SMOOTHING_COUNT; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, sdFB[i]);
        
        GLenum attach[1];
        attach[0] = GL_COLOR_ATTACHMENT0;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attach[0], GL_TEXTURE_2D, sdTexture[i % 2], 0);
        glDrawBuffers(1, attach);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER " << i << "-th:: Framebuffer is not complete! Code " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }
    
    // quantize angle fb
    glGenFramebuffers(1, &angleFB);
    glBindFramebuffer(GL_FRAMEBUFFER, angleFB);
    
    glGBIPTexture2D(&angleTexture, w, h);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                         angleTexDrawBuffers[0],
                         GL_TEXTURE_2D,
                         angleTexture, 0);
    glDrawBuffer(angleTexDrawBuffers[0]);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER Framebuffer is not complete! Code " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    
    //
    // test
    //
    std::cout << "--- Test Program Init ---\n";
    
    quadProgram.loadShader("quad.vert", "quad.frag");
    
    //
    // quads
    //
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    
    glGBDArrayBuffer(GL_ARRAY_BUFFER, &quadVBO, 24 * sizeof(float), quadVertices);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    
    tamTexLoad();
}

//
// render
//

void pdRender(GLFWwindow *window)
{
    
    glm::mat4 modelMat = glm::mat4({{1, 0, 0, 0},
                                    {0, 1, 0, 0},
                                    {0, 0, 1, 0},
                                    {0, -1.5, 0, 1}});
    // modelMat = glm::scale(glm::vec3(0.01)) * modelMat;
    glm::mat4 rotateX = glm::rotate(cameraPhi, glm::vec3(1, 0, 0));
    glm::mat4 rotateY = glm::rotate(cameraTheta, glm::vec3(0, 1, 0));
    glm::vec3 eye(0, 0, 5);
    glm::vec3 eyePosition = rotateY * rotateX * glm::vec4(eye, 1);
    glm::mat4 viewMat = glm::lookAt(eyePosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glm::mat4 projMat = glm::perspective(60 / (float)180 * PI, w / (float)h, 0.01f, 1000.0f);
    
    //
    // Normal & Position texture render
    //
    
    glUseProgram(normalPositionProgram.programID);
    
    glBindFramebuffer(GL_FRAMEBUFFER, dataFB[NORMAL]);
    glBindVertexArray(normalVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexElement);
    
    glViewport(0, 0, w, h);
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    GLuint modelMatLoc = glGetUniformLocation(normalPositionProgram.programID, "modelMat");
    GLuint viewMatLoc  = glGetUniformLocation(normalPositionProgram.programID, "viewMat");
    GLuint projMatLoc  = glGetUniformLocation(normalPositionProgram.programID, "projMat");
    glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, glm::value_ptr(projMat));
    
    GLuint lightPositionLoc = glGetUniformLocation(normalPositionProgram.programID, "lightPosition");
    GLuint eyePositionLoc   = glGetUniformLocation(normalPositionProgram.programID, "eyePosition");
    GLuint lightColorLoc    = glGetUniformLocation(normalPositionProgram.programID, "lightColor");
    GLuint diffuseColorLoc  = glGetUniformLocation(normalPositionProgram.programID, "diffuseColor");
    GLuint specularColorLoc = glGetUniformLocation(normalPositionProgram.programID, "specularColor");
    GLuint shininessLoc     = glGetUniformLocation(normalPositionProgram.programID, "shininess");

    glm::vec3 lightPosition(10, 10, 5);
    glm::vec3 lightColor(140);
    glm::vec3 diffuseColor(1, 1, 1);
    glm::vec3 specularColor(0.33, 0.33, 0.33);
    float shininess = 12;

    glUniform3fv(lightPositionLoc, 1, glm::value_ptr(lightPosition));
    glUniform3fv(eyePositionLoc, 1, glm::value_ptr(eyePosition));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform3fv(diffuseColorLoc, 1, glm::value_ptr(diffuseColor));
    glUniform3fv(specularColorLoc, 1, glm::value_ptr(specularColor));
    glUniform1f(shininessLoc, shininess);
    
    GLuint inverseSizeLoc = glGetUniformLocation(normalPositionProgram.programID, "inverseSize");
    glm::vec2 inverseSize(1 / (float)w, 1 / (float)h);
    glUniform2fv(inverseSizeLoc, 1, glm::value_ptr(inverseSize));
    
    GLuint edgeThresholdLoc = glGetUniformLocation(normalPositionProgram.programID,
                                                   "edgeThreshold");
    glUniform1f(edgeThresholdLoc, edgeThreshold);
    
//    GLuint isViewLoc = glGetUniformLocation(normalPositionProgram.programID, "isView");
//    glUniform1i(isViewLoc, isView);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dataTexture[PHONG]);
    GLuint ptLoc = glGetUniformLocation(normalPositionProgram.programID, "positionTex");
    glUniform1i(ptLoc, 0);
    
    GLuint npIndex = glGetSubroutineUniformLocation(normalPositionProgram.programID, GL_FRAGMENT_SHADER, "renderPass");
    if (npIndex == -1)
    {
        std::cout << "Subroutine indexing error" << std::endl;
        return;
    }
    
    for(int i = 0 ; i < 4; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, dataFB[i]);
        
        glViewport(0, 0, w, h);
        
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        std::string pass = "pass1";
        pass[4] = '1' + i;
        GLuint npPass = glGetSubroutineIndex(normalPositionProgram.programID,
                                             GL_FRAGMENT_SHADER,
                                             pass.c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &npPass);
        
        glDrawElements(GL_TRIANGLES, obj.nElements3 * 3, GL_UNSIGNED_SHORT, 0);
        
    }
    GLenum e = glGetError();
    if(e) {
        std::cout<<"Error "<<e<<std::endl;
    }
    
    //
    // Principal Direction 1 render
    //
    
    glUseProgram(pdProgram.programID);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pdFB);
    glBindVertexArray(quadVAO);
    
    glViewport(0, 0, w, h);
    
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // value below (0 and 1) is same with GL_TEXTURE{value}
    GLuint normTexLoc = glGetUniformLocation(pdProgram.programID, "normalTexture");
    glUniform1i(normTexLoc, 0);
    GLuint posTexLoc = glGetUniformLocation(pdProgram.programID, "positionTexture");
    glUniform1i(posTexLoc, 1);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dataTexture[NORMAL]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, dataTexture[POSITION]);
    
    // test values
//    GLuint OFFSETLoc = glGetUniformLocation(pdProgram.programID, "OFFSET");
//    glUniform1f(OFFSETLoc, testOffset);
    GLuint enableCaseTestLoc = glGetUniformLocation(pdProgram.programID, "enableCaseTest");
    glUniform1i(enableCaseTestLoc, enableCaseTest);
    GLuint closeToZeroLoc = glGetUniformLocation(pdProgram.programID, "CLOSETOZERO");
    glUniform1f(closeToZeroLoc, testCloseToZero);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //
    // umbolic solver render
    //
    
    glUseProgram(usProgram.programID);
    
    glBindFramebuffer(GL_FRAMEBUFFER, usFB);
    glBindVertexArray(quadVAO);
    
    glViewport(0, 0, w, h);
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    GLuint pdTextureLoc = glGetUniformLocation(usProgram.programID, "pdTexture");
    glUniform1i(pdTextureLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pdTexture);
    
    enableCaseTestLoc = glGetUniformLocation(usProgram.programID, "enableCaseTest");
    glUniform1i(enableCaseTestLoc, enableCaseTest);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //
    // smoothing pd
    //
    
    glUseProgram(sdProgram.programID);
    
    GLuint index = glGetSubroutineUniformLocation(sdProgram.programID, GL_FRAGMENT_SHADER, "renderPass");
    if (index == -1)
    {
        std::cout << "Subroutine indexing error" << std::endl;
        return;
    }
    
    GLuint sdPass1 = glGetSubroutineIndex(sdProgram.programID, GL_FRAGMENT_SHADER, "pass1");
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &sdPass1);
    
    for (int i = 0; i < sdCount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, sdFB[i]);
        glBindVertexArray(quadVAO);
        
        glViewport(0, 0, w, h);
        
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        GLuint testTexLoc = glGetUniformLocation(sdProgram.programID, "pdTex");
        glUniform1i(testTexLoc, i % 2);
        glActiveTexture(GL_TEXTURE0 + i % 2);
        GLuint tex = i == 0 ? usTexture : sdTexture[(i - 1) % 2];
        glBindTexture(GL_TEXTURE_2D, tex);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, dataTexture[EDGE]);
        GLuint positionTexLoc = glGetUniformLocation(sdProgram.programID, "edgeTex");
        glUniform1i(positionTexLoc, 2);
        
        GLuint inverseSizeLoc = glGetUniformLocation(sdProgram.programID, "inverseSize");
        glUniform2fv(inverseSizeLoc, 1, glm::value_ptr(inverseSize));
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    // angle quantization
    
    glBindFramebuffer(GL_FRAMEBUFFER, angleFB);
    glBindVertexArray(quadVAO);
    glViewport(0, 0, w, h);
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    GLuint sdPass2 = glGetSubroutineIndex(sdProgram.programID, GL_FRAGMENT_SHADER, "pass2");
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &sdPass2);
    
    std::string tamvar = "tam0";
    GLenum texId[TONE_COUNT] = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5};
    for (int i = 0; i < TONE_COUNT; i++)
    {
        // std::cout << "Texture Activate: tamvar = " << tamvar << std ::endl;
        GLuint ttLoc = glGetUniformLocation(sdProgram.programID, tamvar.c_str());
        glUniform1i(ttLoc, i);
        glActiveTexture(texId[i]);
        glBindTexture(GL_TEXTURE_2D, TAMTexture[i]);
        tamvar[3]++;
    }
    
    int texIndex = (sdCount - 1) % 2;
    GLuint testTexLoc = glGetUniformLocation(sdProgram.programID, "pdTex");
    glUniform1i(testTexLoc, 6);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, sdTexture[texIndex]);
    GLuint phongTexLoc = glGetUniformLocation(sdProgram.programID, "phong");
    glUniform1i(phongTexLoc, 7);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, dataTexture[PHONG]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //
    // quad
    //
    
    glUseProgram(quadProgram.programID);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(quadVAO);
    
    glViewport(0, 0, w, h);
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    GLuint quadTexLoc = glGetUniformLocation(quadProgram.programID, "tex");
    glUniform1i(quadTexLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, finalTexture);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glfwSwapBuffers(window);
}

//
// main
//

using namespace std::chrono_literals;

int main()
{
    // XCode bug: instance duplication occurs when below code removed.
    // IDK the reason why it works but, you have to wait for 1 second. not even 500ms.
    // From: https://developer.apple.com/forums/thread/765445
    std::this_thread::sleep_for(1000ms);
    
    // init
    
    if (!glfwInit())
    {
        std::cout << "GLFW Init Failed" << std::endl;
        return -1;
    }
    
    // MacOS Setting
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    
    GLFWwindow *window = glfwCreateWindow(640, 480, "Line Art Illustration", 0, 0);
    glfwMakeContextCurrent(window);
    
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW Init Failed" << std::endl;
        return -1;
    }
    
    // init(window);
    pdInit(window);
    
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);
    
    // render
    
    while (!glfwWindowShouldClose(window))
    {
        // render(window);
        pdRender(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
