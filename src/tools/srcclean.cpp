// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    srcclean.cpp

    Basic source code cleanear.

****************************************************************************/

/*
    Known general limitations:
    * Always uses filename.orig as backup location, and attempts to
      overwrite if it exists (doesn't try to generate unique name)
    * Assumes any input is UTF-8
    * No way to override hard-coded internal extension to syntax mapping
    * All Unicode characters are treated as occupying a single column
      (doesn't account for combining, non-spacing, fullwidth, etc.)

    Known C++ limitations:
    * No filtering of control characters
    * Will not produce expected output for a string continuation within
      a preprocessor macro, e.g this:
      #define MY_MACRO \
              "string that \
              continues"
    * Numeric literals broken by line continuations are not recognised
    * Will not recognise a comment delimiter broken by multiple line
      continuations. e.g. this:
      /\
      \
      / preprocessor abuse

    Known Lua limitations:
    * Whitespace normalisation is applied inside long string literals
      which can cause changes in behaviour
    * Disabled code inside long comments gets no special treatment and
      may have spacing adjusted in a way that affects behaviour when
      uncommented

    Known XML limitations:
    * No special handling for CDATA
    * No special handling for processing instructions
    * Doesn't do any kind of validation of structure
    * Doesn't do anything special for illegal -- in comment

    Features not carried over from previous version:
    * Stripping empty continuation lines
    * Stripping empty lines following open brace
*/

#include "corefile.h"
#include "corestr.h"
#include "osdcomm.h"
#include "strformat.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>



namespace {

/***************************************************************************
    SOURCE CLEANER BASE CLASS
***************************************************************************/

class cleaner_base
{
public:
	enum class newline
	{
		DOS,
		UNIX,
		MACINTOSH,
		VMS
	};

	virtual ~cleaner_base() = default;

	template <typename InputIt>
	void process(InputIt begin, InputIt end);
	void finalise();

	virtual bool affected() const;
	virtual void summarise(std::ostream &os) const;

protected:
	static constexpr char32_t HORIZONTAL_TAB            = 0x0000'0009U;
	static constexpr char32_t LINE_FEED                 = 0x0000'000aU;
	static constexpr char32_t VERTICAL_TAB              = 0x0000'000bU;
	static constexpr char32_t SPACE                     = 0x0000'0020U;
	static constexpr char32_t DOUBLE_QUOTE              = 0x0000'0022U;
	static constexpr char32_t SINGLE_QUOTE              = 0x0000'0027U;
	static constexpr char32_t HYPHEN_MINUS              = 0x0000'002dU;
	static constexpr char32_t QUESTION_MARK             = 0x0000'003fU;
	static constexpr char32_t BACKSLASH                 = 0x0000'005cU;
	static constexpr char32_t BASIC_LATIN_LAST          = 0x0000'007fU;
	static constexpr char32_t CYRILLIC_SUPPLEMENT_LAST  = 0x0000'052fU;

	template <typename OutputIt>
	cleaner_base(OutputIt &&output, newline newline_mode, unsigned tab_width);

	void output_character(char32_t ch);

	void set_tab_limit();
	void reset_tab_limit();

private:
	static constexpr char32_t CARRIAGE_RETURN       = 0x0000'000dU;
	static constexpr char32_t HIGH_SURROGATE_FIRST  = 0x0000'd800U;
	static constexpr char32_t HIGH_SURROGATE_LAST   = 0x0000'dbffU;
	static constexpr char32_t LOW_SURROGATE_FIRST   = 0x0000'dc00U;
	static constexpr char32_t LOW_SURROGATE_LAST    = 0x0000'dfffU;
	static constexpr char32_t NONCHARACTER_FIRST    = 0x0000'fdd0U;
	static constexpr char32_t NONCHARACTER_LAST     = 0x0000'fdefU;
	static constexpr char32_t ZERO_WIDTH_NB_SPACE   = 0x0000'feffU;
	static constexpr char32_t REPLACEMENT_CHARACTER = 0x0000'fffdU;
	static constexpr char32_t SUPPLEMENTARY_FIRST   = 0x0001'0000U;
	static constexpr char32_t SUPPLEMENTARY_LAST    = 0x0010'ffffU;

	static constexpr char32_t CODE_LENGTH_THRESHOLDS[6]{
			0x0000'0000U, 0x0000'0080U, 0x0000'0800U, 0x0001'0000U, 0x0020'0000U, 0x0400'0000 };

	typedef std::function<void (char)> output_function;

	virtual void process_characters(char32_t const *begin, char32_t const *end) = 0;
	virtual void input_complete() = 0;

	void flush_whitespace();
	void output_utf8(char32_t ch);
	void commit_character(char32_t ch);
	void process_if_full();
	void handle_lead_byte(std::uint8_t ch);
	void handle_codepoint(char32_t cp);

