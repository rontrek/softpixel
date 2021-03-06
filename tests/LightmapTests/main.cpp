//
// SoftPixel Engine - Lightmap Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR

#include "../common.hpp"

#include "Framework/Tools/LightmapGenerator/spKDTreeBufferMapper.hpp"
#include "Framework/Tools/LightmapGenerator/spLightmapShaderDispatcher.hpp"

SP_TESTS_DECLARE

static scene::Light* CreateLightSource(const dim::vector3df &Point, const video::color &Color = 255, f32 Radius = 1500.0f)
{
    // Create light source
    scene::Light* LightObj = spScene->createLight(scene::LIGHT_POINT);
    
    LightObj->setVolumetric(true);
    LightObj->setVolumetricRadius(Radius);
    LightObj->setLightingColor(Color);
    LightObj->setPosition(Point);
    
    // Create visual light model
    scene::Mesh* VisualLightModel = spScene->createMesh(scene::MESH_SPHERE);
    
    video::MaterialStates* Material = VisualLightModel->getMaterial();
    
    Material->setColorMaterial(false);
    Material->setDiffuseColor(0);
    Material->setAmbientColor(Color);
    
    VisualLightModel->setParent(LightObj);
    VisualLightModel->setScale(0.35f);
    
    return LightObj;
}

u64 ElapsedTime = 0;

/**

=== Timging results (TestScene.spm): ===

DEBUG MODE / With 2 light sources:
 -> Single threaded: ~2800 ms.
 -> Multi threaded (8 Threads): ~1900 ms.

RELEASE MODE / With 2 light sources:
 -> Single threaded: ~1080 ms.
 -> Multi threaded (8 Threads): 780 ms.


=== Timging results (TestSceneLarge.spm): ===

DEBUG MODE / With 4 light sources:
 -> Single threaded: ~35 s.
 -> Multi threaded (8 Threads): ~23 s.
 -> Hardware accelerated (With tree hierarchy): ~6 s.
 -> Hardware accelerated (Without tree hierarchy): ~2 s.


=== kd-Tree optimization results: ===

Without polygon clipping: 2543 ms. incl. 141 ms. tree generation
With    polygon clipping: 2293 ms. incl. 405 ms. tree generation

*/

static bool ProgressCallback(f32 Progress)
{
    return true;
}

static void StateCallback(const tool::ELightmapGenerationStates State, const io::stringc &Info)
{
    const io::stringc StateStr("State: " + tool::Debugging::toString(State));
    
    u64 CurrentTime = io::Timer::millisecs();
    const io::stringc TimeStr(" [ " + io::stringc(CurrentTime - ElapsedTime) + " elapsed ms. ]");
    ElapsedTime = CurrentTime;
    
    io::Log::message(
        (Info.empty() ? StateStr : StateStr + " ( " + Info + " )") + TimeStr
    );
}

static void ChangeTexFilter(bool IsLinear)
{
    spRenderer->setTextureGenFlags(
        video::TEXGEN_FILTER,
        IsLinear ? video::FILTER_LINEAR : video::FILTER_SMOOTH
    );
}

