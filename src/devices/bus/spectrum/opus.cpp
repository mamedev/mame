// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Discovery disc system

**********************************************************************/

#include "emu.h"
#include "opus.h"
#include "softlist_dev.h"

#include "formats/opd_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_OPUS, spectrum_opus_device, "spectrum_opus", "Opus Discovery")


//-------------------------------------------------
//  INPUT_PORTS( opus )
//-------------------------------------------------

static INPUT_PORTS_START( opus )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_opus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( opus );
}

//-------------------------------------------------
//  MACHINE_DRIVER( opus )
//-------------------------------------------------

void spectrum_opus_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_OPD_FORMAT);
}

static void spectrum_floppies(device_slot_interface &device)
{
	device.option_add("35ssdd", FLOPPY_35_SSDD);
	device.option_add("35dd", FLOPPY_35_DD);
}

ROM_START(opus)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("opus22")
	ROM_SYSTEM_BIOS(0, "opus22", "Opus v2.2")
	ROMX_LOAD("opus-22.rom", 0x0000, 0x2000, CRC(50f0eae0) SHA1(0eee1c503f71709fce8b7560dadc2d07d15edb80), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "opus21", "Opus v2.1")
	ROMX_LOAD("opus-21.rom", 0x0000, 0x2000, CRC(619973f9) SHA1(31999a68901392bba907cf5a15e264b6759f1a29), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "opus222", "Opus v2.22")
	ROMX_LOAD("opus-222.rom", 0x0000, 0x2000, CRC(08ce9949) SHA1(71f1c8a8b923f7751d1ff48d30b8e18a15b92591), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "quickdos", "QuickDOS v2.31") // MegaSoft
	ROMX_LOAD("quickdos-231.rom", 0x0000, 0x2000, CRC(d042b32a) SHA1(2975f7eb61d44e898cdd6e3196893e95637f17ff), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "excom", "EXCOM v2.28") // Paul Cheffings
	ROMX_LOAD("excom-228.rom", 0x0000, 0x2000, CRC(29257418) SHA1(098a812c4707251f647553a2abc1436afa38f43c), ROM_BIOS(4))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_opus_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, 16_MHz_XTAL / 2);
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));

	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, spectrum_floppies, "35dd", spectrum_opus_device::floppy_formats).enable_sound(true);

	/* parallel printer port */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_pia, FUNC(pia6821_device::ca2_w));
	m_centronics->busy_handler().set(FUNC(spectrum_opus_device::busy_w));

	/* pia */
	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(spectrum_opus_device::pia_out_a));
	m_pia->writepb_handler().set(FUNC(spectrum_opus_device::pia_out_b));
	m_pia->cb2_handler().set("centronics", FUNC(centronics_device::write_strobe));

	/* software list */
	SOFTWARE_LIST(config, "flop_list").set_original("spectrum_flop_opus");

	/* passthru without NMI */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}

const tiny_rom_entry *spectrum_opus_device::device_rom_region() const
{
	return ROM_NAME( opus );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_opus_device - constructor
//-------------------------------------------------

spectrum_opus_device::spectrum_opus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_OPUS, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_joy(*this, "JOY")
	, m_rom(*this, "rom")
	, m_pia(*this, "pia")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0U)
	, m_centronics(*this, "centronics")
	, m_exp(*this, "exp")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_opus_device::device_start()
{
	memset(m_ram, 0, sizeof(m_ram));

	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_opus_device::device_reset()
{
	m_romcs = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_opus_device::romcs()
{
	return m_romcs || m_exp->romcs();
}

void spectrum_opus_device::post_opcode_fetch(offs_t offset)
{
	m_exp->post_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0008: case 0x0048: case 0x1708:
			m_romcs = 1;
			break;
		case 0x1748:
			m_romcs = 0;
			break;
		}
	}
}

uint8_t spectrum_opus_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	// PIA bit 7 is enable joystick and selected on A5 only
	if (!BIT(m_pia->a_output(), 7) && (~offset & 0x20))
	{
		data = m_joy->read() & 0x1f;
	}
	return data;
}

uint8_t spectrum_opus_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xf800)
		{
		case 0x0000: case 0x0800: case 0x1000: case 0x1800:
			data = m_rom->base()[offset & 0x1fff];
			break;
		case 0x2000:
			data = m_ram[offset & 0x7ff];
			break;
		case 0x2800:
			data = m_fdc->read(offset & 0x03);
			break;
		case 0x3000:
			data = m_pia->read(offset & 0x03);
			break;
		case 0x3800: // Extra 2K described in QuickDOS manual - not used
			data = m_ram[offset & 0xfff];
			break;
		}
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_opus_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xf800)
		{
		case 0x2000:
			m_ram[offset & 0x7ff] = data;
			break;
		case 0x2800:
			m_fdc->write(offset & 0x03, data);
			break;
		case 0x3000:
			m_pia->write(offset & 0x03, data);
			break;
		case 0x3800: // Extra 2K described in QuickDOS manual - not used
			m_ram[offset & 0xfff] = data;
			break;
		}
	}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

void spectrum_opus_device::pia_out_a(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// bit 0, 1: drive select
	if (!BIT(data, 0)) floppy = m_floppy[1]->get_device();
	if (!BIT(data, 1)) floppy = m_floppy[0]->get_device();
	m_fdc->set_floppy(floppy);

	// bit 4: side select
	if (floppy)
		floppy->ss_w(BIT(data, 4));

	// bit 5: density
	m_fdc->dden_w(BIT(data, 5));
}

void spectrum_opus_device::pia_out_b(uint8_t data)
{
	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

void spectrum_opus_device::busy_w(int state)
{
	m_pia->set_a_input(state << 6);
}
