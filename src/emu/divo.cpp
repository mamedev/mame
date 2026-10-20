// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    divo.h

    Video output, aka screens

***************************************************************************/

#include "emu.h"
#include "validity.h"

#include "rendutil.h"
#include "video.h"

u32 device_video_output_interface::m_id_counter = 0;

device_video_output_interface::device_video_output_interface(const machine_config &mconfig,
															 device_t &device) :
	device_interface(device, "video_output"),
	m_container(nullptr),
	m_orientation(ROT0)
{
	m_unique_id = m_id_counter;
	m_id_counter++;
}

device_video_output_interface::~device_video_output_interface()
{
}

void device_video_output_interface::interface_config_complete()
{
	m_orientation = orientation_add(m_orientation, device().mconfig().gamedrv().flags & machine_flags::MASK_ORIENTATION);
}

void device_video_output_interface::interface_pre_start()
{
	m_is_primary_screen = (this == video_output_interface_enumerator(device().machine().root_device()).first());
}

void device_video_output_interface::update_if_primary()
{
	if (m_is_primary_screen)
		device().machine().video().frame_update();
}

int device_video_output_interface::hpos() const
{
	return 0;
}

int device_video_output_interface::vpos() const
{
	return 0;
}

bool device_video_output_interface::is_vector() const
{
	return false;
}
