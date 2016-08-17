/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_ERROR_H_HEADER_GUARD
#define BX_ERROR_H_HEADER_GUARD

#include "bx.h"

#define BX_ERROR_SET(_ptr, _result, _msg) \
			BX_MACRO_BLOCK_BEGIN \
				BX_TRACE("Error %d: %s", _result.code, "" _msg); \
				_ptr->setError(_result,  "" _msg); \
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
		Error()
			: m_code(0)
		{
		}

		void setError(ErrorResult _errorResult, const char* _msg)
		{
			BX_CHECK(0 != _errorResult.code, "Invalid ErrorResult passed to setError!");

			if (!isOk() )
			{
				return;
			}

			m_code = _errorResult.code;
			m_msg  = _msg;
		}

		bool isOk() const
		{
			return 0 == m_code;
		}

		ErrorResult get() const
		{
			ErrorResult result = { m_code };
			return result;
		}

		bool operator==(ErrorResult _rhs) const
		{
			return _rhs.code == m_code;
		}

	private:
		const char* m_msg;
		uint32_t    m_code;
	};

	///
	class ErrorScope
	{
		BX_CLASS(ErrorScope
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		ErrorScope(Error* _err)
			: m_err(_err)
		{
			BX_CHECK(NULL != _err, "_err can't be NULL");
		}

		~ErrorScope()
		{
			BX_CHECK(m_err->isOk(), "Error: %d", m_err->get().code);
		}

	private:
		Error* m_err;
	};

} // namespace bx

#endif // BX_ERROR_H_HEADER_GUARD
