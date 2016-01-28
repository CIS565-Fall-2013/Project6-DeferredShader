#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "Common.h"

struct ShaderConstantSignature;
class ConstantBuffer;
class ShaderConstantManager
{
    std::unordered_map<uint32_t, ConstantBuffer*> m_constantBufferNameToDataMap;
    static std::weak_ptr<ShaderConstantManager> singleton;
    static uint32_t resolver;

    ShaderConstantManager();
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
    
    ~ShaderConstantManager();

    void SetupConstantBuffer(std::string& constantBufferName, int32_t constantBufferSize, std::vector<ShaderConstantSignature>& constantBufferSignature);
    void SetShaderConstant(const char* constantName, const std::string& constantBufferName, const void* value_in);
    void ApplyShaderConstantChanges(const std::string& constantBufferName = std::string()) const;
    GLType_uint GetConstantBufferObject(const std::string& constantBufferName) const;
    
    static std::shared_ptr<ShaderConstantManager> Create(); // Caller gets the owning reference.
    static std::weak_ptr<ShaderConstantManager>& GetSingleton();
    static SupportedTypes GetTypeFromString(const std::string& typeString);
    static uint32_t GetSizeForType(ShaderConstantManager::SupportedTypes type);
    static uint32_t GetAlignmentForType(ShaderConstantManager::SupportedTypes type);
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
    GLType_uint m_id;

public:
    ConstantBuffer();
    ~ConstantBuffer();

    friend class ShaderConstantManager;
};