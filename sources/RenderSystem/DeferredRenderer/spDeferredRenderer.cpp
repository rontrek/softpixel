/*
 * Deferred renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spDeferredRenderer.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderClass.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>


//#define _DEB_PERFORMANCE_ //!!!
#ifdef _DEB_PERFORMANCE_
#   include "Base/spTimer.hpp"
#endif


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace video
{


#define ISFLAG(n) ((Flags_ & DEFERREDFLAG_##n) != 0)

static s32 DefRendererFlags = 0;

static void GBufferObjectShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
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

static void GBufferSurfaceShaderCallback(ShaderClass* ShdClass, const std::vector<TextureLayer*> &TextureLayers)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    u32 TexCount = TextureLayers.size();
    
    if (DefRendererFlags & DEFERREDFLAG_USE_TEXTURE_MATRIX)
    {
        /*if (TexCount > 0)
            VertShd->setConstant("TextureMatrix", TextureLayers.front().Matrix);
        else*/
            VertShd->setConstant("TextureMatrix", dim::matrix4f());
    }
    
    if ((DefRendererFlags & DEFERREDFLAG_HAS_SPECULAR_MAP) == 0)
        ++TexCount;
    
    if (DefRendererFlags & DEFERREDFLAG_HAS_LIGHT_MAP)
        FragShd->setConstant("EnableLightMap", TexCount >= ((DefRendererFlags & DEFERREDFLAG_PARALLAX_MAPPING) != 0 ? 5u : 4u));
    
    if (DefRendererFlags & DEFERREDFLAG_PARALLAX_MAPPING)
    {
        FragShd->setConstant("EnablePOM", TexCount >= 4);//!!!
        FragShd->setConstant("MinSamplesPOM", 0);//!!!
        FragShd->setConstant("MaxSamplesPOM", 50);//!!!
        FragShd->setConstant("HeightMapScale", 0.015f);//!!!
        FragShd->setConstant("ParallaxViewRange", 2.0f);//!!!
    }
    
    FragShd->setConstant("SpecularFactor", 1.0f);//!!!
}

static void DeferredShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    Shader* VertShd = ShdClass->getVertexShader();
    Shader* FragShd = ShdClass->getPixelShader();
    
    const dim::matrix4f ViewTransform(
        __spSceneManager->getActiveCamera()->getTransformMatrix(true)
    );
    
    #if 0
    
    const dim::matrix4f Proj(__spSceneManager->getActiveCamera()->getProjectionMatrix());
    const dim::matrix4f View(ViewTransform.getInverse());
    dim::matrix4f InvViewProj;
    
    InvViewProj = Proj.getInverse();
    
    FragShd->setConstant("InvViewProjection", InvViewProj);
    
    #endif
    
    VertShd->setConstant("ProjectionMatrix", __spVideoDriver->getProjectionMatrix());
    
    FragShd->setConstant("ViewTransform", ViewTransform);
    FragShd->setConstant("ViewPosition", ViewTransform.getPosition());
    FragShd->setConstant("ScreenWidth", static_cast<f32>(gSharedObjects.ScreenWidth));
    FragShd->setConstant("ScreenHeight", static_cast<f32>(gSharedObjects.ScreenHeight));
}

static void ShadowShaderCallback(ShaderClass* ShdClass, const scene::MaterialNode* Object)
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


DeferredRenderer::DeferredRenderer() :
    GBufferShader_  (0      ),
    DeferredShader_ (0      ),
    ShadowShader_   (0      ),
    Flags_          (0      ),
    AmbientColor_   (0.07f  )
{
    if (!gSharedObjects.CgContext)
        __spDevice->createCgShaderContext();
}
DeferredRenderer::~DeferredRenderer()
{
    deleteShaders();
    GBuffer_.deleteGBuffer();
}

