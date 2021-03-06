//
// SoftPixel Engine Tutorial: Collisions - (06/08/2012)
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../../common.hpp"

/*
INFO:
Until now, this is actually just a test app and not a tutorial!
*/

/* === Global members === */

SoftPixelDevice* spDevice       = 0;
io::InputControl* spControl     = 0;
video::RenderSystem* spRenderer = 0;
video::RenderContext* spContext = 0;

scene::SceneGraph* spScene      = 0;
scene::CollisionGraph* spWorld  = 0;

scene::Mesh* MeshSphere     = 0;
scene::Mesh* MeshCapsule    = 0;
scene::Mesh* MeshCube       = 0;
scene::Mesh* MeshCone       = 0;
scene::Mesh* MeshPlane      = 0;
scene::Mesh* MeshWorld      = 0;

scene::CollisionMaterial* CollObjMaterial   = 0;
scene::CollisionMaterial* CollWorldMaterial = 0;
scene::CollisionMaterial* CharCtrlMaterial  = 0;

scene::CollisionNode*       CollCtrlNode    = 0;
scene::CollisionSphere*     CollSphere      = 0;
scene::CollisionCapsule*    CollCapsule     = 0;
scene::CollisionBox*        CollCube        = 0;
scene::CollisionPlane*      CollPlane       = 0;
scene::CollisionMesh*       CollCastle      = 0;

scene::CharacterController* CharCtrl = 0;

const s32 ScrWidth = 1024, ScrHeight = 600;

scene::Camera* Cam  = 0;
scene::Light* Light = 0;

video::Font* Font = 0;

// Declarations
void InitDevice();
void CreateScene();
void UpdateScene();
void DrawScene();
void DrawCenteredText(const dim::point2di &Pos, const io::stringc &Text, const video::color &Color);


/* === All function definitions === */

int main()
{
    InitDevice();
    CreateScene();
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        UpdateScene();
        DrawScene();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}

void InitDevice()
{
    spDevice    = createGraphicsDevice(
        /*ChooseRenderer()*/video::RENDERER_OPENGL, dim::size2di(ScrWidth, ScrHeight), 32, "SoftPixel Engine - Collisions Tutorial"
    );
    
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    spControl   = spDevice->getInputControl();
    
    //spContext->setVsync(false);
    
    spScene     = spDevice->createSceneGraph();
    spWorld     = spDevice->createCollisionGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    //spDevice->setFrameRate(100);
}

bool CollisionContact(
    scene::CollisionMaterial* Material, scene::CollisionNode* Node, const scene::CollisionNode* Rival, const scene::SCollisionContact &Contact)
{
    spRenderer->beginDrawing3D();
    spRenderer->setLineSize(5);
    {
        spRenderer->draw3DLine(Contact.Point, Contact.Point + Contact.Normal * 0.5f, video::color(50, 50, 255));
    }
    spRenderer->setLineSize(1);
    spRenderer->endDrawing3D();
    
    io::Log::message("Impact = " + io::stringc(Contact.Impact));
    
    return true;
}

bool CharContactProc(scene::CharacterController* Controller, const scene::CollisionNode* Rival, const scene::SCollisionContact &Contact)
{
    spRenderer->beginDrawing3D();
    spRenderer->setLineSize(5);
    {
        spRenderer->draw3DTriangle(0, Contact.Triangle, video::color(255, 0, 0));
        spRenderer->draw3DLine(Contact.Point, Contact.Point + Contact.Normal * 0.5f, video::color(50, 50, 255));
    }
    spRenderer->setLineSize(1);
    spRenderer->endDrawing3D();
    
    io::Log::message("Impact = " + io::stringc(Contact.Impact));
    
    return true;
}

static scene::Mesh* CreateCapsuleMesh(bool isCentered = true)
{
    scene::Mesh* MeshCaps = spScene->createMesh(scene::MESH_CYLINDER);
    MeshCaps->meshTransform(dim::vector3df(1, 3, 1));
    MeshCaps->getMaterial()->setColorMaterial(false);
    
    scene::Mesh* MeshWorld1b = spScene->createMesh(scene::MESH_SPHERE);
    MeshWorld1b->setParent(MeshCaps);
    MeshWorld1b->setPosition(dim::vector3df(0, 1.5f, 0));
    
    scene::Mesh* MeshWorld1c = spScene->createMesh(scene::MESH_SPHERE);
    MeshWorld1c->setParent(MeshCaps);
    MeshWorld1c->setPosition(dim::vector3df(0, -1.5f, 0));
    
    if (!isCentered)
    {
        MeshCaps->meshTranslate(dim::vector3df(0, 1.5f, 0));
        MeshWorld1b->meshTranslate(dim::vector3df(0, 1.5f, 0));
        MeshWorld1c->meshTranslate(dim::vector3df(0, 1.5f, 0));
    }
    
    return MeshCaps;
}

