// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    SDI - Commodore IEC Bus / VC1541 interface
    (c) 198? Milan Uroševič, Ivan Gerenčir

    Simple DIY device from Yugoslavian / Slovenian "Moj mikro" magazine.
    Article scan http://www.zxspectrum.it.omegahg.com/rom/SDI/SDI.pdf
    Components: i8255, 4K ROM.

    Enter "RANDOMIZE USR 16000" to init / activate interface.

    Notes / TODOs:
    - floppy drive access doesn't work even with "quantum perfect" host machine config, cause is unknown.

*********************************************************************/

#include "emu.h"
#include "sdi.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_SDI, spectrum_sdi_device, "spectrum_sdi", "SDI Interface")


//-------------------------------------------------
//  ROM( sdi )
//-------------------------------------------------

ROM_START(sdi)
	ROM_REGION(0x1000, "rom", 0)
	ROM_DEFAULT_BIOS("sdiv1")

	ROM_SYSTEM_BIOS(0, "sdiv1", "DOS v1.0")
	ROMX_LOAD("sdi10.bin", 0x0000, 0x1000, CRC(d0678a71) SHA1(52405bb42e46a12b57806fd7f26691681afe2d8e), ROM_BIOS(0))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_sdi_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_ppi);
	m_ppi->in_pc_callback().set(FUNC(spectrum_sdi_device::ppic_r));
	m_ppi->out_pc_callback().set(FUNC(spectrum_sdi_device::ppic_w));

	cbm_iec_slot_device::add(config, m_iec, "c1541");
}

const tiny_rom_entry *spectrum_sdi_device::device_rom_region() const
{
	return ROM_NAME(sdi);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_sdi_device - constructor
//-------------------------------------------------

spectrum_sdi_device::spectrum_sdi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_SDI, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_ppi(*this, "ppi")
	, m_iec(*this, "iec_bus")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_sdi_device::device_start()
{
	save_item(NAME(m_romcs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_sdi_device::device_reset()
{
	m_romcs = 0;
	m_iec->reset();
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_sdi_device::romcs)
{
	return m_romcs;
}

void spectrum_sdi_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if ((offset & 0xfc00) == 0x3c00)
			m_romcs = 1;

		if (offset < 0x3000)
			m_romcs = 0;
	}
}

uint8_t spectrum_sdi_device::mreq_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0xfff];

	return data;
}

uint8_t spectrum_sdi_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	if ((offset & 0x9f) == 0x9f)
		data = m_ppi->read((offset >> 5) & 3);

	return data;
}

void spectrum_sdi_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0x9f) == 0x9f)
		m_ppi->write((offset >> 5) & 3, data);
}

uint8_t spectrum_sdi_device::ppic_r()
{
	u8 data = 0;

	data |= m_iec->clk_r() << 6;
	data |= m_iec->data_r() << 7;

	return data;
}

void spectrum_sdi_device::ppic_w(uint8_t data)
{
	m_iec->host_atn_w(!BIT(data, 0));
	m_iec->host_clk_w(!BIT(data, 2));
	m_iec->host_data_w(!BIT(data, 3));
}
