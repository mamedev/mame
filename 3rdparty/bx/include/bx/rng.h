/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_RNG_H_HEADER_GUARD
#define BX_RNG_H_HEADER_GUARD

#include "bx.h"
#include "math.h"
#include "uint32_t.h"

namespace bx
{
	/// George Marsaglia's MWC
	class RngMwc
	{
	public:
		///
		RngMwc(uint32_t _z = 12345, uint32_t _w = 65435);

		///
		void reset(uint32_t _z = 12345, uint32_t _w = 65435);

		///
		uint32_t gen();

	private:
		uint32_t m_z;
		uint32_t m_w;
	};

	/// George Marsaglia's SHR3
	class RngShr3
	{
	public:
		///
		RngShr3(uint32_t _jsr = 34221);

		///
		void reset(uint32_t _jsr = 34221);

		///
		uint32_t gen();

	private:
		uint32_t m_jsr;
	};

	/// Returns random number between 0.0f and 1.0f.
	template <typename Rng>
	float frnd(Rng* _rng);

	/// Returns random number between -1.0f and 1.0f.
	template <typename Rng>
	float frndh(Rng* _rng);

	/// Generate random point on unit circle.
	template <typename Rng>
	bx::Vec3 randUnitCircle(Rng* _rng);

	/// Generate random point on unit sphere.
	template <typename Rng>
	bx::Vec3 randUnitSphere(Rng* _rng);

	/// Generate random point on unit hemisphere.
	template <typename Ty>
	bx::Vec3 randUnitHemisphere(Ty* _rng, const bx::Vec3& _normal);

	/// Sampling with Hammersley and Halton Points.
	void generateSphereHammersley(void* _data, uint32_t _stride, uint32_t _num, float _scale = 1.0f);

	/// Fisher-Yates shuffle.
	template<typename Rng, typename Ty>
	void shuffle(Rng* _rng, Ty* _array, uint32_t _num);

} // namespace bx

#include "inline/rng.inl"

#endif // BX_RNG_H_HEADER_GUARD
