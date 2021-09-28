/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#pragma once

// clang-format off

/// \file
/// Definition of the Diligent::ITexture interface and related data structures

#include "GraphicsTypes.h"
#include "DeviceObject.h"
#include "TextureView.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)


// {A64B0E60-1B5E-4CFD-B880-663A1ADCBE98}
static const INTERFACE_ID IID_Texture =
    {0xa64b0e60, 0x1b5e, 0x4cfd,{0xb8, 0x80, 0x66, 0x3a, 0x1a, 0xdc, 0xbe, 0x98}};


/// Miscellaneous texture flags

/// The enumeration is used by TextureDesc to describe misc texture flags
DILIGENT_TYPED_ENUM(MISC_TEXTURE_FLAGS, Uint8)
{
    MISC_TEXTURE_FLAG_NONE             = 0u,

    /// Allow automatic mipmap generation with IDeviceContext::GenerateMips()

    /// \note A texture must be created with BIND_RENDER_TARGET bind flag.
    MISC_TEXTURE_FLAG_GENERATE_MIPS    = 1u << 0,

    /// The texture will be used as a transient framebuffer attachment.

    /// \note Memoryless textures may only be used within a render pass in a framebuffer;
    ///       the corresponding subpass load operation must be CLEAR or DISCARD, and the
    ///       subpass store operation must be DISCARD.
    MISC_TEXTURE_FLAG_MEMORYLESS      = 1u << 1,

    /// For sparse textures, allow binding the same memory range in different texture
    /// regions or in different sparse textures.
    MISC_TEXTURE_FLAG_SPARSE_ALIASING = 1u << 2,
};
DEFINE_FLAG_ENUM_OPERATORS(MISC_TEXTURE_FLAGS)

/// Texture description
struct TextureDesc DILIGENT_DERIVE(DeviceObjectAttribs)

    /// Texture type. See Diligent::RESOURCE_DIMENSION for details.
    RESOURCE_DIMENSION Type DEFAULT_INITIALIZER(RESOURCE_DIM_UNDEFINED);

    /// Texture width, in pixels.
    Uint32 Width            DEFAULT_INITIALIZER(0);

    /// Texture height, in pixels.
    Uint32 Height           DEFAULT_INITIALIZER(0);

    union
    {
        /// For a 1D array or 2D array, number of array slices
        Uint32 ArraySize    DEFAULT_INITIALIZER(1);

        /// For a 3D texture, number of depth slices
        Uint32 Depth;
    };

    /// Texture format, see Diligent::TEXTURE_FORMAT.
    /// Use IRenderDevice::GetTextureFormatInfo() to check if format is supported.
    TEXTURE_FORMAT Format       DEFAULT_INITIALIZER(TEX_FORMAT_UNKNOWN);

    /// Number of Mip levels in the texture. Multisampled textures can only have 1 Mip level.
    /// Specify 0 to create full mipmap chain.
    Uint32          MipLevels   DEFAULT_INITIALIZER(1);

    /// Number of samples.\n
    /// Only 2D textures or 2D texture arrays can be multisampled.
    Uint32          SampleCount DEFAULT_INITIALIZER(1);

    /// Bind flags, see Diligent::BIND_FLAGS for details. \n
    /// Use IRenderDevice::GetTextureFormatInfoExt() to check which bind flags are supported.
    BIND_FLAGS      BindFlags   DEFAULT_INITIALIZER(BIND_NONE);

    /// Texture usage. See Diligent::USAGE for details.
    USAGE           Usage       DEFAULT_INITIALIZER(USAGE_DEFAULT);

    /// CPU access flags or 0 if no CPU access is allowed,
    /// see Diligent::CPU_ACCESS_FLAGS for details.
    CPU_ACCESS_FLAGS CPUAccessFlags     DEFAULT_INITIALIZER(CPU_ACCESS_NONE);

    /// Miscellaneous flags, see Diligent::MISC_TEXTURE_FLAGS for details.
    MISC_TEXTURE_FLAGS MiscFlags        DEFAULT_INITIALIZER(MISC_TEXTURE_FLAG_NONE);

    /// Optimized clear value
    OptimizedClearValue ClearValue;

    /// Defines which immediate contexts are allowed to execute commands that use this texture.

    /// When ImmediateContextMask contains a bit at position n, the texture may be
    /// used in the immediate context with index n directly (see DeviceContextDesc::ContextId).
    /// It may also be used in a command list recorded by a deferred context that will be executed
    /// through that immediate context.
    ///
    /// \remarks    Only specify these bits that will indicate those immediate contexts where the texture
    ///             will actually be used. Do not set unnecessary bits as this will result in extra overhead.
    Uint64 ImmediateContextMask         DEFAULT_INITIALIZER(1);


