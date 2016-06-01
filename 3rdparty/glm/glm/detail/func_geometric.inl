/// @ref core
/// @file glm/detail/func_geometric.inl

#include "func_exponential.hpp"
#include "func_common.hpp"
#include "type_vec2.hpp"
#include "type_vec4.hpp"
#include "type_float.hpp"

namespace glm{
namespace detail
{
	template <template <class, precision> class vecType, typename T, precision P>
	struct compute_dot{};

	template <typename T, precision P>
	struct compute_dot<tvec1, T, P>
	{
		GLM_FUNC_QUALIFIER static T call(tvec1<T, P> const & a, tvec1<T, P> const & b)
		{
			return a.x * b.x;
		}
	};

	template <typename T, precision P>
	struct compute_dot<tvec2, T, P>
	{
		GLM_FUNC_QUALIFIER static T call(tvec2<T, P> const & x, tvec2<T, P> const & y)
		{
			tvec2<T, P> tmp(x * y);
			return tmp.x + tmp.y;
		}
	};

	template <typename T, precision P>
	struct compute_dot<tvec3, T, P>
	{
		GLM_FUNC_QUALIFIER static T call(tvec3<T, P> const & x, tvec3<T, P> const & y)
		{
			tvec3<T, P> tmp(x * y);
			return tmp.x + tmp.y + tmp.z;
		}
	};

	template <typename T, precision P>
	struct compute_dot<tvec4, T, P>
	{
		GLM_FUNC_QUALIFIER static T call(tvec4<T, P> const & x, tvec4<T, P> const & y)
		{
			tvec4<T, P> tmp(x * y);
			return (tmp.x + tmp.y) + (tmp.z + tmp.w);
		}
	};
}//namespace detail

	// length
	template <typename genType>
	GLM_FUNC_QUALIFIER genType length(genType x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559, "'length' accepts only floating-point inputs");

		return abs(x);
	}

	template <typename T, precision P, template <typename, precision> class vecType>
	GLM_FUNC_QUALIFIER T length(vecType<T, P> const & v)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'length' accepts only floating-point inputs");

		return sqrt(dot(v, v));
	}

	// distance
	template <typename genType>
	GLM_FUNC_QUALIFIER genType distance(genType const & p0, genType const & p1)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559, "'distance' accepts only floating-point inputs");

		return length(p1 - p0);
	}

	template <typename T, precision P, template <typename, precision> class vecType>
	GLM_FUNC_QUALIFIER T distance(vecType<T, P> const & p0, vecType<T, P> const & p1)
	{
		return length(p1 - p0);
	}

	// dot
	template <typename T>
	GLM_FUNC_QUALIFIER T dot(T x, T y)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'dot' accepts only floating-point inputs");
		return x * y;
	}

	template <typename T, precision P, template <typename, precision> class vecType>
	GLM_FUNC_QUALIFIER T dot(vecType<T, P> const & x, vecType<T, P> const & y)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'dot' accepts only floating-point inputs");
		return detail::compute_dot<vecType, T, P>::call(x, y);
	}

	// cross
	template <typename T, precision P>
	GLM_FUNC_QUALIFIER tvec3<T, P> cross(tvec3<T, P> const & x, tvec3<T, P> const & y)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'cross' accepts only floating-point inputs");

		return tvec3<T, P>(
			x.y * y.z - y.y * x.z,
			x.z * y.x - y.z * x.x,
			x.x * y.y - y.x * x.y);
	}

	// normalize
	template <typename genType>
	GLM_FUNC_QUALIFIER genType normalize(genType const & x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559, "'normalize' accepts only floating-point inputs");

		return x < genType(0) ? genType(-1) : genType(1);
	}

	template <typename T, precision P, template <typename, precision> class vecType>
	GLM_FUNC_QUALIFIER vecType<T, P> normalize(vecType<T, P> const & x)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'normalize' accepts only floating-point inputs");

		return x * inversesqrt(dot(x, x));
	}

	// faceforward
	template <typename genType>
	GLM_FUNC_QUALIFIER genType faceforward(genType const & N, genType const & I, genType const & Nref)
	{
		return dot(Nref, I) < static_cast<genType>(0) ? N : -N;
	}

	template <typename T, precision P, template <typename, precision> class vecType>
	GLM_FUNC_QUALIFIER vecType<T, P> faceforward(vecType<T, P> const & N, vecType<T, P> const & I, vecType<T, P> const & Nref)
	{
		return dot(Nref, I) < static_cast<T>(0) ? N : -N;
	}

	// reflect
	template <typename genType>
	GLM_FUNC_QUALIFIER genType reflect(genType const & I, genType const & N)
	{
		return I - N * dot(N, I) * static_cast<genType>(2);
	}

	// refract
	template <typename genType>
	GLM_FUNC_QUALIFIER genType refract(genType const & I, genType const & N, genType const & eta)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559, "'refract' accepts only floating-point inputs");

		genType const dotValue(dot(N, I));
		genType const k(static_cast<genType>(1) - eta * eta * (static_cast<genType>(1) - dotValue * dotValue));
		return (eta * I - (eta * dotValue + sqrt(k)) * N) * static_cast<genType>(k >= static_cast<genType>(0));
	}

	template <typename T, precision P, template <typename, precision> class vecType>
	GLM_FUNC_QUALIFIER vecType<T, P> refract(vecType<T, P> const & I, vecType<T, P> const & N, T eta)
	{
		GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'refract' accepts only floating-point inputs");

		T const dotValue(dot(N, I));
		T const k(static_cast<T>(1) - eta * eta * (static_cast<T>(1) - dotValue * dotValue));
		return (eta * I - (eta * dotValue + std::sqrt(k)) * N) * static_cast<T>(k >= static_cast<T>(0));
	}
}//namespace glm

#if GLM_ARCH != GLM_ARCH_PURE
#	include "func_geometric_simd.inl"
#endif
