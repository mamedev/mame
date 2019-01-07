/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg#license-bsd-2-clause
 */

#ifndef BIMG_IMAGE_H_HEADER_GUARD
#define BIMG_IMAGE_H_HEADER_GUARD

#include <stdint.h> // uint32_t
#include <stdlib.h> // NULL

#define BIMG_API_VERSION UINT32_C(7)

namespace bx
{
	struct AllocatorI;
	class  Error;
	struct ReaderSeekerI;
	struct WriterI;

} // namespace bx

namespace bimg
{
	typedef void (*PackFn)(void*, const float*);
	typedef void (*UnpackFn)(float*, const void*);

	/// Texture format enum.
	///
	/// Notation:
	///
	///       RGBA16S
	///       ^   ^ ^
	///       |   | +-- [ ]Unorm
	///       |   |     [F]loat
	///       |   |     [S]norm
	///       |   |     [I]nt
	///       |   |     [U]int
	///       |   +---- Number of bits per component
	///       +-------- Components
	///
	/// @attention Availability depends on Caps (see: formats).
	///
	/// @attention C99 equivalent is `bgfx_texture_format_t`.
	///
	struct TextureFormat
	{
		/// Texture formats:
		enum Enum
		{
			BC1,          //!< DXT1
			BC2,          //!< DXT3
			BC3,          //!< DXT5
			BC4,          //!< LATC1/ATI1
			BC5,          //!< LATC2/ATI2
			BC6H,         //!< BC6H
			BC7,          //!< BC7
			ETC1,         //!< ETC1 RGB8
			ETC2,         //!< ETC2 RGB8
			ETC2A,        //!< ETC2 RGBA8
			ETC2A1,       //!< ETC2 RGB8A1
			PTC12,        //!< PVRTC1 RGB 2BPP
			PTC14,        //!< PVRTC1 RGB 4BPP
			PTC12A,       //!< PVRTC1 RGBA 2BPP
			PTC14A,       //!< PVRTC1 RGBA 4BPP
			PTC22,        //!< PVRTC2 RGBA 2BPP
			PTC24,        //!< PVRTC2 RGBA 4BPP
			ATC,          //!< ATC RGB 4BPP
			ATCE,         //!< ATCE RGBA 8 BPP explicit alpha
			ATCI,         //!< ATCI RGBA 8 BPP interpolated alpha
			ASTC4x4,      //!< ASTC 4x4 8.0 BPP
			ASTC5x5,      //!< ASTC 5x5 5.12 BPP
			ASTC6x6,      //!< ASTC 6x6 3.56 BPP
			ASTC8x5,      //!< ASTC 8x5 3.20 BPP
			ASTC8x6,      //!< ASTC 8x6 2.67 BPP
			ASTC10x5,     //!< ASTC 10x5 2.56 BPP

			Unknown,      // Compressed formats above.

			R1,
			A8,
			R8,
			R8I,
			R8U,
			R8S,
			R16,
			R16I,
			R16U,
			R16F,
			R16S,
			R32I,
			R32U,
			R32F,
			RG8,
			RG8I,
			RG8U,
			RG8S,
			RG16,
			RG16I,
			RG16U,
			RG16F,
			RG16S,
			RG32I,
			RG32U,
			RG32F,
			RGB8,
			RGB8I,
			RGB8U,
			RGB8S,
			RGB9E5F,
			BGRA8,
			RGBA8,
			RGBA8I,
			RGBA8U,
			RGBA8S,
			RGBA16,
			RGBA16I,
			RGBA16U,
			RGBA16F,
			RGBA16S,
			RGBA32I,
			RGBA32U,
			RGBA32F,
			R5G6B5,
			RGBA4,
			RGB5A1,
			RGB10A2,
			RG11B10F,

			UnknownDepth, // Depth formats below.

			D16,
			D24,
			D24S8,
			D32,
			D16F,
			D24F,
			D32F,
			D0S8,

			Count
		};
	};

	///
	struct Orientation
	{
		///
		enum Enum
		{
			R0,
			R90,
			R180,
			R270,
			HFlip,
			HFlipR90,
			HFlipR270,
			VFlip,
		};
	};

	/// Texture info.
	///
	/// @attention C99 equivalent is `bgfx_texture_info_t`.
	///
	struct TextureInfo
	{
		TextureFormat::Enum format; //!< Texture format.
		uint32_t storageSize;       //!< Total amount of bytes required to store texture.
		uint16_t width;             //!< Texture width.
		uint16_t height;            //!< Texture height.
		uint16_t depth;             //!< Texture depth.
		uint16_t numLayers;         //!< Number of layers in texture array.
		uint8_t numMips;            //!< Number of MIP maps.
		uint8_t bitsPerPixel;       //!< Format bits per pixel.
		bool    cubeMap;            //!< Texture is cubemap.
	};

	struct ImageContainer
	{
		bx::AllocatorI* m_allocator;
		void*           m_data;

