// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    FloppyOne DOS Interface
    (c) 1984/5 Rocky P. Gush (RSA)

    FD1791-based floppy drive and printer interface with 4K RAM and 8K ROM
    Was mainly designed as tape replacement, and sort of emulate how tapes works,
    which allows to copy and use most of existing software and (not protected) games with no modification.

    Doube-side drives considered as 2 separate floppies (0 and 4, 1 and 5 etc) probably to mimic tape sides A/B,
    disks is password protected.

    Main commands:
     !D=n - change current drive, where 'n' 0-3 = FDD 0-3 side 0, 4-7 = FDD 0-3 side 1, 8 = enable LOAD from tape
     CAT - list disk contents
     LOAD "name" - load program, LOAD "" will load first program (unless was disabled by !d=8 command)
     LOAD *"m";1;"name" - same as above
     SAVE "name" - save program
     !FORMAT "diskname";"password";tracks - format disk, if disk was already formatted you'll be prompted for password.
     !6=n - set text mode, 0 - regular 32-column, 3 - 64-column
     !B=rs232delay, 0=use parallel printer (default)
    There is more of special "!x=n" commands, but theirs functions is not known, manual is missing.
    Some information about this device https://worldofspectrum.org/forums/discussion/42944/rocky-gush-floppy-drive-interface/p1

    Notes / TODOs:
     - Interface1 compatibility mode is not well understood and not fully implemented
     - RS232 partially implemented

*********************************************************************/

#include "emu.h"
#include "floppyone.h"

#include "formats/fl1_dsk.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_FLPONE, spectrum_flpone_device, "spectrum_flpone", "FloppyOne DOS Interface")


//-------------------------------------------------
//  INPUT_PORTS
//-------------------------------------------------

INPUT_PORTS_START(flpone)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Snapshot Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_flpone_device, snapshot_button, 0)

	PORT_START("SW1")
	PORT_CONFNAME(0x01, 0x01, "Disc Interface")
	PORT_CONFSETTING(0x01, "Enabled")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_START("SW2")
	PORT_CONFNAME(0x01, 0x00, "Interface 1")
	PORT_CONFSETTING(0x01, "Enable")
	PORT_CONFSETTING(0x00, "Disable")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_flpone_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(flpone);
}

//-------------------------------------------------
//  SLOT_INTERFACE( floppies )
//-------------------------------------------------

static void flpone_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525ssqd", FLOPPY_525_SSQD);
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  floppy_formats
//-------------------------------------------------

void spectrum_flpone_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_FL1_FORMAT);
}

//-------------------------------------------------
//  ROM( flpone )
//-------------------------------------------------

ROM_START(flpone)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("v4")

	ROM_SYSTEM_BIOS(0, "v4", "FloppyOne DOS V4")
	ROMX_LOAD("fldos4.bin", 0x0000, 0x2000, CRC(a012f64f) SHA1(f54b47d83c1d45c0e1e10302e519693bc734ae5c), ROM_BIOS(0))

	ROM_REGION(0x4000, "prom", 0)
	ROM_LOAD("ula.bin", 0x0000, 0x2000, CRC(d67d85aa) SHA1(eb1d28bf8aa35bfab3a45a5827f252498ac2c651)) // 1st half is not used, A13 tied to Vcc
	ROM_CONTINUE(0x0000, 0x2000)
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_flpone_device::device_add_mconfig(machine_config &config)
{
	FD1791(config, m_fdc, 1_MHz_XTAL);

	FLOPPY_CONNECTOR(config, "fdc:0", flpone_floppies, "525qd", spectrum_flpone_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", flpone_floppies, "525qd", spectrum_flpone_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", flpone_floppies, nullptr, spectrum_flpone_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", flpone_floppies, nullptr, spectrum_flpone_device::floppy_formats).enable_sound(true);

	// parallel printer port
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(spectrum_flpone_device::busy_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

const tiny_rom_entry *spectrum_flpone_device::device_rom_region() const
{
	return ROM_NAME(flpone);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_flpone_device - constructor
//-------------------------------------------------

spectrum_flpone_device::spectrum_flpone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_FLPONE, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_prom(*this, "prom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
	, m_centronics(*this, "centronics")
	, m_rs232(*this, "rs232")
	, m_sw1(*this, "SW1")
	, m_sw2(*this, "SW2")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_flpone_device::device_start()
{
	memset(m_ram, 0, sizeof(m_ram));

	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
	save_item(NAME(m_busy));
	save_item(NAME(m_shifter));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_flpone_device::device_reset()
{
	m_romcs = 0;
	m_if1cs = 0;
	m_busy = 1;
	m_shifter = 0;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_flpone_device::romcs()
{
	return m_romcs || m_exp->romcs();
}

void spectrum_flpone_device::post_opcode_fetch(offs_t offset)
{
	m_exp->post_opcode_fetch(offset);

	if (!machine().side_effects_disabled() && m_sw1->read())
	{
		if (offset < 0x2000)
		{
			u8 data = m_prom->base()[offset];
			m_romcs |= BIT(data, 4) ^ 1;
			m_romcs &= BIT(data, 6);
			if (m_sw2->read())
			{
				m_if1cs |= BIT(data, 7) ^ 1;
				m_if1cs &= BIT(data, 5);
				m_romcs &= !m_if1cs;
			}
		}
	}
}

uint8_t spectrum_flpone_device::mreq_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xf000)
		{
		case 0x0000: case 0x1000:
			data = m_rom->base()[offset];
			break;
		case 0x3000:
			data = m_ram[offset & 0xfff];
			break;
		case 0x2000:
			switch (offset & 0xc)
			{
			case 0: case 8:
				data = m_fdc->read(offset & 3);
				break;
			case 4: // D5 - Centronics /BUSY / RS232 /DSR
				data &= ~(m_busy << 5);
				//data &= ~(m_rs232->dsr_r() << 5); // TODO findout how both of these combined at same bit
				break;
			case 0xc: // Centronics STROBE pulse
				m_centronics->write_strobe(1);
				m_centronics->write_strobe(0);
				m_centronics->write_strobe(1);
				break;
			}
			break;
		}
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_flpone_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xf000)
		{
		case 0x3000:
			m_ram[offset & 0xfff] = data;
			break;
		case 0x2000:
			switch (offset & 0xc)
			{
			case 0: case 8:
				m_fdc->write(offset & 3, data);
				break;
			case 4: // D7 - Centronics shifter data / RS232 TX
				m_shifter = (m_shifter << 1) | BIT(data, 7);

				m_centronics->write_data0(BIT(m_shifter, 0));
				m_centronics->write_data1(BIT(m_shifter, 1));
				m_centronics->write_data2(BIT(m_shifter, 2));
				m_centronics->write_data3(BIT(m_shifter, 3));
				m_centronics->write_data4(BIT(m_shifter, 4));
				m_centronics->write_data5(BIT(m_shifter, 5));
				m_centronics->write_data6(BIT(m_shifter, 6));
				m_centronics->write_data7(BIT(m_shifter, 7));

				m_rs232->write_txd(BIT(data, 7));
				break;
			case 0xc:
			{
				floppy_image_device* floppy = nullptr;
				for (int i = 3; i >= 0; i--)
				{
					floppy_image_device* fl = m_floppy[i]->get_device();
					if (fl)
					{
						fl->ss_w(BIT(data, 4));
						fl->mon_w((BIT(data, 5) && BIT(data, i)) ? CLEAR_LINE : ASSERT_LINE);
						if (BIT(data, i))
							floppy = fl;
					}
				}
				m_fdc->set_floppy(floppy);
			}
			break;
			}
			break;
		}
	}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}
