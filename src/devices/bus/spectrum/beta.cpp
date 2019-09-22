// license:BSD-3-Clause
// copyright-holders:Nigel Barnes, David Haywood
/*********************************************************************

    Technology Research Beta Disk interface & clones
    these are designed for the 48k Spectrum models

    There are multiple versions of this

    'hand made' PCB with V2 ROM:
     - possible prototype / low production run
     - 4k ROM
     - FORMAT, COPY etc. must be loaded from a disk to be used
     - disks are password protected
     - uses 1771 disk controller
       https://www.youtube.com/watch?v=gSJIuZjbFYs

    Original Beta Disk release with V3 ROM:
     - same features as above
     - uses a 1793 controller

    Re-release dubbed "Beta Disk plus" with V4 ROM:
     - many operations moved into a larger capacity (8k) ROM rather
       than requiring a utility disk
     - uses a 1793 controller
     - adds 'magic button' to dump the running state of the machine
       to disk
     - disk password system removed

    Many clones exist, some specific to the various Spectrum clones.
    (not yet added)

    Original Beta Disk (V3) clones
     - Sandy FDD2 SP-DOS
     - AC DOS P.Z.APINA

    Beta Disk plus (V4) clones
     - CAS DOS Cheyenne Advanced System
     - CBI-95
     - SYNCHRON IDS91
     - SYNCHRON IDS2001ne
     - ARCADE AR-20
     - Vision Desktop Betadisk

    Some units also exist that allow population of both V3 and V4
    ROM types with a switch (unofficial, for compatibility?)

    ---

    NOTE:

    ROMs really need verifying, real dumps appear to be bitswapped
    on original boards, so we're using those ones where possible,
    however sizes are unconfirmed (some sources state that the data
    is duplicated across the 16k in ROM, others state it just mirrors
    in memory) and some might be modified or bad.

    beta128.cpp could be modified to expand on this, as it builds
    on the features of the betaplus, but for now I've kept them
    separate as the enable / disable mechanisms are different and
    remaining mappings of devices unconfirmed

    ---

    Based on older BDI schematics, it seems the logic is like:

    memory access 0x3CXX (any type of access: code or data, read or write) -> temporary use BDI ROM (NOT permanent latch/switch like in beta128)
    memory access <0x4000 area and BDI ROM_latch==true -> use BDI ROM

    IO write to port 0bxxxxxx00 -> D7 master_latch, 0=enable, 1=disable

    while master_latch is enabled access to regular Spectrum IO is blocked (output /IORQ forced to 1) but enabled BDI ports:

    IO write to port 0b1xxxx111 -> D7 BDI ROM_latch (0=enable, 1=disble), D6 - FDC DDEN, D4 - SIDE, D3 - FDC HLT, D2 - FDC /MR (reset), D0-1 - floppy drive select.
    IO read port 0b1xxxx111 <- D7 - FDC INTRQ, D6 - FDC DRQ
    IO read/write ports 0b0YYxx111 - access FDC ports YY

    So mostly the same as beta128, except for new BDI ROM_latch bit


*********************************************************************/

#include "emu.h"
#include "beta.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_BETAV2,   spectrum_betav2_device, "spectrum_betav2", "TR Beta Disk Interface (older, FD1771 based)")
DEFINE_DEVICE_TYPE(SPECTRUM_BETAV3,   spectrum_betav3_device, "spectrum_betav3", "TR Beta Disk Interface (newer, FD1793 based)")
DEFINE_DEVICE_TYPE(SPECTRUM_BETAPLUS, spectrum_betaplus_device, "spectrum_betaplus", "TR Beta Disk Plus Interface")

//-------------------------------------------------
//  SLOT_INTERFACE( beta_floppies )
//-------------------------------------------------

static void beta_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_betav2_device::floppy_formats)
	FLOPPY_TRD_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( beta )
//-------------------------------------------------

ROM_START(betav2)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd20")
	ROM_SYSTEM_BIOS(0, "trd20", "TR-DOS v2.0")
	ROMX_LOAD("trd20.bin", 0x0000, 0x1000, CRC(dd269fb2) SHA1(ab394a19461f314fffd592645a273b85e76fadec), ROM_BIOS(0))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
