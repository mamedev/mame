// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Logotron Sprite Board

**********************************************************************/


#include "emu.h"
#include "sprite.h"
#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_SPRITE, bbc_sprite_device, "bbc_sprite", "Logotron Sprite Board");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_sprite_device::device_add_mconfig(machine_config &config)
{
	TMS9129(config, m_vdp, 10.738635_MHz_XTAL);
	m_vdp->int_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_vdp->set_screen("screen");
	m_vdp->set_vram_size(0x4000);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_sprite_device - constructor
//-------------------------------------------------

bbc_sprite_device::bbc_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_SPRITE, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_vdp(*this, "vdp")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_sprite_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_sprite_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_sprite_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xa0:
		data = m_vdp->vram_read();
		break;
	case 0xa2:
		data = m_vdp->register_read();
		break;
	}
	return data;
}

void bbc_sprite_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xa1:
		m_vdp->vram_write(data);
		break;
	case 0xa3:
		m_vdp->register_write(data);
		break;
	}
}
