// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    charconv.cpp

    Imgtool character set conversion routines.

***************************************************************************/

#include "corestr.h"
#include "charconv.h"
#include "unicode.h"
#include "coretmpl.h"

imgtool::simple_charconverter imgtool::charconverter_iso_8859_1(nullptr, nullptr);


//-------------------------------------------------
//  from_utf8
//-------------------------------------------------

void imgtool::charconverter::from_utf8(std::ostream &dest, const std::string &src) const
{
	from_utf8(dest, src.c_str(), src.size());
}


//-------------------------------------------------
//  to_utf8
//-------------------------------------------------

void imgtool::charconverter::to_utf8(std::ostream &dest, const std::string &src) const
{
	to_utf8(dest, src.c_str(), src.size());
}


//-------------------------------------------------
//  from_utf8
//-------------------------------------------------

void imgtool::simple_charconverter::from_utf8(std::ostream &dest, const char *src, size_t src_length) const
{
	// normalize the incoming unicode
	std::string normalized_src = normalize_unicode(src, src_length, m_norm);

	auto iter = normalized_src.begin();
	while(iter != normalized_src.end())
	{
		// get the next character
		char32_t ch;
		int rc = uchar_from_utf8(&ch, &*iter, normalized_src.end() - iter);
		if (rc < 0)
		{
			ch = 0xFFFD;
			rc = 1;
		}	
		iter += rc;

		// look in all pages
		const char32_t *pages[2];
		pages[0] = m_lowpage;
		pages[1] = m_highpage;

		bool found = false;
		for (int i = 0; !found && i < ARRAY_LENGTH(pages); i++)
		{
			if (pages[i] == nullptr)
			{
				// null page; perhaps we can just emit this
				if (ch >= i * 0x80 && (ch < (i + 1) * 0x80))
				{
					dest << (char)ch;
					found = true;
				}
			}
			else
			{
				// non-null page; perform a lookup
				// if we have a page, perform the lookup
				const util::contiguous_sequence_wrapper<const char32_t> lookup(pages[i], 0x80);
				auto lookup_iter = std::find(lookup.begin(), lookup.end(), ch);
				if (lookup_iter != lookup.end())
				{
					// and emit the result
					dest << (char)((i * 0x80) + (lookup_iter - lookup.begin()));
					found = true;
				}
			}
		}
		if (!found)
			throw charconverter_exception();
	}
}


//-------------------------------------------------
//  to_utf8
//-------------------------------------------------

void imgtool::simple_charconverter::to_utf8(std::ostream &dest, const char *src, size_t src_length) const
{
	for (size_t i = 0; i < src_length; i++)
	{
		// which page is this in?
		const char32_t *page = ((src[i] & 0x80) == 0) ? m_lowpage : m_highpage;

		// is this page present?
		if ((src[i] & 0x80) == 0)
		{
			// no - pass it on
			dest << src[i];
		}
		else
		{
			// yes - we need to do a lookup
			size_t base = ((src[i] & 0x80) == 0) ? 0x00 : 0x80;
			char32_t ch = page[((unsigned char)(src[i])) - base];
			if (ch == 0)
				throw charconverter_exception();

			dest << utf8_from_uchar(ch);
		}
	}
}
