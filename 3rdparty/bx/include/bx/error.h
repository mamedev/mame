/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_ERROR_H_HEADER_GUARD
#define BX_ERROR_H_HEADER_GUARD

#include "string.h"

#define BX_ERROR_SET(_ptr, _result, _msg)    \
	BX_MACRO_BLOCK_BEGIN                     \
		(_ptr)->setError(_result,  "" _msg); \
	BX_MACRO_BLOCK_END

#define BX_ERROR_USE_TEMP_WHEN_NULL(_ptr)                           \
	const bx::Error tmpError; /* It should not be used directly! */ \
	_ptr = NULL == _ptr ? const_cast<bx::Error*>(&tmpError) : _ptr

#define BX_ERROR_SCOPE(_ptr, ...)                                  \
	BX_ERROR_USE_TEMP_WHEN_NULL(_ptr);                             \
	bx::ErrorScope bxErrorScope(const_cast<bx::Error*>(&tmpError), "" __VA_ARGS__)

#define BX_ERROR_RESULT(_err, _code)                          \
	BX_STATIC_ASSERT(_code != 0, "ErrorCode 0 is reserved!"); \
	static constexpr bx::ErrorResult _err = { _code }

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
			);

	public:
		///
		Error();

		///
		void reset();

		///
		void setError(ErrorResult _errorResult, const StringLiteral& _msg, const Location& _location = Location::current() );

		///
		bool isOk() const;

		///
		ErrorResult get() const;

		///
		const StringLiteral& getMessage() const;

		///
		const Location& getLocation() const;

		///
		bool operator==(const ErrorResult& _rhs) const;

		///
		bool operator!=(const ErrorResult& _rhs) const;

	private:
		Location      m_location;
		StringLiteral m_msg;
		uint32_t      m_code;
	};

	/// Do nothing even if error is set.
	class ErrorIgnore final : public Error
	{
	public:
		///
		operator Error*();
	};

	/// In debug build assert if error is set.
	class ErrorAssert final : public Error
	{
	public:
		///
		~ErrorAssert();

		///
		operator Error*();
	};

	/// Exit application if error is set.
	class ErrorFatal final : public Error
	{
	public:
		///
		~ErrorFatal();

		///
		operator Error*();
	};

	///
	class ErrorScope
	{
		BX_CLASS(ErrorScope
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		ErrorScope(Error* _err, const StringLiteral& _name);

		///
		~ErrorScope();

		///
		const StringLiteral& getName() const;

	private:
		Error* m_err;
		const StringLiteral m_name;
	};

} // namespace bx

#include "inline/error.inl"

#endif // BX_ERROR_H_HEADER_GUARD
