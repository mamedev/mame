/// @ref core
/// @file glm/detail/func_matrix_simd.inl

#include "type_mat4x4.hpp"
#include "func_geometric.hpp"
#include "../simd/matrix.h"

namespace glm
{
#	if GLM_HAS_UNRESTRICTED_UNIONS
		template <>
		GLM_FUNC_QUALIFIER tmat4x4<float, simd> inverse(tmat4x4<float, simd> const& m)
		{
			tmat4x4<float, simd> Result(uninitialize);
			glm_inverse_ps(
				*reinterpret_cast<__m128 const(*)[4]>(&m[0].data),
				*reinterpret_cast<__m128(*)[4]>(&Result[0].data));
			return Result;
		}
#	endif// GLM_HAS_UNRESTRICTED_UNIONS
}//namespace glm