#if DILIGENT_CPP_INTERFACE
    constexpr TextureDesc() noexcept {}

    constexpr TextureDesc(const Char*         _Name,
                          RESOURCE_DIMENSION  _Type,
                          Uint32              _Width,
                          Uint32              _Height,
                          Uint32              _ArraySizeOrDepth,
                          TEXTURE_FORMAT      _Format,
                          Uint32              _MipLevels            = TextureDesc{}.MipLevels,
                          Uint32              _SampleCount          = TextureDesc{}.SampleCount,
                          USAGE               _Usage                = TextureDesc{}.Usage,
                          BIND_FLAGS          _BindFlags            = TextureDesc{}.BindFlags,
                          CPU_ACCESS_FLAGS    _CPUAccessFlags       = TextureDesc{}.CPUAccessFlags,
                          MISC_TEXTURE_FLAGS  _MiscFlags            = TextureDesc{}.MiscFlags,
                          OptimizedClearValue _ClearValue           = TextureDesc{}.ClearValue,
                          Uint64              _ImmediateContextMask = TextureDesc{}.ImmediateContextMask) noexcept :
        DeviceObjectAttribs  {_Name            },
        Type                 {_Type            },
        Width                {_Width           },
        Height               {_Height          },
        ArraySize            {_ArraySizeOrDepth},
        Format               {_Format          },
        MipLevels            {_MipLevels       },
        SampleCount          {_SampleCount     },
        BindFlags            {_BindFlags       },
        Usage                {_Usage           },
        CPUAccessFlags       {_CPUAccessFlags  },
        MiscFlags            {_MiscFlags       },
        ClearValue           {_ClearValue      },
        ImmediateContextMask {_ImmediateContextMask}
    {}

    constexpr Uint32 ArraySizeOrDepth() const { return ArraySize; }

    /// Tests if two structures are equivalent

    /// \param [in] RHS - reference to the structure to perform comparison with
    /// \return
    /// - True if all members of the two structures except for the Name are equal.
    /// - False otherwise.
    /// The operator ignores DeviceObjectAttribs::Name field as it does not affect
    /// the texture description state.
    constexpr bool operator ==(const TextureDesc& RHS)const
    {
                // Name is primarily used for debug purposes and does not affect the state.
                // It is ignored in comparison operation.
        return  // strcmp(Name, RHS.Name) == 0          &&
                Type                 == RHS.Type           &&
                Width                == RHS.Width          &&
                Height               == RHS.Height         &&
                ArraySizeOrDepth()   == RHS.ArraySizeOrDepth() &&
                Format               == RHS.Format         &&
                MipLevels            == RHS.MipLevels      &&
                SampleCount          == RHS.SampleCount    &&
                Usage                == RHS.Usage          &&
                BindFlags            == RHS.BindFlags      &&
                CPUAccessFlags       == RHS.CPUAccessFlags &&
                MiscFlags            == RHS.MiscFlags      &&
                ClearValue           == RHS.ClearValue     &&
                ImmediateContextMask == RHS.ImmediateContextMask;
    }

    constexpr bool IsArray() const
    {
        return Type == RESOURCE_DIM_TEX_1D_ARRAY ||
               Type == RESOURCE_DIM_TEX_2D_ARRAY ||
               Type == RESOURCE_DIM_TEX_CUBE     ||
               Type == RESOURCE_DIM_TEX_CUBE_ARRAY;
    }

    constexpr bool Is1D() const
    {
        return Type == RESOURCE_DIM_TEX_1D      ||
               Type == RESOURCE_DIM_TEX_1D_ARRAY;
    }

    constexpr bool Is2D() const
    {
        return Type == RESOURCE_DIM_TEX_2D       ||
               Type == RESOURCE_DIM_TEX_2D_ARRAY ||
               Type == RESOURCE_DIM_TEX_CUBE     ||
               Type == RESOURCE_DIM_TEX_CUBE_ARRAY;
    }

    constexpr bool Is3D() const
    {
        return Type == RESOURCE_DIM_TEX_3D;
    }

    constexpr bool IsCube() const
    {
        return Type == RESOURCE_DIM_TEX_CUBE     ||
               Type == RESOURCE_DIM_TEX_CUBE_ARRAY;
    }

    constexpr Uint32 GetArraySize() const
    {
        return IsArray() ? ArraySize : 1u;
    }

    constexpr Uint32 GetWidth() const
    {
        return Width;
    }

    constexpr Uint32 GetHeight() const
    {
        return Is1D() ? 1u : Height;
    }

    constexpr Uint32 GetDepth() const
    {
        return Is3D() ? Depth : 1u;
    }
