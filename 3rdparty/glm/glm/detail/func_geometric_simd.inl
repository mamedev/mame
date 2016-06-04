#include "../simd/geometric.h"

namespace glm{
namespace detail
{
#	if GLM_HAS_UNRESTRICTED_UNIONS
		template <>
		struct compute_dot<tvec4, float, simd>
		{
			GLM_FUNC_QUALIFIER static float call(tvec4<float, simd> const& x, tvec4<float, simd> const& y)
			{
				__m128 const dot0 = glm_dot_ss(x.data, y.data);

				float Result = 0;
				_mm_store_ss(&Result, dot0);
				return Result;
			}
		};
#	endif//GLM_HAS_UNRESTRICTED_UNIONS
}//namespace detail
}//namespace glm