bool DeferredRenderer::generateResources(
    s32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, s32 MultiSampling)
{
    /* Setup resource flags */
    Flags_ = Flags;
    LayerModel_.clear();
    
    const bool IsGL = (__spVideoDriver->getRendererType() == RENDERER_OPENGL);
    const dim::size2di Resolution(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight);
    
    const bool CompileGLSL = IsGL && true;//!!!
    
    /* Initialize light objects */
    MaxPointLightCount = math::Max(MaxPointLightCount, MaxSpotLightCount);
    
    Lights_.resize(MaxPointLightCount);
    LightsEx_.resize(MaxSpotLightCount);
    
    /* Setup shader compilation options */
    std::list<io::stringc> GBufferCompilerOp, DeferredCompilerOp;
    setupCompilerOptions(GBufferCompilerOp, DeferredCompilerOp);
    
    Shader::addOption(DeferredCompilerOp, "MAX_LIGHTS " + io::stringc(MaxPointLightCount));
    Shader::addOption(DeferredCompilerOp, "MAX_EX_LIGHTS " + io::stringc(MaxSpotLightCount));
    
    /* Delete old shaders and shadow maps */
    deleteShaders();
    ShadowMapper_.deleteShadowMaps();
    
    /* Create new vertex formats */
    createVertexFormats();
    
    /* Setup g-buffer shader source code */
    std::list<io::stringc> GBufferShdBufVert(GBufferCompilerOp), GBufferShdBufFrag(GBufferCompilerOp);
    
    if (CompileGLSL)
    {
        Shader::addShaderCore(GBufferShdBufVert);
        Shader::addShaderCore(GBufferShdBufFrag);
        
        GBufferShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderStr.glvert"
        );
        
        GBufferShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderHeaderStr.glfrag"
        );
        GBufferShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderMainStr.shader"
        );
        GBufferShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderBodyStr.glfrag"
        );
    }
    else
    {
        Shader::addShaderCore(GBufferShdBufVert, true);
        
        GBufferShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spGBufferShaderStr.cg"
        );
    }
    
    /* Generate g-buffer shader */
    if (!buildShader(
            "g-buffer", GBufferShader_, &VertexFormat_, &GBufferShdBufVert,
            CompileGLSL ? &GBufferShdBufFrag : &GBufferShdBufVert,
            "VertexMain", "PixelMain", CompileGLSL ? SHADERBUILD_GLSL : SHADERBUILD_CG))
    {
        return false;
    }
    
    GBufferShader_->setObjectCallback(GBufferObjectShaderCallback);
    GBufferShader_->setSurfaceCallback(GBufferSurfaceShaderCallback);
    
    if (CompileGLSL)
        setupGBufferSampler(GBufferShader_->getPixelShader());
    
    /* Setup deferred shader source code */
    std::list<io::stringc> DeferredShdBufVert(DeferredCompilerOp), DeferredShdBufFrag(DeferredCompilerOp);
    
    if (CompileGLSL)
    {
        Shader::addShaderCore(DeferredShdBufVert);
        Shader::addShaderCore(DeferredShdBufFrag);
        
        DeferredShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderStr.glvert"
        );
        
        DeferredShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderHeaderStr.glfrag"
        );
        DeferredShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderProcsStr.shader"
        );
        DeferredShdBufFrag.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderBodyStr.glfrag"
        );
    }
    else
    {
        Shader::addShaderCore(DeferredShdBufVert, true);
        
        DeferredShdBufVert.push_back(
            #include "RenderSystem/DeferredRenderer/spDeferredShaderStr.cg"
        );
    }
    
    /* Generate deferred shader */
    if (!buildShader(
            "deferred", DeferredShader_, &VertexFormat_, &DeferredShdBufVert,
            CompileGLSL ? &DeferredShdBufFrag : &DeferredShdBufVert,
            "VertexMain", "PixelMain", CompileGLSL ? SHADERBUILD_GLSL : SHADERBUILD_CG))
    {
        return false;
    }
    
    DeferredShader_->setObjectCallback(DeferredShaderCallback);
    
    if (CompileGLSL)
        setupDeferredSampler(DeferredShader_->getPixelShader());
    
    setupLightShaderConstants();
    
    /* Generate bloom filter shader */
    if (ISFLAG(BLOOM))
    {
        if (!BloomEffect_.createResources(Resolution))
            Flags_ ^= DEFERREDFLAG_BLOOM;
    }
    
    /* Generate shadow shader */
    if (ISFLAG(SHADOW_MAPPING))
    {
        /* Create the shadow maps */
        ShadowMapper_.createShadowMaps(
            ShadowTexSize, MaxPointLightCount, MaxSpotLightCount, true, ISFLAG(GLOBAL_ILLUMINATION)
        );
        
        /* Setup shader compilation options */
        std::list<io::stringc> ShadowShdBuf;
        
        Shader::addOption(ShadowShdBuf, "USE_VSM");
        Shader::addOption(ShadowShdBuf, "USE_TEXTURE");
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
            Shader::addOption(ShadowShdBuf, "USE_RSM");
        
        //if (ISFLAG(USE_TEXTURE_MATRIX))
        //    Shader::addOption(ShadowShdBuf, "USE_TEXTURE_MATRIX");
        
        Shader::addShaderCore(ShadowShdBuf, true);
        
        /* Build shadow shader */
        ShadowShdBuf.push_back(
            #include "RenderSystem/DeferredRenderer/spShadowShaderStr.cg"
        );
        
        if (!buildShader("shadow", ShadowShader_, &VertexFormat_, &ShadowShdBuf, &ShadowShdBuf, "VertexMain", "PixelMain"))
            return false;
        
        ShadowShader_->setObjectCallback(ShadowShaderCallback);
    }
    
    /* Build g-buffer */
    return GBuffer_.createGBuffer(Resolution, MultiSampling, ISFLAG(HAS_LIGHT_MAP));
}

