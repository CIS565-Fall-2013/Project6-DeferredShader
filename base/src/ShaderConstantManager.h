#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>

struct ShaderConstant;
class ShaderConstantManager
{
    std::map<std::string, ShaderConstant*> m_shaderConstantNameToDataMap;
    std::map<uint32_t, std::vector<std::string>*> m_programShaderConstantsMap;
    mutable int32_t m_lastUsedProgram;

    void SetupConstantDataStore();
public:

    enum SupportedTypes
    {
        MAT4,
        VEC4,
        VEC3,
        FLOAT,
        INT,
        BOOL,
    };
    
    ShaderConstantManager();
    ~ShaderConstantManager();
    void SetupConstantAssociationsForProgram(uint32_t programId);
    template<typename T> void SetShaderConstant(const std::string& constantName, const T& value_in);
    template<> void SetShaderConstant(const std::string& constantName, const uint32_t& value);
    template<> void SetShaderConstant(const std::string& constantName, const bool& value);
    void SetShaderConstant(const std::string& constantName, const void* value_in);
    void ApplyShaderConstantsForProgram(uint32_t program) const;
};

template<typename T>
void ShaderConstantManager::SetShaderConstant(const std::string& constantName, const T& value)
{
    SetShaderConstant(constantName, reinterpret_cast<const void*>(&value));
}

template<>
void ShaderConstantManager::SetShaderConstant(const std::string& constantName, const uint32_t& value)
{
    SetShaderConstant(constantName, static_cast<const int32_t>(value));
}

template<>
void ShaderConstantManager::SetShaderConstant(const std::string& constantName, const bool& value)
{
    SetShaderConstant(constantName, static_cast<const int32_t>(value));
}

struct ShaderConstant
{
    void* data;
    bool dirty;
    ShaderConstantManager::SupportedTypes type;
};