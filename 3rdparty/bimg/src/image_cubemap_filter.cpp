/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#include "bimg_p.h"
#include <bimg/encode.h>
#include <bx/rng.h>

namespace bimg
{
	/*
	 * Copyright 2014-2015 Dario Manesku. All rights reserved.
	 * License: http://www.opensource.org/licenses/BSD-2-Clause
	 */

	//                  +----------+
	//                  |-z       2|
	//                  | ^  +y    |
	//                  | |        |
	//                  | +---->+x |
	//       +----------+----------+----------+----------+
	//       |+y       1|+y       4|+y       0|+y       5|
	//       | ^  -x    | ^  +z    | ^  +x    | ^  -z    |
	//       | |        | |        | |        | |        |
	//       | +---->+z | +---->+x | +---->-z | +---->-x |
	//       +----------+----------+----------+----------+
	//                  |+z       3|
	//                  | ^  -y    |
	//                  | |        |
	//                  | +---->+x |
	//                  +----------+
	//
	struct CubeMapFace
	{
		enum Enum
		{
			PositiveX,
			NegativeX,
			PositiveY,
			NegativeY,
			PositiveZ,
			NegativeZ,

			Count
		};

		struct Edge
		{
			enum Enum
			{
				Left,
				Right,
				Top,
				Bottom,

				Count
			};
		};

		//    --> U    _____
		//   |        |     |
		//   v        | +Y  |
		//   V   _____|_____|_____ _____
		//      |     |     |     |     |
		//      | -X  | +Z  | +X  | -Z  |
		//      |_____|_____|_____|_____|
		//            |     |
		//            | -Y  |
		//            |_____|
		//
		// Neighbour faces in order: left, right, top, bottom.
		// FaceEdge is the edge that belongs to the neighbour face.
		struct Neighbour
		{
			uint8_t m_faceIdx;
			uint8_t m_faceEdge;
		};

		bx::Vec3 uv[3];
	};

	static const CubeMapFace s_cubeMapFace[] =
	{
		{{ // +x face
			{  0.0f,  0.0f, -1.0f }, // u -> -z
			{  0.0f, -1.0f,  0.0f }, // v -> -y
			{  1.0f,  0.0f,  0.0f }, // +x face
		}},
		{{ // -x face
			{  0.0f,  0.0f,  1.0f }, // u -> +z
			{  0.0f, -1.0f,  0.0f }, // v -> -y
			{ -1.0f,  0.0f,  0.0f }, // -x face
		}},
		{{ // +y face
			{  1.0f,  0.0f,  0.0f }, // u -> +x
			{  0.0f,  0.0f,  1.0f }, // v -> +z
			{  0.0f,  1.0f,  0.0f }, // +y face
		}},
		{{ // -y face
			{  1.0f,  0.0f,  0.0f }, // u -> +x
			{  0.0f,  0.0f, -1.0f }, // v -> -z
			{  0.0f, -1.0f,  0.0f }, // -y face
		}},
		{{ // +z face
			{  1.0f,  0.0f,  0.0f }, // u -> +x
			{  0.0f, -1.0f,  0.0f }, // v -> -y
			{  0.0f,  0.0f,  1.0f }, // +z face
		}},
		{{ // -z face
			{ -1.0f,  0.0f,  0.0f }, // u -> -x
			{  0.0f, -1.0f,  0.0f }, // v -> -y
			{  0.0f,  0.0f, -1.0f }, // -z face
		}},
	};

	static const CubeMapFace::Neighbour s_cubeMapFaceNeighbours[6][4] =
	{
		{ // +X
			{ CubeMapFace::PositiveZ, CubeMapFace::Edge::Right  },
			{ CubeMapFace::NegativeZ, CubeMapFace::Edge::Left   },
			{ CubeMapFace::PositiveY, CubeMapFace::Edge::Right  },
			{ CubeMapFace::NegativeY, CubeMapFace::Edge::Right  },
		},
		{ // -X
			{ CubeMapFace::NegativeZ, CubeMapFace::Edge::Right  },
			{ CubeMapFace::PositiveZ, CubeMapFace::Edge::Left   },
			{ CubeMapFace::PositiveY, CubeMapFace::Edge::Left   },
			{ CubeMapFace::NegativeY, CubeMapFace::Edge::Left   },
		},
		{ // +Y
			{ CubeMapFace::NegativeX, CubeMapFace::Edge::Top    },
			{ CubeMapFace::PositiveX, CubeMapFace::Edge::Top    },
			{ CubeMapFace::NegativeZ, CubeMapFace::Edge::Top    },
			{ CubeMapFace::PositiveZ, CubeMapFace::Edge::Top    },
		},
		{ // -Y
			{ CubeMapFace::NegativeX, CubeMapFace::Edge::Bottom },
			{ CubeMapFace::PositiveX, CubeMapFace::Edge::Bottom },
			{ CubeMapFace::PositiveZ, CubeMapFace::Edge::Bottom },
			{ CubeMapFace::NegativeZ, CubeMapFace::Edge::Bottom },
		},
		{ // +Z
			{ CubeMapFace::NegativeX, CubeMapFace::Edge::Right  },
			{ CubeMapFace::PositiveX, CubeMapFace::Edge::Left   },
			{ CubeMapFace::PositiveY, CubeMapFace::Edge::Bottom },
			{ CubeMapFace::NegativeY, CubeMapFace::Edge::Top    },
		},
		{ // -Z
			{ CubeMapFace::PositiveX, CubeMapFace::Edge::Right  },
			{ CubeMapFace::NegativeX, CubeMapFace::Edge::Left   },
			{ CubeMapFace::PositiveY, CubeMapFace::Edge::Top    },
			{ CubeMapFace::NegativeY, CubeMapFace::Edge::Bottom },
		},
	};

