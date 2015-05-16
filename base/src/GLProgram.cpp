#include "GLProgram.h"
#include "ShaderConstantManager.h"
#include "Utility.h"

GLProgram::GLProgram()
    : m_id(0)
{
}

GLProgram::GLProgram(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles, 
    const std::map<std::string, uint32_t>& attributeBindIndices, const std::map<std::string, uint32_t>& outputBindIndices)
    : m_id(0)
{
    for (auto itr : attributeBindIndices)
    {
        SetAttributeBindLocation(itr.first, itr.second);
    }

    for (auto itr : outputBindIndices)
    {
        SetOutputBindLocation(itr.first, itr.second);
    }

    Create(programType, shaderSourceFiles);
}

GLProgram::~GLProgram()
{
}

void GLProgram::Create(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles)
{
    Utility::shaders_t shaders;

    std::string vert_shader, frag_shader;     // More shader types to be supported later.
    for (auto i : shaderSourceFiles)
    {
        if (i.second == RenderEnums::VERT)
            vert_shader = i.first;
        else if (i.second == RenderEnums::FRAG)
            frag_shader = i.first;
    }
    shaders = Utility::loadShaders(vert_shader.c_str(), frag_shader.c_str());
    m_id = glCreateProgram();
    assert(m_id != 0);

    for (auto itr : m_attributeBindIndicesMap)
        glBindAttribLocation(m_id, itr.second, itr.first.c_str());
    for (auto itr : m_outputBindIndicesMap)
        glBindFragDataLocation(m_id, itr.second, itr.first.c_str());
    GLenum gl_error = glGetError();
    assert(gl_error == GL_NO_ERROR);

    Utility::attachAndLinkProgram(m_id, shaders);

    ShaderConstantManager::GetSingleton()->SetupConstantAssociationsForProgram(m_id);
    SetupTextureBindings();
}

void GLProgram::SetActive() const
{
    glUseProgram(m_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLProgram::SetShaderConstant(const std::string& constantName, const void* value_in) const
{
    ShaderConstantManager::GetSingleton()->SetShaderConstant(constantName, value_in);
}

void GLProgram::SetupTextureBindings()
{
    // This will be replaced with code to scan shaders on the fly and create a list of textures.
    std::string constants[] =
    {
        "u_Depthtex", "u_Normaltex", "u_Positiontex", "u_depthTex", "u_normalTex", "u_positionTex", "u_Colortex", "u_RandomNormaltex", "u_RandomScalartex", "u_GlowMask", "u_Posttex"
    };

    uint32_t arrayLength = 11;
    for (std::string& i : constants)
    {
        int32_t constantBindLocation = glGetUniformLocation(m_id, i.c_str());
        if (constantBindLocation > -1)
        {
            m_textureBindIndicesMap[i] = std::make_pair(constantBindLocation, 0);
        }
    }
}

void GLProgram::SetTexture(const std::string& textureName, uint32_t textureObject)
{
    try
    {
        std::pair<uint32_t, uint32_t>& textureBindPoint = m_textureBindIndicesMap.at(textureName);
        textureBindPoint.second = textureObject;
    }
    catch (std::out_of_range&)
    {
        Utility::LogOutput("Trying to bind an invalid texture!\n");
    }
}

bool GLProgram::GetAttributeBindLocation(const std::string& attributeName, uint32_t& bindLocation) const
{
    try
    {
        bindLocation = m_attributeBindIndicesMap.at(attributeName);
    }
    catch (std::out_of_range&)
    {
        return false;   // Invalid attribute name.
    }

    return true;
}

bool GLProgram::GetOutputBindLocation(const std::string& outputName, uint32_t& bindLocation) const
{
    try
    {
        bindLocation = m_outputBindIndicesMap.at(outputName);
    }
    catch (std::out_of_range&)
    {
        return false;   // Invalid output name.
    }

    return true;
}

void GLProgram::CommitConstantBufferBindings() const
{
    //for (auto itr : m_constantBufferBindIndicesMap)
    //{
    //    glBindBufferBase(GL_UNIFORM_BUFFER, itr.second, ShaderConstantManager::GetSingleton()->GetConstantBufferObject(itr.first));
    //}
    ShaderConstantManager::GetSingleton()->ApplyShaderConstantsForProgram(m_id);
}

void GLProgram::CommitTextureBindings() const
{
    uint32_t i = 0;
    for (auto itr : m_textureBindIndicesMap)
    {
        if (itr.second.second > 0)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, itr.second.second);
            glUniform1i(itr.second.first, i);
            ++i;
        }
    }
}
