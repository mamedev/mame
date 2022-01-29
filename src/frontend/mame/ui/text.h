// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Vas Crabb
/***************************************************************************

    text.h

    Text functionality for MAME's crude user interface

***************************************************************************/
#ifndef MAME_FRONTEND_UI_TEXT_H
#define MAME_FRONTEND_UI_TEXT_H

#pragma once

#include <memory>
#include <string_view>
#include <vector>


class render_font;
class render_container;


namespace ui {

/***************************************************************************
TYPE DEFINITIONS
***************************************************************************/

class text_layout
{
public:
	// justification options for text
	enum class text_justify
	{
		LEFT = 0,
		CENTER,
		RIGHT
	};

	// word wrapping options
	enum class word_wrapping
	{
		NEVER,
		TRUNCATE,
		WORD
	};

	// ctor/dtor
	text_layout(render_font &font, float xscale, float yscale, float width, text_justify justify, word_wrapping wrap);
	text_layout(text_layout &&that);
	~text_layout();

	// accessors
	render_font &font() const { return m_font; }
	float xscale() const { return m_xscale;  }
	float yscale() const { return m_yscale; }
	float width() const { return m_width; }
	text_justify justify() const { return m_justify; }
	word_wrapping wrap() const { return m_wrap; }

	// methods
	float actual_left();
	float actual_width();
	float actual_height();
	bool empty() const { return m_lines.empty(); }
	size_t lines() const { return m_lines.size(); }
	bool hit_test(float x, float y, size_t &start, size_t &span);
	void restyle(size_t start, size_t span, rgb_t *fgcolor, rgb_t *bgcolor);
	void emit(render_container &container, float x, float y);
	void emit(render_container &container, size_t start, size_t lines, float x, float y);
	void add_text(std::string_view text, rgb_t fgcolor = rgb_t::white(), rgb_t bgcolor = rgb_t::transparent(), float size = 1.0)
	{
		add_text(text, justify(), char_style{ fgcolor, bgcolor, size });
	}
	void add_text(std::string_view text, text_justify line_justify, rgb_t fgcolor = rgb_t::white(), rgb_t bgcolor = rgb_t::transparent(), float size = 1.0)
	{
		add_text(text, line_justify, char_style{ fgcolor, bgcolor, size });
	}

private:
	// text style information - in a struct to facilitate copying
	struct char_style
	{
		rgb_t fgcolor;
		rgb_t bgcolor;
		float size;
	};

	// class to represent a line
	struct source_info;
	struct positioned_char;
	class line;

	// instance variables
	render_font &m_font;
	float m_xscale;
	float m_yscale;
	float m_width;
	mutable float m_calculated_actual_width;
	text_justify m_justify;
	word_wrapping m_wrap;
	std::vector<std::unique_ptr<line> > m_lines;
	line *m_current_line;
	size_t m_last_break;
	size_t m_text_position;
	bool m_truncating;

	// methods
	void add_text(std::string_view text, text_justify line_justify, char_style const &style);
	void start_new_line(float height);
	float get_char_width(char32_t ch, float size);
	void truncate_wrap();
	void word_wrap();
	void invalidate_calculated_actual_width();
};

} // namespace ui

#endif // MAME_FRONTEND_UI_TEXT_H