int main()
{
    SP_TESTS_INIT_EX2(
        video::RENDERER_DIRECT3D11, dim::size2di(800, 600), "Lightmap", false, SDeviceFlags()
    )
    
    spRenderer->setClearColor(video::color(255));
    
    #if 0
    {
        scene::CollisionGraph* CollGraph = spDevice->createCollisionGraph();
        scene::CollisionMesh* CollMesh = CollGraph->createMesh(0, spScene->createMesh(scene::MESH_SPIRAL));
        tool::LightmapGen::ShaderDispatcher ShdDispatcher;
        ShdDispatcher.createResources(CollMesh, false, 256);
    }
    #endif
    
    // Setup scene
    Lit->setLightModel(scene::LIGHT_POINT);
    
    const io::stringc ResPath("../AdvancedRendererTests/");
    
    scene::Mesh* World = spScene->loadMesh(ResPath + "TestSceneLarge.spm");
    
    //World->addTexture(spRenderer->loadTexture("../../help/tutorials/ShaderLibrary/media/StoneColorMap.jpg"));
    //World->textureAutoMap(0);
    
    math::Randomizer::seedRandom();
    
    const video::color AmbColors[] =
    {
        video::color(20),
        video::color(50),
        video::color(50, 0, 0),
        video::color(0, 50, 0),
        video::color(0, 0, 50),
        video::color(50, 50, 0)
    };
    
    // Lightmap generation
    //#define TEST_BARYCENTRIC_COORDS
    
    #ifndef TEST_BARYCENTRIC_COORDS
    
    std::vector<tool::SCastShadowObject> CastObjList;
    std::vector<tool::SGetShadowObject> GetObjList;
    std::vector<tool::SLightmapLight> LitSources;
    
    CastObjList.push_back(World);
    GetObjList.push_back(World);
    
    #   if 1
    LitSources.push_back(CreateLightSource(0.0f, video::color(0, 0, 255), 150.0f));
    LitSources.push_back(CreateLightSource(dim::vector3df(2, -0.5f, -1), video::color(255, 0, 0), 150.0f));
    LitSources.push_back(CreateLightSource(dim::vector3df(-2, -0.5f, -1), video::color(0, 255, 0), 150.0f));
    
    LitSources.push_back(CreateLightSource(dim::vector3df(0, 0, 15), video::color(255, 230, 50), 500.0f));
    #       if 0
    for (u32 i = 0; i < 10; ++i)
    {
        LitSources.push_back(CreateLightSource(
            dim::vector3df(
                math::Randomizer::randFloat(-4.0f, 4.0f),
                -0.5f,
                math::Randomizer::randFloat(4.0f, 50.0f)
            ),
            math::Randomizer::randColor(), 150.0f
        ));
    }
    #       endif
    #   else
    LitSources.push_back(CreateLightSource(0.0f));
    #   endif
    
    bool UseRawLights = true;
    ChangeTexFilter(UseRawLights);
    
    s32 BlurFactor = (UseRawLights ? 0 : tool::DEF_LIGHTMAP_BLURRADIUS);
    
    u64 t = io::Timer::millisecs();
    ElapsedTime = t;
    
    tool::LightmapGenerator* LightmapPlotter = new tool::LightmapGenerator();
    
    LightmapPlotter->setProgressCallback(ProgressCallback);
    LightmapPlotter->setStateCallback(StateCallback);
    
    LightmapPlotter->generateLightmaps(
        CastObjList,
        GetObjList,
        LitSources,
        tool::SLightmapGenConfig(
            tool::DEF_LIGHTMAP_AMBIENT,
            256,//tool::DEF_LIGHTMAP_SIZE,
            tool::DEF_LIGHTMAP_DENSITY,
            BlurFactor
        ),
        8,
        tool::LIGHTMAPFLAG_NOTRANSPARENCY
        | tool::LIGHTMAPFLAG_GPU_ACCELERATION
        //| tool::LIGHTMAPFLAG_GPU_TREE_HIERARCHY
    );
    
    io::Log::message("Duration: " + io::stringc(io::Timer::millisecs() - t) + " ms.");
    
    //World->setVisible(false);
    
    io::Log::message("Lightmaps: " + io::stringc(LightmapPlotter->getLightmapTextures().size()));
    io::Log::message("Old Surfaces: " + io::stringc(World->getMeshBufferCount()));
    io::Log::message("New Surfaces: " + io::stringc(LightmapPlotter->getFinalModel()->getMeshBufferCount()));
    
    #else
    
    video::Texture* Tex = spRenderer->loadTexture("TestImage.png");
    Tex->setFilter(video::FILTER_LINEAR);
    
    spRenderer->setClearColor(100);
    
    #endif
    
    // Command line
    tool::CommandLineUI* cmd = new tool::CommandLineUI();
    cmd->setBackgroundColor(video::color(0, 0, 0, 128));
    cmd->setRect(dim::rect2di(0, 0, spContext->getResolution().Width, spContext->getResolution().Height));
    
    bool isCmdActive = false;
    spControl->setWordInput(isCmdActive);
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        #ifndef TEST_BARYCENTRIC_COORDS
        
        if (!isCmdActive && spContext->isWindowActive())
            tool::Toolset::moveCameraFree(0, 0.1f, 0.25f, 90.0f, false);
        
        if (spControl->mouseHit(io::MOUSE_RIGHT))
            Cam->setPosition(0.0f);
        
        spScene->renderScene();
        
        //Draw2DText(dim::point2di(15, 15), "...", 0);
        
        if (!isCmdActive)
        {
            if (spControl->keyHit(io::KEY_PAGEUP))
            {
                if (++BlurFactor > 5)
                    BlurFactor = 5;
                if (LightmapPlotter->updateBluring(static_cast<u32>(BlurFactor)))
                    io::Log::message("Updated Bluring (Radius = " +  io::stringc(BlurFactor) +")");
            }
            if (spControl->keyHit(io::KEY_PAGEDOWN))
            {
                if (--BlurFactor < 0)
                    BlurFactor = 0;
                if (LightmapPlotter->updateBluring(static_cast<u32>(BlurFactor)))
                    io::Log::message("Updated Bluring (Radius = " +  io::stringc(BlurFactor) +")");
            }
            
            if (spControl->keyHit(io::KEY_RETURN))
            {
                static s32 ColorIndex;
                if (++ColorIndex > 5)
                    ColorIndex = 0;
                
                const video::color Color(AmbColors[ColorIndex]);
                
                if (LightmapPlotter->updateAmbientColor(Color))
                    io::Log::message("Updated Ambient Color " + tool::Debugging::toString(Color));
            }
            
            if (spControl->keyHit(io::KEY_SPACE))
            {
                UseRawLights = !UseRawLights;
                LightmapPlotter->getFinalModel()->getMeshBuffer(0)->getTexture()->setMinMagFilter(
                    UseRawLights ? video::FILTER_LINEAR : video::FILTER_SMOOTH
                );
            }
        }
        
        #else
        
        spRenderer->beginDrawing2D();
        
        const dim::point2df MousePos(spControl->getCursorPosition().cast<f32>());
        const dim::size2df ScrSize(spContext->getResolution().cast<f32>());
        const dim::size2df TexSize(Tex->getSize().cast<f32>());
        
        const dim::triangle3d<f32, dim::point2df> Tri(
            dim::point2df(50, 150),
            dim::point2df(650, 50),
            dim::point2df(250, 650)
        );
        const dim::triangle3df Map(
            dim::vector3df(0.0f, 0.0f, 0.0f),
            dim::vector3df(1.0f, 0.0f, 0.0f),
            dim::vector3df(0.0f, 1.0f, 0.0f)
        );
        
        f32 x = math::MinMax(MousePos.X/ScrSize.Width, 0.0f, 1.0f);
        f32 y = math::MinMax(MousePos.Y/ScrSize.Height, 0.0f, 1.0f);
        
        x *= TexSize.Width;
        y *= TexSize.Height;
        
        x = floor(x);
        y = floor(y);
        
        x += 0.5f;
        y += 0.5f;
        
        x /= TexSize.Width;
        y /= TexSize.Height;
        
        //dim::vector3df Coord(Map.getBarycentricCoord(dim::point2df(x, y)));
        //Coord /= (Coord.X + Coord.Y + Coord.Z);
        dim::vector3df Coord(math::getBarycentricCoord(Map, dim::vector3df(x, y, 0.0f)));
        
        const dim::point2df Point(Tri.getBarycentricPoint(Coord));
        
        scene::SPrimitiveVertex2D Verts[3] =
        {
            scene::SPrimitiveVertex2D(Tri.PointA.X, Tri.PointA.Y, Map.PointA.X, Map.PointA.Y),
            scene::SPrimitiveVertex2D(Tri.PointB.X, Tri.PointB.Y, Map.PointB.X, Map.PointB.Y),
            scene::SPrimitiveVertex2D(Tri.PointC.X, Tri.PointC.Y, Map.PointC.X, Map.PointC.Y)
        };
        
        spRenderer->draw2DPolygonImage(video::PRIMITIVE_TRIANGLES, Tex, Verts, 3);
        
        const video::color PointColor(255, 0, 0);
        const dim::point2di RealPoint(Point.cast<s32>());
        
        #   if 0
        spRenderer->draw2DBox(RealPoint, 15, PointColor);
        #   else
        spRenderer->draw2DLine(dim::point2di(RealPoint.X - 20, RealPoint.Y), dim::point2di(RealPoint.X + 20, RealPoint.Y), PointColor);
        spRenderer->draw2DLine(dim::point2di(RealPoint.X, RealPoint.Y - 20), dim::point2di(RealPoint.X, RealPoint.Y + 20), PointColor);
        #   endif
        
        spRenderer->endDrawing2D();
        
        Draw2DText(
            dim::point2di(15, 15),
            "Mouse: X = " + io::stringc(x) + ", Y = " + io::stringc(y)
        );
        Draw2DText(
            dim::point2di(15, 30),
            "Barycentric Coordinate: X = " + io::stringc(Coord.X) + ", Y = " + io::stringc(Coord.Y) + ", Z = " + io::stringc(Coord.Z)
        );
        
        #endif
        
        if (isCmdActive)
            cmd->render();
        
        if (spControl->keyHit(io::KEY_F3))
        {
            isCmdActive = !isCmdActive;
            spControl->setWordInput(isCmdActive);
        }
        
        spContext->flipBuffers();
    }
    
    delete LightmapPlotter;
    delete cmd;
    
    deleteDevice();
    
    return 0;
}

#else

int main()
{
    io::Log::error("Engine was not compiled with \"LightmapGenerator\"");
    io::Log::pauseConsole();
    return 0;
}

#endif
