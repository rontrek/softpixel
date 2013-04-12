/*
 * Deferred renderer shader classbacks file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spDeferredRendererShaderCallbacks.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/DeferredRenderer/spDeferredRendererFlags.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


s32 gDRFlags = 0;


/*
 * Constant buffer structures
 */

#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SP_PACK_STRUCT
#elif defined(__GNUC__)
#   define SP_PACK_STRUCT __attribute__((packed))
#else
#   define SP_PACK_STRUCT
#endif

struct SGBufferMainCB
{
    dim::matrix4f WVPMatrix;
    dim::matrix4f WorldMatrix;
    dim::vector3df ViewPosition;
    f32 Pad0;
}
SP_PACK_STRUCT;

struct SGBufferReliefCB
{
    f32 SpecularFactor;
    f32 HeightMapScale;
    f32 ParallaxViewRange;
    f32 Pad0;
    s32 EnablePOM;
    s32 MinSamplesPOM;
    s32 MaxSamplesPOM;
    s32 Pad1;
}
SP_PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT


/*
 * Shader callbacks
 */

void DfRnGBufferObjectShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup transformations */
    const dim::vector3df ViewPosition(
        __spSceneManager->getActiveCamera()->getPosition(true)
    );
    
    dim::matrix4f WVPMatrix(__spVideoDriver->getProjectionMatrix());
    WVPMatrix *= __spVideoDriver->getViewMatrix();
    WVPMatrix *= __spVideoDriver->getWorldMatrix();
    
    /* Setup shader constants */
    VertShd->setConstant("WorldViewProjectionMatrix", WVPMatrix);
    VertShd->setConstant("WorldMatrix", __spVideoDriver->getWorldMatrix());
    VertShd->setConstant("ViewPosition", ViewPosition);
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}

void DfRnGBufferObjectShaderCallbackCB(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup transformation */
    SGBufferMainCB BufferMain;
    {
        BufferMain.WVPMatrix = __spVideoDriver->getProjectionMatrix();
        BufferMain.WVPMatrix *= __spVideoDriver->getViewMatrix();
        BufferMain.WVPMatrix *= __spVideoDriver->getWorldMatrix();
        
        BufferMain.ViewPosition = __spSceneManager->getActiveCamera()->getPosition(true);
    }
    VertShd->setConstantBuffer(0, &BufferMain);
    FragShd->setConstantBuffer(0, &BufferMain);
}

void DfRnGBufferSurfaceShaderCallback(ShaderClass* ShdClass, const std::vector<TextureLayer*> &TextureLayers)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup texture layers */
    u32 TexCount = TextureLayers.size();
    
    if (gDRFlags & DEFERREDFLAG_USE_TEXTURE_MATRIX)
    {
        /*if (TexCount > 0)
            VertShd->setConstant("TextureMatrix", TextureLayers.front().Matrix);
        else*/
            VertShd->setConstant("TextureMatrix", dim::matrix4f::IDENTITY);
    }
    
    if ((gDRFlags & DEFERREDFLAG_HAS_SPECULAR_MAP) == 0)
        ++TexCount;
    
    if (gDRFlags & DEFERREDFLAG_HAS_LIGHT_MAP)
        FragShd->setConstant("EnableLightMap", TexCount >= ((gDRFlags & DEFERREDFLAG_PARALLAX_MAPPING) != 0 ? 5u : 4u));
    
    if (gDRFlags & DEFERREDFLAG_PARALLAX_MAPPING)
    {
        FragShd->setConstant("EnablePOM", TexCount >= 4);//!!!
        FragShd->setConstant("MinSamplesPOM", 0);//!!!
        FragShd->setConstant("MaxSamplesPOM", 50);//!!!
        FragShd->setConstant("HeightMapScale", 0.015f);//!!!
        FragShd->setConstant("ParallaxViewRange", 2.0f);//!!!
    }
    
    FragShd->setConstant("SpecularFactor", 1.0f);//!!!
}

void DfRnGBufferSurfaceShaderCallbackCB(ShaderClass* ShdClass, const std::vector<TextureLayer*> &TextureLayers)
{
    /* Get vertex- and pixel shaders */
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    /* Setup texture layers */
    SGBufferReliefCB BufferRelief;
    {
        BufferRelief.SpecularFactor = 1.0f;
        //...
    }
    FragShd->setConstantBuffer(1, &BufferRelief);
}

void DfRnDeferredShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    scene::Camera* Cam = __spSceneManager->getActiveCamera();
    
    dim::matrix4f ViewMatrix(Cam->getTransformMatrix(true));
    const dim::vector3df ViewPosition(ViewMatrix.getPosition());
    ViewMatrix.setPosition(0.0f);
    ViewMatrix.setInverse();
    
    dim::matrix4f InvViewProj(Cam->getProjection().getMatrixLH());
    InvViewProj *= ViewMatrix;
    InvViewProj.setInverse();
    
    VertShd->setConstant("ProjectionMatrix", __spVideoDriver->getProjectionMatrix());
    VertShd->setConstant("InvViewProjection", InvViewProj);
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}

void DfRnShadowShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    const dim::vector3df ViewPosition(
        __spSceneManager->getActiveCamera()->getPosition(true)
    );
    
    VertShd->setConstant(
        "WorldViewProjectionMatrix",
        __spVideoDriver->getProjectionMatrix() * __spVideoDriver->getViewMatrix() * __spVideoDriver->getWorldMatrix()
    );
    VertShd->setConstant(
        "WorldMatrix",
        __spVideoDriver->getWorldMatrix()
    );
    
    FragShd->setConstant("ViewPosition", ViewPosition);
}

void DfRnDebugVPLShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    Shader* VertShd = ShdClass->getVertexShader();
    
    VertShd->setConstant(
        "WorldViewProjectionMatrix",
        __spVideoDriver->getProjectionMatrix() * __spVideoDriver->getViewMatrix() * __spVideoDriver->getWorldMatrix()
    );
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================