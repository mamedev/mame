// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert

#include "emu.h"
#include "luna_68k_video.h"

#include "luna_68k_gpu.h"
#include "luna_68k_bm.h"

luna_68k_video_connector::luna_68k_video_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LUNA_68K_VIDEO_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_luna_68k_video_interface>(mconfig, *this)
{
}

void luna_68k_video_connector::device_start()
{
}

void luna_68k_video_connector::install_vme_map(address_space_installer &space)
{
	auto card = get_card_device();
	if(card)
		card->install_vme_map(space);
}


void luna_68k_video_intf(device_slot_interface &device)
{
	device.option_add("gpu", LUNA_68K_GPU);
	device.option_add("bm", LUNA_68K_BM);
}

device_luna_68k_video_interface::device_luna_68k_video_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "luna_68k_video"),
	m_connector(nullptr)
{
}

device_luna_68k_video_interface::~device_luna_68k_video_interface()
{
}

void device_luna_68k_video_interface::interface_pre_start()
{
	m_connector = downcast<luna_68k_video_connector *>(device().owner());
}

void device_luna_68k_video_interface::install_vme_map(address_space_installer &space)
{
	space.install_device(0, 0xffffffff, *this, &device_luna_68k_video_interface::vme_map);
}

DEFINE_DEVICE_TYPE(LUNA_68K_VIDEO_CONNECTOR, luna_68k_video_connector, "luna_68k_video_connector", "Luna68k Video board connector")

