//
//  main.cpp
//  LineArtIllustrationXCode
//
//  Created by 이용규 on 3/7/25.
//

#include <objreader.hpp>
ObjData obj, human;

#include <YGLWindow.hpp>
YGLWindow* yglWindow;

Program modelShader, fquadShader;

Framebuffer modelFbo, fquadFbo;

#include <camera.hpp>

GLuint finalTexture = -1;
GLuint umbilicSovedPD;

const int SMOOTHCOUNT = 5;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        finalTexture = modelFbo.textureIDs[1];
        std::cout << "Normal Texture" << std::endl;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        finalTexture = modelFbo.textureIDs[0];
        std::cout << "World Position Texture" << std::endl;
    }
    else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        finalTexture = modelFbo.textureIDs[2];
        std::cout << "Phong Texture" << std::endl;
    }
    else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        finalTexture = fquadFbo.textureIDs[1];
        std::cout << "Edge Detection Texture" << std::endl;
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        finalTexture = fquadFbo.textureIDs[2];
        std::cout << "Principal Direction Texture" << std::endl;
    }
    else if (key == GLFW_KEY_U && action == GLFW_PRESS) {
        finalTexture = umbilicSovedPD;
        std::cout << "Umbilic Solver Texture" << std::endl;
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        finalTexture = fquadFbo.textureIDs[4+((SMOOTHCOUNT-1)%2)];
        std::cout << "Smoothed Direction Texture" << std::endl;
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        finalTexture = fquadFbo.textureIDs[6];
        std::cout << "Angle Texture" << std::endl;
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if(finalTexture == fquadFbo.textureIDs[3])
            finalTexture = umbilicSovedPD;
        else finalTexture = fquadFbo.textureIDs[3];
        std::cout << "Angle Texture" << std::endl;
    }
}

