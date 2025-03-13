#include "objreader.hpp"

#include <fstream>
#include <limits>
#include <regex>
#include <string>
#include <unistd.h>

void ObjData::setPrefix(const std::string &prefixName) {
    char *dir = getcwd(NULL, 0);
    std::cout << "Directory: " << dir << std::endl;
    this->prefix = prefixName + '/';
}

void ObjData::loadMtl(const std::string &mtlFileName) {
    std::fstream file(prefix + mtlFileName);
    if (!file.is_open()) {
        std::cerr << "No .mtl file" << std::endl;
        return;
    }
    std::cout << "Read " << mtlFileName << std::endl;

    std::string type;
    while (!file.eof()) {
        file >> type;
        if (file.eof())
            break;
        else if (type == "newmtl") {
            std::string mName;
            file >> mName;
            this->materialData.push_back(MtlData(mName));
        } else if (type == "Ka") {
            float r, g, b;
            file >> r >> g >> b;
            this->materialData.back().ambientColor = {r, g, b};
        } else if (type == "Kd") {
            float r, g, b;
            file >> r >> g >> b;
            this->materialData.back().diffuseColor = {r, g, b};
        } else if (type == "Ks") {
            float r, g, b;
            file >> r >> g >> b;
            this->materialData.back().specularColor = {r, g, b};
        }
    }

    std::cout << "Material Count: " << this->materialData.size() << std::endl;
    for (auto m : this->materialData) {
        std::cout << m << std::endl;
    }
}

void ObjData::loadObject(const std::string &objFileName) {
    isOk = false;

    std::fstream file(prefix + objFileName);
    if (!file.is_open()) {
        std::cerr << "No .obj file" << std::endl;
        return;
    }
    std::cout << "Read " << prefix + objFileName << std::endl;

    std::vector<std::string> faces;

    std::string type;
    while (!file.eof()) {
        file >> type;
        if (file.eof())
            break;
        if (type == "mtllib") {
            file >> this->materialFile;
            this->loadMtl(this->materialFile);
        } else if (type == "usemtl") {
            file >> this->material;
        } else if (type == "o" || type == "g") {
            if (!file.ignore(std::numeric_limits<std::streamsize>::max(),
                             file.widen('\n'))) {
                std::cerr << "Group Skip!" << std::endl;
            }
        } else if (type == "v") {
            float x, y, z;
            file >> x >> y >> z;
            this->vertices.push_back({x, y, z});
        } else if (type == "vt") {
            float tx, ty;
            file >> tx >> ty;
            this->textures.push_back({tx, ty});
        } else if (type == "vn") {
            float nx, ny, nz;
            file >> nx >> ny >> nz;
            this->normals.push_back({nx, ny, nz});
        } else if (type == "f") {
            std::string f;
            std::getline(file, f);

            faces.push_back(f);
        } else if (type == "l") {
            // not in this case
            if (!file.ignore(std::numeric_limits<std::streamsize>::max(),
                             file.widen('\n'))) {
                std::cerr << "Line polygon skip!" << std::endl;
            }
        } else {
            if (!file.ignore(std::numeric_limits<std::streamsize>::max(),
                             file.widen('\n'))) {
                std::cerr << "Weird situation! input " << type
                          << " is not supported." << std::endl;
                return;
            }
        }
        // std::cout << "Processing type " << type << std::endl;
    }

    this->nVertices = (int)this->vertices.size();

    std::vector<std::vector<glm::vec3>> sNormals(this->nVertices);

    for (auto f : faces) {
        // case by case?
        std::vector<GLuint> elem;

        std::regex re("\\d+/\\d*/\\d+");
        auto start = std::sregex_iterator(f.begin(), f.end(), re);
        auto end = std::sregex_iterator();
        while (start != end) {
            std::string str = start->str();
            GLuint vertex = std::stoi(str.substr(0, str.find('/'))) - 1;
            elem.push_back(vertex);

            GLuint normal =
                std::stoi(str.substr(str.find_last_of('/') + 1)) - 1;
            sNormals[vertex].push_back(this->normals[normal]);

            start++;
        }

        if (elem.size() == 4) {
            this->elements4.push_back({elem[0], elem[1], elem[2], elem[3]});
            this->elements3.push_back({elem[0], elem[1], elem[2]});
            this->elements3.push_back({elem[0], elem[2], elem[3]});
        } else if (elem.size() == 3)
            this->elements3.push_back({elem[0], elem[1], elem[2]});
        else {
            std::cerr << "Weird situation! f elements size is not 3 or 4."
                      << std::endl;
            return;
        }
    }

    for (auto sn : sNormals) {
        glm::vec3 sum(0);
        for (auto n : sn)
            sum += n;
        sum /= sn.size();
        this->syncedNormals.push_back(sum);
    }

    this->nElements3 = (int)this->elements3.size();
    this->nElements4 = (int)this->elements4.size();
    this->nNormals = (int)this->normals.size();
    this->nSyncedNormals = (int)this->syncedNormals.size();

    std::cout << "nVertices: " << this->nVertices << std::endl;
    std::cout << "nElements3: " << this->nElements3 << std::endl;
    std::cout << "nElements4: " << this->nElements4 << std::endl;
    std::cout << "nNormals: " << this->nNormals << std::endl;
    std::cout << "nSyncedNormals: " << this->nSyncedNormals << std::endl;

    file.close();

    isOk = true;
    
    std::cout << "--- Wavefront Object Loaded ---" << std::endl;

    return;
}

void ObjData::loadObject(const std::string &prefixName,
                         const std::string &objFileName) {
    this->setPrefix(prefixName);
    return this->loadObject(objFileName);
}
