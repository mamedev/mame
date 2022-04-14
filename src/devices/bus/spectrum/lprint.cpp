// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Common printer interfaces

    ZX Lprint
    (c) 1983 Euroelectronics (UK)

    ZX Lprint III
    (c) 1984 Euroelectronics (UK)
    Centronics and RS232 interface, most common printer interface for ZX-Spectrum.

    Hilderbay Interface (c) 1983 Hilderbay Ltd (UK)
    AKA
    Kempston Centronics S Interface (c) 1983 Kempston Micro Electronics Ltd (UK)
    ROM-less device, require printer driver to be loaded from tape.

    Kempston Centronics E Interface
    (c) 1984 Kempston Micro Electronics Ltd (UK)

    Notes/TODOs:
     - add information and docs

*********************************************************************/

#include "emu.h"
#include "lprint.h"
#include <algorithm>


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_LPRINT, spectrum_lprint_device, "spectrum_lprint", "ZX Lprint")
DEFINE_DEVICE_TYPE(SPECTRUM_LPRINT3, spectrum_lprint3_device, "spectrum_lprint3", "ZX Lprint III")
DEFINE_DEVICE_TYPE(SPECTRUM_KEMPCENTRS, spectrum_kempcentrs_device, "spectrum_kempcentrs", "Hilderbay / Kempston Centronics S")
DEFINE_DEVICE_TYPE(SPECTRUM_KEMPCENTREF, spectrum_kempcentre_device, "spectrum_kempcentref", "Kempston Centronics E (flat)")
DEFINE_DEVICE_TYPE(SPECTRUM_KEMPCENTREU, spectrum_kempcentreu_device, "spectrum_kempcentreu", "Kempston Centronics E (upright)")

//-------------------------------------------------
//  ROMs
//-------------------------------------------------

ROM_START(lprint)
	ROM_REGION(0x800, "rom", 0)
	ROM_LOAD("lprint.rom", 0x0000, 0x0800, CRC(3c3483e5) SHA1(3e4c7ecd8c3eb011cdf75eb23faf8d8ea8329757)) // permanently overlay 0x0800-0x0fff ROM area
ROM_END

ROM_START(lprint3)
	ROM_REGION(0x800, "rom", 0)
	ROM_DEFAULT_BIOS("v2")

	// original
	ROM_SYSTEM_BIOS(0, "v1", "v1.0")
	ROMX_LOAD("v10.rom", 0x0000, 0x0800, CRC(a5014f40) SHA1(fe823a8b87a8bdacb14aac848bc7fe946f76faae), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "v2.0")
	ROMX_LOAD("v20.rom", 0x0000, 0x0800, CRC(b8f9a58d) SHA1(4966664badbed44ddb1c1a2ca01bdef60f0a30f8), ROM_BIOS(1))

	// clones
	ROM_SYSTEM_BIOS(2, "kp", "Sandy") // from Sandy Disco V3 (Kempston Disc clone) lower PCB, also have joystick port and passthru connector
	ROMX_LOAD("sandy.rom", 0x0000, 0x0800, CRC(77e752f2) SHA1(11f44c5e743a413e8bc0d07c4249f36d2c0a172a), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "rus", "Art East Computers (RU)") // added support for popular Soviet and Eastern Bloc made printers
	ROMX_LOAD("himak.rom", 0x0000, 0x0800, CRC(b1f15976) SHA1(d9c0297c05e8092fb7e0198f4172d3d4e6ed252c), ROM_BIOS(3))
ROM_END

ROM_START(kempcentref)
	ROM_REGION(0x800, "rom", 0)
	ROM_LOAD("kemp-e.rom", 0x0000, 0x0800, CRC(a58b1e97) SHA1(b26e9f720a215019b8710a4a094d6fde6ecae5fd))
ROM_END

ROM_START(kempcentreu)
	ROM_REGION(0x800, "rom", 0)
	ROM_LOAD("7f.rom", 0x0000, 0x0800, CRC(b67a416d) SHA1(6d0772e81cfd9bc994b1c91d4e01b45e0fc0d3d3)) // scrambled
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_lprint_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_lprint_device::busy_w));
}

void spectrum_lprint3_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_lprint3_device::busy_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	// passthru
	// this is not really accurate, original ZX-Lprint boards had no passthru connector, but only some of clones
	// TODO: populate expansion port only for devices which really had it
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

void spectrum_kempcentrs_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_kempcentrs_device::busy_w));
}

void spectrum_kempcentre_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_kempcentre_device::busy_w));
}

const tiny_rom_entry *spectrum_lprint_device::device_rom_region() const
{
	return ROM_NAME(lprint);
}