const int MIPMAP_COUNT = 4;
GLuint TAMTexture[2];
// tex tone0~2 --> TAMTexture[0]
// tex tone3~5 --> TAMTexture[1]
void tamTexLoad(bool verbose = false)
{
    std::string filename = "TAM/tone0mip0.png";
    stbi_set_flip_vertically_on_load(1);
    glGenTextures(2, TAMTexture);
    for (int tone = 0; tone < 2; tone++)
    {
        glBindTexture(GL_TEXTURE_2D, TAMTexture[tone]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MIPMAP_COUNT - 1);
        for (int mip = 0; mip < MIPMAP_COUNT; mip++)
        {
            filename[8] = '0' + tone;
            filename[12] = '0' + mip;
            int x, y, n;
            unsigned char *data1 = stbi_load(filename.c_str(), &x, &y, &n, 0);
            filename[8] = '0' + tone + 1;
            unsigned char *data2 = stbi_load(filename.c_str(), &x, &y, &n, 0);
            filename[8] = '0' + tone + 2;
            unsigned char *data3 = stbi_load(filename.c_str(), &x, &y, &n, 0);
            if(verbose) std::cout << filename << " Image: x = " << x << ", y = " << y << ", n = " << n << std::endl;
            std::vector<unsigned char> data;
            int size = x*y;
            for(int i = 0; i < size; i++) {
                data.push_back(data1[4*i]);
                data.push_back(data2[4*i]);
                data.push_back(data3[4*i]);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, mip, GL_RGB8, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
            stbi_image_free(data1);
            stbi_image_free(data2);
            stbi_image_free(data3);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    std::cout << "--- TAM Texture Loaded ---" << std::endl;
}


struct Plane : ObjData {
    Plane() {
        vertices = {
            {-0.5, 0, -0.5}, {-0.5, 0, 0.5}, {0.5, 0, 0.5}, {0.5, 0, -0.5}
        };
        nVertices = 6;
        
        syncedNormals = std::vector<glm::vec3>(6, {0,1,0});
        nSyncedNormals = 6;
        
        elements3 = {
            {0, 1, 2}, {0, 2, 3}
        };
        nElements3 = 2;
    }
};
Plane plane;

void init() {
//    pdInit(yglWindow->getGLFWWindow());
    camera.setPosition({0, 0, 5});
    camera.glfwSetCallbacks(yglWindow->getGLFWWindow());
    
    tamTexLoad();
    glErr("after tamTexLoad(): TAM Texture Loading");
    
    obj.loadObject("obj", "UtahTeapot.obj");
    obj.adjustCenter(true);
    obj.generateBuffers();
    glErr("after generateBuffer(): teapot object");
    
    human.loadObject("obj", "human.obj");
    human.adjustCenter(true);
    human.generateBuffers();
    
    plane.generateBuffers();
    glErr("after generateBuffer(): plane object");
    
    modelShader.loadShader("model.vert", "model.frag");
    fquadShader.loadShader("fquad.vert", "fquad.frag");
    glErr("after loadShader(): model and fquad");
    
    modelFbo.init(yglWindow->getGLFWWindow());
    modelFbo.attachRenderBuffer(GL_DEPTH24_STENCIL8);
    modelFbo.attachTexture2D(3, GL_RGBA32F);
    glErr("after glFramebufferTexture2D: dataFB, dataTexture");
    
    fquadFbo.init(yglWindow->getGLFWWindow());
    fquadFbo.attachTexture2D(4, GL_RGBA32F); // result, edge, pd, casetest
    fquadFbo.attachTexture2D(2, GL_RGBA32F, yglWindow->width()/2, yglWindow->height()/2); // pinpong smooth texture
    fquadFbo.attachTexture2D(1, GL_RGBA32F); // stroke mapped
    
    glGenTextures(1, &umbilicSovedPD);
    glBindTexture(GL_TEXTURE_2D, umbilicSovedPD);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, yglWindow->width(), yglWindow->height(), 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glfwSetKeyCallback(yglWindow->getGLFWWindow(), keyCallback);
}

void render() {
//    pdRender(yglWindow->getGLFWWindow());
    int w = yglWindow->width();
    int h = yglWindow->height();
    
    // ============================== Position & Normal & Phong
    
    modelFbo.bind();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0, 0, w, h);
    
    modelShader.use();
    
    glm::vec3 eyePosition = camera.getCurPosition();
    glm::mat4 viewMat = camera.lookAt();
    glm::mat4 projMat = camera.perspective(yglWindow->aspect(), 0.1f, 1000.f);
    
    modelShader.setUniform("viewMat", viewMat);
    modelShader.setUniform("projMat", projMat);
    
    glm::vec3 lightPosition(10, 10, 5);
    glm::vec3 lightColor(160);
    float shininess = 12;
    
    modelShader.setUniform("lightPosition", lightPosition);
    modelShader.setUniform("eyePosition", eyePosition);
    modelShader.setUniform("lightColor", lightColor);
    modelShader.setUniform("diffuseColor", obj.materialData[0].diffuseColor);
    modelShader.setUniform("specularColor", obj.materialData[0].specularColor);
    modelShader.setUniform("shininess", shininess);
    
    glm::mat4 modelMat = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.5, 0, 0)), glm::vec3(5, 5, 5));
    modelShader.setUniform("modelMat", modelMat);
    obj.render();
    
    modelMat = glm::scale(glm::translate(glm::mat4(1), glm::vec3(1.5, 0, 0)), glm::vec3(2.5, 2.5, 2.5));
    modelShader.setUniform("modelMat", modelMat);
    human.render();
    
    modelMat = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, -1.3, 0)), glm::vec3(10, 0, 10));
    modelShader.setUniform("modelMat", modelMat);
    plane.render();
    
    // ============================= Sobel
    
    fquadFbo.bind();
    glDisable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glViewport(0, 0, w, h);
    
    fquadShader.use();
    glm::vec2 inverseSize(1 / (float)w, 1 / (float)h);
    fquadShader.setUniform("inverseSize", inverseSize);
    fquadShader.setTexture("phongTexture", 0, modelFbo.textureIDs[2]);
    
    fquadShader.setSubroutine("sobelFilter");
    fquadFbo.setDrawBuffers({1});
    fquadFbo.renderFullScreenQuad();
    glErr("after sobel filter");
    
    // =================================== Principal Direction
    
    fquadShader.setTexture("positionTexture", 0, modelFbo.textureIDs[0]);
    fquadShader.setTexture("normalTexture", 1, modelFbo.textureIDs[1]);
    
    fquadShader.setSubroutine("estimatePD");
    fquadFbo.setDrawBuffers({2,3});
    fquadFbo.renderFullScreenQuad();
    
    // =================================== Smoothing PD
    
    int size = w*h;
    std::vector<GLfloat> pdData(size*4);
    glBindTexture(GL_TEXTURE_2D, fquadFbo.textureIDs[2]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pdData.data());
    
    if(pdData[3] == 0) {
        pdData[0] = 1;
        pdData[1] = 0;
        pdData[2] = 0;
        pdData[3] = 1;
    }
    glm::vec3 prevGood = {1, 0, 0};
    for(int i = 1; i < size; i++) {
        if(pdData[i*4+3] == 0) { // umbilic point
            pdData[i*4] = prevGood.x;
            pdData[i*4+1] = prevGood.y;
            pdData[i*4+2] = prevGood.z;
            pdData[i*4+3] = 1;
        } else {
            glm::vec3 p = {pdData[i*4], pdData[i*4+1], pdData[i*4+2]};
            if(glm::dot(p, p) > 0.8) prevGood = p;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, umbilicSovedPD);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, pdData.data());
    
    // =================================== Smoothing Direction
    
    GLuint prev = umbilicSovedPD;
    fquadShader.setTexture("edgeTexture", 0, fquadFbo.textureIDs[1]);
    for(int i = 0; i < SMOOTHCOUNT; i++) {
        fquadShader.setTexture("pdTexture", 1, prev);
        fquadShader.setUniform("smoothTarget", i%2);
        fquadShader.setSubroutine("smoothingPD");
        fquadFbo.setDrawBuffers({4+uint(i%2)});
        fquadFbo.renderFullScreenQuad();
        
        prev = fquadFbo.textureIDs[4+(i%2)];
    }
    
    // =================================== Angle Quatization
    
    fquadShader.setTexture("pdTexture", 0, prev);
    fquadShader.setTexture("tam0", 1, TAMTexture[0]);
    fquadShader.setTexture("tam1", 2, TAMTexture[1]);
    fquadShader.setSubroutine("strokeMapping");
    fquadFbo.setDrawBuffers({6});
    fquadFbo.renderFullScreenQuad();
    
    // =================================== result
    
    if(finalTexture == -1) finalTexture = umbilicSovedPD;
    fquadShader.setTexture("tex", 0, finalTexture);
    fquadShader.setSubroutine("result");
    
    fquadFbo.setDrawBuffers({0});
    fquadFbo.renderFullScreenQuad(true);
}
int main() {
    yglWindow = new YGLWindow(640, 480, "Line-art Illustration");
    yglWindow->mainLoop(init, render);
    
    return 0;
}
