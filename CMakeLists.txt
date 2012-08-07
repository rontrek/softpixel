
# === CMake lists for the SoftPixel Engine - (09/07/2012) ===

cmake_minimum_required(VERSION 2.8)
project(SoftPixelEngine)


# === Build path ===

set(dir ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)


# === Preprocessor definitions ===

add_definitions(-D_CRT_SECURE_NO_WARNINGS)


# === Global files ===

file(GLOB FilesBase						sources/Base/sp*)
file(GLOB FilesFormatsImage				sources/FileFormats/Image/sp*)
file(GLOB FilesFormatsMesh				sources/FileFormats/Mesh/sp*)
file(GLOB FilesFormatsScene				sources/FileFormats/Scene/sp*)
file(GLOB FilesFormatsSound				sources/FileFormats/Sound/sp*)
file(GLOB FilesFrameworkCg				sources/Framework/Cg/sp*)
file(GLOB FilesFrameworkNetwork			sources/Framework/Network/sp*)
file(GLOB FilesFrameworkOpenCL			sources/Framework/OpenCL/sp*)
file(GLOB FilesFrameworkPhysics			sources/Framework/Physics/sp*)
file(GLOB FilesFrameworkPhysicsBullet	sources/Framework/Physics/Bullet/sp*)
file(GLOB FilesFrameworkPhysicsNewton	sources/Framework/Physics/Newton/sp*)
file(GLOB FilesFrameworkPhysicsPhysX	sources/Framework/Physics/PhysX/sp*)
file(GLOB FilesFrameworkTools			sources/Framework/Tools/sp*)
file(GLOB FilesGUI						sources/GUI/sp*)
file(GLOB FilesPlatform					sources/Platform/sp*)
file(GLOB FilesPlugins					sources/Plugins/*.h)
file(GLOB FilesPluginsJpeg				sources/Plugins/jpeglib/*)
file(GLOB FilesPluginsPng				sources/Plugins/libpng/*)
file(GLOB FilesPluginsZLib				sources/Plugins/zlib/*)
file(GLOB FilesRenderer					sources/RenderSystem/sp*)
file(GLOB FilesRendererD3D9				sources/RenderSystem/Direct3D9/spDirect3D9*)
file(GLOB FilesRendererD3D11			sources/RenderSystem/Direct3D11/spDirect3D11*)
file(GLOB FilesRendererGL				sources/RenderSystem/OpenGL/sp*)
file(GLOB FilesRendererGLES				sources/RenderSystem/OpenGLES/sp*)
file(GLOB FilesScene					sources/SceneGraph/sp*)
file(GLOB FilesSceneAnimation			sources/SceneGraph/Animation/sp*)
file(GLOB FilesSceneCollision			sources/SceneGraph/Collision/sp*)
file(GLOB FilesSound					sources/SoundSystem/spSound*)
file(GLOB FilesSoundDummy				sources/SoundSystem/spDummy*)
file(GLOB FilesSoundAL					sources/SoundSystem/OpenAL/*)
file(GLOB FilesSoundSLES				sources/SoundSystem/OpenSLES/*)
file(GLOB FilesSoundWinMM				sources/SoundSystem/WinMM/*)
file(GLOB FilesSoundXAudio2				sources/SoundSystem/XAudio2/*)

file(GLOB FilesBaseDim					sources/Base/spDimension*)
file(GLOB FilesBaseIO					sources/Base/spInputOutput*)
file(GLOB FilesBaseMath					sources/Base/spMath*)
file(GLOB FilesTreeNodes				sources/Base/spTreeNode*)
file(GLOB FilesVertexFormats			sources/Base/spVertexFormat*)
file(GLOB FilesImageBuffers				sources/Base/spImage*)
file(GLOB FilesMaterial					sources/Base/spMaterial*)
file(GLOB FilesSceneCameras				sources/SceneGraph/spCamera*)
file(GLOB FilesSceneGraphs				sources/SceneGraph/spSceneGraph*)
file(GLOB FilesTools					sources/Framework/Tools/spTool*)
file(GLOB FilesStoryboard				sources/Framework/Tools/spStoryboard*)
file(GLOB FilesShader					sources/RenderSystem/spShader*)

set(
	FilesReadme
	History.txt
	Information.txt
	LICENSE.txt
)

set(
	FilesResources
	sources/Resources/D3D11DefaultBasicShader2D\(SM4\).h
	sources/Resources/D3D11DefaultBasicShader3D\(SM4\).h
)


# === Source group folders ===

source_group(
	"Engine\\Audio\\FileFormats" FILES
	${FilesFormatsSound}
)

source_group(
	"Engine\\Audio\\SoundSystem\\Dummy" FILES
	${FilesSoundDummy}
)

source_group(
	"Engine\\Audio\\SoundSystem\\OpenAL" FILES
	${FilesSoundAL}
)

source_group(
	"Engine\\Audio\\SoundSystem\\OpenSLES" FILES
	${FilesSoundSLES}
)

source_group(
	"Engine\\Audio\\SoundSystem\\WinMM" FILES
	${FilesSoundWinMM}
)

source_group(
	"Engine\\Audio\\SoundSystem\\XAudio2" FILES
	${FilesSoundXAudio2}
)

source_group(
	"Engine\\Audio\\SoundSystem" FILES
	${FilesSound}
)

source_group(
	"Engine\\Base\\Threading" FILES
	sources/Base/spCriticalSection.cpp
	sources/Base/spCriticalSection.hpp
	sources/Base/spThreadManager.cpp
	sources/Base/spThreadManager.hpp
)

source_group(
	"Engine\\Base" FILES
	sources/Base/spBaseObject.hpp
	sources/Base/spBaseTypes.hpp
	sources/Base/spCompilationOptions.hpp
	sources/Base/spInternalDeclarations.hpp
	sources/Base/spMemoryManagement.hpp
	sources/Base/spNode.cpp
	sources/Base/spNode.hpp
	sources/Base/spSharedObjects.hpp
	sources/Base/spStandard.cpp
	sources/Base/spStandard.hpp
)

source_group(
	"Engine\\Device" FILES
	${FilesPlatform}
)

source_group(
	"Engine\\Dim" FILES
	${FilesBaseDim}
)

source_group(
	"Engine\\GUI" FILES
	${FilesGUI}
)

source_group(
	"Engine\\IO" FILES
	sources/Base/spBaseFileFormatHandler.cpp
	sources/Base/spBaseFileFormatHandler.hpp
	sources/Base/spInputKeyCodes.hpp
	${FilesBaseIO}
	sources/Base/spTimer.cpp
	sources/Base/spTimer.hpp
)

source_group(
	"Engine\\Math" FILES
	${FilesBaseMath}
)

source_group(
	"Engine\\Network\\Base" FILES
	sources/Framework/Network/spNetworkAddress.cpp
	sources/Framework/Network/spNetworkAddress.hpp
	sources/Framework/Network/spNetworkClient.cpp
	sources/Framework/Network/spNetworkClient.hpp
	sources/Framework/Network/spNetworkCore.hpp
	sources/Framework/Network/spNetworkMember.cpp
	sources/Framework/Network/spNetworkMember.hpp
	sources/Framework/Network/spNetworkPacket.cpp
	sources/Framework/Network/spNetworkPacket.hpp
	sources/Framework/Network/spNetworkServer.cpp
	sources/Framework/Network/spNetworkServer.hpp
	sources/Framework/Network/spNetworkSocket.cpp
	sources/Framework/Network/spNetworkSocket.hpp
	sources/Framework/Network/spNetworkStructures.hpp
)

source_group(
	"Engine\\Network\\System" FILES
	sources/Framework/Network/spNetworkBase.cpp
	sources/Framework/Network/spNetworkBase.hpp
	sources/Framework/Network/spNetworkBaseUDP.cpp
	sources/Framework/Network/spNetworkBaseUDP.hpp
	sources/Framework/Network/spNetworkSessionLogin.cpp
	sources/Framework/Network/spNetworkSessionLogin.hpp
	sources/Framework/Network/spNetworkSessionReception.cpp
	sources/Framework/Network/spNetworkSessionReception.hpp
	sources/Framework/Network/spNetworkSystem.cpp
	sources/Framework/Network/spNetworkSystem.hpp
	sources/Framework/Network/spNetworkSystemTCP.cpp
	sources/Framework/Network/spNetworkSystemTCP.hpp
	sources/Framework/Network/spNetworkSystemUDP.cpp
	sources/Framework/Network/spNetworkSystemUDP.hpp
)

source_group(
	"Engine\\Physics" FILES
	${FilesFrameworkPhysics}
)

source_group(
	"Engine\\Physics\\Bullet" FILES
	${FilesFrameworkPhysicsBullet}
)

source_group(
	"Engine\\Physics\\Newton" FILES
	${FilesFrameworkPhysicsNewton}
)

source_group(
	"Engine\\Physics\\PhysX" FILES
	${FilesFrameworkPhysicsPhysX}
)

source_group(
	"Engine\\Scene" FILES
	sources/Base/spBasicMeshGenerator.cpp
	sources/Base/spBasicMeshGenerator.hpp
	sources/Base/spGeometryStructures.hpp
	sources/Base/spStencilManager.cpp
	sources/Base/spStencilManager.hpp
)

source_group(
	"Engine\\Scene\\Animation" FILES
	${FilesSceneAnimation}
)

source_group(
	"Engine\\Scene\\Collision" FILES
	${FilesSceneCollision}
)

source_group(
	"Engine\\Scene\\FileFormats" FILES
	${FilesFormatsMesh}
	${FilesFormatsScene}
)

source_group(
	"Engine\\Scene\\Objects" FILES
	sources/SceneGraph/spBoundingVolume.cpp
	sources/SceneGraph/spBoundingVolume.hpp
	${FilesSceneCameras}
	sources/SceneGraph/spMaterialNode.cpp
	sources/SceneGraph/spMaterialNode.hpp
	sources/SceneGraph/spRenderNode.cpp
	sources/SceneGraph/spRenderNode.hpp
	sources/SceneGraph/spSceneBillboard.cpp
	sources/SceneGraph/spSceneBillboard.hpp
	sources/SceneGraph/spSceneCamera.cpp
	sources/SceneGraph/spSceneCamera.hpp
	sources/SceneGraph/spSceneLight.cpp
	sources/SceneGraph/spSceneLight.hpp
	sources/SceneGraph/spSceneMesh.cpp
	sources/SceneGraph/spSceneMesh.hpp
	sources/SceneGraph/spSceneNode.cpp
	sources/SceneGraph/spSceneNode.hpp
	sources/SceneGraph/spSceneTerrain.cpp
	sources/SceneGraph/spSceneTerrain.hpp
)

source_group(
	"Engine\\Scene\\SceneGraph" FILES
	${FilesSceneGraphs}
)

source_group(
	"Engine\\Scene\\TreeNode" FILES
	${FilesTreeNodes}
	sources/Base/spTreeBuilder.cpp
	sources/Base/spTreeBuilder.hpp
)

source_group(
	"Engine\\Utility" FILES
	sources/Framework/Tools/spExtendedToolset.cpp
	sources/Framework/Tools/spExtendedToolset.hpp
	${FilesTools}
	sources/Framework/Tools/spUtilityInputService.cpp
	sources/Framework/Tools/spUtilityInputService.hpp
)

source_group(
	"Engine\\Utility\\Storyboard" FILES
	${FilesStoryboard}
)

source_group(
	"Engine\\Video" FILES
	sources/RenderSystem/spMovieSequencePacket.hpp
	sources/RenderSystem/spRenderSystemFont.cpp
	sources/RenderSystem/spRenderSystemFont.hpp
	sources/RenderSystem/spRenderSystemMovie.cpp
	sources/RenderSystem/spRenderSystemMovie.hpp
	sources/RenderSystem/spVideoModeEnumerator.cpp
	sources/RenderSystem/spVideoModeEnumerator.hpp
)

source_group(
	"Engine\\Video\\FileFormats" FILES
	${FilesFormatsImage}
)

source_group(
	"Engine\\Video\\HardwareBuffer" FILES
	sources/Base/spIndexFormat.cpp
	sources/Base/spIndexFormat.hpp
	sources/Base/spMeshBuffer.cpp
	sources/Base/spMeshBuffer.hpp
	${FilesVertexFormats}
)

source_group(
	"Engine\\Video\\ImageManagement" FILES
	${FilesImageBuffers}
)

source_group(
	"Engine\\Video\\Material" FILES
	${FilesMaterial}
)

source_group(
	"Engine\\Video\\RenderSystem" FILES
	sources/RenderSystem/spDesktopRenderContext.cpp
	sources/RenderSystem/spDesktopRenderContext.hpp
	sources/RenderSystem/spRenderContext.cpp
	sources/RenderSystem/spRenderContext.hpp
	sources/RenderSystem/spRenderSystem.cpp
	sources/RenderSystem/spRenderSystem.hpp
	sources/RenderSystem/spSharedRenderContext.hpp
	sources/RenderSystem/spTextureBase.cpp
	sources/RenderSystem/spTextureBase.hpp
	sources/RenderSystem/spTextureFlags.hpp
)

source_group(
	"Engine\\Video\\RenderSystem\\Direct3D11" FILES
	${FilesRendererD3D11}
)

source_group(
	"Engine\\Video\\RenderSystem\\Direct3D9" FILES
	${FilesRendererD3D9}
)

source_group(
	"Engine\\Video\\RenderSystem\\Dummy" FILES
	sources/RenderSystem/spDummyRenderSystem.cpp
	sources/RenderSystem/spDummyRenderSystem.hpp
)

source_group(
	"Engine\\Video\\RenderSystem\\OpenGL" FILES
	sources/RenderSystem/OpenGL/spOpenGLFunctionsARB.cpp
	sources/RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp
	sources/RenderSystem/OpenGL/spOpenGLRenderContext.cpp
	sources/RenderSystem/OpenGL/spOpenGLRenderContext.hpp
	sources/RenderSystem/OpenGL/spOpenGLRenderSystem.cpp
	sources/RenderSystem/OpenGL/spOpenGLRenderSystem.hpp
	sources/RenderSystem/OpenGL/spOpenGLShader.cpp
	sources/RenderSystem/OpenGL/spOpenGLShader.hpp
	sources/RenderSystem/OpenGL/spOpenGLShaderClass.cpp
	sources/RenderSystem/OpenGL/spOpenGLShaderClass.hpp
	sources/RenderSystem/OpenGL/spOpenGLSharedRenderContext.cpp
	sources/RenderSystem/OpenGL/spOpenGLSharedRenderContext.hpp
	sources/RenderSystem/OpenGL/spOpenGLTexture.cpp
	sources/RenderSystem/OpenGL/spOpenGLTexture.hpp
)

source_group(
	"Engine\\Video\\RenderSystem\\OpenGLES1" FILES
	sources/RenderSystem/OpenGLES/spOpenGLES1RenderSystem.cpp
	sources/RenderSystem/OpenGLES/spOpenGLES1RenderSystem.hpp
	sources/RenderSystem/OpenGLES/spOpenGLES1Texture.cpp
	sources/RenderSystem/OpenGLES/spOpenGLES1Texture.hpp
)

source_group(
	"Engine\\Video\\RenderSystem\\OpenGLES2" FILES
	sources/RenderSystem/OpenGLES/spOpenGLES2RenderSystem.cpp
	sources/RenderSystem/OpenGLES/spOpenGLES2RenderSystem.hpp
	sources/RenderSystem/OpenGLES/spOpenGLES2Texture.cpp
	sources/RenderSystem/OpenGLES/spOpenGLES2Texture.hpp
)

source_group(
	"Engine\\Video\\RenderSystem\\OpenGLPipeline" FILES
	sources/RenderSystem/OpenGL/spOpenGLCoreHeader.hpp
	sources/RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp
	sources/RenderSystem/OpenGLES/spOpenGLESRenderContext.cpp
	sources/RenderSystem/OpenGLES/spOpenGLESRenderContext.hpp
	sources/RenderSystem/OpenGL/spOpenGLFrameBufferObject.cpp
	sources/RenderSystem/OpenGL/spOpenGLFrameBufferObject.hpp
	sources/RenderSystem/OpenGL/spOpenGLPipelineBase.cpp
	sources/RenderSystem/OpenGL/spOpenGLPipelineBase.hpp
	sources/RenderSystem/OpenGL/spOpenGLPipelineFixed.cpp
	sources/RenderSystem/OpenGL/spOpenGLPipelineFixed.hpp
	sources/RenderSystem/OpenGL/spOpenGLPipelineProgrammable.cpp
	sources/RenderSystem/OpenGL/spOpenGLPipelineProgrammable.hpp
	sources/RenderSystem/OpenGL/spOpenGLTextureBase.cpp
	sources/RenderSystem/OpenGL/spOpenGLTextureBase.hpp
)

source_group(
	"Engine\\Video\\Shader\\Cg" FILES
	${FilesFrameworkCg}
)

source_group(
	"Engine\\Video\\Shader\\OpenCL" FILES
	${FilesFrameworkOpenCL}
)

source_group(
	"Engine\\Video\\Shader" FILES
	sources/RenderSystem/spComputeShader.hpp
	${FilesShader}
)

source_group(
	"Engine" FILES
	sources/SoftPixelEngine.hpp
)

source_group(
	"Readme" FILES
	${FilesReadme}
)

source_group(
	"Resources\\Plugins\\jpeglib" FILES
	${FilesPluginsJpeg}
)

source_group(
	"Resources\\Plugins\\libpng" FILES
	${FilesPluginsPng}
)

source_group(
	"Resources\\Plugins\\zlib" FILES
	${FilesPluginsZLib}
)

source_group(
	"Resources\\Plugins" FILES
	${FilesPlugins}
)

source_group(
	"Resources" FILES
	${FilesResources}
)


# === Shared library ===

add_library(
	SoftPixelEngine
	SHARED
	
	sources/SoftPixelEngine.hpp
	
	${FilesBase}
	${FilesFormatsImage}
	${FilesFormatsMesh}
	${FilesFormatsScene}
	${FilesFormatsSound}
	${FilesFrameworkCg}
	${FilesFrameworkNetwork}
	${FilesFrameworkOpenCL}
	${FilesFrameworkPhysics}
	${FilesFrameworkPhysicsBullet}
	${FilesFrameworkPhysicsNewton}
	${FilesFrameworkPhysicsPhysX}
	${FilesFrameworkTools}
	${FilesGUI}
	${FilesPlatform}
	${FilesPlugins}
	${FilesPluginsJpeg}
	${FilesPluginsPng}
	${FilesPluginsZLib}
	${FilesRenderer}
	${FilesRendererD3D9}
	${FilesRendererD3D11}
	${FilesRendererGL}
	${FilesRendererGLES}
	${FilesResources}
	${FilesScene}
	${FilesSceneAnimation}
	${FilesSceneCollision}
	${FilesSound}
	${FilesSoundDummy}
	${FilesSoundAL}
	${FilesSoundSLES}
	${FilesSoundWinMM}
	${FilesSoundXAudio2}
	
	${FilesReadme}
)

set_target_properties(SoftPixelEngine PROPERTIES LINKER_LANGUAGE CXX)


# === External libraries ===

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sources)

if(APPLE)
	include_directories(/System/Library/Frameworks)
endif(APPLE)

# Boost C++ Library
find_package(Boost 1.47.0 REQUIRED)
if(Boost_FOUND)
	message("Found Boost C++ Library")
	include_directories(${Boost_INCLUDE_DIRS})
else(Boost_FOUND)
	message(SEND_ERROR "Missing Boost C++ Library")
endif(Boost_FOUND)

# OpenGL Library
find_package(OpenGL)
if(OPENGL_FOUND)
	message("Found OpenGL Library")
	include_directories(${OPENGL_INCLUDE_DIR})
	target_link_libraries(SoftPixelEngine ${OPENGL_LIBRARIES})
else(OPENGL_FOUND)
	message("Missing OpenGL Library")
endif(OPENGL_FOUND)

# OpenAL Library
include(sources/FindOpenAL.cmake)
if(OPENAL_FOUND)
	message("Found OpenAL Library")
	include_directories(${OPENAL_INCLUDE_DIR})
	target_link_libraries(SoftPixelEngine ${OPENAL_LIBRARY} ${OPENAL_EFXUTIL_LIBRARY})
else(OPENAL_FOUND)
	message("Missing OpenAL Library")
endif(OPENAL_FOUND)

# OpenCL Library
include(sources/FindOpenCL.cmake)
if(OPENCL_INCLUDE_DIRS)
	message("Found OpenCL Library")
	include_directories(${OPENCL_INCLUDE_DIRS})
	target_link_libraries(SoftPixelEngine ${OPENCL_LIBRARIES})
else(OPENCL_INCLUDE_DIRS)
	message("Missing OpenCL Library")
endif(OPENCL_INCLUDE_DIRS)

# Bullet Physics Engine
find_package(Bullet)
if(BULLET_FOUND)
	message("Found Bullet Physics Engine")
	include_directories(${BULLET_INCLUDE_DIRS})
	target_link_libraries(SoftPixelEngine ${BULLET_LIBRARIES})
else(BULLET_FOUND)
	message("Missing Bullet Physics Engine")
endif(BULLET_FOUND)

if(WIN32)
	# WinMM
	target_link_libraries(SoftPixelEngine winmm)
	
	# Socket Library
	target_link_libraries(SoftPixelEngine WS2_32)
	target_link_libraries(SoftPixelEngine NetAPI32)
	target_link_libraries(SoftPixelEngine Vfw32)
	target_link_libraries(SoftPixelEngine Iphlpapi)
endif(WIN32)

# === DirectX SDK ===

if(WIN32)
	include(sources/FindDirectXSDK.cmake)
	
	include_directories(${DirectX_INCLUDE_DIRS})
	target_link_libraries(
		SoftPixelEngine
		${DirectX_D3D9_LIBRARY}
		${DirectX_D3D11_LIBRARY}
		${DirectX_D2D1_LIBRARY}
		${DirectX_DWRITE_LIBRARY}
		${DirectX_DXGUID_LIBRARY}
		${DirectX_D3DCOMPILER_LIBRARY}
	)
endif(WIN32)

# === NVIDIA Cg Toolkit ===

include(sources/FindCgToolkit.cmake)

if(CG_INCLUDE_PATH)
	message("Found NVIDIA Cg Toolkit")
	include_directories(${CG_INCLUDE_PATH})
	target_link_libraries(SoftPixelEngine ${CG_LIBRARY})
	target_link_libraries(SoftPixelEngine ${CG_GL_LIBRARY})
	if(WIN32)
		target_link_libraries(SoftPixelEngine ${CG_D3D9_LIBRARY})
		target_link_libraries(SoftPixelEngine ${CG_D3D11_LIBRARY})
	endif(WIN32)
else(CG_INCLUDE_PATH)
	message("Missing NVIDIA Cg Toolkit")
endif(CG_INCLUDE_PATH)

# === X11 Lib ===

#if(UNIX)
#	find_package(X11)
#	
#endif(UNIX)

# === Other CMakeLists ===

set(TutorialPath help/tutorials)
set(ToolPath tools)

include(sources/Plugins/SpSbImportExport/CMakeLists.txt)

include(${TutorialPath}/Collisions/sources/CMakeLists.txt)
include(${TutorialPath}/Drawing2D/sources/CMakeLists.txt)
include(${TutorialPath}/GettingStarted/sources/CMakeLists.txt)
include(${TutorialPath}/GraphicsUserInterface/sources/CMakeLists.txt)
include(${TutorialPath}/Networking/sources/CMakeLists.txt)
include(${TutorialPath}/Primitives/sources/CMakeLists.txt)
include(${TutorialPath}/SceneLoader/sources/CMakeLists.txt)
include(${TutorialPath}/ShaderLibrary/sources/CMakeLists.txt)
include(${TutorialPath}/Shaping/sources/CMakeLists.txt)
include(${TutorialPath}/Tessellation/sources/CMakeLists.txt)

include(${ToolPath}/MeshViewer/sources/CMakeLists.txt)


target_link_libraries(SoftPixelEngine SpSbImportExport)
