#include "ShaderConstantManager.h"
#include "Utility.h"
#include "gl/glew.h"
#include <glm/glm.hpp>
#include <sstream>

static bool AreFloatsEqual(const float a, const float b)
{
    return ((a >= (b - 1e-6)) && (a <= (b + 1e-6)));
}

ShaderConstantManager* ShaderConstantManager::singleton = nullptr;
uint32_t ShaderConstantManager::resolver = 0;

ShaderConstantManager::ShaderConstantManager()
{
}

ShaderConstantManager::~ShaderConstantManager()
{
    for (auto& iterator : m_shaderConstantNameToConstantBufferMap)
    {
        //iterator->second->clear();
        iterator.second = nullptr;
    }

    uint32_t* constantBufferIds = new uint32_t[m_constantBufferNameToDataMap.size()];
    uint32_t i = 0;
    for (auto& iterator : m_constantBufferNameToDataMap)
    {
        //iterator->second->clear();
        if (iterator.second)
        {
            constantBufferIds[i] = iterator.second->m_id;
            ++i;
        }
        delete iterator.second;
        iterator.second = nullptr;
    }

    glDeleteBuffers(i, constantBufferIds);
}

void ShaderConstantManager::Create()
{
    assert(singleton == nullptr);
    singleton = new ShaderConstantManager();
    assert(singleton != nullptr);
}

void ShaderConstantManager::Destroy()
{
    assert(singleton != nullptr);
    delete singleton;
    singleton = nullptr;
}

ShaderConstantManager::SupportedTypes ShaderConstantManager::GetTypeFromString(const std::string& typeString)
{
    if (typeString.compare("float") == 0)
        return FLOAT;
    else if (typeString.compare("int") == 0)
        return INT;
    else if (typeString.compare("bool") == 0)
        return BOOL;
    else if (typeString.find("mat4") != std::string::npos)
        return MAT4;
    else if (typeString.compare("vec3") == 0)
        return VEC3;
    else if (typeString.compare("vec4") == 0)
        return VEC4;
    else
        assert(false);
}

uint32_t ShaderConstantManager::GetSizeForType(ShaderConstantManager::SupportedTypes type)
{
    switch (type)
    {
    case MAT4:
        return 64;
    case VEC3:
    case VEC4:
        return 16;
    case BOOL:
    case INT:
    case FLOAT:
        return 4;
    default:
        assert(false);
    }
}

void ShaderConstantManager::SetupConstantBuffer(std::string& constantBufferName, std::vector<ShaderConstantSignature>& constantBufferSignature)
{
    try
    {
        ConstantBuffer* existingBuffer = m_constantBufferNameToDataMap.at(constantBufferName);

        // Already exists. Check if the signatures match:
        uint32_t i;
        for (i = 0; i < constantBufferSignature.size(); ++i)
        {
            try
            {
                const ShaderConstantSignature& thisSignature = existingBuffer->m_signature.at(constantBufferSignature[i].name);
                if ((constantBufferSignature[i].type == thisSignature.type) && (constantBufferSignature[i].size == thisSignature.size) && (constantBufferSignature[i].offset == thisSignature.offset))
                    continue;
                else
                    break;
            }
            catch (std::out_of_range&)
            {
                break;
            }
        }

        if (i == constantBufferSignature.size())    // Signatures match. Don't create a duplicate. 
            return;
        
        // If not, try again with resolver integer appended to buffer name.
        std::ostringstream newConstantBufferName;
        newConstantBufferName << constantBufferName << resolver++;
        constantBufferName = std::string(newConstantBufferName.str());
        m_constantBufferNameToDataMap.at(constantBufferName);
    }
    catch (std::out_of_range&)
    {
        ConstantBuffer* newConstantBuffer = new ConstantBuffer();
        assert(newConstantBuffer != nullptr);
        uint32_t bufferSize = 0;
        for (const auto& iterator : constantBufferSignature)
        {
            newConstantBuffer->m_signature[iterator.name] = iterator;
            bufferSize += iterator.offset;
        }
        bufferSize += constantBufferSignature.back().size;

        newConstantBuffer->m_data = new char[bufferSize];
        memset(newConstantBuffer->m_data, 0, bufferSize);
        glGenBuffers(1, &newConstantBuffer->m_id);
        m_constantBufferNameToDataMap[constantBufferName] = newConstantBuffer;
    }

    ApplyShaderConstantChanges(constantBufferName);
}

