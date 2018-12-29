/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_ERROR_H_HEADER_GUARD
#define BX_ERROR_H_HEADER_GUARD

#include "string.h"

#define BX_ERROR_SET(_ptr, _result, _msg) \
			BX_MACRO_BLOCK_BEGIN \
				(_ptr)->setError(_result,  "" _msg); \
			BX_MACRO_BLOCK_END

#define BX_ERROR_USE_TEMP_WHEN_NULL(_ptr) \
			const bx::Error tmpError; /* It should not be used directly! */ \
			_ptr = NULL == _ptr ? const_cast<bx::Error*>(&tmpError) : _ptr

#define BX_ERROR_SCOPE(_ptr) \
			BX_ERROR_USE_TEMP_WHEN_NULL(_ptr); \
			bx::ErrorScope bxErrorScope(const_cast<bx::Error*>(&tmpError) )

#define BX_ERROR_RESULT(_err, _code) \
			BX_STATIC_ASSERT(_code != 0, "ErrorCode 0 is reserved!"); \
			static const bx::ErrorResult _err = { _code }

namespace bx
{
	///
	struct ErrorResult
	{
		uint32_t code;
	};

	///
	class Error
	{
		BX_CLASS(Error
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		Error();

		///
		void reset();

		///
		void setError(ErrorResult _errorResult, const StringView& _msg);

		///
		bool isOk() const;

		///
		ErrorResult get() const;

		///
		const StringView& getMessage() const;

		///
		bool operator==(const ErrorResult& _rhs) const;

		///
		bool operator!=(const ErrorResult& _rhs) const;

	private:
		StringView m_msg;
		uint32_t   m_code;
	};

	///
	class ErrorScope
	{
		BX_CLASS(ErrorScope
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		ErrorScope(Error* _err);

		///
		~ErrorScope();

	private:
		Error* m_err;
	};

} // namespace bx

#include "inline/error.inl"

#endif // BX_ERROR_H_HEADER_GUARD
