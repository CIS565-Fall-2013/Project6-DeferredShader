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
    void SetShaderConstant(const std::string& constantName, const void* value_in); 
    void ApplyShaderConstantsForProgram(uint32_t program) const;
};

struct ShaderConstant
{
    void* data;
    bool dirty;
    ShaderConstantManager::SupportedTypes type;
};