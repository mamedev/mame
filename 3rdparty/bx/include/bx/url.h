/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_URL_H_HEADER_GUARD
#define BX_URL_H_HEADER_GUARD

#include "string.h"

namespace bx
{
	///
	class UrlView
	{
	public:
		enum Enum
		{
			Scheme,
			UserName,
			Password,
			Host,
			Port,
			Path,
			Query,
			Fragment,

			Count
		};

		///
		UrlView();

		///
		void clear();

		///
		bool parse(const StringView& _url);

		///
		const StringView& get(Enum _token) const;

	private:
		StringView m_tokens[Count];
	};

	///
	void urlEncode(char* _out, uint32_t _max, const StringView& _str);

} // namespace bx

#endif // BX_URL_H_HEADER_GUARD
