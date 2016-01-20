/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_RNG_H_HEADER_GUARD
#define BX_RNG_H_HEADER_GUARD

#include "bx.h"
#include "fpumath.h"

namespace bx
{
	// George Marsaglia's MWC
	class RngMwc
	{
	public:
		RngMwc(uint32_t _z = 12345, uint32_t _w = 65435)
			: m_z(_z)
			, m_w(_w)
		{
		}

		void reset(uint32_t _z = 12345, uint32_t _w = 65435)
		{
			m_z = _z;
			m_w = _w;
		}

		uint32_t gen()
		{
			m_z = 36969*(m_z&65535)+(m_z>>16);
			m_w = 18000*(m_w&65535)+(m_w>>16);
			return (m_z<<16)+m_w;
		}

	private:
		uint32_t m_z;
		uint32_t m_w;
	};

	// George Marsaglia's FIB
	class RngFib
	{
	public:
		RngFib()
			: m_a(9983651)
			, m_b(95746118)
		{
		}

		void reset()
		{
			m_a = 9983651;
			m_b = 95746118;
		}

		uint32_t gen()
		{
			m_b = m_a+m_b;
			m_a = m_b-m_a;
			return m_a;
		}

	private:
		uint32_t m_a;
		uint32_t m_b;
	};

	// George Marsaglia's SHR3
	class RngShr3
	{
	public:
		RngShr3(uint32_t _jsr = 34221)
			: m_jsr(_jsr)
		{
		}

		void reset(uint32_t _jsr = 34221)
		{
			m_jsr = _jsr;
		}

		uint32_t gen()
		{
			m_jsr ^= m_jsr<<17;
			m_jsr ^= m_jsr>>13;
			m_jsr ^= m_jsr<<5;
			return m_jsr;
		}

	private:
		uint32_t m_jsr;
	};

	/// Returns random number between 0.0f and 1.0f.
	template <typename Ty>
	inline float frnd(Ty* _rng)
	{
		uint32_t rnd = _rng->gen() & UINT16_MAX;
		return float(rnd) * 1.0f/float(UINT16_MAX);
	}

	/// Returns random number between -1.0f and 1.0f.
	template <typename Ty>
	inline float frndh(Ty* _rng)
	{
		return 2.0f * bx::frnd(_rng) - 1.0f;
	}

	/// Generate random point on unit sphere.
	template <typename Ty>
	static inline void randUnitSphere(float _result[3], Ty* _rng)
	{
		float rand0 = frnd(_rng) * 2.0f - 1.0f;
		float rand1 = frnd(_rng) * pi * 2.0f;

		float sqrtf1 = sqrtf(1.0f - rand0*rand0);
		_result[0] = sqrtf1 * cosf(rand1);
		_result[1] = sqrtf1 * sinf(rand1);
		_result[2] = rand0;
	}

	/// Generate random point on unit hemisphere.
	template <typename Ty>
	static inline void randUnitHemisphere(float _result[3], Ty* _rng, const float _normal[3])
	{
		float dir[3];
		randUnitSphere(dir, _rng);

		float DdotN = dir[0]*_normal[0]
					+ dir[1]*_normal[1]
					+ dir[2]*_normal[2]
					;

		if (0.0f > DdotN)
		{
			dir[0] = -dir[0];
			dir[1] = -dir[1];
			dir[2] = -dir[2];
		}

		_result[0] = dir[0];
		_result[1] = dir[1];
		_result[2] = dir[2];
	}

	/// Sampling with Hammersley and Halton Points
	/// http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoints.html
	///
	static inline void generateSphereHammersley(void* _data, uint32_t _stride, uint32_t _num, float _scale = 1.0f)
	{
		uint8_t* data = (uint8_t*)_data;

		for (uint32_t ii = 0; ii < _num; ii++)
		{
			float tt = 0.0f;
			float pp = 0.5;
			for (uint32_t jj = ii; jj; jj >>= 1)
			{
				tt += (jj & 1) ? pp : 0.0f;
				pp *= 0.5f;
			}

			tt = 2.0f * tt - 1.0f;

			const float phi = (ii + 0.5f) / _num;
			const float phirad =  phi * 2.0f * pi;
			const float st = sqrtf(1.0f-tt*tt) * _scale;

			float* xyz = (float*)data;
			data += _stride;

			xyz[0] = st * cosf(phirad);
			xyz[1] = st * sinf(phirad);
			xyz[2] = tt * _scale;
		}
	}

} // namespace bx

#endif // BX_RNG_H_HEADER_GUARD
