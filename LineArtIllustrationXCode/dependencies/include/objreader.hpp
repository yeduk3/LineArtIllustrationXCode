

#pragma once

#include <GL/glew.h> // GLuint

#include <glm/glm.hpp> // vec3

#include <vector>
#include <iostream>

struct MtlData
{
    std::string materialName;

    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;

    MtlData(const std::string mName) : materialName(mName) {}

    friend std::ostream &operator<<(std::ostream &os, const MtlData &mtl)
    {
        os << "Material Name: " << mtl.materialName << std::endl;
        os << "Ambient: " << mtl.ambientColor.r << " " << mtl.ambientColor.g << " " << mtl.ambientColor.b << std::endl;
        os << "Diffuse: " << mtl.diffuseColor.r << " " << mtl.diffuseColor.g << " " << mtl.diffuseColor.b << std::endl;
        os << "Specular: " << mtl.specularColor.r << " " << mtl.specularColor.g << " " << mtl.specularColor.b;
        return os;
    }
};

struct ObjData
{
    std::string prefix = "";
    std::string materialFile = "";
    std::string material = "";
    GLuint nVertices = 0;
    GLuint nElements3 = 0;
    GLuint nElements4 = 0;
    GLuint nNormals = 0;
    GLuint nSyncedNormals = 0;
    bool isOk;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> syncedNormals;
    std::vector<glm::u16vec3> elements3;
    std::vector<glm::u16vec4> elements4;

    std::vector<MtlData> materialData;

    void setPrefix(const std::string &prefixName);
    void loadMtl(const std::string &mtlFileName);
    void loadObject(const std::string &objFileName);
    void loadObject(const std::string &prefixName, const std::string &objFileName);
};
