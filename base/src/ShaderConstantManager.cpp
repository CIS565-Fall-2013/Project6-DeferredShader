#include "ShaderConstantManager.h"
#include "Utility.h"
#include "gl/glew.h"
#include <glm/glm.hpp>
#include <sstream>

static bool AreVec3sEqual(const glm::vec3& a, const glm::vec3& b)
{
    return (AreFloatsEqual(a.x, b.x) && AreFloatsEqual(a.y, b.y) && AreFloatsEqual(a.z, b.z));
}

static bool AreVec4sEqual(const glm::vec4& a, const glm::vec4& b)
{
    return (AreFloatsEqual(a.x, b.x) && AreFloatsEqual(a.y, b.y) && AreFloatsEqual(a.z, b.z) && AreFloatsEqual(a.w, b.w));
}

static bool AreMat4sEqual(const glm::mat4& a, const glm::mat4& b)
{
    return (AreVec4sEqual(a[0], b[0]) && AreVec4sEqual(a[1], b[1]) && AreVec4sEqual(a[2], b[2]) && AreVec4sEqual(a[3], b[3]));
}

std::weak_ptr<ShaderConstantManager> ShaderConstantManager::singleton;
uint32_t ShaderConstantManager::resolver = 0;

ShaderConstantManager::ShaderConstantManager()
{
}

