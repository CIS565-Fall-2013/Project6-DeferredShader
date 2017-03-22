#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <vector>

#include "Common.h"
#include "ShaderResourceReferences.h"

class GLProgram
{
    GLType_uint m_id;
    std::map<TextureReference, std::pair<GLType_uint, GLType_uint>> m_textureBindIndicesMap;   // Key: String hash value. Value Pair: first -> texture bind point; second -> texture object name.
    std::map<ConstantBufferIndex, GLType_uint> m_constantBufferBindIndicesMap;
    std::unordered_map<ShaderConstantReference, ConstantBufferIndex, std::hash<uint32_t>, std::equal_to<uint32_t>> m_shaderConstantToConstantBufferBindingMap;
    std::map<std::string, GLType_uint> m_attributeBindIndicesMap;
    std::map<std::string, GLType_uint> m_outputBindIndicesMap;

    void SetupTextureBindings(const std::vector<std::string>& textureNames);
    void SetShaderConstant(ShaderConstantReference constantHandle, const void* value_in) const;
    void PreprocessShaderSource(std::string& shaderSource, const std::string& workingDirectory) const;
    void SetupTextureBindingsAndConstantBuffers(const std::string& shaderSource);

public:
    GLProgram();
    GLProgram(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles, 
        const std::map<std::string, GLType_uint>& attributeBindIndices = std::map<std::string, GLType_uint>(),
        const std::map<std::string, GLType_uint>& outputBindIndices = std::map<std::string, GLType_uint>());
    ~GLProgram();

    void Create(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles);

    void SetAttributeBindLocation(const std::string& attributeName, GLType_uint bindLocation) { m_attributeBindIndicesMap[attributeName] = bindLocation; }
    void SetOutputBindLocation(const std::string& outputName, GLType_uint bindLocation) { m_outputBindIndicesMap[outputName] = bindLocation; }
    void SetTexture(TextureReference textureHandle, GLType_uint textureObject);

    void SetActive() const;

    template<typename T> void SetShaderConstant(ShaderConstantReference constantHandle, const T& value_in) const;
    template<> void SetShaderConstant(ShaderConstantReference constantHandle, const uint32_t& value) const;
    template<> void SetShaderConstant(ShaderConstantReference constantHandle, const bool& value) const;

    bool GetAttributeBindLocation(const std::string& attributeName, GLType_uint& bindLocation) const;
    bool GetOutputBindLocation(const std::string& outputName, GLType_uint& bindLocation) const;

    void CommitConstantBufferChanges() const;
    void CommitTextureBindings() const;
};

template<typename T>
void GLProgram::SetShaderConstant(ShaderConstantReference constantHandle, const T& value) const
{
    SetShaderConstant(constantHandle, reinterpret_cast<const void*>(&value));
}

template<>
void GLProgram::SetShaderConstant(ShaderConstantReference constantHandle, const uint32_t& value) const
{
    SetShaderConstant(constantHandle, static_cast<const int32_t>(value));
}

template<>
void GLProgram::SetShaderConstant(ShaderConstantReference constantHandle, const bool& value) const
{
    SetShaderConstant(constantHandle, static_cast<const int32_t>(value));
}