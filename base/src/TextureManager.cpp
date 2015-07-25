#include "TextureManager.h"
#include <string>
#include "GLRenderer.h"
#include "gl/glew.h"
#include "SOIL/SOIL.h"
#include "Utility.h"

TextureManager* TextureManager::singleton = nullptr;

TextureManager::TextureManager()
{}

TextureManager::~TextureManager()
{
    for (auto& eachElement : m_textureNameToObjectMap)
        glDeleteTextures(1, &eachElement.first);

    m_textureNameToObjectMap.clear();
}

void TextureManager::Create()
{
    singleton = new TextureManager();
}

void TextureManager::Destroy()
{
    delete singleton;
    singleton = nullptr;
}

GLType_uint TextureManager::Acquire(const std::string& textureName)
{
    uint32_t textureNameHash = Utility::HashCString(textureName.c_str());
    GLType_uint acquiredTextureObject = 0;

    try
    {
        std::pair<GLType_uint, uint32_t>& textureObject = m_textureNameToObjectMap.at(textureNameHash);
        ++textureObject.second;

        acquiredTextureObject = textureObject.first;
    }
    catch (std::out_of_range&)
    {
        std::pair<GLType_uint, uint32_t> newTextureObject;
        newTextureObject.first = SOIL_load_OGL_texture(textureName.c_str(), 0, 0, SOIL_FLAG_TEXTURE_REPEATS);
        glGetError();   // SOIL is brain dead.
        newTextureObject.second = 1;
        m_textureNameToObjectMap[textureNameHash] = newTextureObject;

        acquiredTextureObject = newTextureObject.first;
    }

    return acquiredTextureObject;
}

void TextureManager::Release(GLType_uint textureObject)
{
    bool found = false;

    auto iterator = m_textureNameToObjectMap.begin();
    for (; iterator != m_textureNameToObjectMap.end(); ++iterator)
    {
        if (iterator->second.first == textureObject)
        {
            found = true;
            break;
        }
    }
    
    if (found)
    {
        --iterator->second.second;
        if (iterator->second.second == 0)
        {
            glDeleteTextures(1, &iterator->second.first);
            m_textureNameToObjectMap.erase(iterator->first);
        }
    }
    else
    {
        assert(false);  // Trying to Release a texture that was not Acquired?
    }
}