/*
 * Light grid header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_LIGHT_GRID_H__
#define __SP_LIGHT_GRID_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spTextureBase.hpp"


namespace sp
{

namespace scene
{
    class Light;
    class Camera;
    class SceneGraph;
}

namespace video
{


class ShaderResource;
class ShaderClass;

/**
The light grid is used by the deferred renderer for tiled shading.
\since Version 3.3
*/
class SP_EXPORT LightGrid
{
    
    public:
        
        LightGrid();
        ~LightGrid();
        
        /* === Functions === */
        
        /**
        Creates the light grid.
        \param[in] Resolution Specifies the resolution. This should be the same as specified for the engine's graphics device.
        \param[in] TileCount Specifies the count of tiles on the X and Y axes.
        \return True if the grid could be created successful.
        */
        bool createGrid(const dim::size2di &Resolution, const dim::size2di &TileCount);
        void deleteGrid();
        
        /**
        Builds the light grid. For Direct3D 11 this function uses a compute shader.
        Otherwise the grid will be computed on the CPU.
        */
        void build(scene::SceneGraph* Graph, scene::Camera* ActiveCamera);

        /**
        Builds the light grid by filling the TLI buffer texture with indices of each light,
        which intersects the respective grid tiles.
        \see fillLightIntoGrid
        */
        template <typename T> void buildGrid(T itBegin, T itEnd)
        {
            /*for (typename T it = itBegin; it != itEnd; ++it)
                fillLightIntoGrid(*it);*/
        }
        
        /**
        Fills the given light source into the grid.
        \see buildGrid
        */
        void fillLightIntoGrid(scene::Light* Obj);
        
        s32 bind(s32 TexLayerBase);
        s32 unbind(s32 TexLayerBase);

        /* === Inline functions === */
        
        //! Returns the TLI (Tile Light Index List) texture object. This is an signed integer texture buffer.
        inline Texture* getTLITexture() const
        {
            return TLITexture_;
        }
        /**
        Returns the TLI (Tile Light Index List) shader resource object. This is an 'int2 shader buffer'.
        \code
        // HLSL Example:
        Buffer<int2> MyTLIBuffer : register(t2);

        // GLSL Example:
        layout(std430, binding = 2) buffer MyTLIBuffer
        {
            vec2 CountAndOffset;
        }
        \endcode
        */
        inline ShaderResource* getTLIShaderResource() const
        {
            return TLIShaderResourceOut_;
        }
        
        /**
        Returns the count of tiles on X and Y axes. This can only be set on light-grid creation time.
        \see createGrid
        */
        inline const dim::size2di& getTileCount() const
        {
            return TileCount_;
        }
        /**
        Returns the grid size. This will be computed on light-grid creation time.
        It depends on the count of tiles on X and Y axes and the specified resolution.
        \see createGrid
        */
        inline const dim::size2di& getGridSize() const
        {
            return GridSize_;
        }

    private:
        
        /* === Functions === */
        
        bool createTLITexture();

        bool createTLIShaderResources();
        bool createTLIComputeShader();

        void buildOnGPU(scene::SceneGraph* Graph, scene::Camera* Cam);
        void buildOnCPU(scene::SceneGraph* Graph, scene::Camera* Cam);

        dim::size2di getGridSize(const dim::size2di &Resolution, const dim::size2di &TileCount) const;

        /* === Members === */
        
        //! This is a texture buffer storing the light indicies. Currently used for OpenGL.
        Texture* TLITexture_;
        
        //! This is a shader resource storing the light indicies. Currently used for Direct3D 11. OpenGL will follow.
        ShaderResource* TLIShaderResourceOut_;
        //! This is the shader resource filled by the compute shader. This is private only.
        ShaderResource* TLIShaderResourceIn_;
        
        ShaderResource* LGShaderResourceOut_;
        ShaderResource* LGShaderResourceIn_;

        //! Shader class for building the tile-light-index list buffer.
        ShaderClass* ShdClass_;

        dim::size2di TileCount_;
        dim::size2di GridSize_;

};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
