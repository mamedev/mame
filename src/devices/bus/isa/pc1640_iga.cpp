// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Amstrad PC1640 Integrated Graphics Adapter emulation

**********************************************************************

    This display controller is integrated on the PC1640 motherboard
    but wired to the ISA bus, and can be disabled with a DIP switch.

    WD Paradise PEGA 1A 38304B 2116-002 8745AAA JAPAN (84 pin PLCC)

    Single chip multimode EGA video controller with
    integral 6845 CRTC. Provides 100% IBM EGA, CGA,
    MDA, Hercules graphics and Plantronics COLORPLUS*
    compatibility

**********************************************************************/

#include "emu.h"
#include "pc1640_iga.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PEGA1A_TAG      "ic910"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_PC1640_IGA, isa8_pc1640_iga_device, "pc1640_iga", "Amstrad PC1640 IGA")


//-------------------------------------------------
//  ROM( pc1640_iga )
//-------------------------------------------------

ROM_START( pc1640_iga )
	ROM_REGION16_LE( 0x8000, "iga", 0)
	ROM_LOAD( "40100.ic913", 0x0000, 0x8000, CRC(d2d1f1ae) SHA1(98302006ee38a17c09bd75504cc18c0649174e33) ) // 8736 E
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_pc1640_iga_device::device_rom_region() const
{
	return ROM_NAME( pc1640_iga );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_pc1640_iga_device - constructor
//-------------------------------------------------

isa8_pc1640_iga_device::isa8_pc1640_iga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa8_ega_device(mconfig, ISA8_PC1640_IGA, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_pc1640_iga_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();

	for (int i = 0; i < 64; i++ )
	{
		uint8_t r = ( ( i & 0x04 ) ? 0xAA : 0x00 ) + ( ( i & 0x20 ) ? 0x55 : 0x00 );
		uint8_t g = ( ( i & 0x02 ) ? 0xAA : 0x00 ) + ( ( i & 0x10 ) ? 0x55 : 0x00 );
		uint8_t b = ( ( i & 0x01 ) ? 0xAA : 0x00 ) + ( ( i & 0x08 ) ? 0x55 : 0x00 );

		m_palette->set_pen_color( i, r, g, b );
	}

	/* Install 256KB Video ram on our EGA card */
	m_vram = make_unique_clear<uint8_t[]>(256 * 1024);

	m_videoram = m_vram.get();
	m_plane[0] = m_videoram + 0x00000;
	m_plane[1] = m_videoram + 0x10000;
	m_plane[2] = m_videoram + 0x20000;
	m_plane[3] = m_videoram + 0x30000;

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "ega", "iga");
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3b0_r)), write8_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3b0_w)));
	m_isa->install_device(0x3c0, 0x3cf, read8_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3c0_r)), write8_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3c0_w)));
	m_isa->install_device(0x3d0, 0x3df, read8_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3d0_r)), write8_delegate(*this, FUNC(isa8_ega_device::pc_ega8_3d0_w)));
}
