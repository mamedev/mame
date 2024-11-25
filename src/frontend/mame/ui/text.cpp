// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Vas Crabb
/*********************************************************************

    text.cpp

    Text functionality for MAME's crude user interface

*********************************************************************/

#include "emu.h"
#include "text.h"

#include "render.h"
#include "rendfont.h"

#include "util/unicode.h"

#include <cstddef>
#include <cstring>
#include <utility>


namespace ui {

/***************************************************************************
INLINE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  is_space_character
//-------------------------------------------------

inline bool is_space_character(char32_t ch)
{
	return ch == ' ';
}


//-------------------------------------------------
//  is_breakable_char - is a given unicode
//  character a possible line break?
//-------------------------------------------------

inline bool is_breakable_char(char32_t ch)
{
	// regular spaces and hyphens are breakable
	if (is_space_character(ch) || ch == '-')
		return true;

	// In the following character sets, any character is breakable:
	//  Hiragana (3040-309F)
	//  Katakana (30A0-30FF)
	//  Bopomofo (3100-312F)
	//  Hangul Compatibility Jamo (3130-318F)
	//  Kanbun (3190-319F)
	//  Bopomofo Extended (31A0-31BF)
	//  CJK Strokes (31C0-31EF)
	//  Katakana Phonetic Extensions (31F0-31FF)
	//  Enclosed CJK Letters and Months (3200-32FF)
	//  CJK Compatibility (3300-33FF)
	//  CJK Unified Ideographs Extension A (3400-4DBF)
	//  Yijing Hexagram Symbols (4DC0-4DFF)
	//  CJK Unified Ideographs (4E00-9FFF)
	if (ch >= 0x3040 && ch <= 0x9fff)
		return true;

	// Hangul Syllables (AC00-D7AF) are breakable
	if (ch >= 0xac00 && ch <= 0xd7af)
		return true;

	// CJK Compatibility Ideographs (F900-FAFF) are breakable
	if (ch >= 0xf900 && ch <= 0xfaff)
		return true;

	return false;
}



/***************************************************************************
CLASS TO REPRESENT A LINE
***************************************************************************/

// information about the "source" of a character - also in a struct
// to facilitate copying
struct text_layout::source_info
{
	size_t start;
	size_t span;
};


// this should really be "positioned glyph" as glyphs != characters, but
// we'll get there eventually
struct text_layout::positioned_char
{
	char32_t character;
	char_style style;
	source_info source;
	float xoffset;
	float xwidth;
};


class text_layout::line
{
public:
	using size_type = size_t;
	static constexpr size_type npos = ~size_type(0);

	line(float yoffset, float height) : m_yoffset(yoffset), m_height(height)
	{
	}

	// methods
	void add_character(text_layout &layout, char32_t ch, char_style const &style, source_info const &source)
	{
		// get the width of this character
		float const chwidth = layout.get_char_width(ch, style.size);

		// append the positioned character
		m_characters.emplace_back(positioned_char{ ch, style, source, m_width, chwidth });
		m_width += chwidth;

		// we might be bigger
		m_height = std::max(m_height, style.size * layout.yscale());
	}

	void truncate(size_t position)
	{
		assert(position <= m_characters.size());

		// are we actually truncating?
		if (position < m_characters.size())
		{
			// set the width as appropriate
			m_width = m_characters[position].xoffset;

			// and resize the array
			m_characters.resize(position);
		}
	}

	void set_justification(text_justify justify)
	{
		switch (justify)
		{
		case text_justify::RIGHT:
			if (npos == m_right_justify_start)
				m_right_justify_start = m_characters.size();
			[[fallthrough]];
		case text_justify::CENTER:
			if (npos == m_center_justify_start)
				m_center_justify_start = m_characters.size();
			break;
		case text_justify::LEFT:
			break;
		}
	}