void DeferredRenderer::renderScene(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, Texture* RenderTarget, bool UseDefaultGBufferShader)
{
    DefRendererFlags = Flags_;
    
    if ( Graph && GBufferShader_ && DeferredShader_ && ( !RenderTarget || RenderTarget->getRenderTarget() ) )
    {
        updateLightSources(Graph, ActiveCamera);
        
        renderSceneIntoGBuffer(Graph, ActiveCamera, UseDefaultGBufferShader);
        renderDeferredShading(RenderTarget);
        
        if (ISFLAG(BLOOM))
            BloomEffect_.drawEffect(RenderTarget);
    }
    #ifdef SP_DEBUGMODE
    else if ( !Graph || ( RenderTarget && !RenderTarget->getRenderTarget() ) )
        io::Log::debug("DeferredRenderer::renderScene");
    #endif
}


/*
 * ======= Protected: =======
 */

void DeferredRenderer::updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera)
{
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_START(debTimer0)
    #endif
    
    /* Update each light source */
    f32 Color[4];
    s32 i = 0, iEx = 0;
    u32 ShadowCubeMapIndex = 0, ShadowMapIndex = 0;
    
    const s32 LightCount = static_cast<s32>(Lights_.size());
    
    std::vector<scene::Light*>::const_iterator it = Graph->getLightList().begin(), itEnd = Graph->getLightList().end();
    
    const bool UseShadow = ISFLAG(SHADOW_MAPPING);
    
    if (UseShadow)
        __spVideoDriver->setGlobalShaderClass(ShadowShader_);
    
    for (; it != itEnd && i < LightCount; ++it)
    {
        /* Get current light source object */
        scene::Light* LightObj = *it;
        
        if ( !LightObj->getVisible() || ( LightObj->getLightModel() != scene::LIGHT_POINT && static_cast<u32>(iEx) >= LightsEx_.size() ) )
            continue;
        
        SLight* Lit = &(Lights_[i]);
        
        LightObj->getDiffuseColor().getFloatArray(Color);
        
        if (UseShadow && LightObj->getShadow())
        {
            /* Render shadow map */
            switch (LightObj->getLightModel())
            {
                case scene::LIGHT_POINT:
                    Lit->ShadowIndex = ShadowCubeMapIndex;
                    ShadowMapper_.renderShadowMap(Graph, ActiveCamera, LightObj, ShadowCubeMapIndex++);
                    break;
                case scene::LIGHT_SPOT:
                    Lit->ShadowIndex = ShadowMapIndex;
                    ShadowMapper_.renderShadowMap(Graph, ActiveCamera, LightObj, ShadowMapIndex++);
                    break;
                default:
                    break;
            }
        }
        else
            Lit->ShadowIndex = -1;
        
        /* Copy basic data */
        Lit->Position           = LightObj->getPosition(true);
        Lit->Radius             = (LightObj->getVolumetric() ? LightObj->getVolumetricRadius() : 1000.0f);
        Lit->Color              = dim::vector3df(Color[0], Color[1], Color[2]);
        Lit->Type               = static_cast<u8>(LightObj->getLightModel());
        Lit->UsedForLightmaps   = (LightObj->getShadow() ? 0 : 1);//!!!
        
        if (Lit->Type != scene::LIGHT_POINT)
        {
            SLightEx* LitEx = &(LightsEx_[iEx]);
            
            /* Copy extended data */
            const scene::Transformation Transform(LightObj->getTransformation(true));
            
            if (Lit->Type == scene::LIGHT_SPOT)
            {
                //LitEx->Projection = LightObj->getProjectionMatrix() * LightObj->getTransformMatrix(true).getInverse();
                dim::matrix4f ProjMat;
                ProjMat.setPerspectiveRH(90.0f, 1.0f, 0.01f, 1000.0f);
                
                LitEx->Projection = ProjMat * Transform.getInverseMatrix();
                
                if (ISFLAG(GLOBAL_ILLUMINATION))
                    LitEx->ViewTransform = Transform.getMatrix();
            }
            
            LitEx->Direction = Transform.getDirection();
            LitEx->Direction.normalize();
            
            LitEx->SpotTheta            = LightObj->getSpotConeInner() * math::DEG;
            LitEx->SpotPhiMinusTheta    = LightObj->getSpotConeOuter() * math::DEG - LitEx->SpotTheta;
            
            ++iEx;
        }
        
        ++i;
    }
    
    if (UseShadow)
        __spVideoDriver->setGlobalShaderClass(0);
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("Light Setup Time: ", debTimer0)
    PERFORMANCE_QUERY_START(debTimer1)
    #endif
    
    /* Update shader constants */
    Shader* FragShd = DeferredShader_->getPixelShader();
    
    FragShd->setConstant(LightDesc_.LightCountConstant, i);
    FragShd->setConstant(LightDesc_.LightExCountConstant, iEx);
    
    for (s32 c = 0; c < i; ++c)
    {
        const SLight& Lit = Lights_[c];
        
        FragShd->setConstant(Lit.Constants[0], dim::vector4df(Lit.Position, Lit.Radius) );
        FragShd->setConstant(Lit.Constants[1], Lit.Color                                );
        FragShd->setConstant(Lit.Constants[2], Lit.Type                                 );
        FragShd->setConstant(Lit.Constants[3], Lit.ShadowIndex                          );
        FragShd->setConstant(Lit.Constants[4], Lit.UsedForLightmaps                     );
    }
    
    for (s32 c = 0; c < iEx; ++c)
    {
        const SLightEx& Lit = LightsEx_[c];
        
        FragShd->setConstant(Lit.Constants[0], Lit.Projection       );
        FragShd->setConstant(Lit.Constants[1], Lit.Direction        );
        FragShd->setConstant(Lit.Constants[2], Lit.SpotTheta        );
        FragShd->setConstant(Lit.Constants[3], Lit.SpotPhiMinusTheta);
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
            FragShd->setConstant(Lit.Constants[4], Lit.ViewTransform);
    }
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("Light Shader Upload Time: ", debTimer1)
    #endif
}

