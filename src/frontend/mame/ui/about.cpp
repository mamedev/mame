// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/about.cpp

    About box

***************************************************************************/

#include "emu.h"
#include "ui/about.h"

#include "ui/ui.h"

#include "mame.h"


namespace ui {

namespace {

#include "copying.ipp"

} // anonymous namespace


/**************************************************
 ABOUT BOX
**************************************************/


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_about::menu_about(mame_ui_manager &mui, render_container &container)
	: menu_textbox(mui, container)
	, m_header{
			util::string_format(
#ifdef MAME_DEBUG
					_("about-header", "%1$s %2$s (%3$s%4$sP%5$s, debug)"),
#else
					_("about-header", "%1$s %2$s (%3$s%4$sP%5$s)"),
#endif
					emulator_info::get_appname(),
					bare_build_version,
					(sizeof(int) == sizeof(void *)) ? "I" : "",
					(sizeof(long) == sizeof(void *)) ? "L" : (sizeof(long long) == sizeof(void *)) ? "LL" : "",
					sizeof(void *) * 8),
			util::string_format(_("about-header", "Revision: %1$s"), bare_vcs_revision) }
{
	set_process_flags(PROCESS_CUSTOM_NAV);
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_about::~menu_about()
{
}


//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_about::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu_textbox::recompute_metrics(width, height, aspect);

	// make space for the title and revision
	set_custom_space((line_height() * m_header.size()) + (tb_border() * 3.0F), 0.0F);
}


//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_about::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// draw the title
	draw_text_box(
			std::begin(m_header), std::end(m_header),
			origx1, origx2, origy1 - top, origy1 - tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR);
}


//-------------------------------------------------
//  populate_text - populate the about box text
//-------------------------------------------------

void menu_about::populate_text(std::optional<text_layout> &layout, float &width, int &lines)
{
	if (!layout || (layout->width() != width))
	{
		rgb_t const color = ui().colors().text_color();
		layout.emplace(create_layout(width));
		for (char const *const *line = copying_text; *line; ++line)
		{
			layout->add_text(*line, color);
			layout->add_text("\n", color);
		}
		lines = layout->lines();
	}
	width = layout->actual_width();
}


//-------------------------------------------------
//  populate - populates the about modal
//-------------------------------------------------

void menu_about::populate()
{
}

} // namespace ui
