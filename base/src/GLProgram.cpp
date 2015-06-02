#include "GLProgram.h"
#include "ShaderConstantManager.h"
#include "Utility.h"

static void tokenizer(const std::string& sourceString, std::vector<std::string>& tokenList);
static ShaderConstantManager::SupportedTypes GLTypeToSupportedType(GLint gltype);

GLProgram::GLProgram()
    : m_id(0)
{
}

GLProgram::GLProgram(RenderEnums::ProgramType programType, const std::vector<std::pair<std::string, RenderEnums::RenderProgramStage>>& shaderSourceFiles, 
    const std::map<std::string, uint32_t>& attributeBindIndices, const std::map<std::string, uint32_t>& outputBindIndices)
    : m_id(0)
{
    for (const auto& itr : attributeBindIndices)
    {
        SetAttributeBindLocation(itr.first, itr.second);
    }

    for (const auto& itr : outputBindIndices)
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

    int32_t size = 0;
    char* shaderSourceRaw = Utility::loadFile(vert_shader.c_str(), size);
    std::string vertShaderSource(shaderSourceRaw);
    delete[] shaderSourceRaw;

    shaderSourceRaw = Utility::loadFile(frag_shader.c_str(), size);
    std::string fragShaderSource(shaderSourceRaw);
    delete[] shaderSourceRaw;
    shaderSourceRaw = nullptr;

    std::string workingDirectory = vert_shader.substr(0, vert_shader.find_last_of('\\'));
    if (!workingDirectory.length())
        workingDirectory = vert_shader.substr(0, vert_shader.find_last_of('/'));
    PreprocessShaderSource(vertShaderSource, workingDirectory);

    workingDirectory = frag_shader.substr(0, frag_shader.find_last_of('\\'));
    if (!workingDirectory.length())
        workingDirectory = frag_shader.substr(0, frag_shader.find_last_of('/'));
    PreprocessShaderSource(fragShaderSource, workingDirectory);

    shaders = Utility::createShaders(vertShaderSource, fragShaderSource);
    m_id = glCreateProgram();
    assert(m_id != 0);

    for (const auto& itr : m_attributeBindIndicesMap)
        glBindAttribLocation(m_id, itr.second, itr.first.c_str());
    for (const auto& itr : m_outputBindIndicesMap)
        glBindFragDataLocation(m_id, itr.second, itr.first.c_str());
    GLenum gl_error = glGetError();
    assert(gl_error == GL_NO_ERROR);

    Utility::attachAndLinkProgram(m_id, shaders);

    SetupTextureBindingsAndConstantBuffers(vertShaderSource);
    SetupTextureBindingsAndConstantBuffers(fragShaderSource);
}