void ShaderConstantManager::SetShaderConstant(const std::string& constantName, const std::string& constantBufferName, const void* value_in)
{
    try
    {
        ConstantBuffer* constantBuffer = m_constantBufferNameToDataMap.at(constantBufferName);
        const ShaderConstantSignature& constantSignature = constantBuffer->m_signature.at(constantName);
        char* data = reinterpret_cast<char*>(constantBuffer->m_data);
        data += constantSignature.offset;
        switch (constantSignature.type)
        {
            case MAT4:
            {
                glm::mat4& constantData = reinterpret_cast<glm::mat4&>(data);
                const glm::mat4& value = reinterpret_cast<const glm::mat4&>(value_in);
                if (glm::any(glm::notEqual(constantData[0], value[0])) ||
                    glm::any(glm::notEqual(constantData[1], value[1])) ||
                    glm::any(glm::notEqual(constantData[2], value[2])) ||
                    glm::any(glm::notEqual(constantData[3], value[3])))
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case VEC3:
            {
                glm::vec3& constantData = reinterpret_cast<glm::vec3&>(data);
                const glm::vec3& value = reinterpret_cast<const glm::vec3&>(value_in);
                if (glm::any(glm::notEqual(constantData, value)))
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case VEC4:
            {
                glm::vec4& constantData = reinterpret_cast<glm::vec4&>(data);
                const glm::vec4& value = reinterpret_cast<const glm::vec4&>(value_in);
                if (glm::any(glm::notEqual(constantData, value)))
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case BOOL:
            case INT:
            {
                int32_t& constantData = reinterpret_cast<int32_t&>(data);
                const int32_t& value = reinterpret_cast<const int32_t&>(value_in);
                if (constantData != value)
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case FLOAT:
            {
                float& constantData = reinterpret_cast<float&>(data);
                const float& value = reinterpret_cast<const float&>(value_in);
                if (!AreFloatsEqual(constantData, value))
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            default:
                assert(false);
                break;
        }
    }
    catch (std::out_of_range&)
    {
        assert(false);  // No such constant buffer, or the specified constant doesn't exist in the specified constant buffer.
    }
}

void ShaderConstantManager::ApplyShaderConstantChanges(const std::string& constantBufferName /* = std::string() */) const
{
    if (constantBufferName.length() == 0)
    {
        for (auto& itr : m_constantBufferNameToDataMap)
        {
            if (itr.second->m_dirty)
            {
                glBindBuffer(GL_UNIFORM_BUFFER, itr.second->m_id);
                glBufferData(GL_UNIFORM_BUFFER, itr.second->m_size, itr.second->m_data, GL_STATIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                itr.second->m_dirty = false;
            }
        }
    }
    else
    {
        try
        {
            ConstantBuffer* constantBuffer = m_constantBufferNameToDataMap.at(constantBufferName);
            if (constantBuffer->m_dirty)
            {
                glBindBuffer(GL_UNIFORM_BUFFER, constantBuffer->m_id);
                glBufferData(GL_UNIFORM_BUFFER, constantBuffer->m_size, constantBuffer->m_data, GL_STATIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                constantBuffer->m_dirty = false;
            }
        }
        catch (std::out_of_range&)
        {
            assert(false);  // No such constant buffer.
        }
    }
}

uint32_t ShaderConstantManager::GetConstantBufferObject(const std::string& constantBufferName) const
{
    try
    {
        ConstantBuffer* buffer = m_constantBufferNameToDataMap.at(constantBufferName);
        assert(buffer != nullptr);
        return buffer->m_id;
    }
    catch (std::out_of_range&)
    {
        assert(false);  // No such constant buffer.
    }
}

ConstantBuffer::ConstantBuffer()
    : m_data(nullptr),
    m_dirty(true),
    m_size(0),
    m_id(0)
{}

ConstantBuffer::~ConstantBuffer()
{
    delete[] m_data;
    m_data = nullptr;
    m_signature.clear();
    m_size = 0;
    m_id = 0;
}