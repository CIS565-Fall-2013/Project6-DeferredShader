#pragma once

#include <map>
#include <string>
#include <vector>

#include "Common.h"

struct ShaderConstantSignature;
class ConstantBuffer;
class ShaderConstantManager
{
    std::map<std::string, ConstantBuffer*> m_shaderConstantNameToConstantBufferMap;
    std::map<std::string, ConstantBuffer*> m_constantBufferNameToDataMap;
    static ShaderConstantManager* singleton;
    static uint32_t resolver;

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
    void SetupConstantBuffer(std::string& constantBufferName, std::vector<const ShaderConstantSignature>& constantBufferSignature);
    void SetShaderConstant(const std::string& constantName, const std::string& constantBufferName, const void* value_in);
    void ApplyShaderConstantChanges(const std::string& constantBufferName) const;
    uint32_t GetConstantBufferObject(const std::string& constantBufferName) const;
    
    static void Create();
    static void Destroy();
    static ShaderConstantManager* GetSingleton() { return singleton; }
};

struct ShaderConstantSignature
{
    std::string name;
    ShaderConstantManager::SupportedTypes type;
    uint32_t offset;
};

class ConstantBuffer
{
    std::map<std::string, ShaderConstantSignature> m_signature;
    void* m_data;
    bool m_dirty;
    uint32_t m_size;
    uint32_t m_id;

public:
    ConstantBuffer();
    ~ConstantBuffer();

    friend class ShaderConstantManager;
};