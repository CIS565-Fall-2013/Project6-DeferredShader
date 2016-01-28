#pragma once
#include "Common.h"
#include <unordered_map>
#include <memory>

class TextureManager
{
    std::unordered_map<uint32_t, std::pair<GLType_uint, uint32_t>> m_textureNameToObjectMap;    // Key: Hash value; Value pair: First -> texture object, Second -> no. of refs. 

    static std::weak_ptr<TextureManager> singleton;

    TextureManager();
    GLType_uint LoadImageAndCreateTexture(const std::string& textureName, int32_t forceChannels, uint32_t reuseTextureName, uint32_t flags);

public:
    ~TextureManager();
    static std::shared_ptr<TextureManager> GetSingleton();

    GLType_uint Acquire(const std::string& textureName);
    void Release(GLType_uint textureObject);

    friend class GLApp;
};