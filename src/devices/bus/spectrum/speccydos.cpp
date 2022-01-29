// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Speccy-DOS Interface
    (c) 1987? Philips(?)

    WD1770/2-based floppy drive interface with 1K RAM and 8K ROM,
    presumable developed by Philips & MBLE Belgium, had some popularity in Hungary.

    Main commands:
     LIST * - list disc contents
     LOAD *"filename" - load and run program
     SAVE *"filename" - save program
     FORMAT *"diskname"dd80 - format double side double density 80 tracks disk
     ASN *drive# - change drive

    Manual: https://sinclair.hu/speccyalista/konyvtar/kezikonyvek/SpeccyDOSv4_manual.pdf

    Partially compatible with Spectrum128, useable after "USR 0" from basic128 or switch to 48K mode.

    Notes / TODOs:
     - address decoding performed by large DIP28 chip, probably PROM, not dumped, exact logic is not known.
     - program ROMs have swapped data lines/bits 1 and 5.
     - original software disc mentioned in manual is missing.
     - Magic/NMI function require NMI routine preloaded in RAM by "MAGIC2.x" program (which is missing).

*********************************************************************/

#include "emu.h"
#include "speccydos.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_SPECCYDOS, spectrum_speccydos_device, "spectrum_speccydos", "Speccy-DOS Interface")


//-------------------------------------------------
//  INPUT_PORTS
//-------------------------------------------------

INPUT_PORTS_START(speccydos)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Magic Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_speccydos_device, magic_button, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_speccydos_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(speccydos);
}

//-------------------------------------------------
//  SLOT_INTERFACE( floppies )
//-------------------------------------------------

static void speccydos_floppies(device_slot_interface &device)
{
	device.option_add("525sssd_35t", FLOPPY_525_SSSD_35T);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dsdd", FLOPPY_525_DD);
	device.option_add("525ssqd", FLOPPY_525_SSQD);
	device.option_add("525dsqd", FLOPPY_525_QD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
	device.option_add("35dsdd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

void spectrum_speccydos_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_SDD_FORMAT);
}

//-------------------------------------------------
//  ROM( speccydos )
//-------------------------------------------------

ROM_START(speccydos)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("sdos42")

	ROM_SYSTEM_BIOS(0, "sdos41", "Speccy DOS v4.1")
	ROMX_LOAD("sdos41.bin", 0x0000, 0x2000, CRC(fcfa6dac) SHA1(bcae27b334bbbee257900682859132f476c3be99), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "sdos42", "Speccy DOS v4.2 (WD1770)")
	ROMX_LOAD("sdos42.bin", 0x0000, 0x2000, CRC(208e7baa) SHA1(80e82c1dd77ff04b86989f7d2a25d93c5aa1dc42), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "sdos422", "Speccy DOS v4.2 (WD1772)") // only 4 byte different from above - step rates for 1772
	ROMX_LOAD("sdos422.bin", 0x0000, 0x2000, CRC(a7317e1d) SHA1(e72d2ea19cbe8f3459e4fc9d3c57975dc9917bd6), ROM_BIOS(2))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_speccydos_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 8_MHz_XTAL);

	FLOPPY_CONNECTOR(config, "fdc:0", speccydos_floppies, "525dsqd", spectrum_speccydos_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", speccydos_floppies, "525dsqd", spectrum_speccydos_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", speccydos_floppies, nullptr, spectrum_speccydos_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", speccydos_floppies, nullptr, spectrum_speccydos_device::floppy_formats).enable_sound(true);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

const tiny_rom_entry *spectrum_speccydos_device::device_rom_region() const
{
	return ROM_NAME(speccydos);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_speccydos_device - constructor
//-------------------------------------------------

spectrum_speccydos_device::spectrum_speccydos_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_SPECCYDOS, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_speccydos_device::device_start()
{
	memset(m_ram, 0, sizeof(m_ram));

	save_item(NAME(m_romcs));
	save_item(NAME(m_control));
	save_item(NAME(m_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_speccydos_device::device_reset()
{
	m_romcs = 0;
	m_control = 0;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_speccydos_device::romcs)
{
	return m_romcs | m_exp->romcs();
}

void spectrum_speccydos_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0066:
		case 0x1292:
		case 0x1b41:
			if (!BIT(m_control, 6))
				m_romcs = 1;
			break;
		}
	}
}

uint8_t spectrum_speccydos_device::mreq_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_romcs)
	{
		if (offset < 0x2000)
		{
			data = m_rom->base()[offset];
			data = bitswap<8>(data, 7, 6, 1, 4, 3, 2, 5, 0);
		}
		else if (offset < 0x2400)
			data = m_ram[offset & 0x3ff];
		else if (offset >= 0x3290 && offset < 0x3294)
			data = m_fdc->read(offset & 3);
		else
			logerror("SpeccyDOS unhandled read %04X\n", offset);
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_speccydos_device::mreq_w(offs_t offset, uint8_t data)
{
	if (offset == 0x3b41)
	{
		m_control = data;
		floppy_image_device* floppy = m_floppy[data & 3]->get_device();
		m_fdc->set_floppy(floppy);

		if (floppy)
			floppy->ss_w(BIT(data, 2));
		m_fdc->dden_w(BIT(data, 7));

		m_romcs = BIT(data, 6) ? 0 : BIT(data, 3);
	}
	else
		if (m_romcs)
		{
			if (offset >= 0x2000 && offset < 0x2400)
				m_ram[offset & 0x3ff] = data;
			else if (offset >= 0x3290 && offset < 0x3294)
				m_fdc->write(offset & 3, data);
			else
			{
				logerror("SpeccyDOS unhandled write %04X %02X\n", offset, data);
				//machine().debug_break();
			}
		}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}
