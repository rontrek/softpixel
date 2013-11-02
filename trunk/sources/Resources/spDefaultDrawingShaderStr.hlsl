"/*\n"
" * D3D11 default drawing shader file\n"
" * \n"
" * This file is part of the \"SoftPixel Engine\" (Copyright (c) 2008 by Lukas Hermanns)\n"
" * See \"SoftPixelEngine.hpp\" for license information.\n"
" */\n"
"/*\n"
" * HLSL (SM 5) shader core file\n"
" * \n"
" * This file is part of the \"SoftPixel Engine\" (Copyright (c) 2008 by Lukas Hermanns)\n"
" * See \"SoftPixelEngine.hpp\" for license information.\n"
" */\n"
"#define SP_HLSL 5\n"
"#define PI      3.14159265359\n"
"#define E      2.71828182846\n"
"#define MUL(m, v)    mul(m, v)\n"
"#define MUL_TRANSPOSED(v, m) mul(v, m)\n"
"#define MUL_NORMAL(n)   (n).xyz = mul((n).xyz, float3x3(Tangent, Binormal, Normal))\n"
"#define CAST(t, v)    ((t)(v))\n"
"#define SAMPLER2D(n, i)   Texture2D n : register(t##i); SamplerState Sampler##n : register(s##i)\n"
"#define SAMPLER2DARRAY(n, i) Texture2DArray n : register(t##i); SamplerState Sampler##n : register(s##i)\n"
"#define SAMPLERCUBEARRAY(n, i) TextureCubeArray n : register(t##i); SamplerState Sampler##n : register(s##i)\n"
"#define DeclSampler2D   SAMPLER2D\n"
"#define DeclSampler2DArray  SAMPLER2DARRAY\n"
"#define DeclSamplerCubeMap  SAMPLERCUBEARRAY\n"
"#define mod(a, b)    fmod(a, b)\n"
"#define floatBitsToInt(v)  asint(v)\n"
"#define floatBitsToUInt(v)  asuint(v)\n"
"#define intBitsToFloat(v)  asfloat(v)\n"
"#define uintBitsToFloat(v)  asfloat(v)\n"
"#define tex2D(s, t)    s.Sample(Sampler##s, t)\n"
"#define tex2DArray(s, t)  s.Sample(Sampler##s, t)\n"
"#define tex2DArrayLod(s, t)  s.SampleLevel(Sampler##s, t.xyz, t.w)\n"
"#define tex2DGrad(s, t, dx, dy) s.SampleGrad(Sampler##s, t, dx, dy)\n"
"#define RWTexture3DUInt   RWTexture3D<uint>\n"
"#define DeclStructuredBuffer(s, n, r) StructuredBuffer<s> n : register(t##r)\n"
"#define DeclBuffer(t, n, r)    Buffer<t> n : register(t##r)\n"
"#define DeclRWStructuredBuffer(s, n, r) RWStructuredBuffer<s> n : register(u##r)\n"
"#define DeclRWBuffer(t, n, r)   RWBuffer<t> n : register(u##r)\n"
"#define DeclConstBuffer(n, r)   cbuffer n : register(b##r)\n"
"struct SFullscreenQuadVertexOutput\n"
"{\n"
"    float4 Position : SV_Position;\n"
"    float2 TexCoord : TEXCOORD0;\n"
"};\n"
"SFullscreenQuadVertexOutput FullscreenQuadVertexMain(uint Id)\n"
"{\n"
"    SFullscreenQuadVertexOutput Out = (SFullscreenQuadVertexOutput)0;\n"
" Out.Position = float4(\n"
"  (Id == 2) ? 3.0 : -1.0,\n"
"  (Id == 0) ? -3.0 : 1.0,\n"
"  0.0,\n"
"  1.0\n"
" );\n"
" Out.TexCoord.x = Out.Position.x * 0.5 + 0.5;\n"
" Out.TexCoord.y = 0.5 - Out.Position.y * 0.5;\n"
" return Out;\n"
"}\n"
"inline void InterlockedImageCompareExchange(RWTexture3DUInt Image, int3 Coord, uint Compare, uint Value, out uint Result)\n"
"{\n"
" InterlockedCompareExchange(Image[Coord], Compare, Value, Result);\n"
"}\n"
"DeclSampler2D(ColorMap, 0);\n"
"cbuffer BufferVS : register(b0)\n"
"{\n"
" float4x4 ProjectionMatrix;  //!< Projection matrix.\n"
" float4x4 WorldMatrix;       //!< Image transformation matrix.\n"
" float4 TextureTransform;    //!< Texture offset (XY), Texture scaling (ZW).\n"
" float4 Position;            //!< Image origin (XY), Image offset (ZW).\n"
"}\n"
"cbuffer BufferPS : register(b0)\n"
"{\n"
" float4 Color;\n"
" int UseTexture;\n"
"}\n"
"struct SVertexInput\n"
"{\n"
"    float2 Position : POSITION;\n"
"    float2 TexCoord : TEXCOORD0;\n"
"};\n"
"struct SVertexOutput\n"
"{\n"
"    float4 Position : SV_Position;\n"
"    float2 TexCoord : TEXCOORD0;\n"
"};\n"
"SVertexOutput VertexMain(SVertexInput In)\n"
"{\n"
"    SVertexOutput Out = (SVertexOutput)0;\n"
"    // Process vertex coordinate\n"
"    float2 Coord = Position.xy + mul(WorldMatrix, float4(Position.zw + In.Position, 0.0, 1.0)).xy;\n"
"    Out.Position = mul(ProjectionMatrix, float4(Coord.x, Coord.y, 0.0, 1.0));\n"
"    // Process texture coordinate\n"
"    Out.TexCoord = TextureTransform.xy + In.TexCoord * TextureTransform.zw;\n"
"    return Out;\n"
"}\n"
"float4 PixelMain(SVertexOutput In) : SV_Target0\n"
"{\n"
"    return UseTexture != 0 ? tex2D(ColorMap, In.TexCoord) * Color : Color;\n"
"}\n"