	void align_text(text_layout const &layout)
	{
		assert(m_right_justify_start >= m_center_justify_start);

		if (m_characters.empty() || m_center_justify_start)
		{
			// at least some of the text is left-justified - anchor to left
			m_anchor_pos = 0.0f;
			m_anchor_target = 0.0f;
			if ((layout.width() > m_width) && (m_characters.size() > m_center_justify_start))
			{
				// at least some text is not left-justified
				if (m_right_justify_start == m_center_justify_start)
				{
					// all text that isn't left-justified is right-justified
					float const right_offset = layout.width() - m_width;
					for (size_t i = m_right_justify_start; m_characters.size() > i; ++i)
						m_characters[i].xoffset += right_offset;
					m_width = layout.width();
				}
				else if (m_characters.size() <= m_right_justify_start)
				{
					// all text that isn't left-justified is center-justified
					float const center_width = m_width - m_characters[m_center_justify_start].xoffset;
					float const center_offset = ((layout.width() - center_width) * 0.5f) - m_characters[m_center_justify_start].xoffset;
					if (0.0f < center_offset)
					{
						for (size_t i = m_center_justify_start; m_characters.size() > i; ++i)
							m_characters[i].xoffset += center_offset;
						m_width += center_offset;
					}
				}
				else
				{
					// left, right and center-justified parts
					float const center_width = m_characters[m_right_justify_start].xoffset - m_characters[m_center_justify_start].xoffset;
					float const center_offset = ((layout.width() - center_width) * 0.5f) - m_characters[m_center_justify_start].xoffset;
					float const right_offset = layout.width() - m_width;
					if (center_offset > right_offset)
					{
						// right-justified text pushes centre-justified text to the left
						for (size_t i = m_center_justify_start; m_right_justify_start > i; ++i)
							m_characters[i].xoffset += right_offset;
					}
					else if (0.0f < center_offset)
					{
						// left-justified text doesn't push centre-justified text to the right
						for (size_t i = m_center_justify_start; m_right_justify_start > i; ++i)
							m_characters[i].xoffset += center_offset;
					}
					for (size_t i = m_right_justify_start; m_characters.size() > i; ++i)
						m_characters[i].xoffset += right_offset;
					m_width = layout.width();
				}
			}
		}
		else if (m_characters.size() <= m_right_justify_start)
		{
			// all text is center-justified - anchor to center
			m_anchor_pos = 0.5f;
			m_anchor_target = 0.5f;
		}
		else
		{
			// at least some text is right-justified - anchor to right
			m_anchor_pos = 1.0f;
			m_anchor_target = 1.0f;
			if ((layout.width() > m_width) && (m_right_justify_start > m_center_justify_start))
			{
				// mixed center-justified and right-justified text
				float const center_width = m_characters[m_right_justify_start].xoffset;
				float const center_offset = (layout.width() - m_width + (center_width * 0.5f)) - (layout.width() * 0.5f);
				if (0.0f < center_offset)
				{
					for (size_t i = m_right_justify_start; m_characters.size() > i; ++i)
						m_characters[i].xoffset += center_offset;
					m_width += center_offset;
				}
			}
		}
	}