const tiny_rom_entry *spectrum_lprint3_device::device_rom_region() const
{
	return ROM_NAME(lprint3);
}

const tiny_rom_entry *spectrum_kempcentre_device::device_rom_region() const
{
	return ROM_NAME(kempcentref);
}

const tiny_rom_entry *spectrum_kempcentreu_device::device_rom_region() const
{
	return ROM_NAME(kempcentreu);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_lprint_device - constructors
//-------------------------------------------------

spectrum_lprint_device::spectrum_lprint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_LPRINT, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_centronics(*this, "centronics")
{
}

spectrum_lprint3_device::spectrum_lprint3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_LPRINT3, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_centronics(*this, "centronics")
	, m_rs232(*this, "rs232")
	, m_exp(*this, "exp")
{
}

spectrum_kempcentrs_device::spectrum_kempcentrs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_KEMPCENTRS, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_centronics(*this, "centronics")
{
}

spectrum_kempcentre_device::spectrum_kempcentre_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_centronics(*this, "centronics")
{
}

spectrum_kempcentre_device::spectrum_kempcentre_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_kempcentre_device(mconfig, SPECTRUM_KEMPCENTREF, tag, owner, clock)
{
}

spectrum_kempcentreu_device::spectrum_kempcentreu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_kempcentre_device(mconfig, SPECTRUM_KEMPCENTREU, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_lprint_device::device_start()
{
	m_busy = 0;
	save_item(NAME(m_romcs));
	save_item(NAME(m_busy));
}

void spectrum_lprint3_device::device_start()
{
	m_busy = 0;
	save_item(NAME(m_romcs));
	save_item(NAME(m_busy));
}

void spectrum_kempcentrs_device::device_start()
{
	m_busy = 0;
	save_item(NAME(m_busy));
}

void spectrum_kempcentre_device::device_start()
{
	m_busy = 0;
	save_item(NAME(m_active));
	save_item(NAME(m_romcs));
	save_item(NAME(m_busy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_lprint_device::device_reset()
{
	m_romcs = 0;
	m_centronics->write_strobe(1);
}

void spectrum_lprint3_device::device_reset()
{
	m_romcs = 0;
	m_centronics->write_strobe(1);
}

void spectrum_kempcentrs_device::device_reset()
{
	m_centronics->write_strobe(1);
}

void spectrum_kempcentre_device::device_reset()
{
	m_active = 0;
	m_romcs = 0;
	m_centronics->write_strobe(1);
}


//**************************************************************************
//  IMPLEMENTATION (lprint)
//**************************************************************************

READ_LINE_MEMBER(spectrum_lprint_device::romcs)
{
	return m_romcs;
}

void spectrum_lprint_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_romcs = offset >= 0x0800 && offset < 0x1000;
}

void spectrum_lprint_device::pre_data_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_romcs = offset >= 0x0800 && offset < 0x1000;
}

uint8_t spectrum_lprint_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0x7ff];

	return data;
}

uint8_t spectrum_lprint_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	if (!BIT(offset, 2))
	{
		// code check this like - if (value & 0xe0) == 0x40, bit 6 probably /BUSY
		data &= ~0xe0;
		data |= !m_busy << 6;
	}

	return data;
}

void spectrum_lprint_device::iorq_w(offs_t offset, uint8_t data)
{
	if (!BIT(offset, 2))
	{
		m_centronics->write_data0(BIT(data, 0));
		m_centronics->write_data1(BIT(data, 1));
		m_centronics->write_data2(BIT(data, 2));
		m_centronics->write_data3(BIT(data, 3));
		m_centronics->write_data4(BIT(data, 4));
		m_centronics->write_data5(BIT(data, 5));
		m_centronics->write_data6(BIT(data, 6));
		m_centronics->write_data7(BIT(data, 7));
		m_centronics->write_strobe(1); // strobe probably fired automatically
		m_centronics->write_strobe(0);
		m_centronics->write_strobe(1);
	}
}


//**************************************************************************
//  IMPLEMENTATION (lprint3)
//**************************************************************************

READ_LINE_MEMBER(spectrum_lprint3_device::romcs)
{
	return m_romcs | m_exp->romcs();
}

uint8_t spectrum_lprint3_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!BIT(offset, 2))
	{
		if (!machine().side_effects_disabled())
			m_romcs = BIT(offset, 7);
		data &= ~0xc0;
		data |= !m_rs232->dsr_r() << 6;
		data |= m_busy << 7;
	}

	return data;
}

