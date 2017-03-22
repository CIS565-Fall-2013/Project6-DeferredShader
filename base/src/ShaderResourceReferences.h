#pragma once

#include "Common.h"

class ShaderConstantReference
{
    uint32_t m_shaderConstantName;

public:
    ShaderConstantReference(uint32_t shaderConstantName = 0) { m_shaderConstantName = shaderConstantName; }
    operator const uint32_t&() const
    {
        return m_shaderConstantName;
    }
};

class TextureReference
{
    uint32_t m_textureName;

public:
    TextureReference(uint32_t textureName = 0) { m_textureName = textureName; }
    operator const uint32_t&() const
    {
        return m_textureName;
    }
};

class ConstantBufferIndex
{
    uint32_t m_constantBufferIndex;

public:
    ConstantBufferIndex(uint32_t constantBufferIndex = 0) { m_constantBufferIndex = constantBufferIndex; }
    operator const uint32_t&() const
    {
        return m_constantBufferIndex;
    }
};

namespace ShaderResourceReferences
{
    struct PerFrameShaderConstantReferences
    {
        ShaderConstantReference ufFar;
        ShaderConstantReference ufNear;
        ShaderConstantReference uiScreenHeight;
        ShaderConstantReference uiScreenWidth;
        ShaderConstantReference ufInvScrHeight;
        ShaderConstantReference ufInvScrWidth;
        ShaderConstantReference um4View;
        ShaderConstantReference um4Persp;
        ShaderConstantReference ufGlowmask;
        ShaderConstantReference ubBloomOn;
        ShaderConstantReference ubToonOn;
        ShaderConstantReference ubDOFOn;
        ShaderConstantReference ubDOFDebug;
        ShaderConstantReference uiDisplayType;
    };
    extern PerFrameShaderConstantReferences perFrameShaderConstants;

    struct GeometryPassShaderConstantReferences
    {
        ShaderConstantReference um4Model;
        ShaderConstantReference um4InvTrans;
        ShaderConstantReference uf3Color;
    };
    extern GeometryPassShaderConstantReferences geometryPassShaderConstants;

    struct LightPassShaderConstantReferences
    {
        ShaderConstantReference uf4Light;
        ShaderConstantReference uf3LightCol;
        ShaderConstantReference uf4DirecLightDir;
        ShaderConstantReference uf3AmbientContrib;
        ShaderConstantReference ufLightIl;
    };
    extern LightPassShaderConstantReferences lightPassShaderConstants;

    struct GeometryPassTextureReferences
    {
        TextureReference t2DDiffuse;
        TextureReference t2DNormal;
        TextureReference t2DSpecular;
    };
    extern GeometryPassTextureReferences geometryPassTextures;

    struct FullScreenPassTextureReferences
    {
        TextureReference u_Depthtex;
        TextureReference u_Colortex;
        TextureReference u_Normaltex;
        TextureReference u_Positiontex;
        TextureReference u_RandomNormaltex;
        TextureReference u_RandomScalartex;
        TextureReference u_Posttex;
    };
    extern FullScreenPassTextureReferences fullScreenPassTextures;

    void Initialize();
}