#endif
};
typedef struct TextureDesc TextureDesc;

/// Describes data for one subresource
struct TextureSubResData
{
    /// Pointer to the subresource data in CPU memory.
    /// If provided, pSrcBuffer must be null
    const void* pData           DEFAULT_INITIALIZER(nullptr);

    /// Pointer to the GPU buffer that contains subresource data.
    /// If provided, pData must be null
    struct IBuffer* pSrcBuffer   DEFAULT_INITIALIZER(nullptr);

    /// When updating data from the buffer (pSrcBuffer is not null),
    /// offset from the beginning of the buffer to the data start
    Uint64 SrcOffset            DEFAULT_INITIALIZER(0);

    /// For 2D and 3D textures, row stride in bytes
    Uint64 Stride               DEFAULT_INITIALIZER(0);

    /// For 3D textures, depth slice stride in bytes
    /// \note On OpenGL, this must be a multiple of Stride
    Uint64 DepthStride          DEFAULT_INITIALIZER(0);


#if DILIGENT_CPP_INTERFACE
    /// Initializes the structure members with default values

    /// Default values:
    /// Member          | Default value
    /// ----------------|--------------
    /// pData           | nullptr
    /// SrcOffset       | 0
    /// Stride          | 0
    /// DepthStride     | 0
    constexpr TextureSubResData() noexcept {}

    /// Initializes the structure members to perform copy from the CPU memory
    constexpr TextureSubResData(const void* _pData, Uint64 _Stride, Uint64 _DepthStride = 0) noexcept :
        pData       (_pData),
        pSrcBuffer  (nullptr),
        SrcOffset   (0),
        Stride      (_Stride),
        DepthStride (_DepthStride)
    {}

    /// Initializes the structure members to perform copy from the GPU buffer
    constexpr TextureSubResData(IBuffer* _pBuffer, Uint64 _SrcOffset, Uint64 _Stride, Uint64 _DepthStride = 0) noexcept :
        pData       {nullptr     },
        pSrcBuffer  {_pBuffer    },
        SrcOffset   {_SrcOffset  },
        Stride      {_Stride     },
        DepthStride {_DepthStride}
    {}
#endif
};
typedef struct TextureSubResData TextureSubResData;

/// Describes the initial data to store in the texture
struct TextureData
{
    /// Pointer to the array of the TextureSubResData elements containing
    /// information about each subresource.
    TextureSubResData* pSubResources    DEFAULT_INITIALIZER(nullptr);

    /// Number of elements in pSubResources array.
    /// NumSubresources must exactly match the number
    /// of subresources in the texture. Otherwise an error
    /// occurs.
    Uint32             NumSubresources  DEFAULT_INITIALIZER(0);

    /// Defines which device context will be used to initialize the texture.

    /// The texture will be in write state after the initialization.
    /// If an application uses the texture in another context afterwards, it
    /// must synchronize the access to the texture using fence.
    /// When null is provided, the first context enabled by ImmediateContextMask
    /// will be used.
    struct IDeviceContext* pContext     DEFAULT_INITIALIZER(nullptr);

#if DILIGENT_CPP_INTERFACE
    constexpr TextureData() noexcept {}

    constexpr TextureData(TextureSubResData* _pSubResources,
                          Uint32             _NumSubresources,
                          IDeviceContext*    _pContext = nullptr) noexcept :
        pSubResources   {_pSubResources  },
        NumSubresources {_NumSubresources},
        pContext        {_pContext       }
    {}
#endif
};
typedef struct TextureData TextureData;

