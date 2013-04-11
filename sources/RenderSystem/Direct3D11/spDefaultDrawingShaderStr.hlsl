"/*\n"
" * D3D11 default drawing shader file\n"
" * \n"
" * This file is part of the \"SoftPixel Engine\" (Copyright (c) 2008 by Lukas Hermanns)\n"
" * See \"SoftPixelEngine.hpp\" for license information.\n"
" */\n"
"\n"
"Texture2D Texture : register(t0);\n"
"SamplerState Sampler : register(s0);\n"
"\n"
"cbuffer BufferMain : register(b0)\n"
"{\n"
"    float4x4 ProjectionMatrix;\n"
"    float4 Color;\n"
"    int UseTexture;\n"
"};\n"
"\n"
"cbuffer BufferMapping : register(b1)\n"
"{\n"
"    float2 Position;\n"
"    float2 TexPosition;\n"
"    float4x4 WorldMatrix;\n"
"    float4x4 TextureMatrix;\n"
"};\n"
"\n"
"struct SVertexInput\n"
"{\n"
"    float2 Position : POSITION;\n"
"    float2 TexCoord : TEXCOORD0;\n"
"};\n"
"\n"
"struct SVertexOutput\n"
"{\n"
"    float4 Position : SV_Position;\n"
"    float2 TexCoord : TEXCOORD0;\n"
"};\n"
"\n"
"SVertexOutput VertexMain(SVertexInput In)\n"
"{\n"
"    SVertexOutput Out = (SVertexOutput)0;\n"
"    \n"
"    // Process vertex coordinate\n"
"    float2 Coord = Position + mul((float2x2)WorldMatrix, In.Position);\n"
"\n"
"    Out.Position = mul(ProjectionMatrix, float4(Coord.x, Coord.y, 0.0, 1.0));\n"
"\n"
"    // Process texture coordinate\n"
"    Out.TexCoord = TexPosition + mul((float2x2)TextureMatrix, In.TexCoord);\n"
"    \n"
"    return Out;\n"
"}\n"
"\n"
"float4 PixelMain(SVertexOutput In) : SV_Target0\n"
"{\n"
"    return UseTexture != 0 ? Texture.Sample(Sampler, In.TexCoord) * Color : Color;\n"
"}\n"
"\n"