	// accessors
	float xoffset(text_layout const &layout) const { return (layout.width() * m_anchor_target) - (m_width * m_anchor_pos); }
	float yoffset() const { return m_yoffset; }
	float width() const { return m_width; }
	float height() const { return m_height; }
	size_t character_count() const { return m_characters.size(); }
	size_t center_justify_start() const { return m_center_justify_start; }
	size_t right_justify_start() const { return m_right_justify_start; }
	const positioned_char &character(size_t index) const { return m_characters[index]; }
	positioned_char &character(size_t index) { return m_characters[index]; }

private:
	std::vector<positioned_char> m_characters;
	size_type m_center_justify_start = npos;
	size_type m_right_justify_start = npos;
	float m_yoffset;
	float m_height;
	float m_width = 0.0f;
	float m_anchor_pos = 0.0f;
	float m_anchor_target = 0.0f;
};



/***************************************************************************
CORE IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

text_layout::text_layout(render_font &font, float xscale, float yscale, float width, text_layout::text_justify justify, text_layout::word_wrapping wrap)
	: m_font(font)
	, m_xscale(xscale), m_yscale(yscale)
	, m_width(width)
	, m_justify(justify), m_wrap(wrap)
	, m_current_line(nullptr), m_last_break(0), m_text_position(0), m_truncating(false)
{
	invalidate_calculated_actual_width();
}


//-------------------------------------------------
//  ctor (move)
//-------------------------------------------------

text_layout::text_layout(text_layout &&that)
	: m_font(that.m_font)
	, m_xscale(that.m_xscale), m_yscale(that.m_yscale)
	, m_width(that.m_width), m_calculated_actual_width(that.m_calculated_actual_width)
	, m_justify(that.m_justify), m_wrap(that.m_wrap)
	, m_lines(std::move(that.m_lines)), m_current_line(that.m_current_line), m_last_break(that.m_last_break), m_text_position(that.m_text_position), m_truncating(false)
{
	that.invalidate_calculated_actual_width();
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

text_layout::~text_layout()
{
}


//-------------------------------------------------
//  add_text
//-------------------------------------------------

void text_layout::add_text(std::string_view text, text_justify line_justify, char_style const &style)
{
	while (!text.empty())
	{
		// adding a character - we might change the width
		invalidate_calculated_actual_width();

		// do we need to create a new line?
		if (!m_current_line)
			start_new_line(style.size);
		m_current_line->set_justification(line_justify);

		// get the current character
		char32_t ch;
		int const scharcount = uchar_from_utf8(&ch, text);
		if (scharcount < 0)
			break;
		text.remove_prefix(scharcount);

		// set up source information
		source_info source = { 0, };
		source.start = m_text_position;
		source.span = scharcount;
		m_text_position += scharcount;

		// is this an endline?
		if (ch == '\n')
		{
			// close up the current line
			m_current_line->align_text(*this);
			m_current_line = nullptr;
		}
		else if (!m_truncating)
		{
			// if we hit a space, remember the location and width *without* the space
			bool const is_space = is_space_character(ch);
			if (is_space)
				m_last_break = m_current_line->character_count();

			// append the character
			m_current_line->add_character(*this, ch, style, source);

			// do we have to wrap?
			if ((wrap() != word_wrapping::NEVER) && (m_current_line->width() > m_width))
			{
				switch (wrap())
				{
				case word_wrapping::TRUNCATE:
					truncate_wrap();
					break;

				case word_wrapping::WORD:
					word_wrap();
					break;

				case word_wrapping::NEVER:
					// can't happen due to if condition, but compile warns about it
					break;
				}
			}
			else
			{
				// we didn't wrap - if we hit any non-space breakable character,
				// remember the location and width *with* the breakable character
				if (!is_space && is_breakable_char(ch))
					m_last_break = m_current_line->character_count();
			}
		}
	}
}


//-------------------------------------------------
//  invalidate_calculated_actual_width
//-------------------------------------------------

void text_layout::invalidate_calculated_actual_width()
{
	m_calculated_actual_width = -1;
}


//-------------------------------------------------
//  actual_left
//-------------------------------------------------

float text_layout::actual_left()
{
	if (m_current_line)
	{
		// TODO: is there a sane way to allow an open line to be temporarily finalised and rolled back?
		m_current_line->align_text(*this);
		m_current_line = nullptr;
	}

	if (empty()) // degenerate scenario
		return 0.0f;

	float result = 1.0f;
	for (auto const &line : m_lines)
	{
		if (line->width())
			result = std::min(result, line->xoffset(*this));
	}
	return result;
}


//-------------------------------------------------
//  actual_width
//-------------------------------------------------

float text_layout::actual_width()
{
	if (m_current_line)
	{
		// TODO: is there a sane way to allow an open line to be temporarily finalised and rolled back?
		m_current_line->align_text(*this);
		m_current_line = nullptr;
	}

	// do we need to calculate the width?
	if (m_calculated_actual_width < 0)
	{
		// calculate the actual width
		m_calculated_actual_width = 0;
		for (const auto &line : m_lines)
			m_calculated_actual_width = std::max(m_calculated_actual_width, line->width());
	}

	// return it
	return m_calculated_actual_width;
}


//-------------------------------------------------
//  actual_height
//-------------------------------------------------

float text_layout::actual_height()
{
	if (!m_lines.empty())
		return m_lines.back()->yoffset() + m_lines.back()->height();
	else
		return 0.0f;
}


//-------------------------------------------------
//  start_new_line
//-------------------------------------------------

void text_layout::start_new_line(float height)
{
	// update the current line
	m_current_line = m_lines.emplace_back(std::make_unique<line>(actual_height(), height * yscale())).get();
	m_last_break = 0;
	m_truncating = false;
}


//-------------------------------------------------
//  get_char_width
//-------------------------------------------------

float text_layout::get_char_width(char32_t ch, float size)
{
	return font().char_width(size * yscale(), xscale() / yscale(), ch);
}


//-------------------------------------------------
//  truncate_wrap
//-------------------------------------------------

void text_layout::truncate_wrap()
{
	const char32_t elipsis = 0x2026;

	// for now, lets assume that we're only truncating the last character
	size_t truncate_position = m_current_line->character_count() - 1;
	const auto& truncate_char = m_current_line->character(truncate_position);

	// copy style information
	char_style style = truncate_char.style;

	// copy source information
	source_info source = { 0, };
	source.start = truncate_char.source.start + truncate_char.source.span;
	source.span = 0;

	// figure out how wide an ellipsis is
	float elipsis_width = get_char_width(elipsis, style.size);

	// where should we really truncate from?
	while (truncate_position > 0 && m_current_line->character(truncate_position).xoffset + elipsis_width > width())
		truncate_position--;

	// truncate!!!
	m_current_line->truncate(truncate_position);

	// and append the ellipsis
	m_current_line->add_character(*this, elipsis, style, source);

	// take note that we are truncating; suppress new characters
	m_truncating = true;
}


//-------------------------------------------------
//  word_wrap
//-------------------------------------------------

void text_layout::word_wrap()
{
	// keep track of the last line and break
	line *const last_line = m_current_line;
	size_t const last_break = m_last_break ? m_last_break : (last_line->character_count() - 1);

	// start a new line with the same justification
	start_new_line(last_line->character(last_line->character_count() - 1).style.size);

	// find the beginning of the word to wrap
	size_t position = last_break;
	while ((last_line->character_count() > position) && is_space_character(last_line->character(position).character))
		position++;

	// carry over justification
	if (last_line->right_justify_start() <= position)
		m_current_line->set_justification(text_justify::RIGHT);
	else if (last_line->center_justify_start() <= position)
		m_current_line->set_justification(text_justify::CENTER);

	// transcribe the characters
	for (size_t i = position; i < last_line->character_count(); i++)
	{
		if (last_line->right_justify_start() == i)
			m_current_line->set_justification(text_justify::RIGHT);
		else if (last_line->center_justify_start() == i)
			m_current_line->set_justification(text_justify::CENTER);
		auto &ch = last_line->character(i);
		m_current_line->add_character(*this, ch.character, ch.style, ch.source);
	}

	// and finally, truncate the previous line and adjust spacing
	last_line->truncate(last_break);
	last_line->align_text(*this);
}


//-------------------------------------------------
//  hit_test
//-------------------------------------------------

bool text_layout::hit_test(float x, float y, size_t &start, size_t &span)
{
	if (m_current_line)
	{
		// TODO: is there a sane way to allow an open line to be temporarily finalised and rolled back?
		m_current_line->align_text(*this);
		m_current_line = nullptr;
	}

	for (const auto &line : m_lines)
	{
		if (y >= line->yoffset() && y < line->yoffset() + line->height())
		{
			float line_xoffset = line->xoffset(*this);
			if (x >= line_xoffset && x < line_xoffset + line->width())
			{
				for (size_t i = 0; i < line->character_count(); i++)
				{
					const auto &ch = line->character(i);
					if (x >= ch.xoffset && x < ch.xoffset + ch.xwidth)
					{
						start = ch.source.start;
						span = ch.source.span;
						return true;
					}
				}
			}
		}
	}
	start = 0;
	span = 0;
	return false;
}


//-------------------------------------------------
//  restyle
//-------------------------------------------------

void text_layout::restyle(size_t start, size_t span, rgb_t const *fgcolor, rgb_t const *bgcolor)
{
	for (const auto &line : m_lines)
	{
		for (size_t i = 0; i < line->character_count(); i++)
		{
			auto &ch = line->character(i);
			if ((ch.source.start + ch.source.span) > (start + span))
			{
				return;
			}
			else if (ch.source.start >= start)
			{
				if (fgcolor)
					ch.style.fgcolor = *fgcolor;
				if (bgcolor)
					ch.style.bgcolor = *bgcolor;
			}
		}
	}
}


//-------------------------------------------------
//  emit
//-------------------------------------------------

void text_layout::emit(render_container &container, float x, float y)
{
	emit(container, 0, m_lines.size(), x, y);
}

void text_layout::emit(render_container &container, size_t start, size_t lines, float x, float y)
{
	if (m_current_line)
	{
		// TODO: is there a sane way to allow an open line to be temporarily finalised and rolled back?
		m_current_line->align_text(*this);
		m_current_line = nullptr;
	}

	float const base_y = (m_lines.size() > start) ? m_lines[start]->yoffset() : 0.0f;
	for (size_t l = start; ((start + lines) > l) && (m_lines.size() > l); ++l)
	{
		auto const &line = m_lines[l];
		float const line_xoffset = line->xoffset(*this);
		float const char_y = y + line->yoffset() - base_y;
		float const char_height = line->height();

		// emit every single character
		for (auto i = 0; i < line->character_count(); i++)
		{
			auto &ch = line->character(i);

			// position this specific character correctly (TODO - this doesn't handle differently sized text (yet)
			float const char_x = x + line_xoffset + ch.xoffset;
			float const char_width = ch.xwidth;

			// render the background of the character (if present)
			if (ch.style.bgcolor.a() != 0)
				container.add_rect(char_x, char_y, char_x + char_width, char_y + char_height, ch.style.bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// render the foreground
			container.add_char(
					char_x,
					char_y,
					char_height,
					xscale() / yscale(),
					ch.style.fgcolor,
					font(),
					ch.character);
		}
	}
}

} // namespace ui
