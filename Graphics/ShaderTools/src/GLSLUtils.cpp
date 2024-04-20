/*
 *  Copyright 2019-2024 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include <cstring>
#include <sstream>

#include "GLSLUtils.hpp"
#include "DebugUtilities.hpp"
#if !DILIGENT_NO_HLSL
#    include "HLSL2GLSLConverterImpl.hpp"
#endif
#include "RefCntAutoPtr.hpp"
#include "DataBlobImpl.hpp"
#include "ShaderToolsCommon.hpp"

namespace Diligent
{

static void AppendGLESExtensions(SHADER_TYPE              ShaderType,
                                 const DeviceFeatures&    Features,
                                 const TextureProperties& TexProps,
                                 const ShaderVersion&     LangVer,
                                 std::string&             GLSLSource)
{
    const bool IsES31OrAbove = LangVer >= ShaderVersion{3, 1};
    const bool IsES32OrAbove = LangVer >= ShaderVersion{3, 2};

    if (Features.SeparablePrograms && !IsES31OrAbove)
        GLSLSource.append("#extension GL_EXT_separate_shader_objects : enable\n");

    if (TexProps.CubemapArraysSupported && !IsES32OrAbove)
        GLSLSource.append("#extension GL_EXT_texture_cube_map_array : enable\n");

    if (ShaderType == SHADER_TYPE_GEOMETRY && !IsES32OrAbove)
        GLSLSource.append("#extension GL_EXT_geometry_shader : enable\n");

    if ((ShaderType == SHADER_TYPE_HULL || ShaderType == SHADER_TYPE_DOMAIN) && !IsES32OrAbove)
        GLSLSource.append("#extension GL_EXT_tessellation_shader : enable\n");
}

static void AppendPrecisionQualifiers(const DeviceFeatures&    Features,
                                      const TextureProperties& TexProps,
                                      const ShaderVersion&     LangVer,
                                      std::string&             GLSLSource)
{
    const bool IsES32OrAbove = LangVer >= ShaderVersion{3, 2};

    GLSLSource.append(
        "precision highp float;\n"
        "precision highp int;\n"
        //"precision highp uint;\n"  // This line causes shader compilation error on NVidia!

        "precision highp sampler2D;\n"
        "precision highp sampler3D;\n"
        "precision highp samplerCube;\n"
        "precision highp samplerCubeShadow;\n"

        "precision highp sampler2DShadow;\n"
        "precision highp sampler2DArray;\n"
        "precision highp sampler2DArrayShadow;\n"

        "precision highp isampler2D;\n"
        "precision highp isampler3D;\n"
        "precision highp isamplerCube;\n"
        "precision highp isampler2DArray;\n"

        "precision highp usampler2D;\n"
        "precision highp usampler3D;\n"
        "precision highp usamplerCube;\n"
        "precision highp usampler2DArray;\n");

    if (IsES32OrAbove)
    {
        GLSLSource.append(
            "precision highp samplerBuffer;\n"
            "precision highp isamplerBuffer;\n"
            "precision highp usamplerBuffer;\n");
    }

    if (TexProps.CubemapArraysSupported)
    {
        GLSLSource.append(
            "precision highp samplerCubeArray;\n"
            "precision highp samplerCubeArrayShadow;\n"
            "precision highp isamplerCubeArray;\n"
            "precision highp usamplerCubeArray;\n");
    }

    if (TexProps.Texture2DMSSupported)
    {
        GLSLSource.append(
            "precision highp sampler2DMS;\n"
            "precision highp isampler2DMS;\n"
            "precision highp usampler2DMS;\n");
    }

    if (Features.ComputeShaders)
    {
        GLSLSource.append(
            "precision highp image2D;\n"
            "precision highp image3D;\n"
            "precision highp imageCube;\n"
            "precision highp image2DArray;\n"

            "precision highp iimage2D;\n"
            "precision highp iimage3D;\n"
            "precision highp iimageCube;\n"
            "precision highp iimage2DArray;\n"

            "precision highp uimage2D;\n"
            "precision highp uimage3D;\n"
            "precision highp uimageCube;\n"
            "precision highp uimage2DArray;\n");

        if (IsES32OrAbove)
        {
            GLSLSource.append(
                "precision highp imageBuffer;\n"
                "precision highp iimageBuffer;\n"
                "precision highp uimageBuffer;\n");
        }
    }
}

String BuildGLSLSourceString(const ShaderCreateInfo&      ShaderCI,
                             const RenderDeviceInfo&      DeviceInfo,
                             const GraphicsAdapterInfo&   AdapterInfo,
                             TargetGLSLCompiler           TargetCompiler,
                             bool                         ZeroToOneClipZ,
                             const char*                  ExtraDefinitions,
                             IHLSL2GLSLConversionStream** ppConversionStream) noexcept(false)
{
    if (!(ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_DEFAULT ||
          ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_GLSL ||
          ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM ||
          ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_HLSL))
    {
        UNSUPPORTED("Unsupported shader source language");
        return "";
    }

    const auto SourceData = ReadShaderSourceFile(ShaderCI);
    if (ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM)
    {
        if (ShaderCI.Macros)
        {
            LOG_WARNING_MESSAGE("Shader macros are ignored when compiling GLSL verbatim");
        }

        return std::string{SourceData.Source, SourceData.SourceLength};
    }

    ShaderVersion GLSLVer;
    bool          IsES = false;
#if PLATFORM_WIN32 || PLATFORM_LINUX

    GLSLVer = ShaderCI.GLSLVersion;
    if (GLSLVer == ShaderVersion{})
        GLSLVer = DeviceInfo.MaxShaderVersion.GLSL;
    if (GLSLVer == ShaderVersion{})
        GLSLVer = ShaderVersion{4, 3};

#elif PLATFORM_MACOS

    if (TargetCompiler == TargetGLSLCompiler::glslang)
    {
        GLSLVer = ShaderVersion{4, 3};
    }
    else if (TargetCompiler == TargetGLSLCompiler::driver)
    {
        GLSLVer = ShaderVersion{4, 1};
    }
    else
    {
        UNEXPECTED("Unexpected target GLSL compiler");
    }

#elif PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_TVOS || PLATFORM_EMSCRIPTEN

    if (DeviceInfo.Type == RENDER_DEVICE_TYPE_VULKAN || DeviceInfo.Type == RENDER_DEVICE_TYPE_METAL)
    {
        GLSLVer = ShaderVersion{3, 1};
    }
    else if (DeviceInfo.Type == RENDER_DEVICE_TYPE_GLES)
    {
        GLSLVer = DeviceInfo.APIVersion;
    }
    else
    {
        UNEXPECTED("Unexpected device type");
    }
    IsES = true;

#else
#    error "Undefined platform"
#endif

    const auto ShaderType = ShaderCI.Desc.ShaderType;

    String GLSLSource;
    {
        std::stringstream verss;
        verss << "#version " << Uint32{GLSLVer.Major} << Uint32{GLSLVer.Minor} << (IsES ? "0 es\n" : "0 core\n");
        GLSLSource = verss.str();
    }

    // All extensions must go right after the version directive
    if (IsES)
    {
        AppendGLESExtensions(ShaderType, DeviceInfo.Features, AdapterInfo.Texture, GLSLVer, GLSLSource);
    }

    if (ShaderCI.GLSLExtensions != nullptr && ShaderCI.GLSLExtensions[0] != '\0')
    {
        GLSLSource.append(ShaderCI.GLSLExtensions);
        GLSLSource.push_back('\n');
    }

    if (IsES)
    {
        GLSLSource.append("#ifndef GL_ES\n"
                          "#  define GL_ES 1\n"
                          "#endif\n");
    }
    else
    {
        GLSLSource.append("#define DESKTOP_GL 1\n");
    }

    if (ZeroToOneClipZ)
    {
        GLSLSource.append("#define _NDC_ZERO_TO_ONE 1\n");
    }

    if (ExtraDefinitions != nullptr)
    {
        GLSLSource.append(ExtraDefinitions);
    }

    AppendPlatformDefinition(GLSLSource);
    AppendShaderTypeDefinitions(GLSLSource, ShaderType);

    if (IsES)
    {
        AppendPrecisionQualifiers(DeviceInfo.Features, AdapterInfo.Texture, GLSLVer, GLSLSource);
    }

    // It would be much more convenient to use row_major matrices.
    // But unfortunately on NVIDIA, the following directive
    // layout(std140, row_major) uniform;
    // does not have any effect on matrices that are part of structures
    // So we have to use column-major matrices which are default in both
    // DX and GLSL.
    GLSLSource.append("layout(std140) uniform;\n");

    AppendShaderMacros(GLSLSource, ShaderCI.Macros);

    if (IsES && GLSLVer == ShaderVersion{3, 0} && DeviceInfo.Features.SeparablePrograms && ShaderType == SHADER_TYPE_VERTEX)
    {
        // From https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_separate_shader_objects.gles.txt:
        //
        // When using GLSL ES 3.00 shaders in separable programs, gl_Position and
        // gl_PointSize built-in outputs must be redeclared according to Section 7.5
        // of the OpenGL Shading Language Specification...
        //
        // add to GLSL ES 3.00 new section 7.5, Built-In Redeclaration and
        // Separable Programs:
        //
        // "The following vertex shader outputs may be redeclared at global scope to
        // specify a built-in output interface, with or without special qualifiers:
        //
        //     gl_Position
        //     gl_PointSize
        //
        //   When compiling shaders using either of the above variables, both such
        //   variables must be redeclared prior to use.  ((Note:  This restriction
        //   applies only to shaders using version 300 that enable the
        //   EXT_separate_shader_objects extension; shaders not enabling the
        //   extension do not have this requirement.))  A separable program object
        //   will fail to link if any attached shader uses one of the above variables
        //   without redeclaration."
        GLSLSource.append("out vec4 gl_Position;\n");
    }

    if (ShaderCI.SourceLanguage == SHADER_SOURCE_LANGUAGE_HLSL)
    {
#if DILIGENT_NO_HLSL
        LOG_ERROR_AND_THROW("Unable to convert HLSL source to GLSL: HLSL support is disabled");
#else
        if (!ShaderCI.Desc.UseCombinedTextureSamplers)
        {
            LOG_ERROR_AND_THROW("Combined texture samplers are required to convert HLSL source to GLSL");
        }
        // Convert HLSL to GLSL
        const auto& Converter = HLSL2GLSLConverterImpl::GetInstance();

        HLSL2GLSLConverterImpl::ConversionAttribs Attribs;
        Attribs.pSourceStreamFactory = ShaderCI.pShaderSourceStreamFactory;
        Attribs.ppConversionStream   = ppConversionStream;
        Attribs.HLSLSource           = SourceData.Source;
        Attribs.NumSymbols           = SourceData.SourceLength;
        Attribs.EntryPoint           = ShaderCI.EntryPoint;
        Attribs.ShaderType           = ShaderCI.Desc.ShaderType;
        Attribs.IncludeDefinitions   = true;
        Attribs.InputFileName        = ShaderCI.FilePath;
        Attribs.SamplerSuffix        = ShaderCI.Desc.CombinedSamplerSuffix != nullptr ?
            ShaderCI.Desc.CombinedSamplerSuffix :
            ShaderDesc{}.CombinedSamplerSuffix;
        // Separate shader objects extension also allows input/output layout qualifiers for
        // all shader stages.
        // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_separate_shader_objects.txt
        // (search for "Input Layout Qualifiers" and "Output Layout Qualifiers").
        Attribs.UseInOutLocationQualifiers = DeviceInfo.Features.SeparablePrograms;
        auto ConvertedSource               = Converter.Convert(Attribs);
        if (ConvertedSource.empty())
        {
            LOG_ERROR_AND_THROW("Failed to convert HLSL source to GLSL");
        }
        GLSLSource.append(ConvertedSource);
#endif
    }
    else
    {
        GLSLSource.append(SourceData.Source, SourceData.SourceLength);
    }

    return GLSLSource;
}

} // namespace Diligent
