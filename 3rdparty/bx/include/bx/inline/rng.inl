/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_RNG_H_HEADER_GUARD
#	error "Must be included from bx/rng.h!"
#endif // BX_RNG_H_HEADER_GUARD

namespace bx
{
	inline RngMwc::RngMwc(uint32_t _z, uint32_t _w)
		: m_z(_z)
		, m_w(_w)
	{
	}

	inline void RngMwc::reset(uint32_t _z, uint32_t _w)
	{
		m_z = _z;
		m_w = _w;
	}

	inline uint32_t RngMwc::gen()
	{
		m_z = 36969*(m_z&65535)+(m_z>>16);
		m_w = 18000*(m_w&65535)+(m_w>>16);
		return (m_z<<16)+m_w;
	}

	inline RngShr3::RngShr3(uint32_t _jsr)
		: m_jsr(_jsr)
	{
	}

	inline void RngShr3::reset(uint32_t _jsr)
	{
		m_jsr = _jsr;
	}

	inline uint32_t RngShr3::gen()
	{
		m_jsr ^= m_jsr<<17;
		m_jsr ^= m_jsr>>13;
		m_jsr ^= m_jsr<<5;
		return m_jsr;
	}

	template <typename Rng>
	inline float frnd(Rng* _rng)
	{
		uint32_t rnd = _rng->gen() & UINT16_MAX;
		return float(rnd) * 1.0f/float(UINT16_MAX);
	}

	template <typename Rng>
	inline float frndh(Rng* _rng)
	{
		return 2.0f * bx::frnd(_rng) - 1.0f;
	}

	template <typename Rng>
	inline bx::Vec3 randUnitCircle(Rng* _rng)
	{
		const float angle = frnd(_rng) * kPi2;

		return
		{
			cos(angle),
			0.0f,
			sin(angle),
		};
	}

	template <typename Rng>
	inline bx::Vec3 randUnitSphere(Rng* _rng)
	{
		const float rand0  = frnd(_rng) * 2.0f - 1.0f;
		const float rand1  = frnd(_rng) * kPi2;
		const float sqrtf1 = sqrt(1.0f - rand0*rand0);

		return
		{
			sqrtf1 * cos(rand1),
			sqrtf1 * sin(rand1),
			rand0,
		};
	}

	template <typename Ty>
	inline bx::Vec3 randUnitHemisphere(Ty* _rng, const bx::Vec3& _normal)
	{
		const bx::Vec3 dir = randUnitSphere(_rng);
		const float ddotn  = bx::dot(dir, _normal);

		if (0.0f > ddotn)
		{
			return bx::neg(dir);
		}

		return dir;
	}

	inline void generateSphereHammersley(void* _data, uint32_t _stride, uint32_t _num, float _scale)
	{
		// Reference(s):
		// - Sampling with Hammersley and Halton Points
		//   https://web.archive.org/web/20190207230709/http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoints.html

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

			const float phi    = (ii + 0.5f) / _num;
			const float phirad =  phi * kPi2;
			const float st     = sqrt(1.0f-tt*tt) * _scale;

			float* xyz = (float*)data;
			data += _stride;

			xyz[0] = st * cos(phirad);
			xyz[1] = st * sin(phirad);
			xyz[2] = tt * _scale;
		}
	}

	template<typename Rng, typename Ty>
	inline void shuffle(Rng* _rng, Ty* _array, uint32_t _num)
	{
		BX_ASSERT(_num != 0, "Number of elements can't be 0!");

		for (uint32_t ii = 0, num = _num-1; ii < num; ++ii)
		{
			uint32_t jj = ii + 1 + _rng->gen() % (num - ii);
			bx::swap(_array[ii], _array[jj]);
		}
	}

} // namespace bx
