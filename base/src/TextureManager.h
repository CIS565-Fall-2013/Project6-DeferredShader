#pragma once
#include "Common.h"
#include <unordered_map>

class TextureManager
{
    std::unordered_map<uint32_t, std::pair<GLType_uint, uint32_t>> m_textureNameToObjectMap;    // Key: Hash value; Value pair: First -> texture object, Second -> no. of refs. 

    static TextureManager* singleton;

    TextureManager();
    ~TextureManager();

    static void Create();
    static void Destroy();

    GLType_uint LoadImageAndCreateTexture(const std::string& textureName, int32_t forceChannels, uint32_t reuseTextureName, uint32_t flags);

public:
    static TextureManager* GetSingleton() { return singleton; }

    GLType_uint Acquire(const std::string& textureName);
    void Release(GLType_uint textureObject);

    friend class GLApp;
};