	/// _u and _v should be center addressing and in [-1.0+invSize..1.0-invSize] range.
	bx::Vec3 texelUvToDir(uint8_t _side, float _u, float _v)
	{
		const CubeMapFace& face = s_cubeMapFace[_side];

		const bx::Vec3 tmp =
		{
			face.uv[0].x * _u + face.uv[1].x * _v + face.uv[2].x,
			face.uv[0].y * _u + face.uv[1].y * _v + face.uv[2].y,
			face.uv[0].z * _u + face.uv[1].z * _v + face.uv[2].z,
		};

		return bx::normalize(tmp);
	}

	void dirToTexelUv(float& _outU, float& _outV, uint8_t& _outSide, const bx::Vec3& _dir)
	{
		const bx::Vec3 absVec = bx::abs(_dir);
		const float max = bx::max(absVec.x, absVec.y, absVec.z);

		if (max == absVec.x)
		{
			_outSide = _dir.x >= 0.0f ? uint8_t(CubeMapFace::PositiveX) : uint8_t(CubeMapFace::NegativeX);
		}
		else if (max == absVec.y)
		{
			_outSide = _dir.y >= 0.0f ? uint8_t(CubeMapFace::PositiveY) : uint8_t(CubeMapFace::NegativeY);
		}
		else
		{
			_outSide = _dir.z >= 0.0f ? uint8_t(CubeMapFace::PositiveZ) : uint8_t(CubeMapFace::NegativeZ);
		}

		const bx::Vec3 faceVec = bx::mul(_dir, 1.0f/max);

		_outU = (bx::dot(s_cubeMapFace[_outSide].uv[0], faceVec) + 1.0f) * 0.5f;
		_outV = (bx::dot(s_cubeMapFace[_outSide].uv[1], faceVec) + 1.0f) * 0.5f;
	}

