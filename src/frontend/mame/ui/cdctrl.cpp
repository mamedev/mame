// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ui/cdctrl.cpp

    Audio CD player control

***************************************************************************/

#include "emu.h"
#include "ui/cdctrl.h"

#include <string_view>


namespace ui {

namespace {

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum : unsigned
{
	CDCMD_SELECT = 1,
	CDCMD_STATUS,
	CDCMD_PLAY,
	CDCMD_PAUSE,
	CDCMD_STOP,
	CDCMD_PREVIOUS,
	CDCMD_NEXT
};


inline std::string_view cd_state_string(device_cd_player_interface &device)
{
	switch (device.state())
	{
	case device_cd_player_interface::transport::PLAYING:
		return _("playing");
	case device_cd_player_interface::transport::PAUSED:
		return _("paused");
	default:
		return _("stopped");
	}
}


inline std::string cd_position_string(device_cd_player_interface &device)
{
	u32 const elapsed = device.track_elapsed_frames() / device_cd_player_interface::FRAMES_PER_SECOND;
	u32 const length = device.track_length_frames() / device_cd_player_interface::FRAMES_PER_SECOND;
	return util::string_format(_("Track %1$02d/%2$02d  %3$02d:%4$02d / %5$02d:%6$02d"),
			device.track(), device.track_count(),
			elapsed / 60, elapsed % 60,
			length / 60, length % 60);
}

} // anonymous namespace


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_cd_control::menu_cd_control(mame_ui_manager &mui, render_target &target, device_cd_player_interface *device)
	: menu_device_control<device_cd_player_interface, cd_player_interface_enumerator>(mui, target, device)
	, m_status_item_index(-1)
{
	set_heading(_("CD Player Control"));
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_cd_control::~menu_cd_control()
{
}


//-------------------------------------------------
//  populate - populates the CD player control menu
//-------------------------------------------------

void menu_cd_control::populate()
{
	m_notifier.reset();
	m_status_item_index = -1;
	if (current_device())
	{
		// repopulate the menu if an image is mounted or unmounted
		m_notifier = current_device()->cd_image().add_media_change_notifier(
				[this] (device_image_interface::media_change_event ev)
				{
					reset(reset_options::REMEMBER_POSITION);
				});

		// name of disc
		item_append(current_display_name(), current_device()->cd_image().exists() ? current_device()->cd_image().filename() : _("No Disc Image loaded"), current_display_flags(), (void *)CDCMD_SELECT);

		if (current_device()->cd_image().exists())
		{
			m_status_item_index = item_append(std::string(cd_state_string(*current_device())), cd_position_string(*current_device()), 0, (void *)CDCMD_STATUS);

			item_append(_("Play"), 0, (void *)CDCMD_PLAY);
			item_append(_("Pause"), 0, (void *)CDCMD_PAUSE);
			item_append(_("Stop"), 0, (void *)CDCMD_STOP);
			item_append(_("Previous Track"), 0, (void *)CDCMD_PREVIOUS);
			item_append(_("Next Track"), 0, (void *)CDCMD_NEXT);
		}

		item_append(menu_item_type::SEPARATOR);
	}
}


//-------------------------------------------------
//  handle - CD player control menu
//-------------------------------------------------

bool menu_cd_control::handle(event const *ev)
{
	if (ev)
	{
		switch (ev->iptkey)
		{
		case IPT_UI_LEFT:
			switch (uintptr_t(ev->itemref))
			{
			case CDCMD_SELECT:
				m_status_item_index = -1;
				previous();
				break;
			case CDCMD_STATUS:
				current_device()->previous_track();
				break;
			}
			break;

		case IPT_UI_RIGHT:
			switch (uintptr_t(ev->itemref))
			{
			case CDCMD_SELECT:
				m_status_item_index = -1;
				next();
				break;
			case CDCMD_STATUS:
				current_device()->next_track();
				break;
			}
			break;

		case IPT_UI_SELECT:
			switch (uintptr_t(ev->itemref))
			{
			case CDCMD_PLAY:
				current_device()->play();
				break;
			case CDCMD_PAUSE:
				current_device()->pause();
				break;
			case CDCMD_STOP:
				current_device()->stop();
				break;
			case CDCMD_PREVIOUS:
				current_device()->previous_track();
				break;
			case CDCMD_NEXT:
				current_device()->next_track();
				break;
			}
			break;
		}
	}

	// update status
	if ((0 <= m_status_item_index) && current_device() && current_device()->cd_image().exists())
	{
		menu_item &status_item(item(m_status_item_index));
		status_item.set_text(cd_state_string(*current_device()));
		status_item.set_subtext(cd_position_string(*current_device()));
		status_item.set_flags(
				((current_device()->track() > 1) ? FLAG_LEFT_ARROW : 0U) |
				((current_device()->track() < current_device()->track_count()) ? FLAG_RIGHT_ARROW : 0U));
	}

	return false;
}

} // namespace ui
