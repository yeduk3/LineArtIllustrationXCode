//
//  framebuffer.hpp
//  ObjectiveOpenGL
//
//  Created by 이용규 on 3/17/25.
//

#ifndef framebuffer_hpp
#define framebuffer_hpp

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <stdio.h>

struct TextureFormat {
    /** Specifies the number of color components in the texture.
     */
    GLint internalFormat;
    /** Specifies the format of the pixel data
     
     */
    GLenum format;
    /** Specifies the data type of the pixel data.
     
     */
    GLenum type;
    
    /** Generate texture format data by internalFormat
    
     - Parameters:
        - parameter internalFormat: Split this into format and type value.
     */
    void generate(const GLint &internalFormat);
};

/** Framebuffer managing object.
 */
struct Framebuffer {
    /** Framebuffer object's ID(handle)*/
    GLuint id = 0;
    int width = 0, height = 0;
    
    GLuint renderbuffer = 0;
    bool depthTest = false;
    
    GLuint *textureIDs = nullptr;
    
    /** Generate a new framebuffer object.
     
     Generate a new framebuffer object with given width and height.
     
     - Parameters:
         - parameter w: **Width** of the framebuffer
         - parameter h: **Height** of the framebuffer
     */
    void init(GLFWwindow *window);
    
    /** Set a default framebuffer object.
     
     Set a default framebuffer object with given width and height.
     
     - Parameters:
         - parameter w: **Width** of the default framebuffer
         - parameter h: **Height** of the default framebuffer
     */
//    void initDefault(GLFWwindow *window);
    
    
    /** Generate new 2D textures and attach to this framebuffer.
     
     Generated textures are canvas of this framebuffer's draw call.
     
     Used functions:
     
     - [glGenTextures](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGenTextures.xhtml)
     
     - [glTexImage2D](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml)
     
     - Parameters:
     
        - parameter nTexture: Number of the textures will be attached.
     
        - parameter format: Texture's format used when creating texture.
     */
    void attachTexture2D(const int &nTexture, const TextureFormat &format);
    
    /** Generate new 2D textures and attach to this framebuffer.
     
     Generated textures are canvas of this framebuffer's draw call.
     
     Used functions:
     
     - [glGenTextures](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGenTextures.xhtml)
     
     - [glTexImage2D](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml)
     
     - Parameters:
     
        - parameter nTexture: Number of the textures will be attached.
     
        - parameter internalFormat: Texture's internalFormat used when create TextureFormat object and call again.
     */
    void attachTexture2D(const unsigned int nTexture, const GLint internalFormat);
    
    void attachRenderBuffer(const GLenum internalFormat);
    
    void render(GLFWwindow* window, const GLuint vao);
    void render(GLFWwindow* window, const GLuint vao, const GLuint veo, const GLsizei count);
    
//private:
    /** Bind this framebuffer object.*/
    void bind();
    /** Unbind this framebuffer object.*/
    void unbind();
    
    void cleanup();
    
    ~Framebuffer();
};

#endif /* framebuffer_hpp */
