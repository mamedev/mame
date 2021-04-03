// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Swift Disc
    (c) 1987 SIXWORD ltd (UK)

    WD1770-based floppy drive interface with 8KB RAM and 16KB ROM, serial printer interface, joystick port and "interrupt" button,
    supports up to 4x 3'5 (normally) or 5"25 drives, 640KB per disk, compatible with ZX Spectrum 128, was positioned as luxury device.
    For additional charge was available ZX Interface 1 microdrive emulator software (currently missing).

    How to use:
    Press "Interrupt" button and you'll get into console where you may type commands, like:
     C[AT] [drive] - list disk contents, drive 0-3
     D[ATE] [dd-mm-yy] - set current date, or show firmware build date
     F[ORMAT] diskname - format disk
     S[AVE] filename - save snapshot to disk
     L[OAD] filename - load snapshot from disk
     A[LTER] address data - change value in RAM
     Q[UIT] - return to BASIC/game
     * - reset
    Starting from Swift Disc O/S V2 most of commands may be called from BASIC using syntax like "COMMAND %drive;"filename"", for example:
     CAT %0
     LOAD %0;"game"
    and so on, plus additional commands for sequential file access.
     FORMAT %#4;"T",1200,79 - printer setup: channel 4, text mode, 1200 baud, 79 column

    Manual: https://k1.spdns.de/Vintage/Sinclair/82/Peripherals/Disc%20Interfaces/SixWord%20Swift%20Disc%20Interface/Swift%20Disc%20Operating%20Manual.pdf


    Swift Disc II
    (c) 1989 SIXWORD ltd (UK)

    Same as above but extended to 32KB ROM, most of discrete ICs replaced with PALs, smaller form factor.
    Was available in several favours - with serial or parallel printer port.
    It seems RAM was planned to be extended to 16KB, but 2nd RAM IC is not populated on known devices.
    Dumped v4.2 firmware supports only 2 disk drives (but PCB still have wired drive 3 and 4 select) and serial port.

    Notes / TODOs:
     - serial out not working properly with v4.2 ROM, it produce short TX low pulse after each character, which (mistakenly) considered as start bit by current diserial.cpp.
     - parallel port hookup based on ROM disassembly and may be not accurate (i.e. ports may require to be enabled by some of m_control bits, or something like that)

*********************************************************************/

#include "emu.h"
#include "sixword.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_SWIFTDISC, spectrum_swiftdisc_device, "spectrum_swiftdisc", "Swift Disc Interface")
DEFINE_DEVICE_TYPE(SPECTRUM_SWIFTDISC2, spectrum_swiftdisc2_device, "spectrum_swiftdisc2", "Swift Disc II Interface")


//-------------------------------------------------
//  INPUT_PORTS
//-------------------------------------------------

INPUT_PORTS_START(swiftdisc)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Interrupt Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_swiftdisc_device, nmi_button, 0)

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

INPUT_PORTS_START(swiftdisc2)
	PORT_INCLUDE(swiftdisc)

	PORT_START("CONF")
	PORT_CONFNAME(0x01, 0x01, "Printer intrface")
	PORT_CONFSETTING(0x01, "Serial")
	PORT_CONFSETTING(0x00, "Parallel")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_swiftdisc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(swiftdisc);
}

ioport_constructor spectrum_swiftdisc2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(swiftdisc2);
}

//-------------------------------------------------
//  SLOT_INTERFACE( floppies )
//-------------------------------------------------

static void swiftdisc_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

void spectrum_swiftdisc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_SWD_FORMAT);
}

//-------------------------------------------------
//  ROM( swiftdisc )
//-------------------------------------------------