struct MappedTextureSubresource
{
    PVoid  pData       DEFAULT_INITIALIZER(nullptr);
    Uint64 Stride      DEFAULT_INITIALIZER(0);
    Uint64 DepthStride DEFAULT_INITIALIZER(0);

#if DILIGENT_CPP_INTERFACE
    constexpr MappedTextureSubresource() noexcept {}

    constexpr MappedTextureSubresource(PVoid  _pData,
                                       Uint64 _Stride,
                                       Uint64 _DepthStride = 0) noexcept :
        pData       {_pData      },
        Stride      {_Stride     },
        DepthStride {_DepthStride}
    {}
#endif
};
typedef struct MappedTextureSubresource MappedTextureSubresource;

/// Describes the sparse texture packing mode
DILIGENT_TYPED_ENUM(SPARSE_TEXTURE_FLAGS, Uint8)
{
    SPARSE_TEXTURE_FLAG_NONE                   = 0,

    // AZ TODO: if MipTailStride == 0, single mip tail is used, so this flag is not needed, remove ?
    /// Specifies that the texture uses a single mip tail region for all array layers
    SPARSE_TEXTURE_FLAG_SINGLE_MIPTAIL         = 1u << 0,

    // AZ TODO: not needed because of FirstMipInTail, remove ?
    /// Specifies that the first mip level whose dimensions are not integer
    /// multiples of the corresponding dimensions of the sparse texture block begins the mip tail region.
    SPARSE_TEXTURE_FLAG_ALIGNED_MIP_SIZE       = 1u << 1,

    // AZ TODO: not needed because of TileSize, remove ?
    /// Specifies that the texture uses non-standard sparse texture block dimensions,
    /// and the TileSize values do not match the standard sparse texture block dimensions.
    SPARSE_TEXTURE_FLAG_NONSTANDARD_BLOCK_SIZE = 1u << 2,

    SPARSE_TEXTURE_FLAG_LAST                   = SPARSE_TEXTURE_FLAG_NONSTANDARD_BLOCK_SIZE
};
DEFINE_FLAG_ENUM_OPERATORS(SPARSE_TEXTURE_FLAGS);

/// Describes the sparse texture properties
struct TextureSparseProperties
{
    /// Texture address space size.
    Uint64  MemorySize      DEFAULT_INITIALIZER(0);

    /// Specifies where to bind the mip tail memory.
    /// Reserved for internal use.
    Uint64  MipTailOffset   DEFAULT_INITIALIZER(0);

    /// Specifies how to calculate the mip tail offset for 2D array texture.
    /// Reserved for internal use.
    Uint64  MipTailStride   DEFAULT_INITIALIZER(0);

    /// Specifies the mip tail size in bytes.
    /// \note Single mip tail for a 2D array may exceed the 32-bit limit.
    Uint64  MipTailSize     DEFAULT_INITIALIZER(0);

    /// The first mip level in the mip tail that is packed into a single memory block.
    Uint32  FirstMipInTail  DEFAULT_INITIALIZER(~0u);

    /// Specifies the tile dimension for a single sparse block, see SparseMemoryProperties::SparseBlockSize.
    Uint32  TileSize[3]     DEFAULT_INITIALIZER({});

    /// Size of the sparse block.

    /// \remarks The offset in the buffer, memory offset and memory size that are used in sparse binding command 
    ///          must be multiples of the block size.
    ///
    ///          The sparse block size is equal to SparseMemoryProperties::StandardBlockSize if Flags don't contain
    ///          SPARSE_TEXTURE_FLAG_NONSTANDARD_BLOCK_SIZE.
    Uint32  BlockSize DEFAULT_INITIALIZER(0);

    /// Flags that describe additional packing modes.
    SPARSE_TEXTURE_FLAGS Flags DEFAULT_INITIALIZER(SPARSE_TEXTURE_FLAG_NONE);
};
typedef struct TextureSparseProperties TextureSparseProperties;

#define DILIGENT_INTERFACE_NAME ITexture
#include "../../../Primitives/interface/DefineInterfaceHelperMacros.h"

#define ITextureInclusiveMethods   \
    IDeviceObjectInclusiveMethods; \
    ITextureMethods Texture