void CreateScene()
{
    // Create default objects we already know from other tutorials
    Font = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    Cam = spScene->createCamera();
    Cam->setPosition(dim::vector3df(5, 5, -5));
    Cam->setRange(0.1f, 250.0f);
    
    Light = spScene->createLight(scene::LIGHT_DIRECTIONAL);
    Light->setRotation(dim::vector3df(25, 25, 0));
    
    spScene->setLighting(true);
    
    // Create collision sphere
    CollObjMaterial     = spWorld->createMaterial();
    CollWorldMaterial   = spWorld->createMaterial();
    CharCtrlMaterial    = spWorld->createMaterial();
    
    CollObjMaterial->addRivalMaterial(CollWorldMaterial);
    //CollObjMaterial->addRivalMaterial(CharCtrlMaterial);
    
    CharCtrlMaterial->addRivalMaterial(CollWorldMaterial);
    CharCtrlMaterial->addRivalMaterial(CollObjMaterial);
    
    CollObjMaterial->setContactCallback(CollisionContact);
    
    MeshSphere = spScene->createMesh(scene::MESH_SPHERE);
    MeshSphere->getMaterial()->setColorMaterial(false);
    MeshSphere->getMaterial()->setDiffuseColor(video::color(200, 50, 50));
    MeshSphere->getMaterial()->setAmbientColor(video::color(55, 0, 0));
    
    CollSphere = spWorld->createSphere(CollObjMaterial, MeshSphere, 0.5f);
    
    //#define KEEP_SCENE_SMALL
    #define LOAD_SCENE
    
    // Create collision capsule
    #ifndef KEEP_SCENE_SMALL
    
    MeshCapsule = CreateCapsuleMesh();
    MeshCapsule->setPosition(dim::vector3df(-2, -1.5f, 0));
    
    CollCapsule = spWorld->createCapsule(CollWorldMaterial, MeshCapsule, 0.5f, 3.0f);
    
    dim::matrix4f Mat;
    Mat.translate(dim::vector3df(0, -1.5f, 0));
    CollCapsule->setOffset(Mat);
    
    // Create 2nd collision capsule
    scene::Mesh* MeshCaps2 = CreateCapsuleMesh();
    MeshCaps2->setPosition(dim::vector3df(-4, -1.5f, 0));
    
    spWorld->createCapsule(CollWorldMaterial, MeshCaps2, 0.5f, 3.0f)->setOffset(Mat);
    
    // Create collision cube
    MeshCube = spScene->createMesh(scene::MESH_CUBE);
    MeshCube->setScale(2);
    MeshCube->setPosition(dim::vector3df(3, 0, 0));
    
    CollCube = spWorld->createBox(CollWorldMaterial, MeshCube, dim::aabbox3df(-0.5f, 0.5f));
    
    #endif
    
    // Create collision main scene
    const io::stringc MediaPath = ROOT_PATH + "Collisions/media/";
    
    #ifndef LOAD_SCENE
    
    MeshWorld = spScene->createMesh(scene::MESH_DODECAHEDRON);//loadMesh(MediaPath + "CollisionScene.spm");
    MeshWorld->meshTransform(2.0f);
    MeshWorld->meshTranslate(dim::vector3df(0, 0, 5));
    
    MeshWorld->addTexture(spRenderer->loadTexture(MediaPath + "Bricks.jpg"));
    MeshWorld->textureAutoMap(0, 0.25f);
    
    #else
    
    spScene->loadScene(MediaPath + "CollisionScene.spsb");
    
    MeshWorld = static_cast<scene::Mesh*>(spScene->findNode("Scene"));
    
    #endif
    
    CollCastle = spWorld->createMesh(CollWorldMaterial, MeshWorld);
    
    // Create collision cone
    MeshCone = spScene->createMesh(scene::MESH_CONE);
    MeshCone->setPosition(dim::vector3df(-6, 0, 0));
    MeshCone->meshTranslate(dim::vector3df(0, 0.5f, 0));
    MeshCone->meshTransform(2);
    
    spWorld->createCone(CollWorldMaterial, MeshCone, 1.0f, 2.0f);
    
    #ifdef LOAD_SCENE
    
    // Create collision plane
    MeshPlane = spScene->createMesh(scene::MESH_PLANE);
    //MeshPlane->setPosition(dim::vector3df(0, -10, 0));
    //MeshPlane->setRotation(dim::vector3df(0, 0, 10));
    MeshPlane->meshTransform(250);
    
    // Add tile texture to plane
    video::Texture* TileTex = spRenderer->createTexture(2, video::PIXELFORMAT_RGBA);
    u32 TileImageBuf[4] = { 0xFFFFFFFF, 0xFF000000, 0xFF000000, 0xFFFFFFFF };
    TileTex->setupImageBuffer(TileImageBuf);
    TileTex->setSize(10);
    MeshPlane->addTexture(TileTex);
    MeshPlane->textureAutoMap(0, 0.1f);
    
    CollPlane = spWorld->createPlane(CollWorldMaterial, MeshPlane, dim::plane3df(dim::vector3df(0, 1, 0), 0.0f));
    
    #endif
    
    // Create character controller
    scene::Mesh* MeshCaps3 = CreateCapsuleMesh(false);
    MeshCaps3->setPosition(dim::vector3df(0, 2, -3));
    //MeshCaps3->meshTranslate(dim::vector3df(0, 1.5f, 0));
    
    CharCtrl = spWorld->createCharacterController(CharCtrlMaterial, MeshCaps3, 0.5f, 3.0f);
    
    CharCtrl->setGravity(dim::vector3df(0, -0.025f, 0));
    
    CharCtrl->setContactCallback(CharContactProc);
    
    Cam->setParent(CharCtrl->getCollisionModel()->getNode());
    Cam->setPosition(dim::vector3df(0, 3, 0));
    
    // Final settings
    CollCtrlNode = CollSphere;
}

