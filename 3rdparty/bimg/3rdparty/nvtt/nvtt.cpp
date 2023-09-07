/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#include "nvtt.h"

#include <string.h>
#include <bx/uint32_t.h>

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4458) // warning C4458: declaration of 'x' hides class member
#include "bc6h/zoh.h"
#include "bc7/avpcl.h"
#include "nvmath/vector.inl"
BX_PRAGMA_DIAGNOSTIC_POP();

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4702) // warning C4702: unreachable code
NVCORE_API int nvAbort(const char *, const char *, int , const char *, const char *, ...)
{
	abort();
	return 0;
}
BX_PRAGMA_DIAGNOSTIC_POP();

namespace nvtt
{
	void compressBC6H(const void* _input, uint32_t _width, uint32_t _height, uint32_t _srcStride, void* _output)
	{
		const uint8_t* src = (const uint8_t*)_input;
		char* dst = (char*)_output;

		for (uint32_t yy = 0; yy < _height; yy += ZOH::Tile::TILE_H)
		{
			for (uint32_t xx = 0; xx < _width; xx += ZOH::Tile::TILE_H)
			{
				const uint32_t bytesPerPixel = sizeof(float)*4;
				const nv::Vector4* srcRgba   = (const nv::Vector4*)&src[yy*_srcStride + xx*bytesPerPixel];
				const uint32_t srcRgbaStride = _srcStride/bytesPerPixel;

				ZOH::Utils::FORMAT = ZOH::SIGNED_F16;
				ZOH::Tile zohTile(ZOH::Tile::TILE_H, ZOH::Tile::TILE_H);

				bx::memSet(zohTile.data, 0, sizeof(zohTile.data) );
				bx::memSet(zohTile.importance_map, 0, sizeof(zohTile.importance_map) );

				for (uint32_t blockY = 0; blockY < ZOH::Tile::TILE_H; ++blockY)
				{
					for (uint32_t blockX = 0; blockX < ZOH::Tile::TILE_W; ++blockX)
					{
						nv::Vector4 color = srcRgba[blockY*srcRgbaStride + blockX];
						zohTile.data[blockY][blockX].x = float(int16_t(bx::halfFromFloat(bx::max(color.x, 0.0f) ) ) );
						zohTile.data[blockY][blockX].y = float(int16_t(bx::halfFromFloat(bx::max(color.y, 0.0f) ) ) );
						zohTile.data[blockY][blockX].z = float(int16_t(bx::halfFromFloat(bx::max(color.z, 0.0f) ) ) );
					}
				}

				zohTile.generate_importance_map();
				ZOH::compress(zohTile, dst);
				dst += ZOH::BLOCKSIZE;
			}
		}
	}

	void compressBC7(const void* _input, uint32_t _width, uint32_t _height, uint32_t _srcStride, void* _output)
	{
		const uint8_t* src = (const uint8_t*)_input;
		char* dst = (char*)_output;

		for (uint32_t yy = 0; yy < _height; yy += 4)
		{
			for (uint32_t xx = 0; xx < _width; xx += 4)
			{
				const uint32_t bytesPerPixel = sizeof(float) * 4;
				const nv::Vector4* srcRgba   = (const nv::Vector4*)&src[yy*_srcStride + xx*bytesPerPixel];
				const uint32_t srcRgbaStride = _srcStride / bytesPerPixel;

				AVPCL::mode_rgb     = false;
				AVPCL::flag_premult = false;
				AVPCL::flag_nonuniform     = false;
				AVPCL::flag_nonuniform_ati = false;

				AVPCL::Tile avpclTile(4, 4);
				bx::memSet(avpclTile.data, 0, sizeof(avpclTile.data) );
				for (uint32_t blockY = 0; blockY < 4; ++blockY)
				{
					for (uint32_t blockX = 0; blockX < 4; ++blockX)
					{
						nv::Vector4 color = srcRgba[blockY*srcRgbaStride + blockX];
						avpclTile.data[blockY][blockX] = color * 255.0f;
						avpclTile.importance_map[blockY][blockX] = 1.0f;
					}
				}

				AVPCL::compress(avpclTile, dst);
				dst += AVPCL::BLOCKSIZE;
			}
		}
	}

} //namespace nvtt