		TextureFormat::Enum m_format;
		Orientation::Enum m_orientation;

		uint32_t m_size;
		uint32_t m_offset;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;
		uint16_t m_numLayers;
		uint8_t  m_numMips;
		bool     m_hasAlpha;
		bool     m_cubeMap;
		bool     m_ktx;
		bool     m_ktxLE;
		bool     m_srgb;
	};

	struct ImageMip
	{
		TextureFormat::Enum m_format;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;
		uint32_t m_blockSize;
		uint32_t m_size;
		uint8_t  m_bpp;
		bool     m_hasAlpha;
		const uint8_t* m_data;
	};

	struct ImageBlockInfo
	{
		uint8_t bitsPerPixel;
		uint8_t blockWidth;
		uint8_t blockHeight;
		uint8_t blockSize;
		uint8_t minBlockX;
		uint8_t minBlockY;
		uint8_t depthBits;
		uint8_t stencilBits;
		uint8_t rBits;
		uint8_t gBits;
		uint8_t bBits;
		uint8_t aBits;
		uint8_t encoding;
	};

	/// Returns true if texture format is compressed.
	bool isCompressed(TextureFormat::Enum _format);

	/// Returns true if texture format is uncompressed.
	bool isColor(TextureFormat::Enum _format);

	/// Returns true if texture format is depth.
	bool isDepth(TextureFormat::Enum _format);

	/// Returns true if texture format is valid.
	bool isValid(TextureFormat::Enum _format);

	/// returns true if texture format encoding is float.
	bool isFloat(TextureFormat::Enum _format);

	/// Returns bits per pixel.
	uint8_t getBitsPerPixel(TextureFormat::Enum _format);

	/// Returns texture block info.
	const ImageBlockInfo& getBlockInfo(TextureFormat::Enum _format);

	/// Converts format to string.
	const char* getName(TextureFormat::Enum _format);

	/// Converts string to format.
	TextureFormat::Enum getFormat(const char* _name);

	/// Returns number of mip-maps required for complete mip-map chain.
	uint8_t imageGetNumMips(
		  TextureFormat::Enum _format
		, uint16_t _width
		, uint16_t _height
		, uint16_t _depth = 0
		);

	/// Returns image size.
	uint32_t imageGetSize(
		  TextureInfo* _info
		, uint16_t _width
		, uint16_t _height
		, uint16_t _depth
		, bool _cubeMap
		, bool _hasMips
		, uint16_t _numLayers
		, TextureFormat::Enum _format
		);

	///
	void imageSolid(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _solid
		);

	///
	void imageCheckerboard(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _step
		, uint32_t _0
		, uint32_t _1
		);

	///
	void imageRgba8Downsample2x2(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _srcPitch
		, uint32_t _dstPitch
		, const void* _src
		);

	///
	void imageRgba32fToLinear(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _srcPitch
		, const void* _src
		);

	///
	void imageRgba32fToLinear(ImageContainer* _imageContainer);

	///
	void imageRgba32fToGamma(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _srcPitch
		, const void* _src
		);

	///
	void imageRgba32fToGamma(ImageContainer* _imageContainer);

	///
	void imageRgba32fLinearDownsample2x2(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _srcPitch
		, const void* _src
		);

	///
	void imageRgba32fDownsample2x2(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _srcPitch
		, const void* _src
		);

	///
	void imageRgba32fDownsample2x2NormalMap(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _srcPitch
		, uint32_t _dstPitch
		, const void* _src
		);

	///
	void imageSwizzleBgra8(
		  void* _dst
		, uint32_t _dstPitch
		, uint32_t _width
		, uint32_t _height
		, const void* _src
		, uint32_t _srcPitch
		);

	///
	void imageCopy(
		  void* _dst
		, uint32_t _height
		, uint32_t _srcPitch
		, uint32_t _depth
		, const void* _src
		, uint32_t _dstPitch
		);

	///
	void imageCopy(
		  void* _dst
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _bpp
		, uint32_t _srcPitch
		, const void* _src
		);

	///
	PackFn getPack(TextureFormat::Enum _format);

	///
	UnpackFn getUnpack(TextureFormat::Enum _format);

	///
	bool imageConvert(
		  TextureFormat::Enum _dstFormat
		, TextureFormat::Enum _srcFormat
		);

	///
	void imageConvert(
		  void* _dst
		, uint32_t _bpp
		, PackFn _pack
		, const void* _src
		, UnpackFn _unpack
		, uint32_t _size
		);

	///
	void imageConvert(
		  void* _dst
		, uint32_t _dstBpp
		, PackFn _pack
		, const void* _src
		, uint32_t _srcBpp
		, UnpackFn _unpack
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _srcPitch
		);

	///
	bool imageConvert(
		  bx::AllocatorI* _allocator
		, void* _dst
		, TextureFormat::Enum _dstFormat
		, const void* _src
		, TextureFormat::Enum _srcFormat
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		);