	ImageContainer* imageCubemapFromLatLongRgba32F(bx::AllocatorI* _allocator, const ImageContainer& _input, bool _useBilinearInterpolation, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		if (_input.m_depth     != 1
		&&  _input.m_numLayers != 1
		&&  _input.m_format    != TextureFormat::RGBA32F
		&&  _input.m_width/2   != _input.m_height)
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "Input image format is not equirectangular projection.");
			return NULL;
		}

		const uint32_t srcWidthMinusOne  = _input.m_width-1;
		const uint32_t srcHeightMinusOne = _input.m_height-1;
		const uint32_t srcPitch = _input.m_width*16;
		const uint32_t dstWidth = _input.m_height/2;
		const uint32_t dstPitch = dstWidth*16;
		const float invDstWidth = 1.0f / float(dstWidth);

		ImageContainer* output = imageAlloc(_allocator
			, _input.m_format
			, uint16_t(dstWidth)
			, uint16_t(dstWidth)
			, uint16_t(1)
			, 1
			, true
			, false
			);

		const uint8_t* srcData = (const uint8_t*)_input.m_data;

		for (uint8_t side = 0; side < 6 && _err->isOk(); ++side)
		{
			ImageMip mip;
			imageGetRawData(*output, side, 0, output->m_data, output->m_size, mip);

			for (uint32_t yy = 0; yy < dstWidth; ++yy)
			{
				for (uint32_t xx = 0; xx < dstWidth; ++xx)
				{
					float* dstData = (float*)&mip.m_data[yy*dstPitch+xx*16];

					const float uu = 2.0f*xx*invDstWidth - 1.0f;
					const float vv = 2.0f*yy*invDstWidth - 1.0f;

					const bx::Vec3 dir = texelUvToDir(side, uu, vv);

					float srcU, srcV;
					bx::toLatLong(&srcU, &srcV, dir);

					srcU *= srcWidthMinusOne;
					srcV *= srcHeightMinusOne;

					if (_useBilinearInterpolation)
					{
						const uint32_t x0 = uint32_t(srcU);
						const uint32_t y0 = uint32_t(srcV);
						const uint32_t x1 = bx::min(x0 + 1, srcWidthMinusOne);
						const uint32_t y1 = bx::min(y0 + 1, srcHeightMinusOne);

						const float* src0 = (const float*)&srcData[y0*srcPitch + x0*16];
						const float* src1 = (const float*)&srcData[y0*srcPitch + x1*16];
						const float* src2 = (const float*)&srcData[y1*srcPitch + x0*16];
						const float* src3 = (const float*)&srcData[y1*srcPitch + x1*16];

						const float tx    = srcU - float(int32_t(x0) );
						const float ty    = srcV - float(int32_t(y0) );
						const float omtx  = 1.0f - tx;
						const float omty  = 1.0f - ty;

						const float p0x = omtx*omty;
						const float p0[4] =
						{
							src0[0] * p0x,
							src0[1] * p0x,
							src0[2] * p0x,
							src0[3] * p0x,
						};

						const float p1x = tx*omty;
						const float p1[4] =
						{
							src1[0] * p1x,
							src1[1] * p1x,
							src1[2] * p1x,
							src1[3] * p1x,
						};

						const float p2x = omtx*ty;
						const float p2[4] =
						{
							src2[0] * p2x,
							src2[1] * p2x,
							src2[2] * p2x,
							src2[3] * p2x,
						};

						const float p3x = tx*ty;
						const float p3[4] =
						{
							src3[0] * p3x,
							src3[1] * p3x,
							src3[2] * p3x,
							src3[3] * p3x,
						};

						const float rr = p0[0] + p1[0] + p2[0] + p3[0];
						const float gg = p0[1] + p1[1] + p2[1] + p3[1];
						const float bb = p0[2] + p1[2] + p2[2] + p3[2];
						const float aa = p0[3] + p1[3] + p2[3] + p3[3];

						dstData[0] = rr;
						dstData[1] = gg;
						dstData[2] = bb;
						dstData[3] = aa;
					}
					else
					{
						const uint32_t x0 = uint32_t(srcU);
						const uint32_t y0 = uint32_t(srcV);
						const float* src0 = (const float*)&srcData[y0*srcPitch + x0*16];

						dstData[0] = src0[0];
						dstData[1] = src0[1];
						dstData[2] = src0[2];
						dstData[3] = src0[3];
					}

				}
			}
		}

		return output;
	}

	ImageContainer* imageCubemapFromStripRgba32F(bx::AllocatorI* _allocator, const ImageContainer& _input, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		if (_input.m_depth     != 1
		&&  _input.m_numLayers != 1
		&&  _input.m_format    != TextureFormat::RGBA32F
		&& ( (_input.m_width   != _input.m_height*6) || (_input.m_width*6 != _input.m_height) ) )
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "Input image format is not strip projection.");
			return NULL;
		}

		const bool   horizontal = _input.m_width == _input.m_height*6;
		const uint32_t srcPitch = _input.m_width*16;
		const uint32_t dstWidth = horizontal ? _input.m_height : _input.m_width;
		const uint32_t dstPitch = dstWidth*16;
		const uint32_t step     = horizontal ? dstPitch : dstPitch*dstWidth;

		ImageContainer* output = imageAlloc(_allocator
			, _input.m_format
			, uint16_t(dstWidth)
			, uint16_t(dstWidth)
			, uint16_t(1)
			, 1
			, true
			, false
			);

		const uint8_t* srcData = (const uint8_t*)_input.m_data;

		for (uint8_t side = 0; side < 6 && _err->isOk(); ++side, srcData += step)
		{
			ImageMip dstMip;
			imageGetRawData(*output, side, 0, output->m_data, output->m_size, dstMip);

			bx::memCopy(const_cast<uint8_t*>(dstMip.m_data), dstPitch, srcData, srcPitch, dstPitch, dstWidth);
		}

		return output;
	}

	inline float areaElement(float _x, float _y)
	{
		return bx::atan2(_x*_y, bx::sqrt(_x*_x + _y*_y + 1.0f));
	}

	float texelSolidAngle(float _u, float _v, float _invFaceSize)
	{
		// Reference(s):
		// - https://web.archive.org/web/20180614195754/http://www.mpia.de/~mathar/public/mathar20051002.pdf
		// - https://web.archive.org/web/20180614195725/http://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/
		//
		const float x0 = _u - _invFaceSize;
		const float x1 = _u + _invFaceSize;
		const float y0 = _v - _invFaceSize;
		const float y1 = _v + _invFaceSize;

		return
			+ areaElement(x1, y1)
			- areaElement(x0, y1)
			- areaElement(x1, y0)
			+ areaElement(x0, y0)
			;
	}

	ImageContainer* imageCubemapNormalSolidAngle(bx::AllocatorI* _allocator, uint32_t _size)
	{
		const uint32_t dstWidth = _size;
		const uint32_t dstPitch = dstWidth*16;
		const float texelSize   = 1.0f / float(dstWidth);

		ImageContainer* output = imageAlloc(_allocator, TextureFormat::RGBA32F, uint16_t(dstWidth), uint16_t(dstWidth), 1, 1, true, false);

		for (uint8_t side = 0; side < 6; ++side)
		{
			ImageMip mip;
			imageGetRawData(*output, side, 0, output->m_data, output->m_size, mip);

			for (uint32_t yy = 0; yy < dstWidth; ++yy)
			{
				for (uint32_t xx = 0; xx < dstWidth; ++xx)
				{
					float* dstData = (float*)&mip.m_data[yy*dstPitch+xx*16];

					const float uu = float(xx)*texelSize*2.0f - 1.0f;
					const float vv = float(yy)*texelSize*2.0f - 1.0f;

					bx::store(dstData, texelUvToDir(side, uu, vv) );
					dstData[3] = texelSolidAngle(uu, vv, texelSize);
				}
			}
		}

		return output;
	}

	struct Aabb
	{
		Aabb()
		{
			m_min[0] = bx::max<float>();
			m_min[1] = bx::max<float>();
			m_max[0] = bx::min<float>();
			m_max[1] = bx::min<float>();
		}

		void add(float _x, float _y)
		{
			m_min[0] = bx::min(m_min[0], _x);
			m_min[1] = bx::min(m_min[1], _y);
			m_max[0] = bx::max(m_max[0], _x);
			m_max[1] = bx::max(m_max[1], _y);
		}

		void clamp(float _min, float _max)
		{
			m_min[0] = bx::clamp(m_min[0], _min, _max);
			m_min[1] = bx::clamp(m_min[1], _min, _max);
			m_max[0] = bx::clamp(m_max[0], _min, _max);
			m_max[1] = bx::clamp(m_max[1], _min, _max);
		}

		bool isEmpty() const
		{
			// Has to have at least two points added so that no value is equal to initial state.
			return ( (m_min[0] == bx::max<float>() )
				||   (m_min[1] == bx::max<float>() )
				||   (m_max[0] == bx::min<float>() )
				||   (m_max[1] == bx::min<float>() )
				);
		}

		float m_min[2];
		float m_max[2];
	};

	void calcFilterArea(Aabb* _outFilterArea, const bx::Vec3& _dir, float _filterSize)
	{
		///   ______
		///  |      |
		///  |      |
		///  |    x |
		///  |______|
		///
		// Get face and hit coordinates.
		float uu, vv;
		uint8_t hitFaceIdx;
		dirToTexelUv(uu, vv, hitFaceIdx, _dir);

		///  ........
		///  .      .
		///  .   ___.
		///  .  | x |
		///  ...|___|
		///
		// Calculate hit face filter bounds.
		Aabb hitFaceFilterBounds;
		hitFaceFilterBounds.add(uu-_filterSize, vv-_filterSize);
		hitFaceFilterBounds.add(uu+_filterSize, vv+_filterSize);
		hitFaceFilterBounds.clamp(0.0f, 1.0f);

		// Output result for hit face.
		bx::memCopy(&_outFilterArea[hitFaceIdx], &hitFaceFilterBounds, sizeof(Aabb));

		/// Filter area might extend on neighbour faces.
		/// Case when extending over the right edge:
		///
		///  --> U
		/// |        ......
		/// v       .      .
		/// V       .      .
		///         .      .
		///  ....... ...... .......
		///  .      .      .      .
		///  .      .  .....__min .
		///  .      .  .   .  |  -> amount
		///  ....... .....x.__|....
		///         .  .   .  max
		///         .  ........
		///         .      .
		///          ......
		///         .      .
		///         .      .
		///         .      .
		///          ......
		///

		struct NeighourFaceBleed
		{
			float m_amount;
			float m_bbMin;
			float m_bbMax;
		};

		const NeighourFaceBleed bleed[CubeMapFace::Edge::Count] =
		{
			{ // Left
				_filterSize - uu,
				hitFaceFilterBounds.m_min[1],
				hitFaceFilterBounds.m_max[1],
			},
			{ // Right
				uu + _filterSize - 1.0f,
				hitFaceFilterBounds.m_min[1],
				hitFaceFilterBounds.m_max[1],
			},
			{ // Top
				_filterSize - vv,
				hitFaceFilterBounds.m_min[0],
				hitFaceFilterBounds.m_max[0],
			},
			{ // Bottom
				vv + _filterSize - 1.0f,
				hitFaceFilterBounds.m_min[0],
				hitFaceFilterBounds.m_max[0],
			},
		};

		// Determine bleeding for each side.
		for (uint8_t side = 0; side < 4; ++side)
		{
			uint8_t currentFaceIdx = hitFaceIdx;

			for (float bleedAmount = bleed[side].m_amount; bleedAmount > 0.0f; bleedAmount -= 1.0f)
			{
				uint8_t neighbourFaceIdx  = s_cubeMapFaceNeighbours[currentFaceIdx][side].m_faceIdx;
				uint8_t neighbourFaceEdge = s_cubeMapFaceNeighbours[currentFaceIdx][side].m_faceEdge;
				currentFaceIdx = neighbourFaceIdx;

				/// https://code.google.com/p/cubemapgen/source/browse/trunk/CCubeMapProcessor.cpp#773
				///
				/// Handle situations when bbMin and bbMax should be flipped.
				///
				///    L - Left           ....................T-T
				///    R - Right          v                     .
				///    T - Top        __________                .
				///    B - Bottom    .          |               .
				///                  .          |               .
				///                  .          |<...R-T        .
				///                  .          |    v          v
				///        .......... ..........|__________ __________
				///       .          .          .          .          .
				///       .          .          .          .          .
				///       .          .          .          .          .
				///       .          .          .          .          .
				///        __________ .......... .......... __________
				///            ^     |          .               ^
				///            .     |          .               .
				///            B-L..>|          .               .
				///                  |          .               .
				///                  |__________.               .
				///                       ^                     .
				///                       ....................B-B
				///
				/// Those are:
				///     B-L, B-B
				///     T-R, T-T
				///     (and in reverse order, R-T and L-B)
				///
				/// If we add, R-R and L-L (which never occur), we get:
				///     B-L, B-B
				///     T-R, T-T
				///     R-T, R-R
				///     L-B, L-L
				///
				/// And if L = 0, R = 1, T = 2, B = 3 as in NeighbourSides enumeration,
				/// a general rule can be derived for when to flip bbMin and bbMax:
				///     if ((a+b) == 3 || (a == b))
				///     {
				///        ..flip bbMin and bbMax
				///     }
				///
				float bbMin = bleed[side].m_bbMin;
				float bbMax = bleed[side].m_bbMax;
				if ( (side == neighbourFaceEdge)
				||   (3    == (side + neighbourFaceEdge) ) )
				{
					// Flip.
					bbMin = 1.0f - bbMin;
					bbMax = 1.0f - bbMax;
				}

				switch (neighbourFaceEdge)
				{
				case CubeMapFace::Edge::Left:
					{
						///  --> U
						/// |  .............
						/// v  .           .
						/// V  x___        .
						///    |   |       .
						///    |   |       .
						///    |___x       .
						///    .           .
						///    .............
						///
						_outFilterArea[neighbourFaceIdx].add(0.0f, bbMin);
						_outFilterArea[neighbourFaceIdx].add(bleedAmount, bbMax);
					}
					break;

				case CubeMapFace::Edge::Right:
					{
						///  --> U
						/// |  .............
						/// v  .           .
						/// V  .       x___.
						///    .       |   |
						///    .       |   |
						///    .       |___x
						///    .           .
						///    .............
						///
						_outFilterArea[neighbourFaceIdx].add(1.0f - bleedAmount, bbMin);
						_outFilterArea[neighbourFaceIdx].add(1.0f, bbMax);
					}
					break;

				case CubeMapFace::Edge::Top:
					{
						///  --> U
						/// |  ...x____ ...
						/// v  .  |    |  .
						/// V  .  |____x  .
						///    .          .
						///    .          .
						///    .          .
						///    ............
						///
						_outFilterArea[neighbourFaceIdx].add(bbMin, 0.0f);
						_outFilterArea[neighbourFaceIdx].add(bbMax, bleedAmount);
					}
					break;

				case CubeMapFace::Edge::Bottom:
					{
						///  --> U
						/// |  ............
						/// v  .          .
						/// V  .          .
						///    .          .
						///    .  x____   .
						///    .  |    |  .
						///    ...|____x...
						///
						_outFilterArea[neighbourFaceIdx].add(bbMin, 1.0f - bleedAmount);
						_outFilterArea[neighbourFaceIdx].add(bbMax, 1.0f);
					}
					break;
				}

				// Clamp bounding box to face size.
				_outFilterArea[neighbourFaceIdx].clamp(0.0f, 1.0f);
			}
		}
	}

	struct Sampler
	{
		Sampler(const ImageContainer& _image, uint16_t _side, float _lod, float (*func)(float) )
		{
			const float lod = bx::clamp(_lod, 0.0f, float(_image.m_numMips - 1) );
			imageGetRawData(
				  _image
				, _side
				, uint8_t(func(lod) )
				, _image.m_data
				, _image.m_size
				, mip
				);
		}

		ImageMip mip;
	};

	void texelFetch(float* _rgba, const Sampler& _sampler, uint32_t _u, uint32_t _v)
	{
		const uint32_t bpp   = _sampler.mip.m_bpp;
		const uint32_t pitch = _sampler.mip.m_width*bpp/8;
		const uint8_t* texel = _sampler.mip.m_data + _v*pitch + _u*bpp/8;

		UnpackFn unpack = getUnpack(_sampler.mip.m_format);
		unpack(_rgba, texel);
	}

	void sampleCubeMap(float* _rgba, const ImageContainer& _image, const bx::Vec3& _dir, float _lod)
	{
		float uu, vv;
		uint8_t side;
		dirToTexelUv(uu, vv, side, _dir);

		const float fu = bx::fract(uu);
		const float fv = bx::fract(vv);
		const float fl = bx::fract(_lod);

		float rgbaA[4];
		{
			Sampler sampler(_image, side, _lod, bx::floor);
			const uint32_t widthMinusOne = sampler.mip.m_width-1;

			const uint32_t u0 = uint32_t(uu*widthMinusOne+0.5f);
			const uint32_t v0 = uint32_t(vv*widthMinusOne+0.5f);
			const uint32_t u1 = bx::min(u0 + 1, widthMinusOne);
			const uint32_t v1 = bx::min(v0 + 1, widthMinusOne);

			float rgba00[4];
			texelFetch(rgba00, sampler, u0, v0);

			float rgba01[4];
			texelFetch(rgba01, sampler, u0, v1);

			float rgba10[4];
			texelFetch(rgba10, sampler, u1, v0);

			float rgba11[4];
			texelFetch(rgba11, sampler, u1, v1);

			rgbaA[0] = bx::lerp(bx::lerp(rgba00[0], rgba01[0], fv), bx::lerp(rgba10[0], rgba11[0], fv), fu);
			rgbaA[1] = bx::lerp(bx::lerp(rgba00[1], rgba01[1], fv), bx::lerp(rgba10[1], rgba11[1], fv), fu);
			rgbaA[2] = bx::lerp(bx::lerp(rgba00[2], rgba01[2], fv), bx::lerp(rgba10[2], rgba11[2], fv), fu);
			rgbaA[3] = bx::lerp(bx::lerp(rgba00[3], rgba01[3], fv), bx::lerp(rgba10[3], rgba11[3], fv), fu);
		}

		float rgbaB[4];
		{
			Sampler sampler(_image, side, _lod, bx::ceil);
			const uint32_t widthMinusOne = sampler.mip.m_width-1;

			const uint32_t u0 = uint32_t(uu*widthMinusOne+0.5f);
			const uint32_t v0 = uint32_t(vv*widthMinusOne+0.5f);
			const uint32_t u1 = bx::min(u0 + 1, widthMinusOne);
			const uint32_t v1 = bx::min(v0 + 1, widthMinusOne);

			float rgba00[4];
			texelFetch(rgba00, sampler, u0, v0);

			float rgba01[4];
			texelFetch(rgba01, sampler, u0, v1);

			float rgba10[4];
			texelFetch(rgba10, sampler, u1, v0);

			float rgba11[4];
			texelFetch(rgba11, sampler, u1, v1);

			rgbaB[0] = bx::lerp(bx::lerp(rgba00[0], rgba01[0], fv), bx::lerp(rgba10[0], rgba11[0], fv), fu);
			rgbaB[1] = bx::lerp(bx::lerp(rgba00[1], rgba01[1], fv), bx::lerp(rgba10[1], rgba11[1], fv), fu);
			rgbaB[2] = bx::lerp(bx::lerp(rgba00[2], rgba01[2], fv), bx::lerp(rgba10[2], rgba11[2], fv), fu);
			rgbaB[3] = bx::lerp(bx::lerp(rgba00[3], rgba01[3], fv), bx::lerp(rgba10[3], rgba11[3], fv), fu);
		}

		_rgba[0] = bx::lerp(rgbaA[0], rgbaB[0], fl);
		_rgba[1] = bx::lerp(rgbaA[1], rgbaB[1], fl);
		_rgba[2] = bx::lerp(rgbaA[2], rgbaB[2], fl);
		_rgba[3] = bx::lerp(rgbaA[3], rgbaB[3], fl);
	}

	bx::Vec3 importanceSampleGgx(float _u, float _v, float _roughness, const bx::Vec3& _normal, const bx::Vec3& _tangentX, const bx::Vec3& _tangentY)
	{
		const float aa  = bx::square(_roughness);
		const float phi = bx::kPi2 * _u;
		const float cosTheta = bx::sqrt( (1.0f - _v) / (1.0f + (bx::square(aa) - 1.0f) * _v) );
		const float sinTheta = bx::sqrt(bx::abs(1.0f - bx::square(cosTheta) ) );

		const float hh[3] =
		{
			sinTheta * bx::cos(phi),
			sinTheta * bx::sin(phi),
			cosTheta,
		};

		return
		{
			_tangentX.x * hh[0] + _tangentY.x * hh[1] + _normal.x * hh[2],
			_tangentX.y * hh[0] + _tangentY.y * hh[1] + _normal.y * hh[2],
			_tangentX.z * hh[0] + _tangentY.z * hh[1] + _normal.z * hh[2],
		};
	}

	float normalDistributionGgx(float _ndoth, float _roughness)
	{
		const float alpha   = bx::square(_roughness);
		const float alphaSq = bx::square(alpha);
		const float denom   = bx::square(_ndoth) * (alphaSq - 1.0f) + 1.0f;
		const float denomSq = bx::square(denom);
		return alphaSq/(bx::kPi * denomSq);
	}

	void processFilterAreaGgx(
		  float* _result
		, const ImageContainer& _image
		, uint8_t _lod
		, const bx::Vec3& _dir
		, float _roughness
		)
	{
		ImageMip mip;
		imageGetRawData(_image, 0, _lod, _image.m_data, _image.m_size, mip);

		const uint32_t bpp = getBitsPerPixel(_image.m_format);

		constexpr int32_t kNumSamples = 512;
		const uint32_t pitch      = mip.m_width*bpp/8;
		const float widthMinusOne = float(mip.m_width-1);
		const float mipBias       = 0.5f*bx::log2(bx::square(float(_image.m_width) )/float(kNumSamples) );

		UnpackFn unpack = getUnpack(_image.m_format);

		float color[3]    = { 0.0f, 0.0f, 0.0f };
		float totalWeight = 0.0f;

		// Golden Ratio Sequences for Low-Discrepancy Sampling
		// https://web.archive.org/web/20180717194847/https://www.graphics.rwth-aachen.de/publication/2/jgt.pdf
		//
		// kGoldenSection = (0.5f*bx::sqrt(5.0f) + 0.5f) - 1.0f = 0.61803398875f
		//
		const float kGoldenSection = 0.61803398875f;
		float offset = kGoldenSection;

		bx::Vec3 tangentX(bx::InitNone);
		bx::Vec3 tangentY(bx::InitNone);
		bx::calcTangentFrame(tangentX, tangentY, _dir);

		for (uint32_t ii = 0; ii < kNumSamples; ++ii)
		{
			offset += kGoldenSection;
			const float vv = ii/float(kNumSamples);

			const bx::Vec3 hh  = importanceSampleGgx(offset, vv, _roughness, _dir, tangentX, tangentY);
			const float ddoth2 = 2.0f * bx::dot(_dir, hh);
			const bx::Vec3 ll  = bx::sub(bx::mul(hh, ddoth2), _dir);

			const float ndotl = bx::clamp(bx::dot(_dir, ll), 0.0f, 1.0f);

			if (ndotl > 0.0f)
			{
				const float ndoth = bx::clamp(bx::dot(_dir, hh), 0.0f, 1.0f);
				const float vdoth = ndoth;

				// Chapter 20. GPU-Based Importance Sampling
				// http://archive.today/2018.07.14-004914/https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
				//
				const float pdf = normalDistributionGgx(ndoth, _roughness) * ndoth / (4.0f * vdoth);
				const float lod = bx::max(0.0f, mipBias - 0.5f*bx::log2(pdf));

				float rgba[4];
				sampleCubeMap(rgba, _image, ll, lod);

				// Optimized Reversible Tonemapper for Resolve
				// https://web.archive.org/web/20180717182019/https://gpuopen.com/optimized-reversible-tonemapper-for-resolve/
				// "a single sample with a large HDR value can over-power all other samples"
				// "instead accept a bias in the resolve and reduce the weighting of samples
				// as a function of how bright they are"
				// Include ndotl here to "fold the weighting into the tonemap operation"
				//
				const float tm = ndotl / (bx::max(rgba[0], rgba[1], rgba[2]) + 1.0f);

				color[0] += rgba[0] * tm;
				color[1] += rgba[1] * tm;
				color[2] += rgba[2] * tm;
				totalWeight += ndotl;
			}
		}

		if (0.0f < totalWeight)
		{
			// Optimized Reversible Tonemapper for Resolve
			// https://web.archive.org/web/20180717182019/https://gpuopen.com/optimized-reversible-tonemapper-for-resolve/
			// Average, then reverse the tonemapper
			//
			const float invWeight = 1.0f/totalWeight;
			color[0] = color[0] * invWeight;
			color[1] = color[1] * invWeight;
			color[2] = color[2] * invWeight;

			const float invTm = 1.0f / (1.0f - bx::max(0.00001f, bx::max(color[0], color[1], color[2])));
			_result[0] = color[0] * invTm;
			_result[1] = color[1] * invTm;
			_result[2] = color[2] * invTm;
		}
		else
		{
			float uu, vv;
			uint8_t face;
			dirToTexelUv(uu, vv, face, _dir);

			imageGetRawData(_image, face, 0, _image.m_data, _image.m_size, mip);

			const uint32_t xx = uint32_t(uu*widthMinusOne);
			const uint32_t yy = uint32_t(vv*widthMinusOne);

			float rgba[4];
			unpack(rgba, mip.m_data + yy*pitch + xx*bpp/8);

			_result[0] = rgba[0];
			_result[1] = rgba[1];
			_result[2] = rgba[2];
		}
	}

	void processFilterArea(
		  float* _result
		, const ImageContainer& _image
		, const ImageContainer& _nsa
		, uint8_t _lod
		, const Aabb* _aabb
		, const bx::Vec3& _dir
		, float _specularPower
		, float _specularAngle
		)
	{
		float color[3]    = { 0.0f, 0.0f, 0.0f };
		float totalWeight = 0.0f;

		const uint32_t bpp = getBitsPerPixel(_image.m_format);

		UnpackFn unpack = getUnpack(_image.m_format);

		for (uint8_t side = 0; side < 6; ++side)
		{
			if (_aabb[side].isEmpty() )
			{
				continue;
			}

			ImageMip nsaMip;
			imageGetRawData(_nsa, side, 0, _nsa.m_data, _nsa.m_size, nsaMip);

			ImageMip mip;
			if (imageGetRawData(_image, side, _lod, _image.m_data, _image.m_size, mip) )
			{
				const uint32_t pitch      = mip.m_width*bpp/8;
				const float widthMinusOne = float(mip.m_width-1);
				const float texelSize     = 1.0f/float(mip.m_width);
				BX_UNUSED(texelSize);

				const uint32_t minX = uint32_t(_aabb[side].m_min[0] * widthMinusOne);
				const uint32_t maxX = uint32_t(_aabb[side].m_max[0] * widthMinusOne);
				const uint32_t minY = uint32_t(_aabb[side].m_min[1] * widthMinusOne);
				const uint32_t maxY = uint32_t(_aabb[side].m_max[1] * widthMinusOne);

				for (uint32_t yy = minY; yy <= maxY; ++yy)
				{
					const uint8_t* row = mip.m_data + yy*pitch;
					BX_UNUSED(row);

					for (uint32_t xx = minX; xx <= maxX; ++xx)
					{
						const float* normal = (const float*)&nsaMip.m_data[(yy*nsaMip.m_width+xx)*(nsaMip.m_bpp/8)];
						const float solidAngle = normal[3];
						const float ndotl = bx::clamp(bx::dot(bx::load<bx::Vec3>(normal), _dir), 0.0f, 1.0f);

						if (ndotl >= _specularAngle)
						{
							const float weight = solidAngle * bx::pow(ndotl, _specularPower);
							float rgba[4];
							unpack(rgba, row + xx*bpp/8);

							color[0] += rgba[0] * weight;
							color[1] += rgba[1] * weight;
							color[2] += rgba[2] * weight;
							totalWeight += weight;
						}
					}
				}

				if (0.0f < totalWeight)
				{
					const float invWeight = 1.0f/totalWeight;
					_result[0] = color[0] * invWeight;
					_result[1] = color[1] * invWeight;
					_result[2] = color[2] * invWeight;
				}
				else
				{
					float uu, vv;
					uint8_t face;
					dirToTexelUv(uu, vv, face, _dir);

					imageGetRawData(_image, face, 0, _image.m_data, _image.m_size, mip);

					const uint32_t xx = uint32_t(uu*widthMinusOne);
					const uint32_t yy = uint32_t(vv*widthMinusOne);

					float rgba[4];
					unpack(rgba, mip.m_data + yy*pitch + xx*bpp/8);

					_result[0] = rgba[0];
					_result[1] = rgba[1];
					_result[2] = rgba[2];
				}
			}
		}
	}

	ImageContainer* imageGenerateMips(bx::AllocatorI* _allocator, const ImageContainer& _image)
	{
		if (_image.m_format != TextureFormat::RGBA8
		&&  _image.m_format != TextureFormat::RGBA32F)
		{
			return NULL;
		}

		ImageContainer* output = imageAlloc(_allocator, _image.m_format, uint16_t(_image.m_width), uint16_t(_image.m_height), uint16_t(_image.m_depth), _image.m_numLayers, _image.m_cubeMap, true);

		const uint32_t numMips   = output->m_numMips;
		const uint32_t numLayers = output->m_numLayers;
		const uint32_t numSides  = output->m_cubeMap ? 6 : 1;

		for (uint32_t layer = 0; layer < numLayers; ++layer)
		{
			for (uint8_t side = 0; side < numSides; ++side)
			{
				ImageMip mip;
				if (imageGetRawData(_image, uint16_t(layer*numSides + side), 0, _image.m_data, _image.m_size, mip) )
				{
					for (uint8_t lod = 0; lod < numMips; ++lod)
					{
						ImageMip srcMip;
						imageGetRawData(*output, uint16_t(layer*numSides + side), lod == 0 ? 0 : lod-1, output->m_data, output->m_size, srcMip);

						ImageMip dstMip;
						imageGetRawData(*output, uint16_t(layer*numSides + side), lod, output->m_data, output->m_size, dstMip);

						uint8_t* dstData = const_cast<uint8_t*>(dstMip.m_data);

						if (0 == lod)
						{
							bx::memCopy(dstData, mip.m_data, mip.m_size);
						}
						else if (output->m_format == TextureFormat::RGBA8)
						{
							imageRgba8Downsample2x2(
								  dstData
								, srcMip.m_width
								, srcMip.m_height
								, srcMip.m_depth
								, srcMip.m_width*4
								, dstMip.m_width*4
								, srcMip.m_data
								);
						}
						else if (output->m_format == TextureFormat::RGBA32F)
						{
							imageRgba32fDownsample2x2(
								  dstData
								, srcMip.m_width
								, srcMip.m_height
								, srcMip.m_depth
								, srcMip.m_width*16
								, srcMip.m_data
								);
						}
					}
				}
			}
		}

		return output;
	}

	/// Returns the angle of cosine power function where the results are above a small empirical treshold.
	static float cosinePowerFilterAngle(float _cosinePower)
	{
		// Bigger value leads to performance improvement but might hurt the results.
		// 0.00001f was tested empirically and it gives almost the same values as reference.
		const float treshold = 0.00001f;

		// Cosine power filter is: pow(cos(angle), power).
		// We want the value of the angle above each result is <= treshold.
		// So: angle = acos(pow(treshold, 1.0 / power))
		return bx::acos(bx::pow(treshold, 1.0f / _cosinePower));
	}

	float glossinessFor(float _mip, float _mipCount)
	{
		return bx::max(0.0f, 1.0f - _mip/(_mipCount-1.0000001f) );
	}

	float applyLightingModel(float _specularPower, LightingModel::Enum _lightingModel)
	{
		// Reference(s):
		// - https://web.archive.org/web/20180622232018/https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering/
		// - https://web.archive.org/web/20180622232041/https://seblagarde.wordpress.com/2012/03/29/relationship-between-phong-and-blinn-lighting-model/
		//
		switch (_lightingModel)
		{
		case LightingModel::PhongBrdf: return _specularPower + 1.0f;
		case LightingModel::Blinn:     return _specularPower/4.0f;
		case LightingModel::BlinnBrdf: return _specularPower/4.0f + 1.0f;
		default: break;
		};

		return _specularPower;
	}

	ImageContainer* imageCubemapRadianceFilter(bx::AllocatorI* _allocator, const ImageContainer& _image, LightingModel::Enum _lightingModel, bx::Error* _err)
	{
		if (!_image.m_cubeMap)
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "Input image is not cubemap.");
			return NULL;
		}

		ImageContainer* input = imageConvert(_allocator, TextureFormat::RGBA32F, _image, true);

		if (1 >= input->m_numMips)
		{
			ImageContainer* temp = imageGenerateMips(_allocator, *input);
			imageFree(input);
			input = temp;
		}

		ImageContainer* output = imageAlloc(_allocator, TextureFormat::RGBA32F, uint16_t(input->m_width), uint16_t(input->m_width), 1, 1, true, true);

		for (uint8_t side = 0; side < 6; ++side)
		{
			ImageMip srcMip;
			imageGetRawData(*input, side, 0, input->m_data, input->m_size, srcMip);

			ImageMip dstMip;
			imageGetRawData(*output, side, 0, output->m_data, output->m_size, dstMip);

			uint8_t* dstData = const_cast<uint8_t*>(dstMip.m_data);

			bx::memCopy(dstData, srcMip.m_data, srcMip.m_size);
		}

		const float glossScale = 10.0f;
		const float glossBias  = 1.0f;

		for (uint8_t lod = 1, numMips = input->m_numMips; lod < numMips; ++lod)
		{
			ImageContainer* nsa = NULL;

			if (LightingModel::Ggx != _lightingModel)
			{
				nsa = imageCubemapNormalSolidAngle(_allocator, bx::max<uint32_t>(_image.m_width>>lod, 1) );
			}

			for (uint8_t side = 0; side < 6; ++side)
			{
				ImageMip mip;
				imageGetRawData(*output, side, lod, output->m_data, output->m_size, mip);

				const uint32_t dstWidth = mip.m_width;
				const uint32_t dstPitch = dstWidth*16;

				const float minAngle = bx::atan2(1.0f, float(dstWidth) );
				const float maxAngle = bx::kPiHalf;
				const float toFilterSize     = 1.0f/(minAngle*dstWidth*2.0f);
				const float glossiness       = glossinessFor(lod, float(numMips) );
				const float roughness        = 1.0f-glossiness;
				const float specularPowerRef = bx::pow(2.0f, glossiness*glossScale + glossBias);
				const float specularPower    = applyLightingModel(specularPowerRef, _lightingModel);
				const float filterAngle      = bx::clamp(cosinePowerFilterAngle(specularPower), minAngle, maxAngle);
				const float cosAngle   = bx::max(0.0f, bx::cos(filterAngle) );
				const float texelSize  = 1.0f/float(dstWidth);
				const float filterSize = bx::max(texelSize, filterAngle * toFilterSize);

				for (uint32_t yy = 0; yy < dstWidth; ++yy)
				{
					for (uint32_t xx = 0; xx < dstWidth; ++xx)
					{
						float* dstData = (float*)&mip.m_data[yy*dstPitch+xx*16];

						const float uu = float(xx)*texelSize*2.0f - 1.0f;
						const float vv = float(yy)*texelSize*2.0f - 1.0f;

						bx::Vec3 dir = texelUvToDir(side, uu, vv);

						if (LightingModel::Ggx == _lightingModel)
						{
							processFilterAreaGgx(dstData, *input, lod, dir, roughness);
						}
						else
						{
							Aabb aabb[6];
							calcFilterArea(aabb, dir, filterSize);

							processFilterArea(dstData, *input, *nsa, lod, aabb, dir, specularPower, cosAngle);
						}
					}
				}
			}

			if (NULL != nsa)
			{
				imageFree(nsa);
			}
		}

		return output;
	}

} // namespace bimg
