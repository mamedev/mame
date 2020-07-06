// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PGSL_H_
#define PGSL_H_

///
/// \file pgsl.h
///
/// This core guidelines gsl implementation currently provides only enough
/// functionality to syntactically support a very limited number of the
/// gsl specification.
///
/// Going forward this approach may be extended.
///

#include "pconfig.h"
#include "pexception.h"
#include "ptypes.h"

#include <exception>
#include <type_traits>
#include <utility>

#if defined(__has_builtin) // clang and gcc 10
	#if __has_builtin(__builtin_unreachable)
		#define gsl_Expects(e) ((e) ? static_cast<void>(0) : __builtin_unreachable())
	#endif
#elif defined(__GNUC__) && !(defined( __CUDACC__ ) && defined( __CUDA_ARCH__ ))
	#define gsl_Expects(e) ((e) ? static_cast<void>(0) : __builtin_unreachable())
#elif defined(_MSC_VER)
	#define gsl_Expects(e) __assume(e)
#else
	#define gsl_Expects(e) ((e) ? static_cast<void>(0) : static_cast<void>(0))
#endif

#undef gsl_Expects
#define gsl_Expects(e) do {} while (0)
#define gsl_Ensures(e) gsl_Expects(e)

namespace plib {
	namespace pgsl {

		struct narrowing_error : public std::exception {};

		template <typename T>
		using owner = T;

		template <typename T>
		using not_null = T;

		/// \brief perform a narrowing cast without checks
		///
		template <typename T, typename O>
		inline constexpr T narrow_cast(O && v) noexcept
		{
			static_assert(plib::is_arithmetic<T>::value && std::is_convertible<std::remove_reference_t<O>, T>::value, "narrow cast expects conversion between arithmetic types");
			return static_cast<T>(std::forward<O>(v));
		}

		/// \brief perform a narrowing cast terminating if loss of precision
		///
		/// The c++ core guidelines require the narrow function to raise an error
		/// This will make narrow noexcept(false). This has shown to have a
		/// measurable impact on performance and thus we deviate we deviate from
		/// the standard here.
		///
		template <typename T, typename O>
		inline T narrow(O && v) noexcept
		{
			static_assert(plib::is_arithmetic<T>::value && std::is_convertible<std::remove_reference_t<O>, T>::value, "narrow cast expects conversion between arithmetic types");

			const auto val = static_cast<T>(std::forward<O>(v));
			if( v == static_cast<std::remove_reference_t<O>>(val))
			{
				return val;
			}

			plib::terminate("narrowing_error");
			// throw narrowing_error();
		}

	} // namespace pgsl

	/// \brief downcast from base tpye to derived type
	///
	/// The cpp core guidelines require a very careful use of static cast
	/// for downcast operations. This template is used to identify these uses
	/// and later for debug builds use dynamic_cast.
	///
	template <typename D, typename B>
	inline constexpr D downcast(B && b) noexcept
	{
		static_assert(std::is_pointer<D>::value || std::is_reference<D>::value, "downcast only supports pointers or reference for derived");
		static_assert(std::is_pointer<B>::value || std::is_reference<B>::value, "downcast only supports pointers or reference for base");
		return static_cast<D>(std::forward<B>(b));
	}

	using pgsl::narrow_cast;
} // namespace plib

//FIXME: This is the place to use more complete implementations
namespace gsl = plib::pgsl;

#endif // PGSL_H_