	///
	ImageContainer* imageConvert(
		  bx::AllocatorI* _allocator
		, TextureFormat::Enum _dstFormat
		, const void* _src
		, uint32_t _size
		);

	///
	ImageContainer* imageConvert(
		  bx::AllocatorI* _allocator
		, TextureFormat::Enum _dstFormat
		, const ImageContainer& _input
		, bool _convertMips = true
		);

	///
	ImageContainer* imageAlloc(
		  bx::AllocatorI* _allocator
		, TextureFormat::Enum _format
		, uint16_t _width
		, uint16_t _height
		, uint16_t _depth
		, uint16_t _numLayers
		, bool _cubeMap
		, bool _hasMips
		, const void* _data = NULL
		);

	///
	void imageFree(
		  ImageContainer* _imageContainer
		);

	///
	int32_t imageWriteTga(
		  bx::WriterI* _writer
		, uint32_t _width
		, uint32_t _height
		, uint32_t _srcPitch
		, const void* _src
		, bool _grayscale
		, bool _yflip
		, bx::Error* _err = NULL
		);

	///
	int32_t imageWritePng(
		  bx::WriterI* _writer
		, uint32_t _width
		, uint32_t _height
		, uint32_t _srcPitch
		, const void* _src
		, TextureFormat::Enum _format
		, bool _yflip
		, bx::Error* _err = NULL
		);

	///
	int32_t imageWriteExr(
		  bx::WriterI* _writer
		, uint32_t _width
		, uint32_t _height
		, uint32_t _srcPitch
		, const void* _src
		, TextureFormat::Enum _format
		, bool _yflip
		, bx::Error* _err
		);

	///
	int32_t imageWriteHdr(
		  bx::WriterI* _writer
		, uint32_t _width
		, uint32_t _height
		, uint32_t _srcPitch
		, const void* _src
		, TextureFormat::Enum _format
		, bool _yflip
		, bx::Error* _err
		);

	///
	int32_t imageWriteDds(
		  bx::WriterI* _writer
		, ImageContainer& _imageContainer
		, const void* _data
		, uint32_t _size
		, bx::Error* _err
		);

	///
	int32_t imageWriteKtx(
		  bx::WriterI* _writer
		, TextureFormat::Enum _format
		, bool _cubeMap
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint8_t _numMips
		, uint32_t _numLayers
		, const void* _src
		, bx::Error* _err = NULL
		);

	///
	int32_t imageWriteKtx(
		  bx::WriterI* _writer
		, ImageContainer& _imageContainer
		, const void* _data
		, uint32_t _size
		, bx::Error* _err = NULL
		);

	///
	bool imageParse(
		  ImageContainer& _imageContainer
		, bx::ReaderSeekerI* _reader
		, bx::Error* _err
		);

	///
	bool imageParse(
		  ImageContainer& _imageContainer
		, const void* _data
		, uint32_t _size
		, bx::Error* _err = NULL
		);

	///
	ImageContainer* imageParseDds(
		  bx::AllocatorI* _allocator
		, const void* _src
		, uint32_t _size
		, bx::Error* _err
		);

	///
	ImageContainer* imageParseKtx(
		  bx::AllocatorI* _allocator
		, const void* _src
		, uint32_t _size
		, bx::Error* _err
		);

	///
	ImageContainer* imageParsePvr3(
		  bx::AllocatorI* _allocator
		, const void* _src
		, uint32_t _size
		, bx::Error* _err
		);

	///
	ImageContainer* imageParseGnf(
		  bx::AllocatorI* _allocator
		, const void* _src
		, uint32_t _size
		, bx::Error* _err
		);

	///
	void imageDecodeToR8(
		  bx::AllocatorI* _allocator
		, void* _dst
		, const void* _src
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _dstPitch
		, TextureFormat::Enum _srcFormat
	);

	///
	void imageDecodeToBgra8(
		  bx::AllocatorI* _allocator
		, void* _dst
		, const void* _src
		, uint32_t _width
		, uint32_t _height
		, uint32_t _dstPitch
		, TextureFormat::Enum _format
		);

	///
	void imageDecodeToRgba8(
		  bx::AllocatorI* _allocator
		, void* _dst
		, const void* _src
		, uint32_t _width
		, uint32_t _height
		, uint32_t _dstPitch
		, TextureFormat::Enum _format
		);

	///
	void imageDecodeToRgba32f(
		  bx::AllocatorI* _allocator
		, void* _dst
		, const void* _src
		, uint32_t _width
		, uint32_t _height
		, uint32_t _depth
		, uint32_t _dstPitch
		, TextureFormat::Enum _format
		);

	///
	bool imageGetRawData(
		  const ImageContainer& _imageContainer
		, uint16_t _side
		, uint8_t _lod
		, const void* _data
		, uint32_t _size
		, ImageMip& _mip
		);

} // namespace bimg

#endif // BIMG_IMAGE_H_HEADER_GUARD
