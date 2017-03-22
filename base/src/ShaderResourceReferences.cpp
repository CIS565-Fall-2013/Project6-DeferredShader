#include "ShaderResourceReferences.h"

#include "Utility.h"
#include <cstring>

namespace ShaderResourceReferences
{
    PerFrameShaderConstantReferences perFrameShaderConstants;
    GeometryPassShaderConstantReferences geometryPassShaderConstants;
    LightPassShaderConstantReferences lightPassShaderConstants;

    GeometryPassTextureReferences geometryPassTextures;
    FullScreenPassTextureReferences fullScreenPassTextures;

    void Initialize()
    {
        perFrameShaderConstants.ufFar = Utility::HashCString("ufFar");
        perFrameShaderConstants.ufNear = Utility::HashCString("ufNear");
        perFrameShaderConstants.uiScreenHeight = Utility::HashCString("uiScreenHeight");
        perFrameShaderConstants.uiScreenWidth = Utility::HashCString("uiScreenWidth");
        perFrameShaderConstants.ufInvScrHeight = Utility::HashCString("ufInvScrHeight");
        perFrameShaderConstants.ufInvScrWidth = Utility::HashCString("ufInvScrWidth");
        perFrameShaderConstants.um4View = Utility::HashCString("um4View");
        perFrameShaderConstants.um4Persp = Utility::HashCString("um4Persp");
        perFrameShaderConstants.ufGlowmask = Utility::HashCString("ufGlowmask");
        perFrameShaderConstants.ubBloomOn = Utility::HashCString("ubBloomOn");
        perFrameShaderConstants.ubToonOn = Utility::HashCString("ubToonOn");
        perFrameShaderConstants.ubDOFOn = Utility::HashCString("ubDOFOn");
        perFrameShaderConstants.ubDOFDebug = Utility::HashCString("ubDOFDebug");
        perFrameShaderConstants.uiDisplayType = Utility::HashCString("uiDisplayType");
        
        geometryPassShaderConstants.um4Model = Utility::HashCString("um4Model");
        geometryPassShaderConstants.um4InvTrans = Utility::HashCString("um4InvTrans");
        geometryPassShaderConstants.uf3Color = Utility::HashCString("uf3Color");

        lightPassShaderConstants.uf4Light = Utility::HashCString("uf4Light");
        lightPassShaderConstants.uf3LightCol = Utility::HashCString("uf3LightCol");
        lightPassShaderConstants.uf4DirecLightDir = Utility::HashCString("uf4DirecLightDir");
        lightPassShaderConstants.uf3AmbientContrib = Utility::HashCString("uf3AmbientContrib");
        lightPassShaderConstants.ufLightIl = Utility::HashCString("ufLightIl");

        geometryPassTextures.t2DDiffuse = Utility::HashCString("t2DDiffuse");
        geometryPassTextures.t2DNormal = Utility::HashCString("t2DNormal");
        geometryPassTextures.t2DSpecular = Utility::HashCString("t2DSpecular");

        fullScreenPassTextures.u_Depthtex = Utility::HashCString("u_Depthtex");
        fullScreenPassTextures.u_Colortex = Utility::HashCString("u_Colortex");
        fullScreenPassTextures.u_Normaltex = Utility::HashCString("u_Normaltex");
        fullScreenPassTextures.u_Positiontex = Utility::HashCString("u_Positiontex");
        fullScreenPassTextures.u_RandomNormaltex = Utility::HashCString("u_RandomNormaltex");
        fullScreenPassTextures.u_RandomScalartex = Utility::HashCString("u_RandomScalartex");
        fullScreenPassTextures.u_Posttex = Utility::HashCString("u_Posttex");
    }
}