ROM_END


// there is an alt set CRC(48f9149f) SHA1(52774757096fdc93ea94c55306481f6f41204e96) with differences at 30e, 3cd, 404, 7bd, it appears to be a bad dump
// 30c : call $2acd        call $6acd
// 3cc : ld hl, $5b00      ld hl, $5b01
// 402 : call $2acd        call $6bcd
// 7bd : ld ($5d02), hl    inc hl, ld (bc),a, ld e,l

// the Profisoft version appears to be different code, maybe based on a different revision or even modified hw?

ROM_START(betav3)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd30")
	ROM_SYSTEM_BIOS(0, "trd30", "TR-DOS v3.0 (set 1)")
	ROMX_LOAD("trd30.bin", 0x0000, 0x1000, CRC(c814179d) SHA1(b617ab59639beaa7b5d8e5b4e4a543a8eb0217c8), ROM_BIOS(0))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
//  ROM_SYSTEM_BIOS(1, "trd30a", "TR-DOS v3.0 (set 2)")
//  ROMX_LOAD("trd30_alt.bin", 0x0000, 0x1000, CRC(48f9149f) SHA1(52774757096fdc93ea94c55306481f6f41204e96), ROM_BIOS(1))
//  ROM_RELOAD(0x1000,0x1000)
//  ROM_RELOAD(0x2000,0x1000)
//  ROM_RELOAD(0x3000,0x1000)
	ROM_SYSTEM_BIOS(1, "trd30p", "TR-DOS v3.0 (set 2, Profisoft)") // is this a clone device?
	ROMX_LOAD("trd30ps.bin", 0x0000, 0x1000, CRC(b0f175a3) SHA1(ac95bb4d89072224deef58a1655e8029f811a7fa), ROM_BIOS(1))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
ROM_END

// some sources indicate these should be 16kb roms with both halves duplicated, others suggest the same but with the first byte being 0x00 instead (maybe an issue with the dumping)
// there are also some '412' unscrambled dumps with most of the data in the mirror regions as 0xff (maybe official, maybe from clone devices that should be treated separately?)
ROM_START(betaplus)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd409")
	ROM_SYSTEM_BIOS(0, "trd409", "TR-DOS v4.09")
	ROMX_LOAD("trd409.bin", 0x0000, 0x2000, CRC(18365bdc) SHA1(a0e7c80905423c54bd497575026ea8821061175a), ROM_BIOS(0))
	ROM_RELOAD(0x2000,0x2000)
	ROM_SYSTEM_BIOS(1, "trd411", "TR-DOS v4.11")
	ROMX_LOAD("trd411.bin", 0x0000, 0x2000, CRC(26902902) SHA1(cb90fc31bf62d5968730db23a600344338e31e7e), ROM_BIOS(1))
	ROM_RELOAD(0x2000,0x2000)
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_betav2_device::device_add_mconfig_base(machine_config& config)
{
	FLOPPY_CONNECTOR(config, "fdc:0", beta_floppies, "525qd", spectrum_betav2_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", beta_floppies, "525qd", spectrum_betav2_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", beta_floppies, nullptr, spectrum_betav2_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", beta_floppies, nullptr, spectrum_betav2_device::floppy_formats).enable_sound(true);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

void spectrum_betav2_device::device_add_mconfig(machine_config &config)
{
	FD1771(config, m_fdc, 4_MHz_XTAL / 4);
	device_add_mconfig_base(config);
}

void spectrum_betav3_device::device_add_mconfig(machine_config& config)
{
	FD1793(config, m_fdc, 4_MHz_XTAL / 4);
	device_add_mconfig_base(config);
}

const tiny_rom_entry *spectrum_betav2_device::device_rom_region() const
{
	return ROM_NAME(betav2);
}

const tiny_rom_entry *spectrum_betav3_device::device_rom_region() const
{
	return ROM_NAME(betav3);
}

const tiny_rom_entry *spectrum_betaplus_device::device_rom_region() const
{
	return ROM_NAME(betaplus);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_betav2_device - constructor
//-------------------------------------------------

spectrum_betav2_device::spectrum_betav2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
{
}

spectrum_betav2_device::spectrum_betav2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav2_device(mconfig, SPECTRUM_BETAV2, tag, owner, clock)
{
}

spectrum_betav3_device::spectrum_betav3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav2_device(mconfig, type, tag, owner, clock)
{
}

spectrum_betav3_device::spectrum_betav3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav3_device(mconfig, SPECTRUM_BETAV3, tag, owner, clock)
{
}

spectrum_betaplus_device::spectrum_betaplus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betav3_device(mconfig, type, tag, owner, clock)
{
}

spectrum_betaplus_device::spectrum_betaplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_betaplus_device(mconfig, SPECTRUM_BETAPLUS, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_betav2_device::device_start()
{
	save_item(NAME(m_romcs));

#if 0 // we do this in realtime instead
	for (int i = 0; i < m_rom->bytes(); i++)
	{
		uint8_t* rom = m_rom->base();

		rom[i] = bitswap<8>(rom[i],0,6,5,4,3,2,1,7);
	}
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_betav2_device::device_reset()
{
	// always paged in on boot? (no mode switch like beta128)
	m_romcs = 1;
	m_romlatch = 0;
//  m_masterportdisable = 1;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_betav2_device::romcs)
{
	return m_romcs | m_exp->romcs();
}

void spectrum_betav2_device::fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if ((offset & 0xff00) == 0x3c00)
			m_romcs = 1;
		else
			m_romcs = 0;

		if (!m_romlatch)
		{
			if (offset < 0x4000)
				m_romcs = 1;
		}
	}
}

void spectrum_betav2_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);
	fetch(offset);
}

void spectrum_betav2_device::pre_data_fetch(offs_t offset)
{
	m_exp->pre_data_fetch(offset);
	fetch(offset);
}

uint8_t spectrum_betav2_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

//  if (!m_masterportdisable)
	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0x1f: case 0x3f: case 0x5f: case 0x7f:
			data = m_fdc->read((offset >> 5) & 0x03);
			break;

		case 0xff:
			data &= 0x3f; // actually open bus
			data |= m_fdc->drq_r() ? 0x40 : 0;
			data |= m_fdc->intrq_r() ? 0x80 : 0;
			break;
		}
	}

	return data;
}

