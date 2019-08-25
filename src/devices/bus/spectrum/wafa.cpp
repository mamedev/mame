// license:BSD-3-Clause
// copyright-holders:David Haywood
/**********************************************************************

    Rotronics Wafadrive

    Provides 2 built-in 'infinite loop' tape based drives that act like
    discs (similar to Microdrive), a Centronics port, a RS232 port, and
    the standard pass-thru connector.

    Tapes could hold 128k, 64k or 16k (approximate sizes) with longer
    seek times on the higher capacity media.  Loading was significantly
    slower than the Microdrive (but faster than cassette)

    Media was highly unreliable and prone to snapping.

    use
    NEW *
    to initialize

**********************************************************************/

#include "emu.h"
#include "wafa.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_WAFA, spectrum_wafa_device, "spectrum_wafa", "Rotronics Wafadrive")


//-------------------------------------------------
//  MACHINE_DRIVER( wafa )
//-------------------------------------------------

ROM_START( wafadrive )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD("wafa108.rom", 0x0000, 0x2000, CRC(179d4600) SHA1(e979f9b1ada4a723ec69734c1848adbbf8d920c5) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_wafa_device::device_add_mconfig(machine_config &config)
{
	WAFADRIVE_IMAGE(config, m_wafa1);
	WAFADRIVE_IMAGE(config, m_wafa2);

	/* passthru */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));

	SOFTWARE_LIST(config, "wafadrive_list").set_original("spectrum_wafadrive");
}

const tiny_rom_entry *spectrum_wafa_device::device_rom_region() const
{
	return ROM_NAME( wafadrive );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_wafa_device - constructor
//-------------------------------------------------

spectrum_wafa_device::spectrum_wafa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_WAFA, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_exp(*this, "exp")
	, m_rom(*this, "rom")
	, m_wafa1(*this, "wafa1")
	, m_wafa2(*this, "wafa2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_wafa_device::device_start()
{
	save_item(NAME(m_romcs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_wafa_device::device_reset()
{
	m_romcs = 0;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_wafa_device::romcs)
{
	return m_romcs | m_exp->romcs();
}

void spectrum_wafa_device::opcode_fetch(offs_t offset)
{
	m_exp->opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0008: case 0x1708:
		{
			//logerror("wafa enabled\n");
			m_romcs = 1;
			break;
		}
		}
	}
}

void spectrum_wafa_device::opcode_fetch_post(offs_t offset)
{
	m_exp->opcode_fetch_post(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		// guess, manual wording indicates it gets disabled via an IN command in RAM tho, but no such thing is executed unless we disable it here first?
		// it seems the port reads are instead are an alt way to reenable it
		case 0x00a0:
		{
			//logerror("^wafa disable\n");
			m_romcs = 0;
			break;
		}
		}
	}
}


uint8_t spectrum_wafa_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
		data &= m_rom->base()[offset & 0x1fff];

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_wafa_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

uint8_t spectrum_wafa_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (((offset & 0x00ff) != 0x00fe) && ((offset & 0x00ff) != 0x000c))
		logerror("%s: iorq_r %04x\n", machine().describe_context(), offset);

	if ((offset & 0x00ff) == 0x000c)
		m_romcs = 1;

	data &= m_exp->iorq_r(offset);
	return data;
}

void spectrum_wafa_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0x00fe) != 0x00fe)
		logerror("%s: iorq_w %04x %02x\n", machine().describe_context(), offset, data);

	m_exp->iorq_w(offset, data);
}
