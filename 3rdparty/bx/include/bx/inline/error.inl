/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_ERROR_H_HEADER_GUARD
#	error "Must be included from bx/error!"
#endif // BX_ERROR_H_HEADER_GUARD

#include <bx/debug.h>

namespace bx
{
	inline Error::Error()
		: m_code(0)
	{
	}

	inline void Error::reset()
	{
		m_code = 0;
		m_msg.clear();
	}

	inline void Error::setError(ErrorResult _errorResult, const StringLiteral& _msg, const Location& _location)
	{
		BX_ASSERT(0 != _errorResult.code, "Invalid ErrorResult passed to setError!");

		if (!isOk() )
		{
			return;
		}

		m_location = _location;
		m_code     = _errorResult.code;
		m_msg      = _msg;
	}

	inline bool Error::isOk() const
	{
		return 0 == m_code;
	}

	inline ErrorResult Error::get() const
	{
		ErrorResult result = { m_code };
		return result;
	}

	inline const StringLiteral& Error::getMessage() const
	{
		return m_msg;
	}

	inline const Location& Error::getLocation() const
	{
		return m_location;
	}

	inline bool Error::operator==(const ErrorResult& _rhs) const
	{
		return _rhs.code == m_code;
	}

	inline bool Error::operator!=(const ErrorResult& _rhs) const
	{
		return _rhs.code != m_code;
	}

	inline ErrorIgnore::operator Error*()
	{
		return this;
	}

	inline ErrorAssert::~ErrorAssert()
	{
		BX_ASSERT_LOC(getLocation(), isOk(), "ErrorAssert: 0x%08x `%S`"
			, get().code
			, &getMessage()
			);
	}

	inline ErrorFatal::operator Error*()
	{
		return this;
	}

	inline ErrorFatal::~ErrorFatal()
	{
		_BX_ASSERT_LOC(getLocation(), isOk(), "ErrorFatal: 0x%08x `%S`"
			, get().code
			, &getMessage()
			);
	}

	inline ErrorAssert::operator Error*()
	{
		return this;
	}

	inline ErrorScope::ErrorScope(Error* _err, const StringLiteral& _name)
		: m_err(_err)
		, m_name(_name)
	{
		BX_UNUSED(m_err);
		BX_ASSERT(NULL != _err, "_err can't be NULL");
	}

	inline ErrorScope::~ErrorScope()
	{
		if (m_name.isEmpty() )
		{
			BX_ASSERT_LOC(
				  m_err->getLocation()
				, m_err->isOk()
				, "ErrorScope: 0x%08x `%S`"
				, m_err->get().code
				, &m_err->getMessage()
				);
		}
		else
		{
			BX_ASSERT_LOC(
				  m_err->getLocation()
				, m_err->isOk()
				, "ErrorScope: %S - 0x%08x `%S`"
				, &m_name
				, m_err->get().code
				, &m_err->getMessage()
				);
		}
	}

	inline const StringLiteral& ErrorScope::getName() const
	{
		return m_name;
	}

} // namespace bx