void DeferredRenderer::renderSceneIntoGBuffer(
    scene::SceneGraph* Graph, scene::Camera* ActiveCamera, bool UseDefaultGBufferShader)
{
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_START(debTimer2)
    #endif
    
    ShaderClass* PrevShaderClass = 0;
    
    if (UseDefaultGBufferShader)
    {
        PrevShaderClass = __spVideoDriver->getGlobalShaderClass();
        __spVideoDriver->setGlobalShaderClass(GBufferShader_);
    }
    
    GBuffer_.bindRenderTargets();
    __spVideoDriver->clearBuffers();
    
    __spDevice->setActiveSceneGraph(Graph);
    
    if (ActiveCamera)
        Graph->renderScene(ActiveCamera);
    else
        Graph->renderScene();
    
    if (UseDefaultGBufferShader)
        __spVideoDriver->setGlobalShaderClass(PrevShaderClass);
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("GBuffer Render Time: ", debTimer2)
    #endif
}

void DeferredRenderer::renderDeferredShading(Texture* RenderTarget)
{
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_START(debTimer3)
    #endif
    
    if (ISFLAG(BLOOM))
        BloomEffect_.bindRenderTargets();
    else
        __spVideoDriver->setRenderTarget(RenderTarget);
    
    const s32 ShadowMapLayerBase = (ISFLAG(HAS_LIGHT_MAP) ? 3 : 2);
    
    __spVideoDriver->setRenderMode(RENDERMODE_DRAWING_2D);
    DeferredShader_->bind();
    {
        DeferredShader_->getPixelShader()->setConstant("AmbientColor", AmbientColor_);
        
        /* Bind shadow map texture-array and draw deferred-shading */
        ShadowMapper_.bind(ShadowMapLayerBase);
        
        GBuffer_.drawDeferredShading();
        
        ShadowMapper_.unbind(ShadowMapLayerBase);
    }
    DeferredShader_->unbind();
    
    __spVideoDriver->setRenderTarget(0);
    
    #ifdef _DEB_PERFORMANCE_
    PERFORMANCE_QUERY_PRINT("Deferred Shading Time: ", debTimer3)
    #endif
}

