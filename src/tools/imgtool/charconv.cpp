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

static const char32_t iso_8859_1_code_page[128] =
{
	// 0x80 - 0x8F
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x90 - 0x9F
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xA0 - 0xAF
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	// 0xB0 - 0xBF
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	// 0xC0 - 0xCF
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	// 0xD0 - 0xDF
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	// 0xE0 - 0xEF
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	// 0xF0 - 0xFF
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

imgtool::simple_charconverter imgtool::charconverter_iso_8859_1(nullptr, iso_8859_1_code_page);


//-------------------------------------------------
//  from_utf8
//-------------------------------------------------

void imgtool::simple_charconverter::from_utf8(std::ostream &dest, const std::string &src) const
{
	// normalize the incoming unicode
	std::string normalized_src = normalize_unicode(src, m_norm);

	auto iter = normalized_src.begin();
	while(iter != normalized_src.end())
	{
		// get the next character
		char32_t ch;
		iter += uchar_from_utf8(&ch, &*iter, normalized_src.end() - iter);

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

void imgtool::simple_charconverter::to_utf8(std::ostream &dest, const std::string &src) const
{
	for (auto iter = src.begin(); iter != src.end(); iter++)
	{
		// which page is this in?
		const char32_t *page = ((*iter & 0x80) == 0) ? m_lowpage : m_highpage;

		// is this page present?
		if ((*iter & 0x80) == 0)
		{
			// no - pass it on
			dest << *iter;
		}
		else
		{
			// yes - we need to do a lookup
			size_t base = ((*iter & 0x80) == 0) ? 0x00 : 0x80;
			char32_t ch = page[((unsigned char)(*iter)) - base];
			if (ch == 0)
				throw charconverter_exception();

			dest << utf8_from_uchar(ch);
		}
	}
}
