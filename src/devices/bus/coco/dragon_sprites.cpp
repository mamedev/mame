// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Premier Microsystems Sprites

***************************************************************************/

#include "emu.h"
#include "dragon_sprites.h"
#include "screen.h"


ROM_START(dragon_sprites)
	ROM_REGION(0x2000, "eprom", 0)
	ROM_LOAD("sprites.rom", 0x0000, 0x2000, CRC(b8f54eaf) SHA1(f1288653a12ba9a17ef721d0f4867160c3ca341f))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DRAGON_SPRITES, dragon_sprites_device, "dragon_sprites", "Dragon Sprites")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dragon_sprites_device - constructor
//-------------------------------------------------

dragon_sprites_device::dragon_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DRAGON_SPRITES, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_eprom(*this, "eprom")
	, m_vdp(*this, "tms9929a")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_sprites_device::device_start()
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dragon_sprites_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	TMS9929A(config, m_vdp, 10.738635_MHz_XTAL); // TODO: verify crystal, manual says 10.7MHz
	m_vdp->set_screen("screen");
	m_vdp->set_vram_size(0x4000);
	m_vdp->int_callback().set(FUNC(dragon_sprites_device::nmi_w));
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dragon_sprites_device::device_rom_region() const
{
	return ROM_NAME( dragon_sprites );
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(dragon_sprites_device::cts_read)
{
	uint8_t data = 0x00;
	switch (offset)
	{
	case 0x2000:
		data = m_vdp->vram_read();
		break;
	case 0x2001:
		data = m_vdp->register_read();
		break;
	default:
		data = m_eprom->base()[offset & 0x1fff];
		break;
	}
	return data;
}


//-------------------------------------------------
//  cts_write
//-------------------------------------------------

WRITE8_MEMBER(dragon_sprites_device::cts_write)
{
	switch (offset)
	{
	case 0x2000:
		m_vdp->vram_write(data);
		break;
	case 0x2001:
		m_vdp->register_write(data);
		break;
	};
}


WRITE_LINE_MEMBER(dragon_sprites_device::nmi_w)
{
	// set the NMI line
	set_line_value(line::NMI, state);
}
