/*
 * Collision graph header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_GRAPH_H__
#define __SP_COLLISION_GRAPH_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionSphere.hpp"
#include "SceneGraph/Collision/spCollisionCapsule.hpp"
#include "SceneGraph/Collision/spCollisionCylinder.hpp"
#include "SceneGraph/Collision/spCollisionCone.hpp"
#include "SceneGraph/Collision/spCollisionBox.hpp"
#include "SceneGraph/Collision/spCollisionPlane.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"
#include "SceneGraph/Collision/spCollisionMaterial.hpp"

#include <boost/function.hpp>


namespace sp
{
namespace scene
{


/**
This is the new collision scene graph use for real-time intersection tests, collision detection and resolving.
Here is a small code example on how to use the new collision system.
\code
// Define the collision contact callback to determine when a collision has been detected.
void CharCollCallback(CollisionMaterial* Material, CollisionNode* Node, CollisionNode* Rival)
{
    // do something with this information ...
}

// ...

// Create a collision material for the world and for the character.
CollisionMaterial* CollMatWorld = spCollGraph->createMaterial();
CollisionMaterial* CollMatChar = spCollGraph->createMaterial();

// Set the world material as rival for the character material.
// Now characters will collide with the world
CollMatChar->addRivalCollisionMaterial(CollMatWorld);

// Set the collision contact callback for the character material.
CollMatChar->setCollisionContactCallback(CharCollCallback);

// Create a world collision node (mesh).
CollisionMesh* WorldCollNode = spCollGraph->createMesh(CollMatWorld, WorldMesh);

// Create a character collision node (capsule).
CollisionCapsule* CharCollNode = spCollGraph->createCapsule(CollMatChar, CharSceneNode, CharRadius, CharHeight);

// ...

// Update the scene for collisions.
spCollGraph->updateScene();
\endcode
\since Version 3.2
\ingroup group_collision
*/
class SP_EXPORT CollisionGraph
{
    
    public:
        
        CollisionGraph();
        virtual ~CollisionGraph();
        
        /* === Functions === */
        
        //! Creates a new collision material and returns a pointer to the new CollisionMaterial object.
        virtual CollisionMaterial* createMaterial();
        //! Deletes the specified collision material.
        virtual void deleteMaterial(CollisionMaterial* Material);
        
        //! Adds the specified collision node into the collision graph. This is similiar to the "addSceneNode" function of the SceneGraph class.
        virtual void addCollisionNode(CollisionNode* Node);
        //! Removes the specified collision node from the collision graph.
        virtual void removeCollisionNode(CollisionNode* Node);
        
        /**
        Creates a new collision sphere.
        \param Material: Specifies the collision material. This may also be 0.
        \param Node: Specifies the scene node object. This must not be 0!
        \param Radius: Specifies the sphere's radius.
        \return Pointer to the new CollisionSphere object.
        */
        virtual CollisionSphere* createSphere(CollisionMaterial* Material, scene::SceneNode* Node, f32 Radius);
        /**
        Creates a new collision capsule.
        \param Height: Specifies the capsule's height. This can not be smaller
        than 0 and the whole capsule's height is (Height + 2 * Radius).
        \return Pointer to the new CollisionCapsule object.
        \see createSphere
        */
        virtual CollisionCapsule* createCapsule(CollisionMaterial* Material, scene::SceneNode* Node, f32 Radius, f32 Height);
        //...
        virtual CollisionCylinder* createCylinder(CollisionMaterial* Material, scene::SceneNode* Node, f32 Radius, f32 Height);
        //...
        virtual CollisionCone* createCone(CollisionMaterial* Material, scene::SceneNode* Node, f32 Radius, f32 Height);
        /**
        Creates a new collision box.
        \param Box: Specifies the box size.
        \return Pointer to the new CollisionBox object.
        \see createSphere
        */
        virtual CollisionBox* createBox(CollisionMaterial* Material, scene::SceneNode* Node, const dim::aabbox3df &Box);
        /**
        Creates a new collision plane.
        \param Plane: Specifies the plane.
        \return Pointer to the new CollisionPlane object.
        \see createSphere
        */
        virtual CollisionPlane* createPlane(CollisionMaterial* Material, scene::SceneNode* Node, const dim::plane3df &Plane);
        /**
        Creates a new collision mesh.
        \param Mesh: Specifies the mesh which is to be used for the collision.
        \param MaxTreeLevel: Specifies the maximal tree hierarchy level.
        \return Pointer to the new CollisionMesh object.
        \see createSphere
        */
        virtual CollisionMesh* createMesh(CollisionMaterial* Material, scene::Mesh* Mesh, u8 MaxTreeLevel = DEF_KDTREE_LEVEL);
        /**
        Creates a new collision mesh out of several meshes.
        \see createMesh
        */
        virtual CollisionMesh* createMeshList(CollisionMaterial* Material, const std::list<Mesh*> &MeshList, u8 MaxTreeLevel = DEF_KDTREE_LEVEL);
        
        //! Deletes the given collision node and returns true on succeed.
        virtual bool deleteNode(CollisionNode* Node);
        
        //! Clears the whole scene from the specified objects.
        virtual void clearScene(bool isDeleteNodes = true, bool isDeleteMaterials = true);
        
        /**
        Makes an intersection test with the whole collision graph.
        \param Line: Specifies the line which is to be tested for intersection.
        \return True if any intersection has been detected.
        \note In most cases this is much faster than getting a list of all intersections. So if you only
        need to know if any intersection has been detected, use this function instead of "findIntersections".
        \see findIntersections
        */
        virtual bool checkIntersection(const dim::line3df &Line) const;
        
        /**
        Makes intersection tests with the whole collision graph.
        \param Line: Specifies the line which is to be tested for intersection.
        \param ContactList: Specifies the list where the intersection result are to be stored.
        \param SearchBidirectional: Specifies whether the search will be performed
        bidirectional (two times) or not. By default false.
        */
        virtual void findIntersections(
            const dim::line3df &Line, std::list<SIntersectionContact> &ContactList, bool SearchBidirectional = false
        ) const;
        
        //! Performs all collision resolving for the whole collision graph.
        virtual void updateScene();
        
        /* === Static functions === */
        
        static void sortContactList(
            const dim::vector3df &LineStart, std::list<SIntersectionContact> &ContactList
        );
        
        /* === Inline functions === */
        
        inline const std::list<CollisionNode*>& getNodeList() const
        {
            return CollNodes_;
        }
        inline const std::list<CollisionMaterial*>& getMaterialList() const
        {
            return CollMaterials_;
        }
        
        //! Returns pointer to the root tree node.
        inline TreeNode* getRootTreeNode() const
        {
            return RootTreeNode_;
        }
        
        inline std::list<SIntersectionContact> findIntersections(const dim::line3df &Line) const
        {
            std::list<SIntersectionContact> ContactList;
            findIntersections(Line, ContactList);
            return ContactList;
        }
        
    protected:
        
        /* === Functions === */
        
        virtual void findIntersectionsUnidirectional(
            const dim::line3df &Line, std::list<SIntersectionContact> &ContactList
        ) const;
        
        /* === Templates === */
        
        template <class T> T* addCollNode(T* Node)
        {
            CollNodes_.push_back(Node);
            return Node;
        }
        
        /* === Members === */
        
        std::list<CollisionNode*> CollNodes_;
        std::list<CollisionMaterial*> CollMaterials_;
        
        TreeNode* RootTreeNode_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