ShaderConstantManager::~ShaderConstantManager()
{
    GLType_uint* constantBufferIds = new GLType_uint[m_constantBufferIndexToDataMap.size()];
    uint32_t i = 0;
    for (auto& iterator : m_constantBufferIndexToDataMap)
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

// We want Create() to create the singular instance, and GetSingleton() to return that instance. Creation needs to be explicit.
std::shared_ptr<ShaderConstantManager> ShaderConstantManager::Create()
{
    try
    {
        return std::shared_ptr<ShaderConstantManager>(ShaderConstantManager::GetSingleton());
    }
    catch (std::bad_weak_ptr&)
    {
        ShaderConstantManager* pShaderConstantManager = new ShaderConstantManager;
        std::shared_ptr<ShaderConstantManager> newShaderConstantManager = std::shared_ptr<ShaderConstantManager>(pShaderConstantManager);
        singleton = newShaderConstantManager;
        return newShaderConstantManager;
    }
}

std::weak_ptr<ShaderConstantManager>& ShaderConstantManager::GetSingleton()
{
    return singleton;
}

ShaderConstantManager::SupportedTypes ShaderConstantManager::GetTypeFromString(const std::string& typeString)
{
    if (typeString.compare("float") == 0)
        return FLOAT;
    else if (typeString.compare("int") == 0)
        return INT;
    else if (typeString.compare("bool") == 0)
        return BOOL;
    else if ((typeString.find("mat4") != std::string::npos) &&
        ((typeString.compare("mat4") == 0) || (typeString.compare("mat4x4") == 0)))
        return MAT4;
    else if (typeString.compare("vec3") == 0)
        return VEC3;
    else if (typeString.compare("vec4") == 0)
        return VEC4;
    else
        assert(false);
    
    return VEC4;
}

uint32_t ShaderConstantManager::GetAlignmentForType(ShaderConstantManager::SupportedTypes type)
{
    switch (type)
    {
    case BOOL:
    case INT:
    case FLOAT:
        return 4;  // If the member is a scalar consuming N basic machine units, the base alignment is N. (Rule 1)
    default:
        assert(false); // Unknown/Unsupported type. Fall through to vec4.
    case VEC4: // If the member is a four-component vector with components consuming N (4) basic machine units, the base alignment is 4N. (Rule 2)
    case VEC3: // If the member is a three-component vector with components consuming N (4) basic machine units, the base alignment is 4N. (Rule 3)
    case MAT4:
        // If the member is an array of scalars or vectors, the base alignment and array stride are set to match the base alignment of a single array element, according to rules 1, 2, and 3, and 
        // rounded up to the base alignment of a vec4. (Rule 4)
        // If the member is a column-major matrix with C (4) columns and R (4) rows, the matrix is stored identically to an array of C column vectors with R components each, according to rule 4. (Rule 5)
        return 16;
//    case VEC2:
//        return 8;
    }
}

uint32_t ShaderConstantManager::GetSizeForType(ShaderConstantManager::SupportedTypes type)
{
    switch (type)
    {
    case MAT4:
        return 64;
    case VEC3:
        return 12;  
//    case VEC2:
//        return 8;
    case BOOL:
    case INT:
    case FLOAT:
        return 4;
    default:
        assert(false); // Unknown/Unsupported type. Fall through to vec4.
    case VEC4:
        return 16;
    }
}

ConstantBufferIndex ShaderConstantManager::SetupConstantBuffer(std::string& constantBufferName, int32_t constantBufferSize, std::vector<ShaderConstantSignature>& constantBufferSignature)
{
    ConstantBufferIndex cbIndex = Utility::HashCString(constantBufferName.c_str());
    auto& mapItr = m_constantBufferIndexToDataMap.find(cbIndex);
    if (mapItr != m_constantBufferIndexToDataMap.end())
    {
        ConstantBuffer* existingBuffer = mapItr->second;

        // Already exists. Check if the signatures match:
        uint32_t i;
        const auto& mapEnd = existingBuffer->m_signature.end();
        for (i = 0; i < constantBufferSignature.size(); ++i)
        {
            const auto& mapItr2 = existingBuffer->m_signature.find(Utility::HashCString(constantBufferSignature[i].name.c_str()));
            if (mapItr2 != mapEnd)
            {
                const ShaderConstantSignature& thisSignature = mapItr2->second;
                if ((constantBufferSignature[i].type == thisSignature.type) && (constantBufferSignature[i].size == thisSignature.size) && (constantBufferSignature[i].offset == thisSignature.offset))
                    continue;
                else
                    break;
            }
            else
            {
                break;
            }
        }

        if (i == constantBufferSignature.size())    // Signatures match. Don't create a duplicate. 
            return cbIndex;
        
        // If not, try again with resolver integer appended to buffer name.
        do
        {
            std::ostringstream newConstantBufferName;
            newConstantBufferName << constantBufferName << resolver++;
            cbIndex = Utility::HashCString(newConstantBufferName.str().c_str());
        } while (m_constantBufferIndexToDataMap.count(cbIndex) != 0); // Currently, it'd be impossible for count to be != 0 here.
    }

    ConstantBuffer* newConstantBuffer = new ConstantBuffer();
    assert(newConstantBuffer != nullptr);
    assert(constantBufferSize > 0);

    for (const auto& thisSignature : constantBufferSignature)
        newConstantBuffer->m_signature[Utility::HashCString(thisSignature.name.c_str())] = thisSignature;
    newConstantBuffer->m_data = new char[constantBufferSize];
    assert(newConstantBuffer->m_data != nullptr);
    newConstantBuffer->m_size = constantBufferSize;
    memset(newConstantBuffer->m_data, 0, constantBufferSize);
    m_constantBufferIndexToDataMap[cbIndex] = newConstantBuffer;

    glCreateBuffers(1, &newConstantBuffer->m_id);
    glNamedBufferStorage(newConstantBuffer->m_id, newConstantBuffer->m_size, newConstantBuffer->m_data, GL_MAP_WRITE_BIT); // We'll use unsynchronized MapBufferRange to modify the data in its data store.
    return cbIndex;
}

void ShaderConstantManager::SetShaderConstant(ShaderConstantReference constantHandle, ConstantBufferIndex indexOfConstantBuffer, const void* value_in)
{
    try
    {
        ConstantBuffer* constantBuffer = m_constantBufferIndexToDataMap.at(indexOfConstantBuffer);
        const ShaderConstantSignature& constantSignature = constantBuffer->m_signature.at(constantHandle);
        char* data = reinterpret_cast<char*>(constantBuffer->m_data);
        data += constantSignature.offset;
        const char* value_in_bytePtr = reinterpret_cast<const char*>(value_in);
        switch (constantSignature.type)
        {
            case MAT4:
            {
                glm::mat4& constantData = reinterpret_cast<glm::mat4&>(*data); // This is safe, because mat4s are an array of 4 vec4s, and each element of all types of arrays have vec4 alignment.
                const glm::mat4& value = reinterpret_cast<const glm::mat4&>(*value_in_bytePtr);
                if (!AreMat4sEqual(constantData, value))
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case VEC3:
            {
                glm::vec3& constantData = reinterpret_cast<glm::vec3&>(*data); // This is safe, because even though vec3s have vec4 alignment, they only use the first 12 bytes from their start pos.
                const glm::vec3& value = reinterpret_cast<const glm::vec3&>(*value_in_bytePtr); 
                if (!AreVec3sEqual(constantData, value))
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case VEC4:
            {
                glm::vec4& constantData = reinterpret_cast<glm::vec4&>(*data);
                const glm::vec4& value = reinterpret_cast<const glm::vec4&>(*value_in_bytePtr);
                if (!AreVec4sEqual(constantData, value))
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case BOOL:
            case INT:
            {
                int32_t& constantData = reinterpret_cast<int32_t&>(*data);
                const int32_t& value = reinterpret_cast<const int32_t&>(*value_in_bytePtr);
                if (constantData != value)
                {
                    constantData = value;
                    constantBuffer->m_dirty = true;
                }
                break;
            }
            case FLOAT:
            {
                float& constantData = reinterpret_cast<float&>(*data);
                const float& value = reinterpret_cast<const float&>(*value_in_bytePtr);
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

void ShaderConstantManager::ApplyShaderConstantChanges(ConstantBufferIndex indexOfCBToApplyChangesTo) const
{
    if (indexOfCBToApplyChangesTo == 0)
    {
        for (auto& itr : m_constantBufferIndexToDataMap)
        {
            if (itr.second->m_dirty)
            {
                void* pDataStore = glMapNamedBufferRange(itr.second->m_id, 0, itr.second->m_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
                memcpy(pDataStore, itr.second->m_data, itr.second->m_size);
                glUnmapNamedBuffer(itr.second->m_id);
                itr.second->m_dirty = false;
            }
        }
    }
    else
    {
        try
        {
            ConstantBuffer* constantBuffer = m_constantBufferIndexToDataMap.at(indexOfCBToApplyChangesTo);
            if (constantBuffer->m_dirty)
            {
                void* pDataStore = glMapNamedBufferRange(constantBuffer->m_id, 0, constantBuffer->m_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
                memcpy(pDataStore, constantBuffer->m_data, constantBuffer->m_size);
                glUnmapNamedBuffer(constantBuffer->m_id);
                constantBuffer->m_dirty = false;
            }
        }
        catch (std::out_of_range&)
        {
            assert(false);  // No such constant buffer.
        }
    }
}

GLType_uint ShaderConstantManager::GetConstantBufferObject(ConstantBufferIndex constantBufferIndex) const
{
    try
    {
        ConstantBuffer* buffer = m_constantBufferIndexToDataMap.at(constantBufferIndex);
        assert(buffer != nullptr);
        return buffer->m_id;
    }
    catch (std::out_of_range&)
    {
        assert(false);  // No such constant buffer.
        return UINT32_MAX;
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