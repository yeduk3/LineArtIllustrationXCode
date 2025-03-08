// 아주대 신현준 교수님 toys.h 참고

// 중복 include 방지.
#pragma once

// 프로그램 선언을 위한 OpenGL 인클루드
#include <GL/glew.h>

#include <fstream>
#include <iostream>

struct Program
{
    GLuint programID = 0;
    GLuint vertexShaderID = 0;
    std::string vertexShaderName = "";
    GLuint geomShaderID = 0;
    std::string fragShaderName = "";
    GLuint fragShaderID = 0;
    std::string geomShaderName = "";

    std::string loadText(const char *filename)
    {
        std::fstream file(filename);
        if (!file.is_open())
        {
            std::cerr << filename << " File Not Found" << std::endl;
            return "";
        }

        std::istreambuf_iterator<char> begin(file), end;
        return std::string(begin, end);
    }

    bool shaderCompileCheck(GLuint shaderID)
    {
        GLint isCompiled = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

            for (auto e : errorLog)
            {
                std::cout << e;
            }
            std::cout << std::endl;

            // Provide the infolog in whatever manor you deem best.
            // Exit with failure.
            glDeleteShader(shaderID); // Don't leak the shader.
            return false;
        }
        std::cout << "Shader ID " << shaderID << " compile successed.\n";
        return true;
    }

    void loadShaderByText(const std::string &vShaderText, const std::string &fShaderText)
    {
        // Create Program
        programID = glCreateProgram();
        std::cout << "Program " << programID << " created" << std::endl;

        // Create Shader by its type
        vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Vertex Shader
        const GLchar *vShaderCode = vShaderText.c_str();
        glShaderSource(vertexShaderID, 1, &vShaderCode, 0);
        glCompileShader(vertexShaderID);
        if (shaderCompileCheck(vertexShaderID))
            glAttachShader(programID, vertexShaderID);
        else
        {
            cleanUp();
            return;
        }
        std::cout << "Vertex Shader: " << vertexShaderName << "(" << vShaderText.length() << ") created with ID " << vertexShaderID << std::endl;

        // Fragment Shader
        const GLchar *fShaderCode = fShaderText.c_str();
        glShaderSource(fragShaderID, 1, &fShaderCode, 0);
        glCompileShader(fragShaderID);
        if (shaderCompileCheck(fragShaderID))
            glAttachShader(programID, fragShaderID);
        else
        {
            cleanUp();
            return;
        }
        std::cout << "Fragment Shader: " << fragShaderName << "(" << fShaderText.length() << ") created with ID " << fragShaderID << std::endl;

        linkShader();
    }
    void loadShader(const char *vShaderFile, const char *fShaderFile)
    {
        vertexShaderName = std::string(vShaderFile);
        fragShaderName = std::string(fShaderFile);

        cleanUp();

        // Read Shader File
        // c_str()은 const char * 값을 반환.
        // Text로 받지 않으면 dangling pointer 발생.
        std::string vShaderText = loadText(vShaderFile);
        // vertex shader는 있어야 함.
        if (vShaderText.length() < 1)
        {
            std::cerr << "Vertex shader has no code" << std::endl;
            return;
        }
        std::string fShaderText = loadText(fShaderFile);

        loadShaderByText(vShaderText, fShaderText);
    }

    // This function doesn't include linking.
    void loadGeomShader(const char *gShaderFile)
    {
        geomShaderName = std::string(gShaderFile);

        std::string gShaderText = loadText(gShaderFile);

        geomShaderID = glCreateShader(GL_GEOMETRY_SHADER);

        const GLchar *gShaderCode = gShaderText.c_str();
        glShaderSource(geomShaderID, 1, &gShaderCode, 0);
        glCompileShader(geomShaderID);
        glAttachShader(programID, geomShaderID);

        std::cout << "Geometry Shader: " << geomShaderName << "(" << gShaderText.length() << ") created with ID " << geomShaderID << std::endl;
    }

    void loadShader(const char *vShaderFile, const char *gShaderFile, const char *fShaderFile)
    {
        loadGeomShader(gShaderFile);
        loadShader(vShaderFile, fShaderFile);
    }

    void printLog()
    {
        GLint maxLength = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(programID, maxLength, &maxLength, &errorLog[0]);

        for (auto e : errorLog)
        {
            std::cout << e;
        }
        std::cout << std::endl;
    }

    void linkShader()
    {
        // 다 붙이면 링크 후 사용 등록
        glLinkProgram(programID);
        GLint linkStatus;
        glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE)
        {
            std::cerr << "Shader Link Error on Program ID " << programID << "!!!!!!\n";

            printLog();

            return;
        }
        else
        {
            std::cout << "Shader Link Success on Program ID " << programID << std::endl;
        }
        glUseProgram(programID);
    }
    void cleanUp()
    {
        // Delete all programs
        if (programID)
            glDeleteProgram(programID);
        if (vertexShaderID)
            glDeleteShader(vertexShaderID);
        if (geomShaderID)
            glDeleteShader(geomShaderID);
        if (fragShaderID)
            glDeleteShader(fragShaderID);

        // value reset
        programID = vertexShaderID = geomShaderID = fragShaderID = 0;
    }
    ~Program()
    {
        cleanUp();
    }
};