void spectrum_betav2_device::iorq_w(offs_t offset, uint8_t data)
{
//  if ((offset & 0x03) == 0x00)
//  {
//      m_masterportdisable = data & 0x80;
//  }

//  if (!m_masterportdisable)
	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0x1f: case 0x3f: case 0x5f: case 0x7f:
			m_fdc->write((offset >> 5) & 0x03, data);
			break;

		case 0xff:
			m_romlatch = data & 0x80;

			floppy_image_device* floppy = m_floppy[data & 3]->get_device();

			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 4) ? 0 : 1);
			m_fdc->dden_w(BIT(data, 6));

			// bit 3 connected to pin 23 "HLT" of FDC and via diode to INDEX
			//m_fdc->hlt_w(BIT(data, 3)); // not handled in current wd_fdc

			if (BIT(data, 2) == 0) // reset
			{
				m_fdc->reset();
				if (floppy)
					floppy->mon_w(ASSERT_LINE);
			}
			else
			{
				// TODO: implement correct motor control, FDD motor and RDY FDC pin controlled by HLD pin of FDC
				if (floppy)
				 floppy->mon_w(CLEAR_LINE);
			}
			break;
		}
	}

	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_betav2_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		data = m_rom->base()[offset & 0x3fff];
		data = bitswap<8>(data,0,6,5,4,3,2,1,7); // proper dumps have bits 0 and 7 swapped?
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_betav2_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

// Betaplus has a 'magic button' for dumping RAM

INPUT_PORTS_START(betaplus)
	PORT_START("BUTTON") // don't use F12, it clashes with the 'exit from debugger' button
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Magic Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_betaplus_device, magic_button, 0)
INPUT_PORTS_END

ioport_constructor spectrum_betaplus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(betaplus);
}

INPUT_CHANGED_MEMBER(spectrum_betaplus_device::magic_button)
{
	if (newval && !oldval)
	{
		m_slot->nmi_w(ASSERT_LINE);
		m_romcs = 1;
		m_romlatch = 0;
	}
	else
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
}
