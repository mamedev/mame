// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    CoCo WordPak 2+

***************************************************************************/

#include "emu.h"
#include "coco_wpk2p.h"
#include "screen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_WPK2P, coco_wpk2p_device, "coco_wpk2p", "CoCo WordPak 2+")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_wpk2p_device - constructor
//-------------------------------------------------

coco_wpk2p_device::coco_wpk2p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COCO_WPK2P, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_v9958(*this, "v9958")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_wpk2p_device::device_start()
{
	install_readwrite_handler(0xff78, 0xff7b, read8sm_delegate(*m_v9958, FUNC(v9958_device::read)), write8sm_delegate(*m_v9958, FUNC(v9958_device::write)));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void coco_wpk2p_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	V9958(config, m_v9958, 21.477272_MHz_XTAL);
	m_v9958->set_screen_ntsc("screen");
	m_v9958->set_vram_size(0x20000);
	m_v9958->int_cb().set([this](int state) { set_line_value(line::NMI, state); });
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}
