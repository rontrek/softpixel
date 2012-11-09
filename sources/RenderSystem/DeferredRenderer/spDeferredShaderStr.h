"/*\n"
" * Deferred Cg shader file\n"
" * \n"
" * This file is part of the \"SoftPixel Engine\" (Copyright (c) 2008 by Lukas Hermanns)\n"
" * See \"SoftPixelEngine.hpp\" for license information.\n"
" */\n"
"\n"
"/*\n"
"\n"
"Compilation options:\n"
"\n"
"SHADOW_MAPPING  -> Enables shadow mapping.\n"
"BLOOM_FILTER    -> Enables bloom filter.\n"
"DEBUG_GBUFFER   -> Renders g-buffer for debugging.\n"
"FLIP_Y_AXIS     -> Flips Y axis for OpenGL FBOs.\n"
"\n"
"*/\n"
"\n"
"#define MAX_LIGHTS              35\n"
"#define MAX_EX_LIGHTS           15\n"
"\n"
"#define LIGHT_DIRECTIONAL       0\n"
"#define LIGHT_POINT             1\n"
"#define LIGHT_SPOT              2\n"
"\n"
"#define AMBIENT_LIGHT_FACTOR    0.1 //!< Should be in the range [0.0 .. 1.0].\n"
"\n"
"#define MIN_VARIANCE            0.001\n"
"\n"
"\n"
"/*\n"
" * ======= Vertex shader: =======\n"
" */\n"
"\n"
"/* === Structures === */\n"
"\n"
"struct SVertexInput\n"
"{\n"
"    float3 Position : POSITION;\n"
"    float2 TexCoord : TEXCOORD0;\n"
"};\n"
"\n"
"struct SVertexOutput\n"
"{\n"
"    float4 Position : POSITION;\n"
"    float2 TexCoord : TEXCOORD0;\n"
"};\n"
"\n"
"\n"
"/* === Uniforms === */\n"
"\n"
"uniform float4x4 ProjectionMatrix;\n"
"\n"
"\n"
"/* === Functions === */\n"
"\n"
"SVertexOutput VertexMain(SVertexInput In)\n"
"{\n"
"    SVertexOutput Out = (SVertexOutput)0;\n"
"    \n"
"    /* Process vertex transformation for position and normal */\n"
"    Out.Position = mul(ProjectionMatrix, float4(In.Position, 1));\n"
"    Out.TexCoord = In.TexCoord;\n"
"    \n"
"    return Out;\n"
"}\n"
"\n"
"\n"
"/*\n"
" * ======= Pixel shader: =======\n"
" */\n"
"\n"
"/* === Structures === */\n"
"\n"
"struct SPixelInput\n"
"{\n"
"    float2 TexCoord : TEXCOORD0;\n"
"    float2 WinCoord : WPOS;\n"
"};\n"
"\n"
"struct SPixelOutput\n"
"{\n"
"    float4 Color : COLOR0;\n"
"    #ifdef BLOOM_FILTER\n"
"    float4 Gloss : COLOR1;\n"
"    #endif\n"
"};\n"
"\n"
"struct SLight\n"
"{\n"
"    float4 PositionAndRadius;   //!< Position (xyz), Radius (w).\n"
"    float3 Color;               //!< Light color (used for diffuse and specular).\n"
"    int Type;                   //!< 0 -> Directional light, 1 -> Point light, 1 -> Spot light.\n"
"    int ShadowIndex;            //!< Shadow map layer index.\n"
"};\n"
"\n"
"struct SLightEx\n"
"{\n"
"    float4x4 Projection;        //!< Spot-/ directional projection matrix.\n"
"    float3 Direction;           //!< Spot-/ directional light direction.\n"
"    float SpotTheta;            //!< First spot cone angle (in radian).\n"
"    float SpotPhiMinusTheta;    //!< Second minus first spot cone angle (in radian).\n"
"};\n"
"\n"
"\n"
"/* === Uniforms === */\n"
"\n"
"uniform sampler2D DiffuseAndSpecularMap         : TEXUNIT0;\n"
"uniform sampler2D NormalAndDepthMap             : TEXUNIT1;\n"
"\n"
"#ifdef SHADOW_MAPPING\n"
"uniform sampler2DARRAY DirLightShadowMaps       : TEXUNIT2;\n"
"uniform samplerCUBEARRAY PointLightShadowMaps   : TEXUNIT3;\n"
"#endif\n"
"\n"
"uniform int LightCount;\n"
"uniform int LightExCount;\n"
"\n"
"uniform SLight Lights[MAX_LIGHTS];\n"
"uniform SLightEx LightsEx[MAX_EX_LIGHTS];\n"
"\n"
"uniform float4x4 ViewTransform; //!< Global camera transformation.\n"
"uniform float3 ViewPosition;    //!< Global camera position.\n"
"uniform float ScreenWidth;      //!< Screen resolution width.\n"
"uniform float ScreenHeight;     //!< Screen resolution height.\n"
"\n"
"\n"
"/* === Functions === */\n"
"\n"
"inline float GetAngle(float3 a, float3 b)\n"
"{\n"
"    return acos(dot(a, b));\n"
"}\n"
"\n"
"void Frustum(inout float x, inout float y, float w, float h)\n"
"{\n"
"    float aspect = (w*3.0) / (h*4.0);\n"
"    x = (x - w*0.5) / (w*0.5) * aspect;\n"
"    y = (y - h*0.5) / (w*0.5) * aspect;\n"
"}\n"
"\n"
"#ifdef SHADOW_MAPPING\n"
"\n"
"// Chebyshev inequality function for VSM (variance shadow maps)\n"
"// see GPUGems3 at nVIDIA for more details: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html\n"
"float ChebyshevUpperBound(float2 Moments, float t)\n"
"{\n"
"    /* One-tailed inequality valid if t > Moments.x */\n"
"    float p = (t <= Moments.x);\n"
"    \n"
"    /* Compute variance */\n"
"    float Variance = Moments.y - (Moments.x*Moments.x);\n"
"    Variance = max(Variance, MIN_VARIANCE);\n"
"    \n"
"    /* Compute probabilistic upper bound. */\n"
"    float d = t - Moments.x;\n"
"    float p_max = Variance / (Variance + d*d);\n"
"    \n"
"    return max(p, p_max);\n"
"}\n"
"\n"
"inline float LinStep(float min, float max, float v)\n"
"{\n"
"    return clamp((v - min) / (max - min), 0, 1);\n"
"}\n"
"\n"
"float ReduceLightBleeding(float p_max, float Amount)\n"
"{\n"
"    /* remove the [0, amount] ail and linearly rescale [amount, 1] */\n"
"    return LinStep(Amount, 1.0, p_max);\n"
"}\n"
"\n"
"float ShadowContribution(float2 Moments, float LightDistance)\n"
"{\n"
"    /* Compute the Chebyshev upper bound */\n"
"    float p_max = ChebyshevUpperBound(Moments, LightDistance);\n"
"    return ReduceLightBleeding(p_max, 0.6);\n"
"}\n"
"\n"
"float4 Projection(float4x4 ProjectionMatrix, float4 Point)\n"
"{\n"
"    float4 ProjectedPoint = mul(ProjectionMatrix, Point);\n"
"\n"
"    ProjectedPoint.xy = (ProjectedPoint.xy / float2(ProjectedPoint.w) + float2(1.0)) * float2(0.5);\n"
"\n"
"    return ProjectedPoint;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"void ComputeLightShading(\n"
"    SLight Light, SLightEx LightEx,\n"
"    float3 Point, float3 Normal, float Shininess,\n"
"    inout vec3 DiffuseColor, inout vec3 SpecularColor)\n"
"{\n"
"    /* Compute light direction vector */\n"
"    float3 LightDir = (float3)0;\n"
"\n"
"    if (Light.Type != LIGHT_DIRECTIONAL)\n"
"        LightDir = normalize(Point - Light.PositionAndRadius.xyz);\n"
"    else\n"
"        LightDir = LightEx.Direction;\n"
"\n"
"    /* Compute phong shading */\n"
"    float NdotL = max(AMBIENT_LIGHT_FACTOR, -dot(Normal, LightDir));\n"
"\n"
"    /* Compute light attenuation */\n"
"    float Distance = distance(Point, Light.PositionAndRadius.xyz);\n"
"\n"
"    float AttnLinear    = Distance / Light.PositionAndRadius.w;\n"
"    float AttnQuadratic = AttnLinear * Distance;\n"
"\n"
"    float Intensity = 1.0 / (1.0 + AttnLinear + AttnQuadratic);\n"
"\n"
"    if (Light.Type == LIGHT_SPOT)\n"
"    {\n"
"        /* Compute spot light cone */\n"
"        float Angle = GetAngle(LightDir, LightEx.Direction);\n"
"        float ConeAngleLerp = (Angle - LightEx.SpotTheta) / LightEx.SpotPhiMinusTheta;\n"
"\n"
"        Intensity *= saturate(1.0 - ConeAngleLerp);\n"
"    }\n"
"\n"
"    /* Compute diffuse color */\n"
"    float3 Diffuse = Light.Color * float3(Intensity * NdotL);\n"
"\n"
"    /* Compute specular color */\n"
"    float3 ViewDir      = normalize(Point - ViewPosition);\n"
"    float3 Reflection   = normalize(reflect(LightDir, Normal));\n"
"\n"
"    float NdotHV = -dot(ViewDir, Reflection);\n"
"\n"
"    float3 Specular = Light.Color * float3(Intensity * pow(max(0.0, NdotHV), Shininess));\n"
"\n"
"    #ifdef SHADOW_MAPPING\n"
"\n"
"    /* Apply shadow */\n"
"    if (Light.ShadowIndex != -1)\n"
"    {\n"
"        if (Light.Type == LIGHT_POINT)\n"
"        {\n"
"            //todo\n"
"        }\n"
"        else if (Light.Type == LIGHT_SPOT)\n"
"        {\n"
"            /* Get shadow map texture coordinate */\n"
"            float4 ShadowTexCoord = Projection(LightEx.Projection, float4(Point, 1.0));\n"
"\n"
"            if ( ShadowTexCoord.x >= 0.0 && ShadowTexCoord.x <= 1.0 &&\n"
"                 ShadowTexCoord.y >= 0.0 && ShadowTexCoord.y <= 1.0 &&\n"
"                 ShadowTexCoord.z < 0.0 )\n"
"            {\n"
"                /* Adjust texture coordinate */\n"
"                ShadowTexCoord.x = 1.0 - ShadowTexCoord.x;\n"
"                ShadowTexCoord.z = float(Light.ShadowIndex);\n"
"                ShadowTexCoord.w = 2.0;\n"
"\n"
"                /* Sample moments from shadow map */\n"
"                float2 Moments = tex2DARRAYlod(DirLightShadowMaps, ShadowTexCoord).ra;\n"
"\n"
"                #if 0 //!!!\n"
"                DiffuseColor = float3(Moments.y*0.01);\n"
"                return;\n"
"                #endif\n"
"\n"
"                /* Compute shadow contribution */\n"
"                float Shadow = ShadowContribution(Moments, Distance);\n"
"\n"
"                Diffuse *= float4(Shadow);\n"
"                Specular *= float4(Shadow);\n"
"            }\n"
"        }\n"
"    }\n"
"\n"
"    #endif\n"
"\n"
"    /* Add light color */\n"
"    DiffuseColor += Diffuse;\n"
"    SpecularColor += Specular;\n"
"}\n"
"\n"
"SPixelOutput PixelMain(SPixelInput In)\n"
"{\n"
"    SPixelOutput Out = (SPixelOutput)0;\n"
"\n"
"    float2 TexCoord = In.TexCoord;\n"
"    float2 WinCoord = In.WinCoord;\n"
"    \n"
"    #ifdef DEBUG_GBUFFER\n"
"\n"
"    TexCoord *= float2(2.0);\n"
"    WinCoord *= float2(2.0);\n"
"\n"
"    float2 debTexCoord = TexCoord;\n"
"\n"
"    if (TexCoord.x > 1.0) TexCoord.x -= 1.0;\n"
"    if (TexCoord.y > 1.0) TexCoord.y -= 1.0;\n"
"\n"
"    if (WinCoord.x > ScreenWidth) WinCoord.x -= ScreenWidth;\n"
"    if (WinCoord.y > ScreenHeight) WinCoord.y -= ScreenHeight;\n"
"\n"
"    #endif\n"
"    \n"
"    /* Get texture colors */\n"
"    float4 DiffuseAndSpecular = tex2D(DiffuseAndSpecularMap, TexCoord);\n"
"    float4 NormalAndDepthDist = tex2D(NormalAndDepthMap, TexCoord);\n"
"\n"
"    /* Compute global pixel position */\n"
"    #ifdef FLIP_Y_AXIS\n"
"    float4 Point = float4(WinCoord.x, ScreenHeight - WinCoord.y, 1.0, 1.0);\n"
"    #else\n"
"    float4 Point = float4(WinCoord.x, WinCoord.y, 1.0, 1.0);\n"
"    #endif\n"
"\n"
"    Frustum(Point.x, Point.y, ScreenWidth, ScreenHeight);\n"
"\n"
"    Point.xyz = normalize(Point.xyz) * float3(NormalAndDepthDist.a);\n"
"    Point = mul(ViewTransform, Point);\n"
"\n"
"    /* Compute light shading */\n"
"    float3 DiffuseLight = (float3)0;\n"
"    float3 SpecularLight = (float3)0;\n"
"    \n"
"    //for (int i = 0, j = 0; i < LightCount; ++i)\n"
"    int i = 0, j = 0;\n"
"    \n"
"    while (i < LightCount)\n"
"    {\n"
"        ComputeLightShading(Lights[i], LightsEx[j], Point.xyz, NormalAndDepthDist.xyz, 90.0, DiffuseLight, SpecularLight);\n"
"        \n"
"        if (Lights[i].Type != LIGHT_POINT)\n"
"            ++j;\n"
"        \n"
"        ++i;\n"
"    }\n"
"\n"
"    DiffuseLight *= DiffuseAndSpecular.rgb;\n"
"    SpecularLight *= float3(DiffuseAndSpecular.a);\n"
"\n"
"    /* Compute final deferred shaded pixel color */\n"
"    Out.Color.rgb   = DiffuseLight + SpecularLight;\n"
"    Out.Color.a     = 1.0;\n"
"\n"
"    #ifdef BLOOM_FILTER\n"
"    Out.Gloss.rgb   = SpecularLight;\n"
"    Out.Gloss.a     = 1.0;\n"
"    #endif\n"
"    \n"
"    #ifdef DEBUG_GBUFFER\n"
"\n"
"    if (debTexCoord.x > 1.0)\n"
"    {\n"
"        if (debTexCoord.y > 1.0)\n"
"            Out.Color.rgb = tex2D(NormalAndDepthMap, TexCoord).rgb * float3(0.5) + float3(0.5);\n"
"        else\n"
"            Out.Color.rgb = DiffuseAndSpecular.rgb;\n"
"    }\n"
"    else\n"
"    {\n"
"        if (debTexCoord.y > 1.0)\n"
"            Out.Color.rgb = Point.xyz * float3(0.1);\n"
"    }\n"
"\n"
"    #endif\n"
"\n"
"    return Out;\n"
"}\n"
