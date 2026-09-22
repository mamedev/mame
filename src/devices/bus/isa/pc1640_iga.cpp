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
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_PC1640_IGA, isa8_pc1640_iga_device, "pc1640_iga", "Amstrad PC1640 IGA")


//-------------------------------------------------
//  ROM( pc1640_iga )
//-------------------------------------------------

ROM_START( pc1640_iga )
	ROM_REGION( 0x8000, "iga", 0 )
	ROM_LOAD( "40100.ic913", 0x0000, 0x8000, CRC(d2d1f1ae) SHA1(98302006ee38a17c09bd75504cc18c0649174e33) ) // 8736 E
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_pc1640_iga_device::device_rom_region() const
{
	return ROM_NAME( pc1640_iga );
}


//-------------------------------------------------
//  INPUT_PORTS( pc1640_iga )
//-------------------------------------------------

static INPUT_PORTS_START( pc1640_iga )
	PORT_START("SW")
	PORT_DIPNAME( 0x0f, 0x09, "Initial Display Mode" ) PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(    0x0b, "Internal MD, External CGA80" )
	PORT_DIPSETTING(    0x0a, "Internal MD, External CGA40" )
	PORT_DIPSETTING(    0x09, "Internal ECD350, External MDA/HERC" )
	PORT_DIPSETTING(    0x08, "Internal ECD200, External MDA/HERC" )
	PORT_DIPSETTING(    0x07, "Internal CD80, External MDA/HERC" )
	PORT_DIPSETTING(    0x06, "Internal CD40, External MDA/HERC" )
	PORT_DIPSETTING(    0x05, "External CGA80, Internal MD" )
	PORT_DIPSETTING(    0x04, "External CGA40, Internal MD" )
	PORT_DIPSETTING(    0x03, "External MDA/HERC, Internal ECD350" )
	PORT_DIPSETTING(    0x02, "External MDA/HERC, Internal ECD200" )
	PORT_DIPSETTING(    0x01, "External MDA/HERC, Internal CD80" )
	PORT_DIPSETTING(    0x00, "External MDA/HERC, Internal CD40" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa8_pc1640_iga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc1640_iga );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_pc1640_iga_device - constructor
//-------------------------------------------------

isa8_pc1640_iga_device::isa8_pc1640_iga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pega1a_device(mconfig, ISA8_PC1640_IGA, tag, owner, clock),
	m_sw(*this, "SW"),
	m_sysw(*this, ":SW"),
	m_installed(false),
	m_enabled(true)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_pc1640_iga_device::device_start()
{
	pega1a_device::device_start();

	save_item(NAME(m_installed));
	save_item(NAME(m_enabled));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_pc1640_iga_device::device_reset()
{
	if (!m_installed)
	{
		m_enabled = !m_sysw.found() || BIT(m_sysw->read(), 9);
	}

	if (!m_enabled)
		return;

	pega1a_device::device_reset();

	if (m_installed)
		return;

	m_installed = true;

	const uint8_t mode = m_sw->read() & 0x0f;
	set_monitor_palette(mode == 0x04 || mode == 0x05 || mode == 0x0a || mode == 0x0b);

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "iga");
	m_isa->install_device(0x3b0, 0x3bf, read8sm_delegate(*this, FUNC(isa8_pc1640_iga_device::pega_3b0_r)), write8sm_delegate(*this, FUNC(isa8_pc1640_iga_device::pega_3b0_w)));
	m_isa->install_device(0x3c0, 0x3cf, read8sm_delegate(*this, FUNC(isa8_pc1640_iga_device::pc1640_3c0_r)), write8sm_delegate(*this, FUNC(pega1a_device::pega_3c0_w)));
	m_isa->install_device(0x3d0, 0x3df, read8sm_delegate(*this, FUNC(isa8_pc1640_iga_device::pega_3d0_r)), write8sm_delegate(*this, FUNC(isa8_pc1640_iga_device::pega_3d0_w)));
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void isa8_pc1640_iga_device::device_post_load()
{
	if (m_enabled)
		pega1a_device::device_post_load();
}


//-------------------------------------------------
//  pc1640_3c0_r - attribute/sequencer/graphics
//  controller and Input Status Register 0 read
//-------------------------------------------------

uint8_t isa8_pc1640_iga_device::pc1640_3c0_r(offs_t offset)
{
	if (offset == 2)
	{
		uint8_t swsts = 0;

		if (!BIT(m_misc_output, 4))
		{
			swsts = BIT(m_sw->read(), 3 - ((m_misc_output >> 2) & 0x03));
		}

		return 0x6f | (m_irq ? 0x80 : 0x00) | (swsts << 4);
	}

	return pc_ega8_3c0_r(offset);
}
