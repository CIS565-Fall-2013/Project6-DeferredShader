#pragma once

#include <cassert>
#include "GLTypes.h"

namespace RenderEnums
{
    enum SceneBuffers
    {
        GBUFFER_FRAMEBUFFER,
        LIGHTING_FRAMEBUFFER,
        SHADOW_FRAMEBUFFER
    };

    enum DrawListType
    {
        OPAQUE_LIST,
        ALPHA_MASKED_LIST,
        TRANSPARENT_LIST,
        LIGHT_LIST
    };

    enum ClearType
    {
        CLEAR_COLOUR = 1 << 0,
        CLEAR_DEPTH = 1 << 1,
        CLEAR_STENCIL = 1 << 2,
        CLEAR_ALL = CLEAR_COLOUR | CLEAR_DEPTH | CLEAR_STENCIL
    };

    enum ProgramType
    {
        RENDER_PROGRAM,
        COMPUTE_PROGRAM
    };

    enum RenderProgramStage
    {
        VERT,
        TESS_CTRL,
        TESS_EVAL,
        GEOM, 
        FRAG
    };
}

