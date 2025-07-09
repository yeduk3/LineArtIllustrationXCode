//
//  test.cpp
//  LineArtIllustrationXCode
//
//  Created by 이용규 on 7/3/25.
//

#include <YGLWindow.hpp>
YGLWindow* window;
#include <objreader.hpp>
ObjData obj;
#include <program.hpp>
Program shader;
#include <camera.hpp>

void init() {
    obj.loadObject("teapot.obj");
    obj.adjustCenter();
    obj.generateBuffers();
    shader.loadShader("phongShader.vert", "phongShader.frag");
    
    camera.glfwSetCallbacks(window->getGLFWWindow());
    
    glEnable(GL_DEPTH_TEST);
}

void render() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shader.use();
    shader.setUniform("modelMat", glm::mat4(1));
    shader.setUniform("viewMat", camera.lookAt());
    shader.setUniform("projMat", camera.perspective(window->aspect(), 0.1f, 1000.0f));
    
    shader.setUniform("lightPosition", glm::vec3(10, 10, 10));
    shader.setUniform("eyePosition", camera.getCurPosition());
    shader.setUniform("lightColor", glm::vec3(300));
    shader.setUniform("diffuseColor", obj.materialData[0].diffuseColor);
    shader.setUniform("specularColor", obj.materialData[0].specularColor);
    shader.setUniform("shininess", 10.f);
    
    obj.render();
}

int main() {
    window = new YGLWindow(640, 480, "test");
    window->mainLoop(init, render);
    
    return 0;
}
