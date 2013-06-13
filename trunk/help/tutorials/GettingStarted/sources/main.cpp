//
// SoftPixel Engine - Getting started
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../../common.hpp"

#include <boost/foreach.hpp>

int main(void)
{
    // Create the graphics device to open the screen (in this case windowed screen).
    SoftPixelDevice* spDevice = createGraphicsDevice(
        ChooseRenderer(), dim::size2di(800, 600), 32, "Getting Started"
    );
    
    // Check for errors while creating the graphics device
    if (!spDevice)
    {
        io::Log::pauseConsole();
        return 0;
    }
    
    video::RenderSystem* spRenderer = spDevice->getRenderSystem();                  // Render system for drawing, rendering and general graphics hardware control.
    video::RenderContext* spContext = spDevice->getRenderContext();                 // Render context is basically only used to flip the video buffers.
    io::InputControl* spControl     = spDevice->getInputControl();                  // Input control to check for user inputs: keyboard, mouse etc.
    
    scene::SceneGraph* spScene      = spDevice->createSceneGraph();                 // Scene graph for creating cameras, lights, meshes and handling the whole scene.
    
    spContext->setWindowTitle(
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"       // Change the window title to display the type of renderer
    );
    
    scene::Camera* Cam  = spScene->createCamera();                                  // Create a camera to make our scene visible.
    scene::Light* Lit   = spScene->createLight();                                   // Create a light (by default directional light) to shade the scene.
    spScene->setLighting(true);                                                     // Activate global lighting
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_TEAPOT);                     // Create one of the standard meshes
    Obj->setPosition(dim::vector3df(0, 0, 3));                                      // Sets the object's position (x, y, z)
    
    video::Texture* Tex = spRenderer->loadTexture(
        //ROOT_PATH + "GettingStarted/media/SphereMap.jpg"                            // Load a texture. With a texture 2D images can be mapped onto 3D objects.
        "media/SphereMap.jpg"
    );
    
    Obj->addTexture(Tex);                                                           // Map the texture onto the mesh.
    Obj->getMeshBuffer(0)->setMappingGen(0, video::MAPGEN_SPHERE_MAP);              // Set texture coordinate generation (mapping gen) to sphere mapping.
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))         // The main loop will update our device
    {
        spRenderer->clearBuffers();                                                 // Clear the color- and depth buffer.
        
        tool::Toolset::presentModel(Obj);                                           // Present the model so that the user can turn the model by clicking and moving the mouse.
        
        spScene->renderScene();                                                     // Render the whole scene. In our example only one object (the teapot).
        
        spContext->flipBuffers();                                                   // Swap the video buffer to make the current frame visible.
    }
    
    deleteDevice();                                                                 // Delete the device context. This will delete and release all objects allocated by the engine.
    
    return 0;
}