bool DeferredRenderer::buildShader(
    const io::stringc &Name,
    ShaderClass* &ShdClass,
    VertexFormat* VertFmt,
    
    const std::list<io::stringc>* ShdBufferVertex,
    const std::list<io::stringc>* ShdBufferPixel,
    
    const io::stringc &VertexMain,
    const io::stringc &PixelMain,
    
    s32 Flags)
{
    if (!ShaderClass::build(Name, ShdClass, VertFmt, ShdBufferVertex, ShdBufferPixel, VertexMain, PixelMain, Flags))
    {
        deleteShaders();
        return false;
    }
    return true;
}

void DeferredRenderer::deleteShaders()
{
    __spVideoDriver->deleteShaderClass(GBufferShader_,  true);
    __spVideoDriver->deleteShaderClass(DeferredShader_, true);
    __spVideoDriver->deleteShaderClass(ShadowShader_,   true);
    
    GBufferShader_  = 0;
    DeferredShader_ = 0;
    ShadowShader_   = 0;
}

void DeferredRenderer::createVertexFormats()
{
    /* Create object vertex format */
    VertexFormat_.clear();
    
    VertexFormat_.addCoord();
    VertexFormat_.addNormal();
    VertexFormat_.addTexCoord();
    
    if (ISFLAG(NORMAL_MAPPING))
    {
        /* Add texture-coordinates for normal-mapping (tangent and binormal is texture-coordinates) */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 3);
    }
    
    if (ISFLAG(HAS_LIGHT_MAP))
    {
        /* Add texture-coordinates for lightmaps */
        VertexFormat_.addTexCoord(DATATYPE_FLOAT, 2);
    }
    
    /* Create 2D image vertex format */
    ImageVertexFormat_.clear();
    
    ImageVertexFormat_.addCoord(DATATYPE_FLOAT, 2);
    ImageVertexFormat_.addTexCoord();
}

