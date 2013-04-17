"/*\n"
" * Deferred shader procedures file\n"
" * \n"
" * This file is part of the \"SoftPixel Engine\" (Copyright (c) 2008 by Lukas Hermanns)\n"
" * See \"SoftPixelEngine.hpp\" for license information.\n"
" */\n"
"\n"
"float GetAngle(in float3 a, in float3 b)\n"
"{\n"
"    return acos(dot(a, b));\n"
"}\n"
"\n"
"float GetSpotLightIntensity(in float3 LightDir, in SLightEx LightEx)\n"
"{\n"
"    /* Compute spot light cone */\n"
"    float Angle = GetAngle(LightDir, LightEx.Direction);\n"
"    float ConeAngleLerp = (Angle - LightEx.SpotTheta) / LightEx.SpotPhiMinusTheta;\n"
"    \n"
"    return saturate(1.0 - ConeAngleLerp);\n"
"}\n"
"\n"
"#ifdef SHADOW_MAPPING\n"
"\n"
"/**\n"
"Chebyshev inequality function for VSM (variance shadow maps)\n"
"see GPUGems3 at nVIDIA for more details: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html\n"
"*/\n"
"float ChebyshevUpperBound(in float2 Moments, in float t)\n"
"{\n"
"    /* One-tailed inequality valid if t > Moments.x */\n"
"    float p = step(t, Moments.x);\n"
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
"float LinStep(in float min, in float max, in float v)\n"
"{\n"
"    return saturate((v - min) / (max - min));\n"
"}\n"
"\n"
"float ReduceLightBleeding(in float p_max, in float Amount)\n"
"{\n"
"    /* remove the [0, amount] ail and linearly rescale [amount, 1] */\n"
"    return LinStep(Amount, 1.0, p_max);\n"
"}\n"
"\n"
"float ShadowContribution(in float2 Moments, in float LightDistance)\n"
"{\n"
"    /* Compute the Chebyshev upper bound */\n"
"    float p_max = ChebyshevUpperBound(Moments, LightDistance);\n"
"    return ReduceLightBleeding(p_max, 0.6);\n"
"}\n"
"\n"
"//! World position projection function.\n"
"float4 Projection(in float4x4 ProjectionMatrix, in float4 WorldPos)\n"
"{\n"
"    float4 ProjectedPoint = ProjectionMatrix * WorldPos;\n"
"\n"
"    ProjectedPoint.xy = (ProjectedPoint.xy / float2(ProjectedPoint.w) + float2(1.0)) * float2(0.5);\n"
"\n"
"    return ProjectedPoint;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"#ifdef SHADOW_MAPPING\n"
"\n"
"#    ifdef GLOBAL_ILLUMINATION\n"
"\n"
"//! Virtual point light shading function.\n"
"bool ComputeVPLShading(in float3 WorldPos, in float3 Normal, in float3 IndirectPoint, inout float IntensityIL)\n"
"{\n"
"    /* Check if VPL is visible to pixel */\n"
"    float3 IndirectDir = IndirectPoint - WorldPos;\n"
"    \n"
"    if (dot(Normal, IndirectDir) <= 0.0)\n"
"        return false;\n"
"    \n"
"    /* Compute light attenuation */\n"
"    float DistanceIL = distance(WorldPos, IndirectPoint);\n"
"    \n"
"    float AttnLinearIL    = DistanceIL;// ... * VPLRadius;\n"
"    float AttnQuadraticIL = AttnLinearIL * DistanceIL;\n"
"    \n"
"    IntensityIL = saturate(1.0 / (1.0 + AttnLinearIL + AttnQuadraticIL));// - LIGHT_CUTOFF);\n"
"    \n"
"    /* Compute phong shading for indirect light */\n"
"    float NdotIL = saturate(dot(Normal, normalize(IndirectDir)));\n"
"    \n"
"    /* Clamp intensity to avoid singularities in VPLs */\n"
"    IntensityIL = min(VPL_SINGULARITY_CLAMP, IntensityIL * NdotIL) * GIReflectivity;\n"
"    \n"
"    return true;\n"
"}\n"
"\n"
"#    endif\n"
"\n"
"//! Light shadowing and global illumination function.\n"
"void ComputeLightShadow(\n"
"    in SLight Light, in SLightEx LightEx,\n"
"    in float3 WorldPos, in float3 Normal, in float Distance,\n"
"    inout float3 Diffuse, inout float3 Specular)\n"
"{\n"
"    if (Light.Type == LIGHT_POINT)\n"
"    {\n"
"        //todo\n"
"    }\n"
"    else if (Light.Type == LIGHT_SPOT)\n"
"    {\n"
"        /* Get shadow map texture coordinate */\n"
"        float4 ShadowTexCoord = Projection(LightEx.ViewProjection, float4(WorldPos, 1.0));\n"
"        \n"
"        if ( ShadowTexCoord.x >= 0.0 && ShadowTexCoord.x <= 1.0 &&\n"
"             ShadowTexCoord.y >= 0.0 && ShadowTexCoord.y <= 1.0 &&\n"
"             ShadowTexCoord.z > 0.0 )\n"
"        {\n"
"            /* Adjust texture coordinate */\n"
"            ShadowTexCoord.y = 1.0 - ShadowTexCoord.y;\n"
"            ShadowTexCoord.z = float(Light.ShadowIndex);\n"
"            ShadowTexCoord.w = 2.0;//Distance*0.25;\n"
"            \n"
"            /* Sample moments from shadow map */\n"
"            float2 Moments = tex2DArrayLod(DirLightShadowMaps, ShadowTexCoord).ra;\n"
"            \n"
"            /* Compute shadow contribution */\n"
"            float Shadow = ShadowContribution(Moments, Distance);\n"
"            \n"
"            Diffuse *= CAST(float3, Shadow);\n"
"            Specular *= CAST(float3, Shadow);\n"
"        }\n"
"        \n"
"        #ifdef GLOBAL_ILLUMINATION\n"
"        \n"
"        /* Compute VPLs (virtual point lights) */\n"
"        float3 IndirectTexCoord = float3(0.0, 0.0, float(Light.ShadowIndex));\n"
"        \n"
"        for (int i = 0; i < 100; ++i)\n"
"        {\n"
"            /* Get VPL offset */\n"
"            IndirectTexCoord.xy = VPLOffsets[i].xy;\n"
"            \n"
"            /* Sample indirect light distance */\n"
"            float IndirectDist = tex2DArray(DirLightShadowMaps, IndirectTexCoord).r;\n"
"            \n"
"            /* Get the indirect light's position */\n"
"            float4 LightRay = float4(IndirectTexCoord.x*2.0 - 1.0, 1.0 - IndirectTexCoord.y*2.0, 1.0, 1.0);\n"
"            LightRay = normalize(LightEx.InvViewProjection * LightRay);\n"
"            float3 IndirectPoint = Light.PositionAndInvRadius.xyz + LightRay.xyz * CAST(float3, IndirectDist);\n"
"            \n"
"            /* Shade indirect light */\n"
"            float IntensityIL = 0.0;\n"
"            \n"
"            if (ComputeVPLShading(WorldPos, Normal, IndirectPoint, IntensityIL))\n"
"            {\n"
"                /* Sample indirect light color */\n"
"                float3 IndirectColor = tex2DArray(DirLightDiffuseMaps, IndirectTexCoord).rgb;\n"
"                \n"
"                /* Apply VPL shading */\n"
"                IndirectColor *= CAST(float3, IntensityIL);\n"
"                Diffuse += IndirectColor;\n"
"            }\n"
"        }\n"
"        \n"
"        #endif\n"
"    }\n"
"}\n"
"\n"
"#    ifdef GLOBAL_ILLUMINATION\n"
"\n"
"//! Low-resolution light shading function for global illumination.\n"
"void ComputeLowResLightShadingVPL(\n"
"    in SLight Light, in SLightEx LightEx, in float3 WorldPos,\n"
"    in float3 Normal, inout float3 Diffuse)\n"
"{\n"
"    /* Compute light direction vector */\n"
"    float3 LightDir = CAST(float3, 0.0);\n"
"    \n"
"    if (Light.Type != LIGHT_DIRECTIONAL)\n"
"        LightDir = normalize(WorldPos - Light.PositionAndInvRadius.xyz);\n"
"    else\n"
"        LightDir = LightEx.Direction;\n"
"    \n"
"    /* Compute phong shading */\n"
"    float NdotL = max(AMBIENT_LIGHT_FACTOR, dot(Normal, -LightDir));\n"
"    \n"
"    /* Compute light attenuation */\n"
"    float Distance = distance(WorldPos, Light.PositionAndInvRadius.xyz);\n"
"    \n"
"    float AttnLinear    = Distance * Light.PositionAndInvRadius.w;\n"
"    float AttnQuadratic = AttnLinear * Distance;\n"
"    \n"
"    float Intensity = saturate(1.0 / (1.0 + AttnLinear + AttnQuadratic) - LIGHT_CUTOFF);\n"
"    \n"
"    if (Light.Type == LIGHT_SPOT)\n"
"        Intensity *= GetSpotLightIntensity(LightDir, LightEx);\n"
"    \n"
"    /* Compute diffuse color */\n"
"    Diffuse = CAST(float3, Intensity * NdotL);\n"
"    \n"
"    float3 Specular = CAST(float3, 0.0);\n"
"    \n"
"    /* Apply shadow */\n"
"    if (Light.ShadowIndex != -1)\n"
"        ComputeLightShadow(Light, LightEx, WorldPos, Normal, Distance, Diffuse, Specular);\n"
"    \n"
"    Diffuse *= Light.Color;\n"
"}\n"
"\n"
"#    endif\n"
"\n"
"#endif\n"
"\n"
"//! Main light shading function.\n"
"void ComputeLightShading(\n"
"    in SLight Light, in SLightEx LightEx,\n"
"    in float3 WorldPos, in float3 Normal, in float Shininess, in float3 ViewRay,\n"
"    #ifdef HAS_LIGHT_MAP\n"
"    inout float3 StaticDiffuseColor, inout float3 StaticSpecularColor,\n"
"    #endif\n"
"    inout float3 DiffuseColor, inout float3 SpecularColor)\n"
"{\n"
"    /* Compute light direction vector */\n"
"    float3 LightDir = CAST(float3, 0.0);\n"
"    \n"
"    if (Light.Type != LIGHT_DIRECTIONAL)\n"
"        LightDir = normalize(WorldPos - Light.PositionAndInvRadius.xyz);\n"
"    else\n"
"        LightDir = LightEx.Direction;\n"
"    \n"
"    /* Compute phong shading */\n"
"    float NdotL = max(AMBIENT_LIGHT_FACTOR, dot(Normal, -LightDir));\n"
"    \n"
"    /* Compute light attenuation */\n"
"    float Distance = distance(WorldPos, Light.PositionAndInvRadius.xyz);\n"
"    \n"
"    float AttnLinear    = Distance * Light.PositionAndInvRadius.w;\n"
"    float AttnQuadratic = AttnLinear * Distance;\n"
"    \n"
"    float Intensity = saturate(1.0 / (1.0 + AttnLinear + AttnQuadratic) - LIGHT_CUTOFF);\n"
"    \n"
"    if (Light.Type == LIGHT_SPOT)\n"
"        Intensity *= GetSpotLightIntensity(LightDir, LightEx);\n"
"    \n"
"    /* Compute diffuse color */\n"
"    float3 Diffuse = CAST(float3, Intensity * NdotL);\n"
"    \n"
"    /* Compute specular color */\n"
"    float3 Reflection = normalize(reflect(LightDir, Normal));\n"
"    \n"
"    float NdotHV = -dot(ViewRay, Reflection);\n"
"    \n"
"    float3 Specular = Light.Color * CAST(float3, Intensity * pow(max(0.0, NdotHV), Shininess));\n"
"    \n"
"    #ifdef SHADOW_MAPPING\n"
"    \n"
"    /* Apply shadow */\n"
"    if (Light.ShadowIndex != -1)\n"
"        ComputeLightShadow(Light, LightEx, WorldPos, Normal, Distance, Diffuse, Specular);\n"
"    \n"
"    #endif\n"
"    \n"
"    Diffuse *= Light.Color;\n"
"    \n"
"    /* Add light color */\n"
"    #ifdef HAS_LIGHT_MAP\n"
"    \n"
"    if (Light.UsedForLightmaps != 0)\n"
"    {\n"
"        StaticDiffuseColor += Diffuse;\n"
"        StaticSpecularColor += Specular;\n"
"    }\n"
"    else\n"
"    {\n"
"        DiffuseColor += Diffuse;\n"
"        SpecularColor += Specular;\n"
"    }\n"
"    \n"
"    #else\n"
"    \n"
"    DiffuseColor += Diffuse;\n"
"    SpecularColor += Specular;\n"
"    \n"
"    #endif\n"
"}\n"