/// Texture interface
DILIGENT_BEGIN_INTERFACE(ITexture, IDeviceObject)
{
#if DILIGENT_CPP_INTERFACE
    /// Returns the texture description used to create the object
    virtual const TextureDesc& METHOD(GetDesc)()const override = 0;
#endif

    /// Creates a new texture view

    /// \param [in] ViewDesc - View description. See Diligent::TextureViewDesc for details.
    /// \param [out] ppView - Address of the memory location where the pointer to the view interface will be written to.
    ///
    /// \remarks To create a shader resource view addressing the entire texture, set only TextureViewDesc::ViewType
    ///          member of the ViewDesc parameter to Diligent::TEXTURE_VIEW_SHADER_RESOURCE and leave all other
    ///          members in their default values. Using the same method, you can create render target or depth stencil
    ///          view addressing the largest mip level.\n
    ///          If texture view format is Diligent::TEX_FORMAT_UNKNOWN, the view format will match the texture format.\n
    ///          If texture view type is Diligent::TEXTURE_VIEW_UNDEFINED, the type will match the texture type.\n
    ///          If the number of mip levels is 0, and the view type is shader resource, the view will address all mip levels.
    ///          For other view types it will address one mip level.\n
    ///          If the number of slices is 0, all slices from FirstArraySlice or FirstDepthSlice will be referenced by the view.
    ///          For non-array textures, the only allowed values for the number of slices are 0 and 1.\n
    ///          Texture view will contain strong reference to the texture, so the texture will not be destroyed
    ///          until all views are released.\n
    ///          The function calls AddRef() for the created interface, so it must be released by
    ///          a call to Release() when it is no longer needed.
    VIRTUAL void METHOD(CreateView)(THIS_
                                    const TextureViewDesc REF ViewDesc,
                                    ITextureView**            ppView) PURE;

    /// Returns the pointer to the default view.

    /// \param [in] ViewType - Type of the requested view. See Diligent::TEXTURE_VIEW_TYPE.
    /// \return Pointer to the interface
    ///
    /// \note The function does not increase the reference counter for the returned interface, so
    ///       Release() must *NOT* be called.
    VIRTUAL ITextureView* METHOD(GetDefaultView)(THIS_
                                                 TEXTURE_VIEW_TYPE ViewType) PURE;


    /// Returns native texture handle specific to the underlying graphics API

    /// \return pointer to ID3D11Resource interface, for D3D11 implementation\n
    ///         pointer to ID3D12Resource interface, for D3D12 implementation\n
    ///         GL buffer handle, for GL implementation
    VIRTUAL Uint64 METHOD(GetNativeHandle)(THIS) PURE;

    /// Sets the usage state for all texture subresources.

    /// \note This method does not perform state transition, but
    ///       resets the internal texture state to the given value.
    ///       This method should be used after the application finished
    ///       manually managing the texture state and wants to hand over
    ///       state management back to the engine.
    VIRTUAL void METHOD(SetState)(THIS_
                                  RESOURCE_STATE State) PURE;

    /// Returns the internal texture state
    VIRTUAL RESOURCE_STATE METHOD(GetState)(THIS) CONST PURE;

    /// Returns the texture sparse memory properties
    VIRTUAL const TextureSparseProperties REF METHOD(GetSparseProperties)(THIS) CONST PURE;
};
DILIGENT_END_INTERFACE

#include "../../../Primitives/interface/UndefInterfaceHelperMacros.h"

#if DILIGENT_C_INTERFACE

// clang-format off

#    define ITexture_GetDesc(This) (const struct TextureDesc*)IDeviceObject_GetDesc(This)

#    define ITexture_CreateView(This, ...)     CALL_IFACE_METHOD(Texture, CreateView,          This, __VA_ARGS__)
#    define ITexture_GetDefaultView(This, ...) CALL_IFACE_METHOD(Texture, GetDefaultView,      This, __VA_ARGS__)
#    define ITexture_GetNativeHandle(This)     CALL_IFACE_METHOD(Texture, GetNativeHandle,     This)
#    define ITexture_SetState(This, ...)       CALL_IFACE_METHOD(Texture, SetState,            This, __VA_ARGS__)
#    define ITexture_GetState(This)            CALL_IFACE_METHOD(Texture, GetState,            This)
#    define ITexture_GetSparseProperties(This) CALL_IFACE_METHOD(Texture, GetSparseProperties, This)

// clang-format on

#endif

DILIGENT_END_NAMESPACE // namespace Diligent