void DeferredRenderer::setupCompilerOptions(
    std::list<io::stringc> &GBufferCompilerOp, std::list<io::stringc> &DeferredCompilerOp)
{
    const bool IsGL = (__spVideoDriver->getRendererType() == RENDERER_OPENGL);
    
    if (ISFLAG(USE_TEXTURE_MATRIX))
        Shader::addOption(GBufferCompilerOp, "USE_TEXTURE_MATRIX");
    if (ISFLAG(HAS_SPECULAR_MAP))
        Shader::addOption(GBufferCompilerOp, "HAS_SPECULAR_MAP");
    
    if (ISFLAG(HAS_LIGHT_MAP))
    {
        Shader::addOption(GBufferCompilerOp, "HAS_LIGHT_MAP");
        Shader::addOption(DeferredCompilerOp, "HAS_LIGHT_MAP");
    }
    
    if (ISFLAG(ALLOW_OVERBLENDING))
        Shader::addOption(DeferredCompilerOp, "ALLOW_OVERBLENDING");
    
    if (ISFLAG(NORMAL_MAPPING))
    {
        Shader::addOption(GBufferCompilerOp, "NORMAL_MAPPING");
        
        if (ISFLAG(PARALLAX_MAPPING))
        {
            Shader::addOption(GBufferCompilerOp, "PARALLAX_MAPPING");
            if (ISFLAG(NORMALMAP_XYZ_H))
                Shader::addOption(GBufferCompilerOp, "NORMALMAP_XYZ_H");
        }
    }
    
    if (ISFLAG(DEBUG_GBUFFER))
    {
        Shader::addOption(GBufferCompilerOp, "DEBUG_GBUFFER");
        Shader::addOption(DeferredCompilerOp, "DEBUG_GBUFFER");
        
        if (ISFLAG(DEBUG_GBUFFER_TEXCOORDS))
            Shader::addOption(GBufferCompilerOp, "DEBUG_GBUFFER_TEXCOORDS");
        if (ISFLAG(DEBUG_GBUFFER_WORLDPOS))
            Shader::addOption(DeferredCompilerOp, "DEBUG_GBUFFER_WORLDPOS");
    }
    
    if (ISFLAG(BLOOM))
    {
        Shader::addOption(DeferredCompilerOp, "BLOOM_FILTER");
        
        if (IsGL)
            Shader::addOption(DeferredCompilerOp, "FLIP_Y_AXIS");
    }
    
    if (ISFLAG(SHADOW_MAPPING))
    {
        Shader::addOption(GBufferCompilerOp, "SHADOW_MAPPING");
        Shader::addOption(DeferredCompilerOp, "SHADOW_MAPPING");
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
            Shader::addOption(DeferredCompilerOp, "GLOBAL_ILLUMINATION");
    }
}

void DeferredRenderer::setupGBufferSampler(Shader* PixelShader)
{
    if (!PixelShader)
        return;
    
    s32 SamplerIndex = 0;
    
    LayerModel_.DiffuseMap = static_cast<u8>(SamplerIndex);
    PixelShader->setConstant("DiffuseMap", SamplerIndex++);
    
    if (ISFLAG(HAS_SPECULAR_MAP))
    {
        LayerModel_.SpecularMap = static_cast<u8>(SamplerIndex);
        PixelShader->setConstant("SpecularMap", SamplerIndex++);
    }
    
    if (ISFLAG(NORMAL_MAPPING))
    {
        LayerModel_.NormalMap = static_cast<u8>(SamplerIndex);
        PixelShader->setConstant("NormalMap", SamplerIndex++);
        
        if (ISFLAG(PARALLAX_MAPPING))
        {
            if (!ISFLAG(NORMALMAP_XYZ_H))
            {
                LayerModel_.HeightMap = static_cast<u8>(SamplerIndex);
                PixelShader->setConstant("HeightMap", SamplerIndex++);
            }
            else
                LayerModel_.HeightMap = LayerModel_.NormalMap;
        }
    }
    
    if (ISFLAG(HAS_LIGHT_MAP))
    {
        LayerModel_.LightMap = static_cast<u8>(SamplerIndex);
        PixelShader->setConstant("LightMap", SamplerIndex++);
    }
}