void GLProgram::SetActive() const
{
    glUseProgram(m_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLProgram::SetShaderConstant(const std::string& constantName, const void* value_in) const
{
    try
    {
        const std::string& mappedConstantBuffer = m_shaderConstantToConstantBufferBindingMap.at(constantName);
        ShaderConstantManager::GetSingleton()->SetShaderConstant(constantName, mappedConstantBuffer, value_in);
    }
    catch (std::out_of_range&)
    {
        assert(false);  // Constant should be mapped to a constant buffer.
    }
}

void GLProgram::PreprocessShaderSource(std::string& shaderSource, const std::string& workingDirectory) const
{
    // Prepend Includes.
    while (1)
    {
        std::size_t includePosition = shaderSource.find("#include");
        if (includePosition != std::string::npos)
        {
            std::size_t includeEndPosition = shaderSource.find('\n', includePosition);
            std::size_t includeNameEnd = includeEndPosition;
            while ((shaderSource[includeNameEnd] == '\r') || (shaderSource[includeNameEnd] == '\n') || (shaderSource[includeNameEnd] == '\t') || (shaderSource[includeNameEnd] == ' '))
                --includeNameEnd;
            std::string headerNameWithQuotes = shaderSource.substr(includePosition + 9, includeNameEnd);  // 9: '#','i','n','c','l','u','d','e',' '
            std::string headerName = headerNameWithQuotes.substr(headerNameWithQuotes.find_first_of('"'), headerNameWithQuotes.find_last_of('"'));
            headerName.insert(0, workingDirectory);

            int32_t headerSize = 0;
            char* includeSourceRaw = Utility::loadFile(headerName.c_str(), headerSize);
            shaderSource.replace(includePosition, includeEndPosition, includeSourceRaw);
            delete[] includeSourceRaw;  includeSourceRaw = nullptr;
        }
        else
            break;
    }
}

void GLProgram::SetupTextureBindingsAndConstantBuffers(const std::string& shaderSource)
{
    // Tokenize
    std::vector<std::string> tokenList;
    tokenizer(shaderSource, tokenList);

    // Inspect all "uniform" tokens
    std::vector<uint32_t> uniformTokenPositions;
    for (uint32_t i = 0; i < tokenList.size(); ++i)
    {
        if (tokenList[i].compare("uniform") == 0)
            uniformTokenPositions.push_back(i);
    }

    std::vector<std::string> activeTextures;
    std::vector<std::string> activeUniforms;
    std::string constBufferName;
    std::vector<ShaderConstantSignature> constBufferSignature;
    for (auto& i : uniformTokenPositions)
    {
        // Gather all sampler uniforms - these will be passed to SetupTextureBindings()
        if (tokenList[i + 1].find("sampler") != std::string::npos)
        {
            activeTextures.push_back(tokenList[i + 2]);
        }
        else
        {
            bool stdLayout = false;
            if (tokenList[i - 1].find("std140") != std::string::npos)
            {
                stdLayout = true;
            }

            constBufferName = tokenList[i + 1];

            uint32_t itr = i+3;
            ShaderConstantSignature thisSignature;
            uint32_t stdOffset = 0;
            while (tokenList[itr].compare("};") != 0)
            {
                if (stdLayout)
                {
                    thisSignature.type = ShaderConstantManager::GetTypeFromString(tokenList[itr++]);
                    thisSignature.name = tokenList[itr++];
                    thisSignature.name.pop_back();  // Get rid of trailing ;

                    // Calculate offset and size using std140 layout rules.
                    thisSignature.offset = stdOffset;
                    thisSignature.size = ShaderConstantManager::GetSizeForType(thisSignature.type);
                    stdOffset += thisSignature.size;
                    constBufferSignature.push_back(thisSignature);
                }
                else
                {
                    // Push all the uniforms into an array, and query the offsets and sizes of active ones. 
                    activeUniforms.push_back(thisSignature.name);
                }
            }

            if (!stdLayout)
            {
                // Query sizes and offsets.
                uint32_t numUniforms = activeUniforms.size();

                if (numUniforms)
                {
                    const char* uniformsList = activeUniforms.data()->c_str();
                    uint32_t* uniformIndicesList = new uint32_t[numUniforms];
                    int32_t* uniformSizesList = new int32_t[numUniforms];
                    int32_t* uniformOffsetsList = new int32_t[numUniforms];
                    int32_t* uniformTypesList = new int32_t[numUniforms];

                    glGetUniformIndices(m_id, numUniforms, &uniformsList, uniformIndicesList);
                    glGetActiveUniformsiv(m_id, numUniforms, uniformIndicesList, GL_UNIFORM_SIZE, uniformSizesList);
                    glGetActiveUniformsiv(m_id, numUniforms, uniformIndicesList, GL_UNIFORM_OFFSET, uniformOffsetsList);
                    glGetActiveUniformsiv(m_id, numUniforms, uniformIndicesList, GL_UNIFORM_TYPE, uniformTypesList);

                    for (uint32_t i = 0; i < numUniforms; ++i)
                    {
                        if (uniformIndicesList[i] != GL_INVALID_INDEX)
                        {
                            thisSignature.name = activeUniforms[i];
                            thisSignature.size = uniformSizesList[i];
                            thisSignature.offset = uniformOffsetsList[i];
                            thisSignature.type = GLTypeToSupportedType(uniformTypesList[i]);

                            constBufferSignature.push_back(thisSignature);
                        }
                    }

                    delete[] uniformIndicesList;
                    delete[] uniformSizesList;
                    delete[] uniformOffsetsList;
                    delete[] uniformTypesList;
                }
            }
            
            std::string constantBufferNameAsInSource(constBufferName);  // When setting up the constant buffer, if a buffer by the same name exists, an aliased version is created by the ShaderConstantManager.
                // This constant buffer is referred to by the aliased name C++-side, but when querying the block index, we still need to use the name as it appears in the shader.
            ShaderConstantManager::GetSingleton()->SetupConstantBuffer(constBufferName, constBufferSignature);
            for (auto& iterator : activeUniforms)
            {
                try
                {
                    std::string& alreadyMappedConstBuffer = m_shaderConstantToConstantBufferBindingMap.at(iterator);
                    if (alreadyMappedConstBuffer.compare(constBufferName) != 0)
                        assert(false);  // This constant is already mapped to a different constant buffer.
                }
                catch (std::out_of_range&)
                {
                    m_shaderConstantToConstantBufferBindingMap[iterator] = constBufferName;
                }
            }

            m_constantBufferBindIndicesMap[constBufferName] = glGetUniformBlockIndex(m_id, constantBufferNameAsInSource.c_str());

            activeUniforms.clear();
        }
    }

    SetupTextureBindings(activeTextures);
}

void GLProgram::SetupTextureBindings(const std::vector<std::string>& textureNames)
{
    for (const std::string& i : textureNames)
    {
        try
        {
            m_textureBindIndicesMap.at(i);
        }
        catch (std::out_of_range&)
        {
            // Add if it doesn't already exist.
            int32_t constantBindLocation = glGetUniformLocation(m_id, i.c_str());
            if (constantBindLocation > -1)
            {
                m_textureBindIndicesMap[i] = std::make_pair(constantBindLocation, 0);
            }
            else
                assert(false); // SetupTextureBindings was passed a texture name that isn't active in the program? Update the shader so that this wouldn't happen anymore.
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
        assert(false); // Trying to bind an invalid texture.
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
    for (const auto& itr : m_constantBufferBindIndicesMap)
    {
        ShaderConstantManager::GetSingleton()->ApplyShaderConstantChanges(itr.first);
        glBindBufferBase(GL_UNIFORM_BUFFER, itr.second, ShaderConstantManager::GetSingleton()->GetConstantBufferObject(itr.first));
    }
}

void GLProgram::CommitTextureBindings() const
{
    uint32_t i = 0;
    for (const auto& itr : m_textureBindIndicesMap)
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

void tokenizer(const std::string& sourceString, std::vector<std::string>& tokenList)
{
    std::string newToken;
    for (const auto& itr : sourceString)
    {
        if ((itr == '\r') || (itr == '\n') || (itr == '\t') || (itr == ' '))
        {
            if (newToken.length())
            {
                tokenList.push_back(newToken);
                newToken.clear();
            }
            continue;
        }

        newToken.push_back(itr);
    }
}

ShaderConstantManager::SupportedTypes GLTypeToSupportedType(GLint gltype)
{
    switch (gltype)
    {
    case GL_FLOAT:
        return ShaderConstantManager::FLOAT;
    case GL_BOOL:
        return ShaderConstantManager::BOOL;
    case GL_INT:
        return ShaderConstantManager::INT;
    case GL_FLOAT_MAT4:
        return ShaderConstantManager::MAT4;
    case GL_FLOAT_VEC4:
        return ShaderConstantManager::VEC4;
    case GL_FLOAT_VEC3:
        return ShaderConstantManager::VEC3;
    default:
        assert(false);  // GL type unsupported.
    }
}