ROM_START(swiftdisc)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("v34")

	// v2.3 known to exists, not dumped
	ROM_SYSTEM_BIOS(0, "v24", "v2.4")
	ROMX_LOAD("24.ic8", 0x0000, 0x4000, CRC(547f660a) SHA1(73a1b96271760fb0edfc1302368372501455bbfc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v34", "v3.4")
	ROMX_LOAD("34.ic8", 0x0000, 0x4000, CRC(f7092c0c) SHA1(5afe0364afedc83c03995d44914fe7f207fb5c28), ROM_BIOS(1))
ROM_END

ROM_START(swiftdisc2)
	ROM_REGION(0x8000, "rom", 0)
	ROM_DEFAULT_BIOS("v42")

	ROM_SYSTEM_BIOS(0, "v42", "v4.2")
	ROMX_LOAD("42.bin", 0x0000, 0x8000, CRC(a6fde15b) SHA1(496be6f655974b6162ff430b6ba0c7002164b736), ROM_BIOS(0))

	ROM_REGION(260, "pals", 0)
	ROM_LOAD("dec1_pal16l8.bin", 0, 260, BAD_DUMP CRC(1433067c) SHA1(a0cf09a86a0d7a71332005bbb04d27d3aa45291c)) // one of logic equations clearly wrong
	ROM_LOAD("dec2_pal16l8.bin", 0, 260, CRC(4d088879) SHA1(d1d60b38068d457db25f5df304a705999f285fd9))
	ROM_LOAD("dec3_pal16l8.bin", 0, 260, CRC(8306722c) SHA1(2ad28d44bc33a50ed5f93f2520f25100dd36841d))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_swiftdisc_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 8_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));

	FLOPPY_CONNECTOR(config, "fdc:0", swiftdisc_floppies, "35dd",  spectrum_swiftdisc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", swiftdisc_floppies, "35dd",  spectrum_swiftdisc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", swiftdisc_floppies, nullptr, spectrum_swiftdisc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", swiftdisc_floppies, nullptr, spectrum_swiftdisc_device::floppy_formats).enable_sound(true);

	RS232_PORT(config, m_rs232, default_rs232_devices, "printer");

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

void spectrum_swiftdisc2_device::device_add_mconfig(machine_config &config)
{
	spectrum_swiftdisc_device::device_add_mconfig(config);

	// parallel printer port
	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->busy_handler().set(FUNC(spectrum_swiftdisc2_device::busy_w));
}

const tiny_rom_entry *spectrum_swiftdisc_device::device_rom_region() const
{
	return ROM_NAME(swiftdisc);
}

const tiny_rom_entry *spectrum_swiftdisc2_device::device_rom_region() const
{
	return ROM_NAME(swiftdisc2);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_swiftdisc_device - constructor
//-------------------------------------------------

spectrum_swiftdisc_device::spectrum_swiftdisc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
	, m_rs232(*this, "rs232")
	, m_joy(*this, "JOY")
{
}

spectrum_swiftdisc_device::spectrum_swiftdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_swiftdisc_device(mconfig, SPECTRUM_SWIFTDISC, tag, owner, clock)
{
}

spectrum_swiftdisc2_device::spectrum_swiftdisc2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_swiftdisc_device(mconfig, SPECTRUM_SWIFTDISC2, tag, owner, clock)
	, m_centronics(*this, "centronics")
	, m_conf(*this, "CONF")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_swiftdisc_device::device_start()
{
	memset(m_ram, 0, sizeof(m_ram));

	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
	save_item(NAME(m_rombank));
	save_item(NAME(m_control));
}

void spectrum_swiftdisc2_device::device_start()
{
	spectrum_swiftdisc_device::device_start();
	save_item(NAME(m_rambank));
	save_item(NAME(m_busy));
	save_item(NAME(m_txd_on));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_swiftdisc_device::device_reset()
{
	m_romcs = 0;
	m_rombank = 0;
	m_control = 0;
	m_rs232->write_txd(1);
}

void spectrum_swiftdisc2_device::device_reset()
{
	spectrum_swiftdisc_device::device_reset();
	m_rambank = 0;
	m_busy = 0;
	m_txd_on = 0;
	m_rs232->write_dtr(1);
	m_centronics->write_strobe(1);
}

//**************************************************************************
//  IMPLEMENTATION (swiftdisc)
//**************************************************************************

READ_LINE_MEMBER(spectrum_swiftdisc_device::romcs)
{
	return m_romcs | m_exp->romcs();
}

void spectrum_swiftdisc_device::post_opcode_fetch(offs_t offset)
{
	m_exp->post_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x000b:
			m_romcs = !m_romcs;
			break;
		case 0x0066:
			if (m_rombank & 0x1000)
				m_romcs = !m_romcs;
			break;
		}
	}
}

uint8_t spectrum_swiftdisc_device::mreq_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xf000)
		{
		case 0x0000:
			data = m_rom->base()[m_rombank + offset];
			break;
		case 0x1000:
		case 0x2000:
			data = m_ram[offset - 0x1000];
			break;
		case 0x3000:
			if (!BIT(offset, 3))
				data = m_fdc->read(offset & 3);
			else
			{
				// D0 - RS232 DSR, D1 - RS232 RX
				data &= ~3;
				data |= m_rs232->dsr_r() << 0;
				data |= m_rs232->rxd_r() << 1;
			}
			break;
		}
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_swiftdisc_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xf000)
		{
		case 0x0000:
			if (BIT(offset, 3))
				m_rombank ^= 0x1000;
			else
			{
				floppy_image_device* floppy = nullptr;
				for (int i = 3; i >= 0; i--)
				{
					floppy_image_device* fl = m_floppy[i]->get_device();
					if (fl)
					{
						fl->ss_w(BIT(data, 4));
						if (BIT(data, i))
							floppy = fl;
					}
				}
				m_fdc->set_floppy(floppy);

				m_rombank &= ~0x2000;
				m_rombank |= BIT(data, 5) << 13;

				// D3 - RS232 /TX
				m_rs232->write_txd(!BIT(data, 3));
				m_control = data;
			}
			break;
		case 0x1000:
		case 0x2000:
			m_ram[offset - 0x1000] = data;
			break;
		case 0x3000:
			if (!BIT(offset, 3))
				m_fdc->write(offset & 3, data);
			break;
		}
	}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

uint8_t spectrum_swiftdisc_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (!BIT(offset, 5))
		data = m_joy->read() & 0x1f;

	return data;
}


