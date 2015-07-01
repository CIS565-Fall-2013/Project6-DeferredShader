#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "Common.h"

struct ShaderConstantSignature;
class ConstantBuffer;
class ShaderConstantManager
{
    std::unordered_map<uint32_t, ConstantBuffer*> m_constantBufferNameToDataMap;
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
    void SetupConstantBuffer(std::string& constantBufferName, int32_t constantBufferSize, std::vector<ShaderConstantSignature>& constantBufferSignature);
    void SetShaderConstant(const char* constantName, const std::string& constantBufferName, const void* value_in);
    void ApplyShaderConstantChanges(const std::string& constantBufferName = std::string()) const;
    uint32_t GetConstantBufferObject(const std::string& constantBufferName) const;
    
    static void Create();
    static void Destroy();
    static ShaderConstantManager* GetSingleton() { return singleton; }
    static SupportedTypes GetTypeFromString(const std::string& typeString);
    static uint32_t GetSizeForType(ShaderConstantManager::SupportedTypes type);
};

struct ShaderConstantSignature
{
    std::string name;
    ShaderConstantManager::SupportedTypes type;
    uint32_t size;
    uint32_t offset;
};

class ConstantBuffer
{
    std::unordered_map<uint32_t, ShaderConstantSignature> m_signature;
    void* m_data;
    bool m_dirty;
    uint32_t m_size;
    uint32_t m_id;

public:
    ConstantBuffer();
    ~ConstantBuffer();

    friend class ShaderConstantManager;
};