	static constexpr bool is_character(char32_t ch)
	{
		return
				(ch <= SUPPLEMENTARY_LAST) &&
				((ch < NONCHARACTER_FIRST) || (ch > NONCHARACTER_LAST)) &&
				((ch & 0x0000'fffeU) != 0x0000'fffeU);
	}

	static constexpr bool is_high_surrogate(char32_t ch)
	{
		return (ch >= HIGH_SURROGATE_FIRST) && (ch <= HIGH_SURROGATE_LAST);
	}

	static constexpr bool is_low_surrogate(char32_t ch)
	{
		return (ch >= LOW_SURROGATE_FIRST) && (ch <= LOW_SURROGATE_LAST);
	}

	static constexpr char32_t combine_surrogates(char32_t high, char32_t low)
	{
		return SUPPLEMENTARY_FIRST + (((high & 0x0000'03ffU) << 10U) | (low & 0x0000'03ffU));
	}

	// configuration
	newline         m_newline_mode;
	unsigned        m_tab_width;
	output_function m_output;

	// output state management
	unsigned                    m_output_column = 0U;
	unsigned                    m_indent;
	unsigned                    m_tab_limit     = std::numeric_limits<unsigned>::max();
	std::vector<char32_t>       m_whitespace;

	// input state management
	char32_t        m_buffer[1024];
	bool            m_stream_start      = true;
	std::size_t     m_position          = 0U;
	char32_t        m_surrogate         = 0U;
	unsigned        m_code_length       = 0U;
	unsigned        m_required_bytes    = 0U;
	char32_t        m_newline_lead      = 0U;

	// statistics
	std::uint64_t   m_overlong              = 0U;
	std::uint64_t   m_incomplete            = 0U;
	std::uint64_t   m_continuations         = 0U;
	std::uint64_t   m_invalid_bytes         = 0U;
	std::uint64_t   m_noncharacters         = 0U;
	std::uint64_t   m_surrogate_pairs       = 0U;
	std::uint64_t   m_lone_high_surrogates  = 0U;
	std::uint64_t   m_lone_low_surrogates   = 0U;
	std::uint64_t   m_leading_zw_nb_sp      = 0U;
	std::uint64_t   m_dos_newlines          = 0U;
	std::uint64_t   m_unix_newlines         = 0U;
	std::uint64_t   m_macintosh_newlines    = 0U;
	std::uint64_t   m_vms_newlines          = 0U;
	std::uint64_t   m_trailing_whitespace   = 0U;
	std::uint64_t   m_tabs_expanded         = 0U;
	std::uint64_t   m_tabs_created          = 0U;
	std::uint64_t   m_spaces_combined       = 0U;
	bool            m_final_newline         = false;
};

constexpr char32_t cleaner_base::CODE_LENGTH_THRESHOLDS[6];


/*--------------------------------------------------
    cleaner_base::process
    process a block of input bytes
--------------------------------------------------*/

template <typename InputIt>
void cleaner_base::process(InputIt begin, InputIt end)
{
	while (begin != end)
	{
		std::uint8_t const byte(*begin++);
		if (m_required_bytes)
		{
			if ((byte & 0xc0U) == 0x80U)
			{
				m_buffer[m_position] <<= 6U;
				m_buffer[m_position] |= char32_t(byte & 0x3fU);
				--m_required_bytes;
			}
			else
			{
				m_required_bytes = 0U;
				++m_incomplete;
				commit_character(REPLACEMENT_CHARACTER);
				handle_lead_byte(byte);
			}
		}
		else
		{
			handle_lead_byte(byte);
		}

		if (!m_required_bytes)
			handle_codepoint(m_buffer[m_position]);
	}
}


/*--------------------------------------------------
    cleaner_base::finalise
    perform final processing on reaching end of
    input
--------------------------------------------------*/

void cleaner_base::finalise()
{
	if (m_surrogate)
	{
		++m_lone_high_surrogates;
		commit_character(REPLACEMENT_CHARACTER);
		m_surrogate = 0U;
	}

	if (m_required_bytes)
	{
		++m_incomplete;
		commit_character(REPLACEMENT_CHARACTER);
	}

	switch (m_newline_lead)
	{
	case LINE_FEED:
		++m_unix_newlines;
		m_newline_lead = 0U;
		m_buffer[m_position++] = LINE_FEED;
		break;
	case CARRIAGE_RETURN:
		++m_macintosh_newlines;
		m_newline_lead = 0U;
		m_buffer[m_position++] = LINE_FEED;
		break;
	default:
		assert(!m_newline_lead);
	}

	if (m_position)
	{
		process_characters(m_buffer, m_buffer + m_position);
		m_position = 0U;
	}

	input_complete();

	if (m_output_column || !m_whitespace.empty())
	{
		m_final_newline = true;
		output_character(LINE_FEED);
	}
}


/*--------------------------------------------------
    cleaner_base::affected
    returns whether any cleanups have been
    applied
--------------------------------------------------*/

bool cleaner_base::affected() const
{
	return
			m_overlong ||
			m_incomplete ||
			m_continuations ||
			m_invalid_bytes ||
			m_noncharacters ||
			m_surrogate_pairs ||
			m_lone_high_surrogates ||
			m_lone_low_surrogates ||
			m_leading_zw_nb_sp ||
			(m_dos_newlines && (newline::DOS != m_newline_mode)) ||
			(m_unix_newlines && (newline::UNIX != m_newline_mode)) ||
			(m_macintosh_newlines && (newline::MACINTOSH != m_newline_mode)) ||
			(m_vms_newlines && (newline::VMS != m_newline_mode)) ||
			m_trailing_whitespace ||
			m_tabs_expanded ||
			m_tabs_created ||
			m_spaces_combined ||
			m_final_newline;
}


/*--------------------------------------------------
    cleaner_base::summarise
    print summary of changes applied
--------------------------------------------------*/

void cleaner_base::summarise(std::ostream &os) const
{
	if (m_overlong)
		util::stream_format(os, "%1$u overlong UTF-8 sequence(s) corrected\n", m_overlong);
	if (m_incomplete)
		util::stream_format(os, "%1$u incomplete UTF-8 sequence(s) replaced\n", m_incomplete);
	if (m_continuations)
		util::stream_format(os, "%1$u UTF-8 continuation(s) replaced\n", m_continuations);
	if (m_invalid_bytes)
		util::stream_format(os, "%1$u invalid UTF-8 byte(s) replaced\n", m_invalid_bytes);
	if (m_noncharacters)
		util::stream_format(os, "%1$u noncharacter(s) replaced\n", m_noncharacters);
	if (m_surrogate_pairs)
		util::stream_format(os, "%1$u surrogate pair(s) combined\n", m_surrogate_pairs);
	if (m_lone_high_surrogates)
		util::stream_format(os, "%1$u lone high surrogate(s) replaced\n", m_lone_high_surrogates);
	if (m_lone_low_surrogates)
		util::stream_format(os, "%1$u lone low surrogate(s) replaced\n", m_lone_low_surrogates);
	if (m_leading_zw_nb_sp)
		util::stream_format(os, "%1$u leading zero-width no-break space(s) removed\n", m_leading_zw_nb_sp);
	if (m_dos_newlines && (newline::DOS != m_newline_mode))
		util::stream_format(os, "%1$u DOS line ending(s) normalised\n", m_dos_newlines);
	if (m_unix_newlines && (newline::UNIX != m_newline_mode))
		util::stream_format(os, "%1$u UNIX line ending(s) normalised\n", m_unix_newlines);
	if (m_macintosh_newlines && (newline::MACINTOSH != m_newline_mode))
		util::stream_format(os, "%1$u Macintosh line ending(s) normalised\n", m_macintosh_newlines);
	if (m_vms_newlines && (newline::VMS != m_newline_mode))
		util::stream_format(os, "%1$u VMS line ending(s) normalised\n", m_vms_newlines);
	if (m_trailing_whitespace)
		util::stream_format(os, "%1$u line(s) with trailing whitespace trimmed\n", m_trailing_whitespace);
	if (m_tabs_expanded)
		util::stream_format(os, "%1$u tab(s) expanded to spaces\n", m_tabs_expanded);
	if (m_tabs_created)
		util::stream_format(os, "%1$u tab(s) created from spaces\n", m_tabs_created);
	if (m_spaces_combined)
		util::stream_format(os, "%1$u space(s) combined into tabs\n", m_spaces_combined);
	if (m_final_newline)
		util::stream_format(os, "line ending added at end of file\n");
}


/*--------------------------------------------------
    cleaner_base::cleaner_base
    base constructor
--------------------------------------------------*/

template <typename OutputIt>
cleaner_base::cleaner_base(
		OutputIt &&output,
		newline newline_mode,
		unsigned tab_width)
	: m_newline_mode(newline_mode)
	, m_tab_width(tab_width)
	, m_output([it = std::forward<OutputIt>(output)] (char ch) mutable { *it++ = ch; })
	, m_whitespace()
{
	m_whitespace.reserve(128U);
}


/*--------------------------------------------------
    cleaner_base::output_character
    output character applying whitespace
    normalisation and line ending translation
--------------------------------------------------*/

void cleaner_base::output_character(char32_t ch)
{
	switch (ch)
	{
	case HORIZONTAL_TAB:
	case SPACE:
		m_whitespace.emplace_back(ch);
		break;

	case LINE_FEED:
		m_output_column = 0U;
		if (!m_whitespace.empty())
		{
			++m_trailing_whitespace;
			m_whitespace.clear();
		}
		switch (m_newline_mode)
		{
		case newline::DOS:
			output_utf8(CARRIAGE_RETURN);
			output_utf8(LINE_FEED);
			break;
		case newline::UNIX:
			output_utf8(LINE_FEED);
			break;
		case newline::MACINTOSH:
			output_utf8(CARRIAGE_RETURN);
			break;
		case newline::VMS:
			output_utf8(LINE_FEED);
			output_utf8(CARRIAGE_RETURN);
			break;
		}
		break;

	default:
		flush_whitespace();
		++m_output_column;
		output_utf8(ch);
	}
}


/*--------------------------------------------------
    cleaner_base::set_tab_limit
    limit leading tabs to number used to indent
    current line
--------------------------------------------------*/

void cleaner_base::set_tab_limit()
{
	if (!m_output_column)
	{
		unsigned limit(0U);
		for (char32_t ch : m_whitespace)
			limit += (HORIZONTAL_TAB == ch) ? (m_tab_width - (limit % m_tab_width)) : 1U;
		m_tab_limit = limit;
	}
	else
	{
		m_tab_limit = m_indent;
	}
}


/*--------------------------------------------------
    cleaner_base::reset_tab_limit
    revert to default handling of leading tabs
--------------------------------------------------*/

void cleaner_base::reset_tab_limit()
{
	m_tab_limit = std::numeric_limits<unsigned>::max();
}


/*--------------------------------------------------
    cleaner_base::flush_whitespace
    send whitespace to output normalising spaces
    and tabs in initial indent
--------------------------------------------------*/

void cleaner_base::flush_whitespace()
{
	bool const  set_indent(!m_output_column);
	bool        expand(m_output_column);
	unsigned    space_count(0U);
	for (char32_t space : m_whitespace)
	{
		assert(!expand || !space_count);
		assert(space_count < m_tab_width);

		if (HORIZONTAL_TAB == space)
		{
			unsigned width(m_tab_width - (m_output_column % m_tab_width));
			expand = expand || ((width + m_output_column) > m_tab_limit);
			if (expand)
			{
				++m_tabs_expanded;
				while (width--)
				{
					++m_output_column;
					output_utf8(SPACE);
				}
			}
			else
			{
				assert(!(m_output_column % m_tab_width));

				m_spaces_combined += space_count;
				m_output_column += width;
				output_utf8(space);
			}
			space_count = 0U;
		}
		else
		{
			assert(SPACE == space);

			++space_count;
			expand = expand || ((space_count + m_output_column) > m_tab_limit);
			if (expand)
			{
				while (space_count)
				{
					space_count--;
					++m_output_column;
					output_utf8(SPACE);
				}
			}
			else
			{
				assert(!(m_output_column % m_tab_width));

				if (space_count == m_tab_width)
				{
					++m_tabs_created;
					m_spaces_combined += space_count;
					space_count = 0U;
					m_output_column += m_tab_width;
					output_utf8(HORIZONTAL_TAB);
				}
			}
		}
	}
	while (space_count--)
	{
		++m_output_column;
		output_utf8(SPACE);
	}
	m_whitespace.clear();
	if (set_indent)
		m_indent = m_output_column;
}


/*--------------------------------------------------
    cleaner_base::output_utf8
    convert codepoint to UFF-8 and send to output
--------------------------------------------------*/

void cleaner_base::output_utf8(char32_t ch)
{
	if (0x0000'0080U > ch)
	{
		m_output(char(std::uint8_t(ch >> 0U)));
	}
	else
	{
		unsigned required =
				(0x0000'0800U > ch) ? 1U :
				(0x0001'0000U > ch) ? 2U :
				(0x0020'0000U > ch) ? 3U :
				(0x0400'0000U > ch) ? 4U : 5U;
		m_output(char(std::uint8_t(((ch >> (6U * required)) & (0x3fU >> required)) | ((0xfcU << (5U - required)) & 0xfcU))));
		while (required--)
			m_output(char(std::uint8_t(((ch >> (6U * required)) & 0x3fU) | 0x80U)));
	}
}


/*--------------------------------------------------
    cleaner_base::commit_character
    store decoded input character in buffer
    applying line ending normalisation and
    replacing noncharacters
--------------------------------------------------*/

void cleaner_base::commit_character(char32_t ch)
{
	assert(ARRAY_LENGTH(m_buffer) > m_position);
	assert(1U <= m_code_length);
	assert(6U >= m_code_length);

	if (CODE_LENGTH_THRESHOLDS[m_code_length - 1] > ch)
		++m_overlong;

	if (m_stream_start)
	{
		assert(!m_position);
		assert(!m_newline_lead);

		if (ZERO_WIDTH_NB_SPACE == ch)
		{
			++m_leading_zw_nb_sp;
			return;
		}
		else
		{
			m_stream_start = false;
		}
	}

	if (!is_character(ch))
	{
		ch = REPLACEMENT_CHARACTER;
		++m_noncharacters;
	}

	switch (ch)
	{
	case LINE_FEED:
		switch (m_newline_lead)
		{
		case LINE_FEED:
			++m_unix_newlines;
			m_buffer[m_position++] = LINE_FEED;
			break;
		case CARRIAGE_RETURN:
			++m_dos_newlines;
			m_newline_lead = 0U;
			m_buffer[m_position++] = LINE_FEED;
			break;
		default:
			assert(!m_newline_lead);
			m_newline_lead = ch;
		}
		break;

	case CARRIAGE_RETURN:
		switch (m_newline_lead)
		{
		case LINE_FEED:
			++m_vms_newlines;
			m_newline_lead = 0U;
			m_buffer[m_position++] = LINE_FEED;
			break;
		case CARRIAGE_RETURN:
			++m_macintosh_newlines;
			m_buffer[m_position++] = LINE_FEED;
			break;
		default:
			assert(!m_newline_lead);
			m_newline_lead = ch;
		}
		break;

	default:
		switch (m_newline_lead)
		{
		case LINE_FEED:
			++m_unix_newlines;
			m_newline_lead = 0U;
			m_buffer[m_position++] = LINE_FEED;
			process_if_full();
			break;
		case CARRIAGE_RETURN:
			++m_macintosh_newlines;
			m_newline_lead = 0U;
			m_buffer[m_position++] = LINE_FEED;
			process_if_full();
			break;
		default:
			assert(!m_newline_lead);
		};
		m_buffer[m_position++] = ch;
	}

	process_if_full();
}


/*--------------------------------------------------
    cleaner_base::process_if_full
    perform processing on decoded characters if
    buffer is full
--------------------------------------------------*/

void cleaner_base::process_if_full()
{
	if (ARRAY_LENGTH(m_buffer) == m_position)
	{
		process_characters(m_buffer, m_buffer + m_position);
		m_position = 0U;
	}
}


/*--------------------------------------------------
    cleaner_base::handle_lead_byte
    handle an input byte that isn't a valid UTF-8
    continuation
--------------------------------------------------*/

void cleaner_base::handle_lead_byte(std::uint8_t byte)
{
	m_required_bytes =
			((byte & 0xfeU) == 0xfcU) ? 5U :
			((byte & 0xfcU) == 0xf8U) ? 4U :
			((byte & 0xf8U) == 0xf0U) ? 3U :
			((byte & 0xf0U) == 0xe0U) ? 2U :
			((byte & 0xe0U) == 0xc0U) ? 1U : 0U;
	m_code_length = m_required_bytes + 1U;
	if (m_required_bytes)
	{
		m_buffer[m_position] = ((char32_t(1U) << (6U - m_required_bytes)) - 1) & char32_t(byte);
	}
	else if ((byte & 0xc0U) == 0x80U)
	{
		m_buffer[m_position] = REPLACEMENT_CHARACTER;
		++m_continuations;
	}
	else if ((byte & 0xfeU)  == 0xfeU)
	{
		m_buffer[m_position] = REPLACEMENT_CHARACTER;
		++m_invalid_bytes;
	}
	else
	{
		m_buffer[m_position] = byte;
	}
}


/*--------------------------------------------------
    cleaner_base::handle_codepoint
    handle a decoded UTF-8 unit dealing with
    surrogates
--------------------------------------------------*/

void cleaner_base::handle_codepoint(char32_t cp)
{
	if (m_surrogate)
	{
		if (is_low_surrogate(cp))
		{
			++m_surrogate_pairs;
			commit_character(combine_surrogates(m_surrogate, cp));
			m_surrogate = 0U;
		}
		else
		{
			++m_lone_high_surrogates;
			commit_character(REPLACEMENT_CHARACTER);
			m_surrogate = 0U;
			handle_codepoint(cp);
		}
	}
	else if (is_high_surrogate(cp))
	{
		m_surrogate = cp;
	}
	else if (is_low_surrogate(cp))
	{
		++m_lone_low_surrogates;
		commit_character(REPLACEMENT_CHARACTER);
	}
	else
	{
		commit_character(cp);
	}
}



/***************************************************************************
    PLAIN TEXT CLEANER CLASS
***************************************************************************/

class text_cleaner : public cleaner_base
{
public:
	template <typename OutputIt>
	text_cleaner(OutputIt &&output, newline newline_mode, unsigned tab_width)
		: cleaner_base(std::forward<OutputIt>(output), newline_mode, tab_width)
	{
	}

private:
	virtual void process_characters(char32_t const *begin, char32_t const *end) override
	{
		while (begin != end)
			output_character(*begin++);
	}

	virtual void input_complete() override
	{
	}
};



/***************************************************************************
    C++ SOURCE CLEANER CLASS
***************************************************************************/

class cpp_cleaner : public cleaner_base
{
public:
	template <typename OutputIt>
	cpp_cleaner(OutputIt &&output, newline newline_mode, unsigned tab_width);

	virtual bool affected() const override;
	virtual void summarise(std::ostream &os) const override;

protected:
	void output_character(char32_t ch);

private:
	static constexpr char32_t ASTERISK              = 0x0000'002aU;
	static constexpr char32_t SLASH                 = 0x0000'002fU;
	static constexpr char32_t UPPERCASE_FIRST       = 0x0000'0041U;
	static constexpr char32_t UPPERCASE_B           = 0x0000'0042U;
	static constexpr char32_t UPPERCASE_X           = 0x0000'0058U;
	static constexpr char32_t UPPERCASE_LAST        = 0x0000'005aU;
	static constexpr char32_t UNDERSCORE            = 0x0000'005fU;
	static constexpr char32_t LOWERCASE_FIRST       = 0x0000'0061U;
	static constexpr char32_t LOWERCASE_B           = 0x0000'0062U;
	static constexpr char32_t LOWERCASE_X           = 0x0000'0078U;
	static constexpr char32_t LOWERCASE_LAST        = 0x0000'007aU;

	static constexpr char32_t DIGIT_FIRST           = 0x0000'0030U;
	static constexpr char32_t DIGIT_BINARY_LAST     = 0x0000'0031U;
	static constexpr char32_t DIGIT_OCTAL_LAST      = 0x0000'0037U;
	static constexpr char32_t DIGIT_DECIMAL_LAST    = 0x0000'0039U;
	static constexpr char32_t DIGIT_HEX_UPPER_FIRST = 0x0000'0041U;
	static constexpr char32_t DIGIT_HEX_UPPER_LAST  = 0x0000'0046U;
	static constexpr char32_t DIGIT_HEX_LOWER_FIRST = 0x0000'0061U;
	static constexpr char32_t DIGIT_HEX_LOWER_LAST  = 0x0000'0066U;

	enum class parse_state
	{
		DEFAULT,
		COMMENT,
		LINE_COMMENT,
		TOKEN,
		STRING_CONSTANT,
		CHARACTER_CONSTANT,
		NUMERIC_CONSTANT
	};

	virtual void process_characters(char32_t const *begin, char32_t const *end) override;
	virtual void input_complete() override;

	void process_default(char32_t ch);
	void process_comment(char32_t ch);
	void process_line_comment(char32_t ch);
	void process_token(char32_t ch);
	void process_text(char32_t ch);
	void process_numeric(char32_t ch);

	bool tail_is(char32_t ch) const
	{
		return !m_tail.empty() && (m_tail.back() == ch);
	}

	void pop_tail()
	{
		if (!m_tail.empty())
			m_tail.pop_back();
	}

	void replace_tail(char32_t ch)
	{
		assert(!m_tail.empty());
		m_tail.back() = ch;
	}

	void flush_tail()
	{
		for (char32_t tail : m_tail)
			cleaner_base::output_character(tail);
		m_tail.clear();
	}

	static constexpr bool is_token_lead(char32_t ch)
	{
		return
				((UPPERCASE_FIRST <= ch) && (UPPERCASE_LAST >= ch)) ||
				((LOWERCASE_FIRST <= ch) && (LOWERCASE_LAST >= ch)) ||
				(UNDERSCORE == ch);
	}

	static constexpr bool is_token_continuation(char32_t ch)
	{
		return
				is_token_lead(ch) ||
				((DIGIT_FIRST <= ch) && (DIGIT_DECIMAL_LAST >= ch));
	}

	static constexpr bool is_numeric_lead(char32_t ch)
	{
		return (DIGIT_FIRST <= ch) && (DIGIT_DECIMAL_LAST >= ch);
	}

	static constexpr bool is_binary_digit(char32_t ch)
	{
		return (DIGIT_FIRST <= ch) && (DIGIT_BINARY_LAST >= ch);
	}

	static constexpr bool is_octal_digit(char32_t ch)
	{
		return (DIGIT_FIRST <= ch) && (DIGIT_OCTAL_LAST >= ch);
	}

	static constexpr bool is_decimal_digit(char32_t ch)
	{
		return (DIGIT_FIRST <= ch) && (DIGIT_DECIMAL_LAST >= ch);
	}

	static constexpr bool is_hexadecimal_digit(char32_t ch)
	{
		return
				((DIGIT_FIRST <= ch) && (DIGIT_DECIMAL_LAST >= ch)) ||
				((DIGIT_HEX_UPPER_FIRST <= ch) && (DIGIT_HEX_UPPER_LAST >= ch)) ||
				((DIGIT_HEX_LOWER_FIRST <= ch) && (DIGIT_HEX_LOWER_LAST >= ch));
	}

	parse_state                 m_parse_state   = parse_state::DEFAULT;
	std::uint64_t               m_input_line    = 1U;
	bool                        m_escape        = false;
	std::deque<char32_t>        m_tail;
	std::uint64_t               m_comment_line  = 0U;
	bool                        m_broken_escape = false;
	char32_t                    m_lead_digit    = 0U;
	unsigned                    m_radix         = 0U;

	std::uint64_t   m_tabs_escaped                  = 0U;
	std::uint64_t   m_broken_comment_delimiters     = 0U;
	std::uint64_t   m_line_comment_continuations    = 0U;
	std::uint64_t   m_string_continuations          = 0U;
	std::uint64_t   m_uppercase_radix               = 0U;
	std::uint64_t   m_non_ascii                     = 0U;
};


template <typename OutputIt>
cpp_cleaner::cpp_cleaner(
		OutputIt &&output,
		newline newline_mode,
		unsigned tab_width)
	: cleaner_base(std::forward<OutputIt>(output), newline_mode, tab_width)
	, m_tail()
{
}


bool cpp_cleaner::affected() const
{
	return
			cleaner_base::affected() ||
			m_tabs_escaped ||
			m_broken_comment_delimiters ||
			m_line_comment_continuations ||
			m_string_continuations ||
			m_uppercase_radix ||
			m_non_ascii;
}


void cpp_cleaner::summarise(std::ostream &os) const
{
	cleaner_base::summarise(os);
	if (m_tabs_escaped)
		util::stream_format(os, "%1$u tab(s) escaped\n", m_tabs_escaped);
	if (m_broken_comment_delimiters)
		util::stream_format(os, "%1$u broken comment delimiter(s) replaced\n", m_broken_comment_delimiters);
	if (m_line_comment_continuations)
		util::stream_format(os, "%1$u line comment continuation(s) replaced\n", m_line_comment_continuations);
	if (m_string_continuations)
		util::stream_format(os, "%1$u string literal continuation(s) replaced\n", m_string_continuations);
	if (m_uppercase_radix)
		util::stream_format(os, "%1$u uppercase radix character(s) normalised\n", m_uppercase_radix);
	if (m_non_ascii)
		util::stream_format(os, "%1$u non-ASCII character(s) replaced\n", m_non_ascii);
}


void cpp_cleaner::output_character(char32_t ch)
{
	switch (m_parse_state)
	{
	case parse_state::DEFAULT:
	case parse_state::TOKEN:
	case parse_state::CHARACTER_CONSTANT:
	case parse_state::NUMERIC_CONSTANT:
		if (BASIC_LATIN_LAST < ch)
		{
			++m_non_ascii;
			ch = QUESTION_MARK;
		}
		break;
	case parse_state::COMMENT:
	case parse_state::LINE_COMMENT:
		break;
	case parse_state::STRING_CONSTANT:
		if (CYRILLIC_SUPPLEMENT_LAST < ch)
		{
			++m_non_ascii;
			ch = QUESTION_MARK;
		}
		break;
	}

	switch (ch)
	{
	case HORIZONTAL_TAB:
	case SPACE:
	case BACKSLASH:
		m_tail.emplace_back(ch);
		break;
	default:
		flush_tail();
		if (LINE_FEED == ch)
			cleaner_base::output_character(ch);
		else
			m_tail.emplace_back(ch);
	}
}


void cpp_cleaner::process_characters(char32_t const *begin, char32_t const *end)
{
	while (begin != end)
	{
		char32_t const ch(*begin++);
		switch (m_parse_state)
		{
		case parse_state::DEFAULT:
			process_default(ch);
			break;
		case parse_state::COMMENT:
			process_comment(ch);
			break;
		case parse_state::LINE_COMMENT:
			process_line_comment(ch);
			break;
		case parse_state::TOKEN:
			process_token(ch);
			break;
		case parse_state::CHARACTER_CONSTANT:
		case parse_state::STRING_CONSTANT:
			process_text(ch);
			break;
		case parse_state::NUMERIC_CONSTANT:
			process_numeric(ch);
			break;
		}

		if (LINE_FEED == ch)
			++m_input_line;
	}
}


void cpp_cleaner::input_complete()
{
	flush_tail();
	switch (m_parse_state)
	{
	case parse_state::COMMENT:
		throw std::runtime_error(util::string_format("unterminated multi-line comment beginning on line %1$u", m_comment_line));
	case parse_state::CHARACTER_CONSTANT:
		throw std::runtime_error(util::string_format("unterminated character literal on line %1$u", m_input_line));
	case parse_state::STRING_CONSTANT:
		throw std::runtime_error(util::string_format("unterminated string literal on line %1$u", m_input_line));
	default:
		break;
	}
}


void cpp_cleaner::process_default(char32_t ch)
{
	switch (ch)
	{
	case LINE_FEED:
		if (m_escape && tail_is(BACKSLASH))
		{
			m_broken_escape = true;
			return;
		}
		break;
	case DOUBLE_QUOTE:
		m_parse_state = parse_state::STRING_CONSTANT;
		break;
	case SINGLE_QUOTE:
		m_parse_state = parse_state::CHARACTER_CONSTANT;
		break;
	case ASTERISK:
		if (m_escape)
		{
			m_parse_state = parse_state::COMMENT;
			m_comment_line = m_input_line;
			set_tab_limit();
			if (m_broken_escape)
			{
				++m_broken_comment_delimiters;
				assert(tail_is(BACKSLASH));
				pop_tail();
				output_character(ch);
				output_character(LINE_FEED);
				m_escape = false;
				m_broken_escape = false;
				return;
			}
		}
		break;
	case SLASH:
		if (m_escape)
		{
			m_parse_state = parse_state::LINE_COMMENT;
			if (m_broken_escape)
			{
				++m_broken_comment_delimiters;
				assert(tail_is(BACKSLASH));
				pop_tail();
				assert(tail_is(SLASH));
				pop_tail();
				output_character(LINE_FEED);
				output_character(SLASH);
				m_broken_escape = false;
			}
		}
		break;
	default:
		if (is_token_lead(ch))
		{
			m_parse_state = parse_state::TOKEN;
		}
		else if (is_numeric_lead(ch))
		{
			m_parse_state = parse_state::NUMERIC_CONSTANT;
			m_escape = false;
			m_broken_escape = false;
			process_numeric(ch);
			return;
		}
	}
	if (m_broken_escape)
		output_character(LINE_FEED);
	m_escape = m_escape ? ((BACKSLASH == ch) && tail_is(SLASH)) : (SLASH == ch);
	m_broken_escape = false;
	output_character(ch);
}


void cpp_cleaner::process_comment(char32_t ch)
{
	switch (ch)
	{
	case LINE_FEED:
		if (m_escape && tail_is(BACKSLASH))
		{
			m_broken_escape = true;
		}
		else
		{
			m_escape = false;
			m_broken_escape = false;
			output_character(ch);
		}
		break;
	case SLASH:
		if (m_broken_escape)
		{
			m_parse_state = parse_state::DEFAULT;
			m_comment_line = 0U;
			++m_broken_comment_delimiters;
			assert(tail_is(BACKSLASH));
			pop_tail();
			assert(tail_is(ASTERISK));
			pop_tail();
			output_character(LINE_FEED);
			output_character(ASTERISK);
			output_character(ch);
			reset_tab_limit();
		}
		else if (m_escape)
		{
			m_parse_state = parse_state::DEFAULT;
			m_comment_line = 0U;
			output_character(ch);
			reset_tab_limit();
		}
		else
		{
			output_character(ch);
		}
		m_escape = false;
		m_broken_escape = false;
		break;
	case BACKSLASH:
		if (m_broken_escape)
		{
			m_escape = false;
			m_broken_escape = false;
			output_character(LINE_FEED);
		}
		else if (m_escape)
		{
			m_escape = tail_is(ASTERISK);
		}
		output_character(ch);
		break;
	default:
		if (m_broken_escape)
			output_character(LINE_FEED);
		m_escape = ASTERISK == ch;
		m_broken_escape = false;
		output_character(ch);
	}
}


void cpp_cleaner::process_line_comment(char32_t ch)
{
	switch (ch)
	{
	case LINE_FEED:
		if (tail_is(BACKSLASH))
		{
			++m_line_comment_continuations;
			pop_tail();
			output_character(ch);
			output_character(SLASH);
			output_character(SLASH);
			break;
		}
		m_parse_state = parse_state::DEFAULT;
		[[fallthrough]];
	default:
		output_character(ch);
	}
}


void cpp_cleaner::process_token(char32_t ch)
{
	if (is_token_continuation(ch))
	{
		output_character(ch);
	}
	else
	{
		m_parse_state = parse_state::DEFAULT;
		process_default(ch);
	}
}


void cpp_cleaner::process_text(char32_t ch)
{
	switch (ch)
	{
	case HORIZONTAL_TAB:
		++m_tabs_escaped;
		if (!m_escape)
			output_character(BACKSLASH);
		output_character(char32_t(std::uint8_t('t')));
		break;
	case LINE_FEED:
		if (parse_state::CHARACTER_CONSTANT == m_parse_state)
		{
			throw std::runtime_error(util::string_format("unterminated character literal on line %1$u", m_input_line));
		}
		else if (tail_is(BACKSLASH))
		{
			++m_string_continuations;
			if (m_escape)
			{
				replace_tail(DOUBLE_QUOTE);
				output_character(ch);
				output_character(DOUBLE_QUOTE);
			}
			else
			{
				pop_tail();
				assert(tail_is(BACKSLASH));
				pop_tail();
				output_character(DOUBLE_QUOTE);
				output_character(ch);
				output_character(DOUBLE_QUOTE);
				output_character(BACKSLASH);
				m_escape = true;
				return;
			}
		}
		else
		{
			throw std::runtime_error(util::string_format("unterminated string literal on line %1$u", m_input_line));
		}
		break;
	case VERTICAL_TAB:
		++m_tabs_escaped;
		if (!m_escape)
			output_character(BACKSLASH);
		output_character(char32_t(std::uint8_t('v')));
		break;
	default:
		output_character(ch);
		if (!m_escape && (((parse_state::STRING_CONSTANT == m_parse_state) ? DOUBLE_QUOTE : SINGLE_QUOTE) == ch))
			m_parse_state = parse_state::DEFAULT;
	}
	m_escape = (BACKSLASH == ch) && !m_escape;
}


void cpp_cleaner::process_numeric(char32_t ch)
{
	if (!m_lead_digit)
	{
		assert(is_numeric_lead(ch));
		assert(!m_radix);

		m_lead_digit = ch;
		if (DIGIT_FIRST != ch)
			m_radix = 10U;
	}
	else if (!m_radix)
	{
		assert(DIGIT_FIRST == m_lead_digit);

		switch (ch)
		{
		case SINGLE_QUOTE:
			if (m_escape)
				throw std::runtime_error(util::string_format("adjacent digit separators on line %1$u", m_input_line));
			else
				m_escape = true;
			break;
		case UPPERCASE_B:
			++m_uppercase_radix;
			ch = LOWERCASE_B;
			[[fallthrough]];
		case LOWERCASE_B:
			m_radix = 2U;
			break;
		case UPPERCASE_X:
			++m_uppercase_radix;
			ch = LOWERCASE_X;
			[[fallthrough]];
		case LOWERCASE_X:
			m_radix = 16U;
			break;
		default:
			if (is_octal_digit(ch))
				m_radix = 8U;
			else if (is_decimal_digit(ch))
				m_parse_state = parse_state::DEFAULT; // this should be an invalid octal literal, but it's probably just an argument to the SHA1 macro
			else
				m_parse_state = parse_state::DEFAULT;
		}
	}
	else
	{
		if (SINGLE_QUOTE == ch)
		{
			if (m_escape)
				throw std::runtime_error(util::string_format("adjacent digit separators on line %1$u", m_input_line));
			else
				m_escape = true;
		}
		else
		{
			m_escape = false;
			switch (m_radix)
			{
			case 2U:
				if (!is_decimal_digit(ch))
					m_parse_state = parse_state::DEFAULT;
				else if (!is_binary_digit(ch))
					m_parse_state = parse_state::DEFAULT; // this should be an invalid binary literal, but it's probably just an argument to the SHA1 macro
				break;
			case 8U:
				if (!is_decimal_digit(ch))
					m_parse_state = parse_state::DEFAULT;
				else if (!is_octal_digit(ch))
					m_parse_state = parse_state::DEFAULT; // this should be an invalid octal literal, but it's probably just an argument to the SHA1 macro
				break;
			case 10U:
				if (!is_decimal_digit(ch))
					m_parse_state = parse_state::DEFAULT;
				break;
			case 16U:
				if (!is_hexadecimal_digit(ch))
					m_parse_state = parse_state::DEFAULT;
				break;
			default:
				assert(false);
				m_parse_state = parse_state::DEFAULT;
			}
		}
	}

	if (parse_state::DEFAULT == m_parse_state)
	{
		m_escape = false;
		m_lead_digit = 0U;
		m_radix = 0U;
		process_default(ch);
	}
	else
	{
		assert(parse_state::NUMERIC_CONSTANT == m_parse_state);

		output_character(ch);
	}
}



/***************************************************************************
    LUA SOURCE CLEANER CLASS
***************************************************************************/

class lua_cleaner : public cleaner_base
{
public:
	template <typename OutputIt>
	lua_cleaner(OutputIt &&output, newline newline_mode, unsigned tab_width);

	virtual bool affected() const override;
	virtual void summarise(std::ostream &os) const override;

protected:
	void output_character(char32_t ch);

private:
	static constexpr char32_t EQUALS        = 0x0000'003dU;
	static constexpr char32_t LEFT_BRACKET  = 0x0000'005bU;
	static constexpr char32_t RIGHT_BRACKET = 0x0000'005dU;

	enum class parse_state
	{
		DEFAULT,
		SHORT_COMMENT,
		LONG_COMMENT,
		STRING_CONSTANT,
		LONG_STRING_CONSTANT
	};

	virtual void process_characters(char32_t const *begin, char32_t const *end) override;
	virtual void input_complete() override;

	void process_default(char32_t ch);
	void process_short_comment(char32_t ch);
	void process_long_comment(char32_t ch);
	void process_string_constant(char32_t ch);
	void process_long_string_constant(char32_t ch);

	parse_state     m_parse_state           = parse_state::DEFAULT;
	std::uint64_t   m_input_line            = 1U;
	int             m_long_bracket_level    = -1;
	bool            m_escape                = false;
	std::uint32_t   m_block_line            = 0U;
	int             m_block_level           = 0;
	bool            m_comment_start         = false;
	char32_t        m_string_quote          = 0U;

	std::uint64_t   m_tabs_escaped      = 0U;
	std::uint64_t   m_newlines_escaped  = 0U;
	std::uint64_t   m_non_ascii         = 0U;
};


template <typename OutputIt>
lua_cleaner::lua_cleaner(
		OutputIt &&output,
		newline newline_mode,
		unsigned tab_width)
	: cleaner_base(std::forward<OutputIt>(output), newline_mode, tab_width)
{
}


bool lua_cleaner::affected() const
{
	return
			cleaner_base::affected() ||
			m_tabs_escaped ||
			m_newlines_escaped ||
			m_non_ascii;
}


void lua_cleaner::summarise(std::ostream &os) const
{
	cleaner_base::summarise(os);
	if (m_tabs_escaped)
		util::stream_format(os, "%1$u tab(s) escaped\n", m_tabs_escaped);
	if (m_newlines_escaped)
		util::stream_format(os, "%1$u escaped line ending(s) converted\n", m_newlines_escaped);
	if (m_non_ascii)
		util::stream_format(os, "%1$u non-ASCII character(s) replaced\n", m_non_ascii);
}


void lua_cleaner::output_character(char32_t ch)
{
	switch (m_parse_state)
	{
	case parse_state::DEFAULT:
		if (BASIC_LATIN_LAST < ch)
		{
			++m_non_ascii;
			ch = QUESTION_MARK;
		}
		break;
	case parse_state::SHORT_COMMENT:
	case parse_state::LONG_COMMENT:
		break;
	case parse_state::STRING_CONSTANT:
	case parse_state::LONG_STRING_CONSTANT:
		if (CYRILLIC_SUPPLEMENT_LAST < ch)
		{
			++m_non_ascii;
			ch = QUESTION_MARK;
		}
		break;
	}

	cleaner_base::output_character(ch);
}


void lua_cleaner::process_characters(char32_t const *begin, char32_t const *end)
{
	while (begin != end)
	{
		char32_t const ch(*begin++);
		switch (m_parse_state)
		{
		case parse_state::DEFAULT:
			process_default(ch);
			break;
		case parse_state::SHORT_COMMENT:
			process_short_comment(ch);
			break;
		case parse_state::LONG_COMMENT:
			process_long_comment(ch);
			break;
		case parse_state::STRING_CONSTANT:
			process_string_constant(ch);
			break;
		case parse_state::LONG_STRING_CONSTANT:
			process_long_string_constant(ch);
			break;
		}

		if (LINE_FEED == ch)
			++m_input_line;
	}
}


void lua_cleaner::input_complete()
{
	switch (m_parse_state)
	{
	case parse_state::LONG_COMMENT:
		throw std::runtime_error(util::string_format("unterminated long comment beginning on line %1$u", m_block_line));
	case parse_state::STRING_CONSTANT:
		throw std::runtime_error(util::string_format("unterminated string literal on line %1$u", m_input_line));
	case parse_state::LONG_STRING_CONSTANT:
		throw std::runtime_error(util::string_format("unterminated long string literal beginning on line %1$u", m_block_line));
	default:
		break;
	}
}


void lua_cleaner::process_default(char32_t ch)
{
	switch (ch)
	{
	case DOUBLE_QUOTE:
	case SINGLE_QUOTE:
		m_string_quote = ch;
		m_parse_state = parse_state::STRING_CONSTANT;
		break;
	case HYPHEN_MINUS:
		if (m_escape)
		{
			m_comment_start = true;
			m_parse_state = parse_state::SHORT_COMMENT;
		}
		break;
	default:
		break;
	}
	if (0 <= m_long_bracket_level)
	{
		switch (ch)
		{
		case EQUALS:
			++m_long_bracket_level;
			break;
		case LEFT_BRACKET:
			m_block_line = m_input_line;
			m_block_level = m_long_bracket_level;
			m_parse_state = parse_state::LONG_STRING_CONSTANT;
			[[fallthrough]];
		default:
			m_long_bracket_level = -1;
		}
	}
	else if (LEFT_BRACKET == ch)
	{
		m_long_bracket_level = 0;
	}
	m_escape = (HYPHEN_MINUS == ch) && !m_escape;
	output_character(ch);
}


void lua_cleaner::process_short_comment(char32_t ch)
{
	if (0 <= m_long_bracket_level)
	{
		switch (ch)
		{
		case EQUALS:
			++m_long_bracket_level;
			break;
		case LEFT_BRACKET:
			m_block_line = m_input_line;
			m_block_level = m_long_bracket_level;
			m_parse_state = parse_state::LONG_COMMENT;
			set_tab_limit();
			[[fallthrough]];
		default:
			m_long_bracket_level = -1;
		}
	}
	else if (m_comment_start && (LEFT_BRACKET == ch))
	{
		m_long_bracket_level = 0;
	}
	else if (LINE_FEED == ch)
	{
		m_parse_state = parse_state::DEFAULT;
	}
	m_comment_start = false;
	output_character(ch);
}


void lua_cleaner::process_long_comment(char32_t ch)
{
	if (0 <= m_long_bracket_level)
	{
		switch (ch)
		{
		case EQUALS:
			++m_long_bracket_level;
			break;
		case RIGHT_BRACKET:
			if (m_long_bracket_level == m_block_level)
			{
				m_parse_state = parse_state::DEFAULT;
				reset_tab_limit();
			}
			else
			{
				m_long_bracket_level = 0;
			}
			break;
		default:
			m_long_bracket_level = -1;
		}
	}
	else if (RIGHT_BRACKET == ch)
	{
		m_long_bracket_level = 0;
	}
	output_character(ch);
}


void lua_cleaner::process_string_constant(char32_t ch)
{
	switch (ch)
	{
	case HORIZONTAL_TAB:
		++m_tabs_escaped;
		if (!m_escape)
			output_character(BACKSLASH);
		output_character(char32_t(std::uint8_t('t')));
		break;
	case LINE_FEED:
		if (m_escape)
		{
			++m_newlines_escaped;
			output_character(char32_t(std::uint8_t('n')));
		}
		else
		{
			throw std::runtime_error(util::string_format("unterminated string literal on line %1$u", m_input_line));
		}
		break;
	case VERTICAL_TAB:
		++m_tabs_escaped;
		if (!m_escape)
			output_character(BACKSLASH);
		output_character(char32_t(std::uint8_t('v')));
		break;
	default:
		output_character(ch);
		if (!m_escape && (m_string_quote == ch))
			m_parse_state = parse_state::DEFAULT;
	}
	m_escape = (BACKSLASH == ch) && !m_escape;
}


void lua_cleaner::process_long_string_constant(char32_t ch)
{
	// this works because they're both closed by a matching long bracket
	process_long_comment(ch);
}



/***************************************************************************
    XML DATA CLEANER CLASS
***************************************************************************/

class xml_cleaner : public cleaner_base
{
public:
	template <typename OutputIt>
	xml_cleaner(OutputIt &&output, newline newline_mode, unsigned tab_width);

private:
	static constexpr char32_t EXCLAMATION           = 0x0000'0021U;
	static constexpr char32_t LEFT_ANGLE_BRACKET    = 0x0000'003cU;
	static constexpr char32_t RIGHT_ANGLE_BRACKET   = 0x0000'003eU;

	enum class parse_state
	{
		DEFAULT,
		COMMENT
	};

	virtual void process_characters(char32_t const *begin, char32_t const *end) override;
	virtual void input_complete() override;

	void process_default(char32_t ch);
	void process_comment(char32_t ch);

	parse_state     m_parse_state   = parse_state::DEFAULT;
	std::uint64_t   m_input_line    = 1U;
	unsigned        m_escape        = 0U;
	std::uint64_t   m_comment_line  = 0U;
};


template <typename OutputIt>
xml_cleaner::xml_cleaner(
		OutputIt &&output,
		newline newline_mode,
		unsigned tab_width)
	: cleaner_base(std::forward<OutputIt>(output), newline_mode, tab_width)
{
}


void xml_cleaner::process_characters(char32_t const *begin, char32_t const *end)
{
	while (begin != end)
	{
		char32_t const ch(*begin++);
		switch (m_parse_state)
		{
		case parse_state::DEFAULT:
			process_default(ch);
			break;
		case parse_state::COMMENT:
			process_comment(ch);
			break;
		}

		if (LINE_FEED == ch)
			++m_input_line;
	}
}


void xml_cleaner::input_complete()
{
	if (parse_state::COMMENT == m_parse_state)
		throw std::runtime_error(util::string_format("unterminated comment beginning on line %1$u", m_comment_line));
}


void xml_cleaner::process_default(char32_t ch)
{
	assert(4U > m_escape);

	switch (m_escape)
	{
	case 0U:
		m_escape = (LEFT_ANGLE_BRACKET == ch) ? (m_escape + 1U) : 0U;
		break;
	case 1U:
		m_escape = (EXCLAMATION == ch) ? (m_escape + 1U) : 0U;
		break;
	case 2U:
	case 3U:
		m_escape = (HYPHEN_MINUS == ch) ? (m_escape + 1U) : 0U;
		break;
	}
	output_character(ch);

	if (4U == m_escape)
	{
		m_parse_state = parse_state::COMMENT;
		m_escape = 0U;
		m_comment_line = m_input_line;
		set_tab_limit();
	}
}


void xml_cleaner::process_comment(char32_t ch)
{
	assert(3U > m_escape);

	switch (m_escape)
	{
	case 0U:
	case 1U:
		m_escape = (HYPHEN_MINUS == ch) ? (m_escape + 1U) : 0U;
		break;
	case 2U:
		m_escape = (RIGHT_ANGLE_BRACKET == ch) ? (m_escape + 1U) : (HYPHEN_MINUS == ch) ? m_escape : 0U;
		break;
	}
	output_character(ch);

	if (3U == m_escape)
	{
		m_parse_state = parse_state::DEFAULT;
		m_escape = 0U;
		m_comment_line = 0U;
		reset_tab_limit();
	}
}



/***************************************************************************
    UTILITY FUNCTIONS
***************************************************************************/

bool is_c_source_extension(char const *ext)
{
	return
			!core_stricmp(ext, ".c") ||
			!core_stricmp(ext, ".h") ||
			!core_stricmp(ext, ".cpp") ||
			!core_stricmp(ext, ".hpp") ||
			!core_stricmp(ext, ".ipp") ||
			!core_stricmp(ext, ".cxx") ||
			!core_stricmp(ext, ".hxx") ||
			!core_stricmp(ext, ".ixx") ||
			!core_stricmp(ext, ".lst");
}


bool is_lua_source_extension(char const *ext)
{
	return
			!core_stricmp(ext, ".lua");
}


bool is_xml_extension(char const *ext)
{
	return
			!core_stricmp(ext, ".hsi") ||
			!core_stricmp(ext, ".lay") ||
			!core_stricmp(ext, ".xml") ||
			!core_stricmp(ext, ".xslt");
}

} // anonymous namespace



/***************************************************************************
    MAIN
***************************************************************************/

int main(int argc, char *argv[])
{
	bool                    keep_backup(false);
	bool                    dry_run(false);
#if defined(WIN32)
	cleaner_base::newline   newline_mode(cleaner_base::newline::DOS);
#else
	cleaner_base::newline   newline_mode(cleaner_base::newline::UNIX);
#endif
	for (bool arg_found = true; arg_found && (argc > 1); )
	{
		if (!std::strcmp(argv[1], "-b"))
			keep_backup = true;
		else if (!std::strcmp(argv[1], "-d"))
			dry_run = true;
		else if (!std::strcmp(argv[1], "-m"))
			newline_mode = cleaner_base::newline::MACINTOSH;
		else if (!std::strcmp(argv[1], "-u"))
			newline_mode = cleaner_base::newline::UNIX;
		else if (!std::strcmp(argv[1], "-w"))
			newline_mode = cleaner_base::newline::DOS;
		else
			arg_found = false;

		if (arg_found)
		{
			argc--;
			argv++;
		}
	}

	if (argc < 2)
	{
		printf("Usage: srcclean [-b] [-d] [-m] [-u] [-w] <file>...\n");
		return 0;
	}

	bool                affected(false);
	unsigned            failures(0U);
	char                original[1024];
	std::vector<char>   output;
	output.reserve(32 * 1024 * 1024);
	for (int i = 1; i < argc; ++i)
	{
		// open the file
		util::core_file::ptr infile;
		osd_file::error const err(util::core_file::open(argv[i], OPEN_FLAG_READ, infile));
		if (osd_file::error::NONE != err)
		{
			if (affected)
				std::cerr << std::endl;
			affected = true;
			util::stream_format(std::cerr, "Can't open %1$s\n", argv[i]);
			++failures;
			continue;
		}

		try
		{
			// instantiate appropriate cleaner implementation
			char const *const ext(std::strrchr(argv[i], '.'));
			bool const is_c_file(ext && is_c_source_extension(ext));
			bool const is_lua_file(ext && is_lua_source_extension(ext));
			bool const is_xml_file(ext && is_xml_extension(ext));
			std::unique_ptr<cleaner_base> cleaner;
			if (is_c_file)
				cleaner = std::make_unique<cpp_cleaner>(std::back_inserter(output), newline_mode, 4U);
			else if (is_lua_file)
				cleaner = std::make_unique<lua_cleaner>(std::back_inserter(output), newline_mode, 4U);
			else if (is_xml_file)
				cleaner = std::make_unique<xml_cleaner>(std::back_inserter(output), newline_mode, 4U);
			else
				cleaner = std::make_unique<text_cleaner>(std::back_inserter(output), newline_mode, 4U);

			// read/process in chunks
			output.clear();
			std::uint64_t remaining(infile->size());
			std::uint32_t block;
			while (remaining && (0U != (block = infile->read(original, (std::min)(std::uint64_t(sizeof(original)), remaining)))))
			{
				remaining -= block;
				cleaner->process(original, original + block);
			}
			if (remaining)
			{
				if (affected)
					std::cerr << std::endl;
				affected = true;
				util::stream_format(std::cerr, "Can't read %1$s\n", argv[i]);
				++failures;
				continue;
			}
			cleaner->finalise();
			infile.reset();
			if (cleaner->affected())
			{
				// print report
				if (affected)
					std::cerr << std::endl;
				affected = true;
				util::stream_format(std::cerr, "Cleaned up %1$s:\n", argv[i]);
				cleaner->summarise(std::cerr);
				cleaner.reset();

				// replace the file if it isn't a dry run
				if (!dry_run)
				{
					using namespace std::string_literals;
					std::string const backup(argv[i] + ".orig"s);
					std::remove(backup.c_str());
					if (std::rename(argv[i], backup.c_str()))
					{
						util::stream_format(std::cerr, "Error moving %1$s to backup location\n", argv[i]);
						++failures;
					}
					else
					{
						std::ofstream outfile(argv[i], std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
						outfile.write(&output[0], output.size());
						outfile.flush();
						if (!outfile)
						{
							util::stream_format(std::cerr, "Error writing output to %1$s\n", argv[i]);
							++failures;
							outfile.close();
							if (std::rename(backup.c_str(), argv[i]))
								util::stream_format(std::cerr, "Error restoring backup of %1$s\n", argv[i]);
						}
						else if (!keep_backup)
						{
							if (std::remove(backup.c_str()))
							{
								util::stream_format(std::cerr, "Error removing backup of %1$s\n", argv[i]);
								++failures;
							}
						}
					}
				}
			}
		}
		catch (std::runtime_error const &ex)
		{
			// print error message and try the next file
			if (affected)
				std::cerr << std::endl;
			affected = true;
			util::stream_format(std::cerr, "Error cleaning %1$s: %2$s\n", argv[i], ex.what());
			++failures;
			continue;
		}
	}

	return failures ? 1 : 0;
}
