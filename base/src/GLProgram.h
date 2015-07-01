#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <vector>

#include "Common.h"

class GLProgram
{
    uint32_t m_id;
    std::map<uint32_t, std::pair<uint32_t, uint32_t>> m_textureBindIndicesMap;   // Key: String hash value. Value Pair: first -> texture bind point; second -> texture object name.
    std::map<std::string, uint32_t> m_constantBufferBindIndicesMap;
    std::unordered_map<uint32_t, std::string> m_shaderConstantToConstantBufferBindingMap;
    std::map<std::string, uint32_t> m_attributeBindIndicesMap;
    std::map<std::string, uint32_t> m_outputBindIndicesMap;

    void SetupTextureBindings(const std::vector<std::string>& textureNames);
    void SetShaderConstant(const char* constantName, const void* value_in) const;
    void PreprocessShaderSource(std::string& shaderSource, const std::string& workingDirectory) const;
    void SetupTextureBindingsAndConstantBuffers(const std::string& shaderSource);

public:
    GLProgram();
    GLProgram(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles, 
        const std::map<std::string, uint32_t>& attributeBindIndices = std::map<std::string, uint32_t>(), 
        const std::map<std::string, uint32_t>& outputBindIndices = std::map<std::string, uint32_t>());
    ~GLProgram();

    void Create(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles);

    void SetAttributeBindLocation(const std::string& attributeName, uint32_t bindLocation) { m_attributeBindIndicesMap[attributeName] = bindLocation; }
    void SetOutputBindLocation(const std::string& outputName, uint32_t bindLocation) { m_outputBindIndicesMap[outputName] = bindLocation; }
    void SetTexture(const char* textureName, uint32_t textureObject);

    void SetActive() const;

    template<typename T> void SetShaderConstant(const char* constantName, const T& value_in) const;
    template<> void SetShaderConstant(const char* constantName, const uint32_t& value) const;
    template<> void SetShaderConstant(const char* constantName, const bool& value) const;

    bool GetAttributeBindLocation(const std::string& attributeName, uint32_t& bindLocation) const;
    bool GetOutputBindLocation(const std::string& outputName, uint32_t& bindLocation) const;

    void CommitConstantBufferBindings() const;
    void CommitTextureBindings() const;
};

template<typename T>
void GLProgram::SetShaderConstant(const char* constantName, const T& value) const
{
    SetShaderConstant(constantName, reinterpret_cast<const void*>(&value));
}

template<>
void GLProgram::SetShaderConstant(const char* constantName, const uint32_t& value) const
{
    SetShaderConstant(constantName, static_cast<const int32_t>(value));
}

template<>
void GLProgram::SetShaderConstant(const char* constantName, const bool& value) const
{
    SetShaderConstant(constantName, static_cast<const int32_t>(value));
}