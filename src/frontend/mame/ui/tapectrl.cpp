// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/tapectrl.cpp

    Tape control

***************************************************************************/

#include "emu.h"
#include "ui/tapectrl.h"

#include <string_view>


namespace ui {

namespace {

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TAPECMD_NULL            ((void *)0x0000)
#define TAPECMD_STOP            ((void *)0x0001)
#define TAPECMD_PLAY            ((void *)0x0002)
#define TAPECMD_RECORD          ((void *)0x0003)
#define TAPECMD_REWIND          ((void *)0x0004)
#define TAPECMD_FAST_FORWARD    ((void *)0x0005)
#define TAPECMD_SLIDER          ((void *)0x0006)
#define TAPECMD_SELECT          ((void *)0x0007)


inline std::string_view tape_state_string(cassette_image_device &device)
{
	cassette_state const state(device.get_state());
	if ((state & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED)
		return _("stopped");
	else if ((state & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
		return ((state & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED) ? _("playing") : _("(playing)");
	else
		return ((state & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED) ? _("recording") : _("(recording)");
}


inline uint32_t tape_position_flags(double position, double length)
{
	return ((position > 0.0) ? menu::FLAG_LEFT_ARROW : 0U) | ((position < length) ? menu::FLAG_RIGHT_ARROW : 0U);
}

} // anonymous namespace


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_tape_control::menu_tape_control(mame_ui_manager &mui, render_container &container, cassette_image_device *device)
	: menu_device_control<cassette_image_device>(mui, container, device)
	, m_slider_item_index(-1)
{
	set_process_flags(PROCESS_LR_REPEAT);
	set_heading(_("Tape Control"));

	if (device)
	{
		m_notifier = device->add_media_change_notifier(
				[this] (device_image_interface::media_change_event ev)
				{
					// repopulate the menu if an image is mounted or unmounted
					reset(reset_options::REMEMBER_POSITION);
				});
	}
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_tape_control::~menu_tape_control()
{
}


//-------------------------------------------------
//  populate - populates the main tape control menu
//-------------------------------------------------

void menu_tape_control::populate()
{
	m_slider_item_index = -1;
	if (current_device())
	{
		// name of tape
		item_append(current_display_name(), current_device()->exists() ? current_device()->filename() : "No Tape Image loaded", current_display_flags(), TAPECMD_SELECT);

		if (current_device()->exists())
		{
			double const t0 = current_device()->get_position();
			double const t1 = current_device()->get_length();
			m_slider_item_index = item_append(
					std::string(tape_state_string(*current_device())),
					util::string_format("%04d/%04d", t1 ? int(t0) : 0, int(t1)),
					tape_position_flags(t0, t1),
					TAPECMD_SLIDER);

			// pause or stop
			item_append(_("Pause/Stop"), 0, TAPECMD_STOP);

			// play
			item_append(_("Play"), 0, TAPECMD_PLAY);

			// record
			item_append(_("Record"), 0, TAPECMD_RECORD);

			// rewind
			item_append(_("Rewind"), 0, TAPECMD_REWIND);

			// fast forward
			item_append(_("Fast Forward"), 0, TAPECMD_FAST_FORWARD);
		}

		item_append(menu_item_type::SEPARATOR);
	}
}


//-------------------------------------------------
//  handle - main tape control menu
//-------------------------------------------------

bool menu_tape_control::handle(event const *ev)
{
	// process the menu
	if (ev)
	{
		switch (ev->iptkey)
		{
		case IPT_UI_LEFT:
			if (ev->itemref == TAPECMD_SLIDER)
			{
				current_device()->seek(-1, SEEK_CUR);
			}
			else if (ev->itemref == TAPECMD_SELECT)
			{
				m_slider_item_index = -1;
				previous();
			}
			break;

		case IPT_UI_RIGHT:
			if (ev->itemref == TAPECMD_SLIDER)
			{
				current_device()->seek(+1, SEEK_CUR);
			}
			else if (ev->itemref == TAPECMD_SELECT)
			{
				m_slider_item_index = -1;
				next();
			}
			break;

		case IPT_UI_SELECT:
			if (ev->itemref == TAPECMD_STOP)
				current_device()->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
			else if (ev->itemref == TAPECMD_PLAY)
				current_device()->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
			else if (ev->itemref == TAPECMD_RECORD)
				current_device()->change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
			else if (ev->itemref == TAPECMD_REWIND)
				current_device()->seek(-30, SEEK_CUR);
			else if (ev->itemref == TAPECMD_FAST_FORWARD)
				current_device()->seek(30, SEEK_CUR);
			else if (ev->itemref == TAPECMD_SLIDER)
				current_device()->seek(0, SEEK_SET);
			break;
		}
	}

	// uupdate counters
	if ((0 <= m_slider_item_index) && current_device() && current_device()->exists())
	{
		menu_item &slider_item(item(m_slider_item_index));
		double const t0(current_device()->get_position());
		double const t1(current_device()->get_length());
		slider_item.set_text(tape_state_string(*current_device()));
		slider_item.set_subtext(util::string_format("%04d/%04d", t1 ? int(t0) : 0, int(t1)));
		slider_item.set_flags(tape_position_flags(t0, t1));
	}

	return false;
}

} // namespace ui
