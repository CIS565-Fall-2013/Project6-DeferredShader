#pragma once

#include <map>
#include <string>
#include <vector>

#include "Common.h"

class GLProgram
{
    uint32_t m_id;
    std::map<std::string, std::pair<uint32_t, uint32_t>> m_textureBindIndicesMap;   // first -> texture bind point; second -> texture object name.
    std::map<std::string, uint32_t> m_constantBufferBindIndicesMap;
    std::map<std::string, uint32_t> m_attributeBindIndicesMap;
    std::map<std::string, uint32_t> m_outputBindIndicesMap;

    void SetupTextureBindings();
    void SetShaderConstant(const std::string& constantName, const void* value_in) const;

public:
    GLProgram();
    GLProgram(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles, 
        const std::map<std::string, uint32_t>& attributeBindIndices = std::map<std::string, uint32_t>(), 
        const std::map<std::string, uint32_t>& outputBindIndices = std::map<std::string, uint32_t>());
    ~GLProgram();

    void Create(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles);

    void SetAttributeBindLocation(const std::string& attributeName, uint32_t bindLocation) { m_attributeBindIndicesMap[attributeName] = bindLocation; }
    void SetOutputBindLocation(const std::string& outputName, uint32_t bindLocation) { m_outputBindIndicesMap[outputName] = bindLocation; }
    void SetTexture(const std::string& textureName, uint32_t textureObject);

    void SetActive() const;

    template<typename T> void SetShaderConstant(const std::string& constantName, const T& value_in) const;
    template<> void SetShaderConstant(const std::string& constantName, const uint32_t& value) const;
    template<> void SetShaderConstant(const std::string& constantName, const bool& value) const;

    bool GetAttributeBindLocation(const std::string& attributeName, uint32_t& bindLocation) const;
    bool GetOutputBindLocation(const std::string& outputName, uint32_t& bindLocation) const;

    void CommitConstantBufferBindings() const;
    void CommitTextureBindings() const;
};

template<typename T>
void GLProgram::SetShaderConstant(const std::string& constantName, const T& value) const
{
    SetShaderConstant(constantName, reinterpret_cast<const void*>(&value));
}

template<>
void GLProgram::SetShaderConstant(const std::string& constantName, const uint32_t& value) const
{
    SetShaderConstant(constantName, static_cast<const int32_t>(value));
}

template<>
void GLProgram::SetShaderConstant(const std::string& constantName, const bool& value) const
{
    SetShaderConstant(constantName, static_cast<const int32_t>(value));
}