void spectrum_lprint3_device::iorq_w(offs_t offset, uint8_t data)
{
	if (!BIT(offset, 2))
	{
		m_centronics->write_data0(BIT(data, 0));
		m_centronics->write_data1(BIT(data, 1));
		m_centronics->write_data2(BIT(data, 2));
		m_centronics->write_data3(BIT(data, 3));
		m_centronics->write_data4(BIT(data, 4));
		m_centronics->write_data5(BIT(data, 5));
		m_centronics->write_data6(BIT(data, 6));
		m_centronics->write_data7(BIT(data, 7));
		m_centronics->write_strobe(BIT(offset, 7));
		m_rs232->write_txd(!BIT(data, 7));
	}

	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_lprint3_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0x7ff];

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}


//**************************************************************************
//  IMPLEMENTATION (kempcentrs)
//**************************************************************************

uint8_t spectrum_kempcentrs_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	switch (offset)
	{
	case 0xe2bf:
		data &= ~1;
		data |= m_busy << 0;
		break;
	}

	return data;
}

void spectrum_kempcentrs_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xe0bf:
		m_centronics->write_data0(BIT(data, 0));
		m_centronics->write_data1(BIT(data, 1));
		m_centronics->write_data2(BIT(data, 2));
		m_centronics->write_data3(BIT(data, 3));
		m_centronics->write_data4(BIT(data, 4));
		m_centronics->write_data5(BIT(data, 5));
		m_centronics->write_data6(BIT(data, 6));
		m_centronics->write_data7(BIT(data, 7));
		break;
	case 0xe3bf:
		// bit 0 is /STROBE, other bits not known, upon init driver writes to this port 0x81 then 0x0f
		m_centronics->write_strobe(BIT(data, 0));
		break;
	}
}


//**************************************************************************
//  IMPLEMENTATION (kempcentre)
//**************************************************************************

READ_LINE_MEMBER(spectrum_kempcentre_device::romcs)
{
	return m_romcs;
}

void spectrum_kempcentre_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_romcs = m_active && offset >= 0x0800 && offset < 0x1000;
}

void spectrum_kempcentre_device::pre_data_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_romcs = m_active && offset >= 0x0800 && offset < 0x1000;
}

uint8_t spectrum_kempcentre_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	if (!BIT(offset, 2)) // earlier version ? uses same paging as Lprint III
	{
		if (!machine().side_effects_disabled())
			m_active = BIT(offset, 7);
	}

	if ((offset & 0xf0) == 0xb0) // BB or BF, actual address decode is not known
	{
		data &= ~0xc0;
		data |= m_busy << 6;
	}

	return data;
}

void spectrum_kempcentre_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xf0) == 0xb0) // BB or BF, actual address decode is not known
	{
		m_centronics->write_data0(BIT(data, 0));
		m_centronics->write_data1(BIT(data, 1));
		m_centronics->write_data2(BIT(data, 2));
		m_centronics->write_data3(BIT(data, 3));
		m_centronics->write_data4(BIT(data, 4));
		m_centronics->write_data5(BIT(data, 5));
		m_centronics->write_data6(BIT(data, 6));
		m_centronics->write_data7(BIT(data, 7));
		m_centronics->write_strobe(BIT(offset, 2));
	}
}

uint8_t spectrum_kempcentre_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0x7ff];

	return data;
}


//**************************************************************************
//  IMPLEMENTATION (kempcentreu)
//**************************************************************************

void spectrum_kempcentreu_device::device_start()
{
	// descramble ROM
	std::vector<uint8_t> temp(m_rom->bytes());
	std::copy_n(m_rom->base(), m_rom->bytes(), &temp[0]);
	for (int i = 0; i < m_rom->bytes(); i++)
	{
		uint16_t addr = bitswap<11>(i, 10, 1, 0, 3, 7, 2, 6, 5, 4, 8, 9);
		m_rom->base()[i] = bitswap<8>(temp[addr], 4, 5, 0, 1, 6, 7, 2, 3);
	}
	spectrum_kempcentre_device::device_start();
}

void spectrum_kempcentreu_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 0x09F4 || offset == 0x0eb4) // later version ? paged at PRINT and COPY routines
			m_active = 1;

		m_romcs = m_active && offset >= 0x0800 && offset < 0x1000;
	}
}

uint8_t spectrum_kempcentreu_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	if ((offset & 0xf0) == 0xb0) // BB or BF, actual address decode is not known
	{
		if (!machine().side_effects_disabled())
			m_active = !BIT(offset, 2);
		data &= ~0xc0;
		data |= m_busy << 6;
		//data |= something_centronics << 7; // wired to centronics port, but connected to GND via wire-mod
	}

	return data;
}
