//
// SoftPixel Engine Tutorial: Collisions - (06/08/2012)
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../../common.hpp"

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
scene::Mesh* MeshCastle     = 0;
scene::Mesh* MeshCone       = 0;

scene::CollisionSphere*     CollSphere  = 0;
scene::CollisionCapsule*    CollCapsule = 0;
scene::CollisionBox*        CollCube    = 0;

const s32 ScrWidth = 800, ScrHeight = 600;

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
    
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
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
    
    spScene     = spDevice->createSceneGraph();
    spWorld     = spDevice->createCollisionGraph();
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"
    );
    
    spDevice->setFrameRate(100);
}

void CollisionContact(
    scene::CollisionMaterial* Material, scene::CollisionNode* Node, const scene::CollisionNode* Rival, const scene::SCollisionContact &Contact)
{
    spRenderer->beginDrawing3D();
    spRenderer->setLineSize(5);
    spRenderer->draw3DLine(Contact.Point, Contact.Point + Contact.Normal, video::color(50, 50, 255));
    spRenderer->setLineSize(1);
    spRenderer->endDrawing3D();
    
    io::Log::message(Contact.Impact);
}

void CreateScene()
{
    // Create default objects we already know from other tutorials
    Font = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    Cam = spScene->createCamera();
    Cam->setPosition(dim::vector3df(0, 0, -5));
    Cam->setRange(0.1f, 250.0f);
    
    Light = spScene->createLight(scene::LIGHT_DIRECTIONAL);
    Light->setRotation(dim::vector3df(45, 10, 0));
    
    spScene->setLighting(true);
    
    // Create collision sphere
    scene::CollisionMaterial* CollSphereMaterial = spWorld->createMaterial();
    scene::CollisionMaterial* CollWorldMaterial = spWorld->createMaterial();
    
    CollSphereMaterial->addRivalCollisionMaterial(CollWorldMaterial);
    CollSphereMaterial->setCollisionContactCallback(CollisionContact);
    
    MeshSphere = spScene->createMesh(scene::MESH_SPHERE);
    MeshSphere->getMaterial()->setColorMaterial(false);
    MeshSphere->getMaterial()->setDiffuseColor(video::color(200, 50, 50));
    MeshSphere->getMaterial()->setAmbientColor(video::color(55, 0, 0));
    
    CollSphere = spWorld->createSphere(CollSphereMaterial, MeshSphere, 0.5f);
    
    // Create collision capsule
    MeshCapsule = spScene->createMesh(scene::MESH_CYLINDER);
    MeshCapsule->meshTransform(dim::vector3df(1, 3, 1));
    //MeshCapsule->meshTranslate(dim::vector3df(0, 1.5f, 0));
    MeshCapsule->setPosition(dim::vector3df(-2, -1.5f, 0));
    
    scene::Mesh* MeshWorld1b = spScene->createMesh(scene::MESH_SPHERE);
    MeshWorld1b->setParent(MeshCapsule);
    MeshWorld1b->setPosition(dim::vector3df(0, 1.5f, 0));
    
    scene::Mesh* MeshWorld1c = spScene->createMesh(scene::MESH_SPHERE);
    MeshWorld1c->setParent(MeshCapsule);
    MeshWorld1c->setPosition(dim::vector3df(0, -1.5f, 0));
    
    CollCapsule = spWorld->createCapsule(CollWorldMaterial, MeshCapsule, 0.5f, 3.0f);
    
    dim::matrix4f Mat;
    Mat.translate(dim::vector3df(0, -1.5f, 0));
    CollCapsule->setOffset(Mat);
    
    // Create collision cube
    MeshCube = spScene->createMesh(scene::MESH_CUBE);
    MeshCube->setScale(2);
    MeshCube->setPosition(dim::vector3df(3, 0, 0));
    
    CollCube = spWorld->createBox(CollWorldMaterial, MeshCube, dim::aabbox3df(-1.0f, 1.0f));
    
    // Create collision castle
    MeshCastle = spScene->loadMesh("D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelEngine/media/DemoCastleNew.spm");
    MeshCastle->setPosition(dim::vector3df(0, -7, -1));
    MeshCastle->meshTransform(0.01f);
    
    spWorld->createMesh(CollWorldMaterial, MeshCastle);
    
    // Create collision cone
    MeshCone = spScene->createMesh(scene::MESH_CONE);
    MeshCone->setPosition(dim::vector3df(-6, 0, 0));
    MeshCone->meshTranslate(dim::vector3df(0, 0.5f, 0));
    MeshCone->meshTransform(2);
    
    spWorld->createCone(CollWorldMaterial, MeshCone, 1.0f, 2.0f);
    
    
    //...
}

void UpdateScene()
{
    // Move collision sphere
    static const f32 MOVE_SPEED = 0.1f;
    
    if (spControl->keyDown(io::KEY_LEFT))
        CollSphere->translate(dim::vector3df(-MOVE_SPEED, 0, 0));
    if (spControl->keyDown(io::KEY_RIGHT))
        CollSphere->translate(dim::vector3df(MOVE_SPEED, 0, 0));
    if (spControl->keyDown(io::KEY_UP))
        CollSphere->translate(dim::vector3df(0, MOVE_SPEED, 0));
    if (spControl->keyDown(io::KEY_DOWN))
        CollSphere->translate(dim::vector3df(0, -MOVE_SPEED, 0));
    if (spControl->keyDown(io::KEY_PAGEUP))
        CollSphere->translate(dim::vector3df(0, 0, MOVE_SPEED));
    if (spControl->keyDown(io::KEY_PAGEDOWN))
        CollSphere->translate(dim::vector3df(0, 0, -MOVE_SPEED));
    
    if (spControl->keyHit(io::KEY_TAB))
    {
        static bool Wire;
        Wire = !Wire;
        spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
    }
    
    if (spControl->keyDown(io::KEY_NUMPAD6))
        CollCapsule->turn(dim::vector3df(0, 0, -1));
    if (spControl->keyDown(io::KEY_NUMPAD4))
        CollCapsule->turn(dim::vector3df(0, 0, 1));
    
    CollCube->turn(dim::vector3df(0, 0, 1));
    
    if (spContext->isWindowActive())
        tool::Toolset::moveCameraFree(0, 0.25f, 0.25f, 90.0f, false);
    
    // Update scene collisions
    spWorld->updateScene();
}

void DrawScene()
{
    
    spScene->renderScene();
    
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
