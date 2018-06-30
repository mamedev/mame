/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
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

		const char* start = _url.getPtr();
		const char* term  = _url.getTerm();
		const char* schemeEnd = strFind(StringView(start, term), "://");
		const char* hostStart = NULL != schemeEnd ? schemeEnd+3 : start;
		const char* pathStart = strFind(StringView(hostStart, term), '/');

		if (NULL == schemeEnd
		&&  NULL == pathStart)
		{
			return false;
		}

		if (NULL != schemeEnd
		&& (NULL == pathStart || pathStart > schemeEnd) )
		{
			StringView scheme(start, schemeEnd);

			if (!isAlpha(scheme) )
			{
				return false;
			}

			m_tokens[Scheme].set(scheme);
		}

		if (NULL != pathStart)
		{
			const char* queryStart    = strFind(StringView(pathStart, term), '?');
			const char* fragmentStart = strFind(StringView(pathStart, term), '#');

			if (NULL != fragmentStart
			&&  fragmentStart < queryStart)
			{
				return false;
			}

			m_tokens[Path].set(pathStart
				, NULL != queryStart    ? queryStart
				: NULL != fragmentStart ? fragmentStart
				: term
				);

			if (NULL != queryStart)
			{
				m_tokens[Query].set(queryStart+1
					, NULL != fragmentStart ? fragmentStart
					: term
					);
			}

			if (NULL != fragmentStart)
			{
				m_tokens[Fragment].set(fragmentStart+1, term);
			}

			term = pathStart;
		}

		const char* userPassEnd   = strFind(StringView(hostStart, term), '@');
		const char* userPassStart = NULL != userPassEnd ? hostStart : NULL;
		hostStart = NULL != userPassEnd ? userPassEnd+1 : hostStart;
		const char* portStart = strFind(StringView(hostStart, term), ':');

		m_tokens[Host].set(hostStart, NULL != portStart ? portStart : term);

		if (NULL != portStart)
		{
			m_tokens[Port].set(portStart+1, term);
		}

		if (NULL != userPassStart)
		{
			const char* passStart = strFind(StringView(userPassStart, userPassEnd), ':');

			m_tokens[UserName].set(userPassStart
				, NULL != passStart ? passStart
				: userPassEnd
				);

			if (NULL != passStart)
			{
				m_tokens[Password].set(passStart+1, userPassEnd);
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
