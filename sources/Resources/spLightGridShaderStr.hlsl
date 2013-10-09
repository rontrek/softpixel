"/*\n"
" * Light grid D3D11 compute shader file\n"
" * \n"
" * This file is part of the \"SoftPixel Engine\" (Copyright (c) 2008 by Lukas Hermanns)\n"
" * See \"SoftPixelEngine.hpp\" for license information.\n"
" */\n"
"/*\n"
" * Deferred shader header file\n"
" * \n"
" * This file is part of the \"SoftPixel Engine\" (Copyright (c) 2008 by Lukas Hermanns)\n"
" * See \"SoftPixelEngine.hpp\" for license information.\n"
" */\n"
"#ifndef MAX_LIGHTS\n"
"# define MAX_LIGHTS            35\n"
"#endif\n"
"#ifndef MAX_EX_LIGHTS\n"
"# define MAX_EX_LIGHTS         15\n"
"#endif\n"
"#ifndef NUM_JITTERD_OFFSETS\n"
"# define NUM_JITTERD_OFFSETS  20\n"
"#endif\n"
"#define LIGHT_DIRECTIONAL        0\n"
"#define LIGHT_POINT              1\n"
"#define LIGHT_SPOT               2\n"
"#define AMBIENT_LIGHT_FACTOR     0.0/*0.1*/ //!< Should be in the range [0.0 .. 1.0].\n"
"#define LIGHT_CUTOFF    0.01\n"
"#define MIN_VARIANCE             1.0\n"
"#define EOL       0xFFFFFFFF //!< Id for 'end-of-linked-list'\n"
"#define VPL_SINGULARITY_CLAMP  0.1\n"
"#define VPL_COUNT     100\n"
"#define SHININESS_FACTOR   90.0\n"
"#ifdef TILED_SHADING\n"
"# define TILED_LIGHT_GRID_WIDTH 32\n"
"# define TILED_LIGHT_GRID_HEIGHT 32\n"
"#endif\n"
"struct SLightNode\n"
"{\n"
" uint LightID; //!< SLight index.\n"
" uint Next;  //!< Next SLightNode index. 'EOL' if end of linked list.\n"
"};\n"
"struct SLight\n"
"{\n"
"    float4 PositionAndInvRadius;    //!< Position (xyz), Inverse Radius (w).\n"
"    float3 Color;                   //!< Light color (used for diffuse and specular).\n"
" float Pad0;\n"
"    int Type;                       //!< 0 -> Directional light, 1 -> Point light, 2 -> Spot light.\n"
"    int ShadowIndex;                //!< Shadow map layer index. -1 -> no shadow map.\n"
" int UsedForLightmaps;      //!< Specifies whether this light is used for lightmaps or not.\n"
" int ExID;      //!< Index to the associated 'SLightEx' element.\n"
"};\n"
"struct SLightEx\n"
"{\n"
"    float4x4 ViewProjection;    //!< Spot-/ directional view-projection matrix.\n"
" float4x4 InvViewProjection; //!< Inverse view-projection matrix.\n"
"    float3 Direction;           //!< Spot-/ directional light direction.\n"
" float Pad0;\n"
"    float SpotTheta;   //!< First spot cone angle (in radian).\n"
"    float SpotPhiMinusTheta; //!< Second minus first spot cone angle (in radian).\n"
" float Pad1[2];\n"
"};\n"
"#ifdef GLOBAL_ILLUMINATION\n"
"struct SVPL\n"
"{\n"
" float3 Position; //!< VPL position (in world space).\n"
" float3 Color;  //!< VPL RGB color.\n"
"};\n"
"#endif\n"
"/*\n"
" * ======= Compute shader: =======\n"
" */\n"
"#define PLANE_NORMAL(p)  p.xyz\n"
"#define PLANE_DISTANCE(p) p.w\n"
"#define SPHERE_POINT(s)  s.xyz\n"
"#define SPHERE_RADIUS(s) s.w\n"
"#define THREAD_GROUP_NUM_X 8\n"
"#define THREAD_GROUP_NUM_Y 8\n"
"#define THREAD_GROUP_SIZE (THREAD_GROUP_NUM_X * THREAD_GROUP_NUM_Y)\n"
"#define LIGHT_GRID_SIZE  float2(32.0, 32.0)\n"
"#define USE_DEPTH_EXTENT\n"
"#ifdef USE_DEPTH_EXTENT\n"
"# define DEPTH_EXTENT_NUM_X 4\n"
"# define DEPTH_EXTENT_NUM_Y 4\n"
"# define DEPTH_EXTENT_SIZE (DEPTH_EXTENT_NUM_X * DEPTH_EXTENT_NUM_Y)\n"
"#endif\n"
"struct SFrustum\n"
"{\n"
" float4 Left;\n"
" float4 Right;\n"
" float4 Top;\n"
" float4 Bottom;\n"
"};\n"
"/**\n"
"Plane -> float4 (xyz for normal and w for distance)\n"
"Sphere -> float4 (xyz for position and w for radius)\n"
"*/\n"
"cbuffer BufferMain : register(b0)\n"
"{\n"
" uint2 NumTiles   : packoffset(c0);\n"
" float2 InvNumTiles  : packoffset(c0.z);\n"
" float2 InvResolution : packoffset(c1);\n"
" float2 Pad0    : packoffset(c1.z);\n"
"};\n"
"cbuffer BufferFrame : register(b1)\n"
"{\n"
" float4x4 InvViewProjection : packoffset(c0); //!< Inverse view-projection matrix (without camera position).\n"
" float4x4 ViewMatrix   : packoffset(c4); //!< View matrix (view-space).\n"
" float4 NearPlane   : packoffset(c8);\n"
" float4 FarPlane    : packoffset(c9);\n"
" float3 ViewPosition   : packoffset(c10); //!< Camera position (world-space).\n"
" uint NumLights    : packoffset(c10.w);\n"
"};\n"
"#ifdef _DEB_USE_LIGHT_TEXBUFFER_\n"
"# ifdef USE_DEPTH_EXTENT\n"
"Buffer<float4> PointLightsPositionAndRadius : register(t1);\n"
"# else\n"
"Buffer<float4> PointLightsPositionAndRadius : register(t0);\n"
"# endif\n"
"#else\n"
"cbuffer BufferLight : register(b2)\n"
"{\n"
" float4 PointLightsPositionAndRadius[MAX_LIGHTS];\n"
"};\n"
"#endif\n"
"#ifdef USE_DEPTH_EXTENT\n"
"Texture2D<half4> DepthTexture : register(t0);\n"
"# ifdef _DEB_DEPTH_EXTENT_\n"
"RWBuffer<float2> _debDepthExt_In_ : register(u3);\n"
"# endif\n"
"groupshared uint ZMax;\n"
"groupshared uint ZMin;\n"
"#endif\n"
"#define _DEB_USE_GROUP_SHARED_\n"
"#ifdef _DEB_USE_GROUP_SHARED_\n"
"# define _DEB_USE_GROUP_SHARED_OPT_\n"
"groupshared uint LocalLightIdList[MAX_LIGHTS];\n"
"groupshared uint LocalLightCounter;\n"
"groupshared uint LightIndexStart;\n"
"#endif\n"
"RWBuffer<uint> LightGrid : register(u0);\n"
"#if defined(_DEB_USE_GROUP_SHARED_) && defined(_DEB_USE_GROUP_SHARED_OPT_)\n"
"RWBuffer<uint> GlobalLightIdList : register(u1);\n"
"#else\n"
"RWStructuredBuffer<SLightNode> GlobalLightIdList : register(u1);\n"
"#endif\n"
"#ifdef _DEB_USE_GROUP_SHARED_\n"
"RWBuffer<uint> GlobalLightCounter : register(u2);\n"
"#endif\n"
"/**\n"
"Normally one calculates a plane's normal with the following equation:\n"
"n = || (b - a) x (c - a) ||\n"
"In this case we already have the directions (b - a) and (c - a)\n"
"through the parameters U and V.\n"
"P is the origin point of the plane (a in the upper equation).\n"
"*/\n"
"void BuildPlane(out float4 Plane, float3 P, float3 U, float3 V)\n"
"{\n"
" PLANE_NORMAL(Plane) = normalize(cross(U, V));\n"
" PLANE_DISTANCE(Plane) = dot(PLANE_NORMAL(Plane), P);\n"
"}\n"
"void BuildFrustum(out SFrustum Frustum, float3 LT, float3 RT, float3 RB, float3 LB)\n"
"{\n"
" BuildPlane(Frustum.Left, ViewPosition, LB, LT);\n"
" BuildPlane(Frustum.Right, ViewPosition, RT, RB);\n"
" BuildPlane(Frustum.Top,  ViewPosition, LT, RT);\n"
" BuildPlane(Frustum.Bottom, ViewPosition, RB, LB);\n"
"}\n"
"float GetPlanePointDistance(float4 Plane, float3 Point)\n"
"{\n"
" return dot(PLANE_NORMAL(Plane), Point) - PLANE_DISTANCE(Plane);\n"
"}\n"
"#ifdef USE_DEPTH_EXTENT\n"
"bool CheckSphereFrustumIntersection(float4 Sphere, SFrustum Frustum, float Near, float Far)\n"
"{\n"
" float3 ViewSpherePos = mul(ViewMatrix, float4(SPHERE_POINT(Sphere), 1.0)).xyz;\n"
" float SphereDist = length(ViewSpherePos);\n"
" #if 1//!!!\n"
" float4 TempNearPlane = NearPlane;\n"
" float4 TempFarPlane = FarPlane;\n"
" PLANE_DISTANCE(TempNearPlane) -= Near;\n"
" PLANE_DISTANCE(TempFarPlane) = -PLANE_DISTANCE(NearPlane) + Far;\n"
" #endif\n"
" return\n"
"  #if 0\n"
"  GetPlanePointDistance(NearPlane,  SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  #elif 0\n"
"  SphereDist > Near - SPHERE_RADIUS(Sphere) &&\n"
"  SphereDist < Far + SPHERE_RADIUS(Sphere) &&\n"
"  #else\n"
"  GetPlanePointDistance(TempNearPlane, SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(TempFarPlane,  SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  #endif\n"
"  GetPlanePointDistance(Frustum.Left,  SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(Frustum.Right, SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(Frustum.Top,  SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(Frustum.Bottom, SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere);\n"
"}\n"
"#else\n"
"bool CheckSphereFrustumIntersection(float4 Sphere, SFrustum Frustum)\n"
"{\n"
" return\n"
"  GetPlanePointDistance(NearPlane,  SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(Frustum.Left,  SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(Frustum.Right, SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(Frustum.Top,  SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&\n"
"  GetPlanePointDistance(Frustum.Bottom, SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere);\n"
"}\n"
"#endif\n"
"#ifdef _DEB_USE_GROUP_SHARED_\n"
"void InsertLightIntoLocalList(uint LightID)\n"
"{\n"
" uint DestIndex = 0;\n"
" InterlockedAdd(LocalLightCounter, 1, DestIndex);\n"
" LocalLightIdList[DestIndex] = LightID;\n"
"}\n"
"#else\n"
"void InsertLightIntoTile(uint TileIndex, uint LightID)\n"
"{\n"
" SLightNode Link;\n"
" Link.LightID = LightID;\n"
" uint LinkID = GlobalLightIdList.IncrementCounter();\n"
" InterlockedExchange(\n"
"  LightGrid[TileIndex],\n"
"  LinkID,\n"
"  Link.Next\n"
" );\n"
" GlobalLightIdList[LinkID] = Link;\n"
"}\n"
"#endif\n"
"void VecFrustum(inout float2 v)\n"
"{\n"
" v = (v - 0.5) * 2.0;\n"
"}\n"
"float3 ProjectViewRay(float2 Pos)\n"
"{\n"
" float4 ViewRay = float4(Pos.x, 1.0 - Pos.y, 1.0, 1.0);\n"
" VecFrustum(ViewRay.xy);\n"
" ViewRay = mul(InvViewProjection, ViewRay);\n"
" return ViewRay.xyz;\n"
"}\n"
"#ifdef USE_DEPTH_EXTENT\n"
"float3 ProjectViewRay(uint2 PixelPos, float Depth)\n"
"{\n"
" float4 ViewRay = float4(\n"
"        ((float)PixelPos.x) * InvResolution.x,\n"
"  1.0 - ((float)PixelPos.y) * InvResolution.y,\n"
"  Depth,\n"
"  1.0\n"
" );\n"
" VecFrustum(ViewRay.xy);\n"
" ViewRay = mul(InvViewProjection, ViewRay);\n"
" return ViewRay.xyz;\n"
"}\n"
"void ComputeMinMaxExtents(uint2 PixelPos)\n"
"{\n"
" float PixelDepth = (float)DepthTexture[PixelPos].w;\n"
" #if 1//!!!TODO: optimize this!!!\n"
" float3 WorldPos = ViewPosition + normalize(ProjectViewRay(PixelPos, 1.0)) * (float3)PixelDepth;\n"
" PixelDepth = -GetPlanePointDistance(NearPlane, WorldPos);\n"
" #endif\n"
" uint Z = asuint(PixelDepth);\n"
" if (PixelDepth != 1.0)\n"
" {\n"
"  InterlockedMax(ZMax, Z);\n"
"  InterlockedMin(ZMin, Z);\n"
" }\n"
"}\n"
"#endif\n"
"[numthreads(THREAD_GROUP_NUM_X, THREAD_GROUP_NUM_Y, 1)]\n"
"void ComputeMain(\n"
" uint3 GroupId : SV_GroupID,\n"
" uint3 GlobalId : SV_DispatchThreadID,\n"
" uint LocalIndex : SV_GroupIndex)\n"
"{\n"
" #ifdef USE_DEPTH_EXTENT\n"
" if (LocalIndex == 0)\n"
" {\n"
"  ZMax = 0;\n"
"  ZMin = 0xFFFFFFFF;\n"
"  #ifdef _DEB_USE_GROUP_SHARED_\n"
"  LocalLightCounter = 0;\n"
"  #endif\n"
" }\n"
" GroupMemoryBarrierWithGroupSync();\n"
" [unroll]\n"
" for (uint k = 0; k < DEPTH_EXTENT_SIZE; ++k)\n"
" {\n"
"  ComputeMinMaxExtents(\n"
"   uint2(\n"
"    GlobalId.x * DEPTH_EXTENT_NUM_X + (k % DEPTH_EXTENT_NUM_X),\n"
"    GlobalId.y * DEPTH_EXTENT_NUM_Y + (k / DEPTH_EXTENT_NUM_X)\n"
"   )\n"
"  );\n"
" }\n"
" GroupMemoryBarrierWithGroupSync();\n"
" float Near = asfloat(ZMin);\n"
" float Far = asfloat(ZMax);\n"
" # if 0//???\n"
" Near = ProjectViewRay(GroupId.xy * uint2(32, 32), Near).z;\n"
" Far = ProjectViewRay(GroupId.xy * uint2(32, 32), Far).z;\n"
" # endif\n"
" #endif\n"
" float4 ViewArea = float4(\n"
"  (float)(GroupId.x) * InvNumTiles.x,\n"
"  (float)(GroupId.y) * InvNumTiles.y,\n"
"  (float)(GroupId.x + 1) * InvNumTiles.x,\n"
"  (float)(GroupId.y + 1) * InvNumTiles.y\n"
" );\n"
" float3 LT = ProjectViewRay(ViewArea.xy);\n"
" float3 RT = ProjectViewRay(ViewArea.zy);\n"
" float3 RB = ProjectViewRay(ViewArea.zw);\n"
" float3 LB = ProjectViewRay(ViewArea.xw);\n"
" SFrustum Frustum;\n"
" BuildFrustum(Frustum, LT, RT, RB, LB);\n"
" uint TileIndex = GroupId.y * NumTiles.x + GroupId.x;\n"
" # ifdef _DEB_DEPTH_EXTENT_\n"
" if (LocalIndex == 0)\n"
"  _debDepthExt_In_[TileIndex] = float2(Near, Far);\n"
" # endif\n"
" #if defined(_DEB_USE_GROUP_SHARED_) && !defined(USE_DEPTH_EXTENT)\n"
" if (LocalIndex == 0)\n"
"  LocalLightCounter = 0;\n"
" GroupMemoryBarrierWithGroupSync();\n"
" #endif\n"
" for (uint i = LocalIndex; i < NumLights; i += THREAD_GROUP_SIZE)\n"
" {\n"
"  #ifdef USE_DEPTH_EXTENT\n"
"  if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum, Near, Far))\n"
"  #else\n"
"  if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum))\n"
"  #endif\n"
"  {\n"
"   #ifdef _DEB_USE_GROUP_SHARED_\n"
"   InsertLightIntoLocalList(i);\n"
"   #else\n"
"   InsertLightIntoTile(TileIndex, i);\n"
"   #endif\n"
"  }\n"
" }\n"
" #ifdef _DEB_USE_GROUP_SHARED_\n"
" GroupMemoryBarrierWithGroupSync();\n"
" uint StartOffset = 0;\n"
" uint NumLights = LocalLightCounter;\n"
" if (LocalIndex == 0)\n"
" {\n"
"  InterlockedAdd(GlobalLightCounter[0], NumLights + 1, StartOffset);\n"
"  #ifdef _DEB_USE_GROUP_SHARED_OPT_\n"
"  LightGrid[TileIndex] = StartOffset;\n"
"  GlobalLightIdList[StartOffset + NumLights] = EOL;\n"
"  #else\n"
"  LightGrid[TileIndex] = (NumLights > 0 ? StartOffset : EOL);\n"
"  #endif\n"
"  LightIndexStart = StartOffset;\n"
" }\n"
" GroupMemoryBarrierWithGroupSync();\n"
" StartOffset = LightIndexStart;\n"
" for (uint j = LocalIndex; j < NumLights; j += THREAD_GROUP_SIZE)\n"
" {\n"
"  #ifndef _DEB_USE_GROUP_SHARED_OPT_\n"
"  SLightNode Link;\n"
"  Link.LightID = LocalLightIdList[j];\n"
"  Link.Next = (j + 1 < NumLights ? (StartOffset + j + 1) : EOL);\n"
"  GlobalLightIdList[StartOffset + j] = Link;\n"
"  #else\n"
"  GlobalLightIdList[StartOffset + j] = LocalLightIdList[j];\n"
"  #endif\n"
" }\n"
" #endif\n"
"}\n"
"[numthreads(1, 1, 1)]\n"
"void ComputeInitMain(uint3 Id : SV_GroupID)\n"
"{\n"
" LightGrid[Id.y * NumTiles.x + Id.x] = EOL;\n"
" #ifdef _DEB_USE_GROUP_SHARED_\n"
" uint GlobalHashId = Id.x + Id.y + Id.z;\n"
" if (GlobalHashId == 0)\n"
"  GlobalLightCounter[0] = 0;\n"
" #endif\n"
"}\n"