void UpdateScene()
{
    static bool TurnCube;
    
    spContext->setWindowTitle(io::Timer::getFPS());
    
    if (spControl->keyHit(io::KEY_RETURN))
    {
        static s32 ObjUsage;
        
        if (++ObjUsage > 2)
            ObjUsage = 0;
        
        scene::Mesh* PrevMesh = 0;
        scene::Mesh* NextMesh = 0;
        scene::CollisionNode* PrevCollNode = 0;
        
        switch (ObjUsage)
        {
            case 0:
                PrevMesh        = MeshCube;
                NextMesh        = MeshSphere;
                PrevCollNode    = CollCube;
                CollCtrlNode    = CollSphere;
                break;
                
            case 1:
                PrevMesh        = MeshSphere;
                NextMesh        = MeshCapsule;
                PrevCollNode    = CollSphere;
                CollCtrlNode    = CollCapsule;
                break;
                
            case 2:
                PrevMesh        = MeshCapsule;
                NextMesh        = MeshCube;
                PrevCollNode    = CollCapsule;
                CollCtrlNode    = CollCube;
                break;
        }
        
        if (PrevCollNode)
            PrevCollNode->setMaterial(CollWorldMaterial);
        if (CollCtrlNode)
            CollCtrlNode->setMaterial(CollObjMaterial);
        
        video::MaterialStates* PrevMaterial = PrevMesh->getMaterial();
        PrevMaterial->setDiffuseColor(video::color(200, 200, 200));
        PrevMaterial->setAmbientColor(video::color(55, 55, 55));
        
        video::MaterialStates* NextMaterial = NextMesh->getMaterial();
        NextMaterial->setDiffuseColor(video::color(200, 50, 50));
        NextMaterial->setAmbientColor(video::color(55, 0, 0));
    }
    
    // Move collision sphere
    f32 MoveSpeed = 0.1f;
    
    if (spControl->keyDown(io::KEY_SHIFT))
        MoveSpeed = 1.0f;
    
    if (CollCtrlNode)
    {
        if (spControl->keyDown(io::KEY_NUMPAD4))
            CollCtrlNode->translate(dim::vector3df(-MoveSpeed, 0, 0));
        if (spControl->keyDown(io::KEY_NUMPAD6))
            CollCtrlNode->translate(dim::vector3df(MoveSpeed, 0, 0));
        if (spControl->keyDown(io::KEY_NUMPAD8))
            CollCtrlNode->translate(dim::vector3df(0, MoveSpeed, 0));
        if (spControl->keyDown(io::KEY_NUMPAD2))
            CollCtrlNode->translate(dim::vector3df(0, -MoveSpeed, 0));
        if (spControl->keyDown(io::KEY_NUMPAD9))
            CollCtrlNode->translate(dim::vector3df(0, 0, MoveSpeed));
        if (spControl->keyDown(io::KEY_NUMPAD3))
            CollCtrlNode->translate(dim::vector3df(0, 0, -MoveSpeed));
    }
    
    if (spControl->keyHit(io::KEY_TAB))
    {
        static bool Wire;
        Wire = !Wire;
        spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
    }
    
    if (CollCapsule)
    {
        if (spControl->keyDown(io::KEY_NUMPAD7))
            CollCapsule->turn(dim::vector3df(0, 0, -1));
        if (spControl->keyDown(io::KEY_NUMPAD1))
            CollCapsule->turn(dim::vector3df(0, 0, 1));
    }
    
    dim::point2df MouseSpeed(spControl->getCursorSpeed().cast<f32>());
    
    if (spControl->keyHit(io::KEY_SHIFT))
        TurnCube = !TurnCube;
    
    if (CollCube)
    {
        if (TurnCube)
        {
            dim::matrix4f Mat;
            Mat.rotateY(-MouseSpeed.X * 0.5f);
            Mat.rotateX(-MouseSpeed.Y * 0.5f);
            
            CollCube->setRotation(Mat * CollCube->getRotation());
        }
        else
            CollCube->turn(dim::vector3df(0, 0, 1));
    }
    
    static bool FPSView = true;
    
    if (spControl->keyHit(io::KEY_V))
    {
        FPSView = !FPSView;
        
        if (FPSView)
        {
            Cam->setParent(CharCtrl->getCollisionModel()->getNode());
            Cam->setPosition(dim::vector3df(0, 3, 0));
        }
        else
        {
            Cam->setParent(0);
            CharCtrl->setViewRotation(0);
        }
    }
    
    if (spContext->isWindowActive())
    {
        if (FPSView)
        {
            static f32 Pitch, Yaw;
            
            Pitch   += MouseSpeed.Y * 0.25f;
            Yaw     += MouseSpeed.X * 0.25f;
            
            math::clamp(Pitch, -90.0f, 90.0f);
            
            Cam->setRotation(dim::vector3df(Pitch, Yaw, 0));
            CharCtrl->setViewRotation(Yaw);
            
            spControl->setCursorPosition(dim::point2di(ScrWidth/2, ScrHeight/2));
        }
        else if (!TurnCube)
            tool::Toolset::moveCameraFree(0, 0.25f, 0.25f, 90.0f, false);
    }
    
    // Update character controller
    if (FPSView)
    {
        const f32 CharMoveSpeed = 0.05f;
        
        if (CharCtrl->stayOnGround())
        {
            if (spControl->keyDown(io::KEY_A))
                CharCtrl->moveLeft(CharMoveSpeed);
            if (spControl->keyDown(io::KEY_D))
                CharCtrl->moveRight(CharMoveSpeed);
            if (spControl->keyDown(io::KEY_W))
                CharCtrl->moveForwards(CharMoveSpeed);
            if (spControl->keyDown(io::KEY_S))
                CharCtrl->moveBackwards(CharMoveSpeed);
        }
    }
    
    if (spControl->keyHit(io::KEY_SPACE) && CharCtrl->stayOnGround())
        CharCtrl->jump(0.75f);
    
    CharCtrl->update();
    
    // Update scene collisions
    spWorld->updateScene();
}

void DrawScene()
{
    
    spScene->renderScene();
    
    spRenderer->beginDrawing2D();
    
    if (CollCtrlNode)
    {
        DrawCenteredText(
            dim::point2di(ScrWidth/2, 15),
            "SpherPos = " + tool::Debugging::toString(CollCtrlNode->getNode()->getPosition(true)),
            255
        );
    }
    
    DrawCenteredText(
        dim::point2di(ScrWidth/2, 35),
        CharCtrl->stayOnGround() ? "Stay On Ground" : "Stay Not On Ground",
        255
    );
    
    spRenderer->endDrawing2D();
    
}

void DrawCenteredText(const dim::point2di &Pos, const io::stringc &Text, const video::color &Color)
{
    // Get the text size
    const dim::size2di TextSize(Font->getStringSize(Text));
    
    // Draw the text (background black and foreground with specified color)
    spRenderer->draw2DText(
        Font, Pos - dim::point2di(TextSize.Width/2, TextSize.Height/2) + 2, Text, video::color(0, 0, 0, Color.Alpha)
    );
    spRenderer->draw2DText(
        Font, Pos - dim::point2di(TextSize.Width/2, TextSize.Height/2), Text, Color
    );
}



// ============================================