void DeferredRenderer::setupDeferredSampler(Shader* PixelShader)
{
    if (!PixelShader)
        return;
    
    s32 SamplerIndex = 0;
    
    PixelShader->setConstant("DiffuseAndSpecularMap", SamplerIndex++);
    PixelShader->setConstant("NormalAndDepthMap", SamplerIndex++);
    
    if (ISFLAG(HAS_LIGHT_MAP))
        PixelShader->setConstant("IlluminationMap", SamplerIndex++);
    
    if (ISFLAG(SHADOW_MAPPING))
    {
        PixelShader->setConstant("DirLightShadowMaps", SamplerIndex++);
        PixelShader->setConstant("PointLightShadowMaps", SamplerIndex++);
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
        {
            PixelShader->setConstant("DirLightDiffuseMaps", SamplerIndex++);
            PixelShader->setConstant("PointLightDiffuseMaps", SamplerIndex++);
            PixelShader->setConstant("DirLightNormalMaps", SamplerIndex++);
            PixelShader->setConstant("PointLightNormalMaps", SamplerIndex++);
        }
    }
}

void DeferredRenderer::setupLightShaderConstants()
{
    Shader* FragShd = DeferredShader_->getPixelShader();
    
    LightDesc_.LightCountConstant   = FragShd->getConstant("LightCount");
    LightDesc_.LightExCountConstant = FragShd->getConstant("LightExCount");
    
    for (u32 i = 0, c = Lights_.size(); i < c; ++i)
    {
        SLight& Lit = Lights_[i];
        
        const io::stringc n = "Lights[" + io::stringc(i) + "].";
        
        Lit.Constants[0] = FragShd->getConstant(n + "PositionAndRadius" );
        Lit.Constants[1] = FragShd->getConstant(n + "Color"             );
        Lit.Constants[2] = FragShd->getConstant(n + "Type"              );
        Lit.Constants[3] = FragShd->getConstant(n + "ShadowIndex"       );
        Lit.Constants[4] = FragShd->getConstant(n + "UsedForLightmaps"  );
    }
    
    for (u32 i = 0, c = LightsEx_.size(); i < c; ++i)
    {
        SLightEx& Lit = LightsEx_[i];
        
        const io::stringc n = "LightsEx[" + io::stringc(i) + "].";
        
        Lit.Constants[0] = FragShd->getConstant(n + "Projection"        );
        Lit.Constants[1] = FragShd->getConstant(n + "Direction"         );
        Lit.Constants[2] = FragShd->getConstant(n + "SpotTheta"         );
        Lit.Constants[3] = FragShd->getConstant(n + "SpotPhiMinusTheta" );
        
        if (ISFLAG(GLOBAL_ILLUMINATION))
            Lit.Constants[4] = FragShd->getConstant(n + "ViewTransform");
    }
}


/*
 * SLight structure
 */

DeferredRenderer::SLight::SLight() :
    Radius          (1000.0f),
    Color           (1.0f   ),
    Type            (0      ),
    ShadowIndex     (-1     ),
    UsedForLightmaps(0      )
{
}
DeferredRenderer::SLight::~SLight()
{
}


/*
 * SLightEx structure
 */

DeferredRenderer::SLightEx::SLightEx() :
    Direction           (0.0f, 0.0f, 1.0f   ),
    SpotTheta           (0.0f               ),
    SpotPhiMinusTheta   (0.0f               )
{
}
DeferredRenderer::SLightEx::~SLightEx()
{
}

#undef ISFLAG


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
