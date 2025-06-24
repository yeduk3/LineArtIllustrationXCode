//
//  framebuffer.cpp
//  ObjectiveOpenGL
//
//  Created by 이용규 on 3/17/25.
//

#include "framebuffer.hpp"

#include <iostream>

void TextureFormat::generate(const GLint &internalFormat) {
    this->internalFormat = internalFormat;
    if (internalFormat == GL_RGBA32F) {
        this->format = GL_RGBA;
        this->type = GL_FLOAT;
    }
}

void Framebuffer::init(GLFWwindow *window) {
    this->cleanup();
    
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    this->width = w;
    this->height = h;
    
    glGenFramebuffers(1, &(this->id));
    std::cout << "Framebuffer Created: " << this->id << std::endl;
}

void Framebuffer::attachTexture2D(const int &nTexture, const TextureFormat &format) {
    this->bind();
    
    this->textureIDs = (GLuint *) malloc(nTexture * sizeof(GLuint));
    
    glGenTextures(nTexture, this->textureIDs);
    for (int i = 0; i < nTexture; i++) {
        glBindTexture(GL_TEXTURE_2D, this->textureIDs[i]);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     format.internalFormat,
                     this->width,
                     this->height,
                     0,
                     format.format,
                     format.type,
                     0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D,
                               this->textureIDs[i],
                               0);
        GLint err = glGetError();
        if (err != GL_NO_ERROR) {
           printf("%08X ", err);
        }

        glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
    }
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
       printf("%08X ", err);
    }
    
    // unbind all
//    glBindTexture(GL_TEXTURE_2D, 0);
    this->unbind();
}

void Framebuffer::attachTexture2D(const unsigned int nTexture, const GLint internalFormat) {
    this->bind();
    
    TextureFormat tf;
    tf.generate(internalFormat);
    
    this->attachTexture2D(nTexture, tf);
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
       printf("%08X ", err);
    }
    
    this->unbind();
}

void Framebuffer::attachRenderBuffer(const GLenum internalFormat) {
    this->bind();
    
    glGenRenderbuffers(1, &(this->renderbuffer));
    glBindRenderbuffer(GL_RENDERBUFFER, this->renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          internalFormat,
                          this->width,
                          this->height);
    
    GLenum attachment;
    if(internalFormat == GL_DEPTH24_STENCIL8) {
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        this->depthTest = true;
    }
    else
        return;
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              attachment,
                              GL_RENDERBUFFER,
                              this->renderbuffer);
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    this->unbind();
}

void Framebuffer::render(GLFWwindow* window, const GLuint vao) {
    this->bind();
//    std::cout << "render id : " << this->id << std::endl;
    
    glViewport(0, 0, this->width, this->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindVertexArray(vao);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
    
    this->unbind();
}

void Framebuffer::render(GLFWwindow* window, const GLuint vao, const GLuint veo, const GLsizei count) {
    this->bind();
//    std::cout << "render id : " << this->id << std::endl;
    
    glViewport(0, 0, this->width, this->height);
    glClearColor(0, 0, 0, 0);
    if (depthTest) {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } else {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
    
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);
    
    this->unbind();
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, this->id);
//    std::cout << "Framebuffer Bounded: " << this->id << std::endl;
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    std::cout << "Framebuffer Unbounded: " << this->id << std::endl;
}

void Framebuffer::cleanup() {
    if (this->id != 0) {
        glDeleteFramebuffers(1, &this->id);
    }
    if (this->textureIDs != nullptr) {
        glDeleteTextures(1, this->textureIDs); // 텍스처 삭제
        free(this->textureIDs);
    }
    if (this->renderbuffer != 0) {
        glDeleteRenderbuffers(1, &this->renderbuffer); // 렌더버퍼 삭제
    }
}

Framebuffer::~Framebuffer() {
    cleanup();
}