//**************************************************************************
//  IMPLEMENTATION (swiftdisc2)
//**************************************************************************

void spectrum_swiftdisc2_device::post_opcode_fetch(offs_t offset)
{
	m_exp->post_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x000b:
			m_romcs = !m_romcs;
			break;
		case 0x0066:
			if (m_rombank & 0x1000)
				m_romcs = !m_romcs;
			break;
		case 0x1709:
			if (BIT(m_control,2))
				m_romcs = !m_romcs;
			break;
		}
	}
}

uint8_t spectrum_swiftdisc2_device::mreq_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xf000)
		{
		case 0x0000:
			data = m_rom->base()[m_rombank + offset];
			break;
		case 0x1000:
		case 0x2000:
			if (!m_rambank)
				data = m_ram[offset - 0x1000];
			else
			{
				data = 0xff; // 2nd RAM IC not populated
				logerror("Swift2 RAM2 read %04x\n", offset);
			}
			break;
		case 0x3000:
			if (!BIT(offset, 4))
				data = m_fdc->read(offset & 3);
			else
				data = control_r();
			break;
		}
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_swiftdisc2_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xf000)
		{
		case 0x0000:
			if (BIT(offset, 8))
				m_rambank = !m_rambank;
			break;
		case 0x1000:
		case 0x2000:
			if (!m_rambank)
				m_ram[offset - 0x1000] = data;
			else
			{
				// 2nd RAM IC not populated
				logerror("Swift2 RAM2 write %04x %02x\n", offset, data);
			}
			break;
		case 0x3000:
			if ((offset & 0x890) == 0)
				m_fdc->write(offset & 3, data);

			if (BIT(offset, 4))
				control_w(data);
			break;
		}
	}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

uint8_t spectrum_swiftdisc2_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (m_romcs && (offset & 0xf890) == 0x3000)
		data = control_r();

	if (!BIT(offset, 5) && !BIT(m_control, 3))
		data = m_joy->read() & 0x1f;

	if (m_conf->read())
	{
		if (BIT(m_control, 2))
		{
			if (!BIT(offset, 3)) // port F7
			{
				// D7 - RS232 /RX
				data &= ~0x80;
				data |= !m_rs232->rxd_r() << 7;
			}
			if (!BIT(offset, 4)) // port EF
			{
				// D3 - RS232 /DSR
				data &= ~0x8;
				data |= !m_rs232->dsr_r() << 3;
			}
		}
	}
	else
	{
		if (!BIT(offset, 3)) // port F7
		{
			// D7 - Centronics /BUSY
			data &= ~0x80;
			data |= !m_busy << 7;
		}
	}

	return data;
}

void spectrum_swiftdisc2_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_romcs && (offset & 0xf890) == 0x3000)
		control_w(data);

	if (m_conf->read())
	{
		if (BIT(m_control, 2))
		{
			if (!BIT(offset, 3)) // port F7
			{
				// D0 - RS232 /TX
				if (m_txd_on)
					m_rs232->write_txd(!BIT(data, 0));
			}
			if (!BIT(offset, 4)) // port EF
			{
				// D0 - 0=reset /TX latch (set output to 1)
				// D4 - RS232 /DTR
				m_txd_on = BIT(data, 0);
				if (!m_txd_on)
					m_rs232->write_txd(1);
				m_rs232->write_dtr(!BIT(data, 4));
			}
		}
	}
	else
	{
		if (!BIT(offset, 3)) // F7
		{
			// D0 - Centronics /STROBE
			m_centronics->write_strobe(BIT(data, 0));
		}
		if (!BIT(offset, 4)) // EF
		{
			// D0 - D7 - Centronics data
			m_centronics->write_data0(BIT(data, 0));
			m_centronics->write_data1(BIT(data, 1));
			m_centronics->write_data2(BIT(data, 2));
			m_centronics->write_data3(BIT(data, 3));
			m_centronics->write_data4(BIT(data, 4));
			m_centronics->write_data5(BIT(data, 5));
			m_centronics->write_data6(BIT(data, 6));
			m_centronics->write_data7(BIT(data, 7));
		}
	}

	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_swiftdisc2_device::control_r()
{
	// D0 - printer interface type, 0 - parallel, 1 - serial
	return 0xfe | m_conf->read();
}

void spectrum_swiftdisc2_device::control_w(uint8_t data)
{
	floppy_image_device* floppy = nullptr;
	for (int i = 3; i >= 0; i--)
	{
		floppy_image_device* fl = m_floppy[i]->get_device();
		if (fl)
		{
			fl->ss_w(BIT(data, 4));
			if (!BIT(data, i))
				floppy = fl;
		}
	}
	m_fdc->set_floppy(floppy);

	m_rombank &= ~0x6000;
	m_rombank |= BIT(data, 5) << 13;
	m_rombank |= BIT(data, 6) << 14;

	if (BIT(data, 7) && !BIT(m_control, 7)) // swap bank at 0->1
		m_rombank ^= 0x1000;
	m_control = data;
}
