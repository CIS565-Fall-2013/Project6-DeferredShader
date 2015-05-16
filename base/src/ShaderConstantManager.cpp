#include "ShaderConstantManager.h"
#include "Utility.h"
#include "gl/glew.h"
#include <glm/glm.hpp>

static bool AreFloatsEqual(const float a, const float b)
{
    return ((a >= (b - 1e-6)) && (a <= (b + 1e-6)));
}

ShaderConstantManager* ShaderConstantManager::singleton = nullptr;

ShaderConstantManager::ShaderConstantManager()
    : m_lastUsedProgram(-1)
{
    SetupConstantDataStore();
}

ShaderConstantManager::~ShaderConstantManager()
{
    for (auto iterator = m_programShaderConstantsMap.begin(); iterator != m_programShaderConstantsMap.end(); ++iterator)
    {
        //iterator->second->clear();
        delete iterator->second;
        iterator->second = nullptr;
    }

    for (auto iterator = m_shaderConstantNameToDataMap.begin(); iterator != m_shaderConstantNameToDataMap.end(); ++iterator)
    {
        //iterator->second->clear();
        delete iterator->second;
        iterator->second = nullptr;
    }
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

void ShaderConstantManager::SetupConstantDataStore()
{
    // Constant data store = data store for every constant we use, ever.
    // Independent of the active constants in a given program.
    // This will be replaced with code to scan shaders on the fly and create a list of uniforms/uniform buffers.
    std::string constants[] =
    {
        "u_Far",
        "u_Near",
        "u_OcclusionType",
        "u_DisplayType",
        "u_ScreenWidth",
        "u_ScreenHeight",
        "u_Light",
        "u_LightIl",
        "u_toonOn",
        "u_Persp",
        "u_Color",
        "glowmask",
        "u_Model",
        "u_View",
        "u_InvTrans",
        "u_LightCol",
        "u_InvScrHeight",
        "u_InvScrWidth",
        "u_mouseTexX",
        "u_mouseTexY",
        "u_lenQuant",
        "u_BloomOn",
        "u_DOFOn",
        "u_DOFDebug"
    };
    ShaderConstantManager::SupportedTypes constantType[] =
    {
        FLOAT,
        FLOAT,
        INT,
        INT,
        INT,
        INT,
        VEC4,
        FLOAT,
        BOOL,
        MAT4,
        VEC3,
        FLOAT,
        MAT4,
        MAT4,
        MAT4,
        VEC3,
        FLOAT,
        FLOAT,
        FLOAT,
        FLOAT,
        FLOAT,
        BOOL,
        BOOL,
        BOOL
    };

    uint32_t arrayLength = 24;
    for (uint32_t i = 0; i < arrayLength; ++i)
    {
        ShaderConstant* constant = new ShaderConstant;
        assert(constant != nullptr);
        constant->type = constantType[i];
        switch (constant->type)
        {
        case MAT4:
            constant->data = new glm::mat4;
            break;
        case VEC4:
            constant->data = new glm::vec4;
            break;
        case FLOAT:
            constant->data = new float;
            break;
        case BOOL:
        case INT:
            constant->data = new int32_t;
            break;
        case VEC3:
            constant->data = new glm::vec3;
            break;
        default:
            assert(true);
            break;
        }

        assert(constant->data != nullptr);
        m_shaderConstantNameToDataMap[constants[i]] = constant;
    }
}

void ShaderConstantManager::SetupConstantAssociationsForProgram(uint32_t programId)
{
    // This will be replaced with code to scan shaders on the fly and create a list of uniforms/uniform buffers.
    std::string constants[] =
    {
        "u_Far",
        "u_Near",
        "u_OcclusionType",
        "u_DisplayType",
        "u_ScreenWidth",
        "u_ScreenHeight",
        "u_Light",
        "u_LightIl",
        "u_toonOn",
        "u_Persp",
        "u_Color",
        "glowmask",
        "u_Model",
        "u_View",
        "u_InvTrans",
        "u_LightCol",
        "u_InvScrHeight",
        "u_InvScrWidth",
        "u_mouseTexX",
        "u_mouseTexY",
        "u_lenQuant",
        "u_BloomOn",
        "u_DOFOn",
        "u_DOFDebug"
    };

    uint32_t arrayLength = 24;
    std::vector<std::string>* constantList = new std::vector<std::string>(arrayLength);
    constantList->resize(arrayLength);

    uint32_t activeConstants = 0;
    for (uint32_t i = 0; i < arrayLength; ++i)
    {
        int32_t constantBindLocation = glGetUniformLocation(programId, constants[i].c_str());
        if (constantBindLocation > -1)
        {
            (*constantList)[constantBindLocation] = constants[i];
            ++activeConstants;
        }
    }
    constantList->resize(activeConstants);

    if (constantList->size())
    {
        m_programShaderConstantsMap[programId] = constantList;
    }
    else
    {
        delete constantList;    // DO NOT delete constantList if it has a size, since it will get assigned to the shaderConstantsMap. It'll get deleted/destroyed when ShaderConstantManager gets destroyed.
    }
}

void ShaderConstantManager::SetShaderConstant(const std::string& constantName, const void* value_in)
{
    try
    {
        ShaderConstant* constant = m_shaderConstantNameToDataMap.at(constantName);
        assert(constant != nullptr);
        assert(constant->data != nullptr);

        if (constant->type == MAT4)
        {
            glm::mat4& constantData = *(reinterpret_cast<glm::mat4*>(constant->data));
            const glm::mat4& value = *(reinterpret_cast<const glm::mat4*>(value_in));
            if (glm::any(glm::notEqual(constantData[0], value[0])) || 
                glm::any(glm::notEqual(constantData[1], value[1])) ||
                glm::any(glm::notEqual(constantData[2], value[2])) || 
                glm::any(glm::notEqual(constantData[3], value[3])))
            {
                constantData = value;
                constant->dirty = true;
            }
        }
        else if (constant->type == VEC3)
        {
            glm::vec3& constantData = *(reinterpret_cast<glm::vec3*>(constant->data));
            const glm::vec3& value = *(reinterpret_cast<const glm::vec3*>(value_in));
            if (glm::any(glm::notEqual(constantData, value)))
            {
                constantData = value;
                constant->dirty = true;
            }
        }
        else if (constant->type == VEC4)
        {
            glm::vec4& constantData = *(reinterpret_cast<glm::vec4*>(constant->data));
            const glm::vec4& value = *(reinterpret_cast<const glm::vec4*>(value_in));
            if (glm::any(glm::notEqual(constantData, value)))
            {
                constantData = value;
                constant->dirty = true;
            }
        }
        else if (constant->type == FLOAT)
        {
            float& constantData = *(reinterpret_cast<float*>(constant->data));
            const float& value = *(reinterpret_cast<const float*>(value_in));
            if (!AreFloatsEqual(constantData, value))
            {
                constantData = value;
                constant->dirty = true;
            }
        }
        else if ((constant->type == INT) || (constant->type == BOOL))
        {
            int32_t& constantData = *(reinterpret_cast<int32_t*>(constant->data));
            const int32_t& value = *(reinterpret_cast<const int32_t*>(value_in));
            if (constantData != value)
            {
                constantData = value;
                constant->dirty = true;
            }
        }
        else
            assert(true);
    }
    catch (std::out_of_range&)
    {
        // Invalid shader constant. Ignore!
        std::string shaderConstantNotFoundMessage;
        //shaderConstantNotFoundMessage.append("Shader constant ");
        //shaderConstantNotFoundMessage.append(constantName);
        //shaderConstantNotFoundMessage.append(" not found in map. Adding...\n");
        shaderConstantNotFoundMessage.append("Invalid shader constant. Ignoring..\n");
        Utility::LogOutput(shaderConstantNotFoundMessage.c_str());
    }
}

void ShaderConstantManager::ApplyShaderConstantsForProgram(uint32_t program) const
{
    const std::vector<std::string>* shaderConstantArrayPointer = nullptr;
    try
    {
        shaderConstantArrayPointer = m_programShaderConstantsMap.at(program);
        assert(shaderConstantArrayPointer != nullptr);
    }
    catch (std::out_of_range&)
    {
        assert(true); // Invalid program!
        return;
    }

    const std::vector<std::string>& shaderConstantArray = *shaderConstantArrayPointer;
    for (uint32_t i = 0; i < shaderConstantArray.size(); ++i)
    {
        const std::string& thisConstant = shaderConstantArray[i];
        ShaderConstant* constant = m_shaderConstantNameToDataMap.at(thisConstant);
        assert(constant != nullptr);
        assert(constant->data != nullptr);
        if ((program != m_lastUsedProgram) || constant->dirty)
        {
            switch (constant->type)
            {
            case MAT4:
                glUniformMatrix4fv(i, 1, GL_FALSE, static_cast<GLfloat*>(constant->data));
                break;
            case VEC4:
                glUniform4fv(i, 1, static_cast<GLfloat*>(constant->data));
                break;
            case VEC3:
                glUniform3fv(i, 1, static_cast<GLfloat*>(constant->data));
                break;
            case FLOAT:
                glUniform1fv(i, 1, static_cast<GLfloat*>(constant->data));
                break;
            case BOOL:
            case INT:
                glUniform1iv(i, 1, static_cast<GLint*>(constant->data));
                break;
            default:
                assert(true);   // Unsupported shader constant type!
                break;
            }
            constant->dirty = false;
        }
    }

    m_lastUsedProgram = program;
}
