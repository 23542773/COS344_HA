#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// Original LoadShaders function (keep for compatibility)
GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

// New Shader class for lighting system
class Shader {
private:
    GLuint programID;
    
public:
    Shader();
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();
    
    void load(const char* vertexPath, const char* fragmentPath);
    void use() const;
    GLuint getProgramID() const { return programID; }
    
    // Uniform setters
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    
private:
    GLuint getUniformLocation(const std::string& name) const;
    void checkCompileErrors(GLuint shader, std::string type) const;
};

#endif