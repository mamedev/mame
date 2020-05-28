/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bnet#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/url.h>

namespace bx
{
	UrlView::UrlView()
	{
	}

	void UrlView::clear()
	{
		for (uint32_t ii = 0; ii < Count; ++ii)
		{
			m_tokens[ii].clear();
		}
	}

	bool UrlView::parse(const StringView& _url)
	{
		clear();

		const char* term  = _url.getTerm();
		StringView schemeEnd = strFind(_url, "://");
		const char* hostStart = !schemeEnd.isEmpty() ? schemeEnd.getTerm() : _url.getPtr();
		StringView path = strFind(StringView(hostStart, term), '/');

		if (schemeEnd.isEmpty()
		&&  path.isEmpty() )
		{
			return false;
		}

		if (!schemeEnd.isEmpty()
		&& (path.isEmpty() || path.getPtr() > schemeEnd.getPtr() ) )
		{
			const StringView scheme(_url.getPtr(), schemeEnd.getPtr() );

			if (!isAlpha(scheme) )
			{
				return false;
			}

			m_tokens[Scheme].set(scheme);
		}

		if (!path.isEmpty() )
		{
			path.set(path.getPtr(), term);
			const StringView query    = strFind(path, '?');
			const StringView fragment = strFind(path, '#');

			if (!fragment.isEmpty()
			&&   fragment.getPtr() < query.getPtr() )
			{
				return false;
			}

			m_tokens[Path].set(path.getPtr()
				, !query.isEmpty()    ? query.getPtr()
				: !fragment.isEmpty() ? fragment.getPtr()
				: term
				);

			if (!query.isEmpty() )
			{
				m_tokens[Query].set(query.getPtr()+1
					, !fragment.isEmpty() ? fragment.getPtr()
					: term
					);
			}

			if (!fragment.isEmpty() )
			{
				m_tokens[Fragment].set(fragment.getPtr()+1, term);
			}

			term = path.getPtr();
		}

		const StringView userPassEnd = strFind(StringView(hostStart, term), '@');
		const char* userPassStart = !userPassEnd.isEmpty() ? hostStart : NULL;
		hostStart = !userPassEnd.isEmpty() ? userPassEnd.getPtr()+1 : hostStart;
		const StringView portStart = strFind(StringView(hostStart, term), ':');

		m_tokens[Host].set(hostStart, !portStart.isEmpty() ? portStart.getPtr() : term);

		if (!portStart.isEmpty())
		{
			m_tokens[Port].set(portStart.getPtr()+1, term);
		}

		if (NULL != userPassStart)
		{
			StringView passStart = strFind(StringView(userPassStart, userPassEnd.getPtr() ), ':');

			m_tokens[UserName].set(userPassStart
				, !passStart.isEmpty() ? passStart.getPtr()
				: userPassEnd.getPtr()
				);

			if (!passStart.isEmpty() )
			{
				m_tokens[Password].set(passStart.getPtr()+1, userPassEnd.getPtr() );
			}
		}

		return true;
	}

	const StringView& UrlView::get(Enum _token) const
	{
		return m_tokens[_token];
	}

	static char toHex(char _nible)
	{
		return "0123456789ABCDEF"[_nible&0xf];
	}

	// https://secure.wikimedia.org/wikipedia/en/wiki/URL_encoding
	void urlEncode(char* _out, uint32_t _max, const StringView& _str)
	{
		_max--; // need space for zero terminator

		const char* str  = _str.getPtr();
		const char* term = _str.getTerm();

		uint32_t ii = 0;
		for (char ch = *str++
			; str <= term && ii < _max
			; ch = *str++
			)
		{
			if (isAlphaNum(ch)
			||  ch == '-'
			||  ch == '_'
			||  ch == '.'
			||  ch == '~')
			{
				_out[ii++] = ch;
			}
			else if (ii+3 < _max)
			{
				_out[ii++] = '%';
				_out[ii++] = toHex(ch>>4);
				_out[ii++] = toHex(ch);
			}
		}

		_out[ii] = '\0';
	}

} // namespace bx
