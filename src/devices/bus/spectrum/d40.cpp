// license:BSD-3-Clause
// copyright-holders:MetalliC
/**********************************************************************

    Didaktik D40/D80 disk interface
    (C) 1991 Didaktik Skalica

    useful commands:
     RUN - boot disc
     CAT - files list
     LIST * - system information
     LOAD *"filename" - load program

**********************************************************************/


#include "emu.h"
#include "d40.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_D40, spectrum_d40_device, "spectrum_d40", "Didaktik D40")
DEFINE_DEVICE_TYPE(SPECTRUM_D80, spectrum_d80_device, "spectrum_d80", "Didaktik D80 (MDOS 1, 2793 FDC)")
DEFINE_DEVICE_TYPE(SPECTRUM_D80V2, spectrum_d80v2_device, "spectrum_d80v2", "Didaktik D80 (MDOS 2, 8272 FDC)")

//-------------------------------------------------
//  INPUT_PORTS
//-------------------------------------------------

INPUT_PORTS_START(d40)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Snapshot Button") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_d40base_device, snapshot_button, 0)

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_d40base_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(d40);
}

//-------------------------------------------------
//  SLOT_INTERFACE( floppies )
//-------------------------------------------------

static void didaktik_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("525dd", FLOPPY_525_DD);
}

//-------------------------------------------------
//  ROM
//-------------------------------------------------

ROM_START(d40)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("mdos11")
	ROM_SYSTEM_BIOS(0, "mdos11", "MDOS 1.0 (1991)")
	ROMX_LOAD("mdos10.bin", 0x0000, 0x4000, CRC(e6b70939) SHA1(308c4b5daf6bb1f05c68a447129d723da423326e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mdos12", "MDOS 1.0+ (1992)") // possible unofficial modification
	ROMX_LOAD("mdos10p.bin", 0x0000, 0x4000, CRC(92c45741) SHA1(b8473235feecff4eccbace56a90cf1d8c79506eb), ROM_BIOS(1))
ROM_END

ROM_START(d80v2)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("mdos2")
	ROM_SYSTEM_BIOS(0, "mdos2", "MDOS 2.0 (1993)")
	ROMX_LOAD("mdos20.bin", 0x0000, 0x4000, CRC(9e79d022) SHA1(e8d3355051fb287dd0dda34ba8824442130c8254), ROM_BIOS(0))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_d40_device::device_add_mconfig(machine_config &config)
{
	WD2797(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(spectrum_d40_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(spectrum_d40_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", didaktik_floppies, "525dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", didaktik_floppies, "525dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("JOY");

	/* software list */
	//SOFTWARE_LIST(config, "flop_list").set_original("spectrum_d40_flop");
}

void spectrum_d80_device::device_add_mconfig(machine_config &config)
{
	WD2797(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(spectrum_d80_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(spectrum_d80_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", didaktik_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", didaktik_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	I8255(config, m_ppi);

	/* software list */
	//SOFTWARE_LIST(config, "flop_list").set_original("spectrum_d40_flop");
}

void spectrum_d80v2_device::device_add_mconfig(machine_config &config)
{
	WD37C65C(config, m_fdc, 16_MHz_XTAL); // actually GM82C765B
	FLOPPY_CONNECTOR(config, "fdc:0", didaktik_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", didaktik_floppies, "35dd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	I8255(config, m_ppi);

	/* software list */
	//SOFTWARE_LIST(config, "flop_list").set_original("spectrum_d40_flop");
}

const tiny_rom_entry *spectrum_d40_device::device_rom_region() const
{
	return ROM_NAME(d40);
}

const tiny_rom_entry *spectrum_d80v2_device::device_rom_region() const
{
	return ROM_NAME(d80v2);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_d40base_device - constructor
//-------------------------------------------------

spectrum_d40base_device::spectrum_d40base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_ppi(*this, "ppi")
	, m_snap(*this, "BUTTON")
{
}

spectrum_d40_device::spectrum_d40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_d40base_device(mconfig, SPECTRUM_D40, tag, owner, clock)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
{
}

spectrum_d40_device::spectrum_d40_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	:spectrum_d40base_device(mconfig, type, tag, owner, clock)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
{
}

spectrum_d80_device::spectrum_d80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_d40_device(mconfig, SPECTRUM_D80, tag, owner, clock)
{
}

spectrum_d80v2_device::spectrum_d80v2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_d40base_device(mconfig, SPECTRUM_D80V2, tag, owner, clock)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_d40base_device::device_start()
{
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
	save_item(NAME(m_snap_flag));
	save_item(NAME(m_8255_reset));
	save_item(NAME(m_8255_enable));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_d40base_device::device_reset()
{
	m_romcs = 0;
	m_snap_flag = 0;
	m_8255_reset = 0;
	m_8255_enable = 0;
}


//**************************************************************************
//  IMPLEMENTATION  spectrum_d40base_device
//**************************************************************************

bool spectrum_d40base_device::romcs()
{
	return m_romcs;
}

void spectrum_d40base_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0000:
		case 0x0008:
			m_romcs = 1;
			nmi_check();
			break;
		case 0x0066:
			if (!m_romcs)
			{
				m_romcs = 1;
				m_snap_flag = 1;
				nmi_check();
			}
			break;
		case 0x1700:
			if (!m_snap_flag)
				m_romcs = 0;
			nmi_check();
			break;
		}
	}
}

uint8_t spectrum_d40base_device::iorq_r(offs_t offset)
{
	uint8_t data = offset & 1 ? m_slot->fb_r() : 0xff;

	switch (offset & 0xf9)
	{
	case 0x81: // FDC
		data = fdc0_r(offset);
		break;
	case 0x89: // FDC
		data = fdc1_r(offset);
		break;
	}

	if (!BIT(offset, 7) && m_8255_enable) // 1F/3F/5F/7F
		data = m_ppi->read((offset >> 5) & 3);

	return data;
}

void spectrum_d40base_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf9)
	{
	case 0x81: // FDC
		fdc0_w(offset, data);
		break;
	case 0x89: // control port
		fdc1_w(offset, data);
		break;
	case 0x91: // PPI reset
		m_8255_reset = BIT(data, 5);
		if (!m_8255_reset)
		{
			m_ppi->reset();
			m_8255_enable = 0;
		}
		break;
	case 0x99: // PPI enable
		if (m_8255_reset)
			m_8255_enable = BIT(data, 4);
		break;
	}

	if (!BIT(offset, 7) && m_8255_enable) // 1F/3F/5F/7F
		m_ppi->write((offset >> 5) & 3, data);
}

uint8_t spectrum_d40base_device::mreq_r(offs_t offset)
{
	if (m_snap_flag && offset == 0x66)
	{
		m_snap_flag = 0; // hacky, real hardware reset this latch by Z80 /WR line
		return 0xc7; // RST 0 inject
	}

	uint8_t data = 0xff;

	if (m_romcs)
	{
		if (offset < 0x3800)
			data = m_rom->base()[offset];
		else
			data = m_ram[offset & 0x7ff];
	}

	return data;
}

void spectrum_d40base_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		if ((offset & 0xf800) == 0x3800)
			m_ram[offset & 0x7ff] = data;
	}
}

INPUT_CHANGED_MEMBER(spectrum_d40base_device::snapshot_button)
{
	nmi_check();
}

void spectrum_d40base_device::nmi_check()
{
	int state = CLEAR_LINE;

	if (!m_romcs && m_snap->read())
		state = ASSERT_LINE;

	m_slot->nmi_w(state);
}

//**************************************************************************
//  IMPLEMENTATION  spectrum_d40_device
//**************************************************************************

void spectrum_d40_device::device_start()
{
	spectrum_d40base_device::device_start();
	save_item(NAME(m_control));
	save_item(NAME(m_intrq));
	save_item(NAME(m_drq));
}

void spectrum_d40_device::device_reset()
{
	spectrum_d40base_device::device_reset();
	m_control = 0;
	m_intrq = 0;
	m_drq = 0;
}

uint8_t spectrum_d40_device::fdc0_r(offs_t offset)
{
	return m_fdc->read((offset >> 1) & 0x03);
}

void spectrum_d40_device::fdc0_w(offs_t offset, uint8_t data)
{
	m_fdc->write((offset >> 1) & 0x03, data);
}

void spectrum_d40_device::fdc1_w(offs_t offset, uint8_t data)
{
	m_control = data;

	floppy_image_device* floppy = nullptr;
	if (BIT(data, 0))
		floppy = m_floppy[0]->get_device();
	else if (BIT(data, 1))
		floppy = m_floppy[1]->get_device();
	m_fdc->set_floppy(floppy);

	m_floppy[0]->get_device()->mon_w(BIT(data, 2) ? 0 : 1);
	m_floppy[1]->get_device()->mon_w(BIT(data, 3) ? 0 : 1);

	nmi_check();
}

void spectrum_d40_device::fdc_intrq_w(int state)
{
	m_intrq = state;
	nmi_check();
}

void spectrum_d40_device::fdc_drq_w(int state)
{
	m_drq = state;
	nmi_check();
}

void spectrum_d40_device::nmi_check()
{
	int state = CLEAR_LINE;

	if (!m_romcs && m_snap->read())
		state = ASSERT_LINE;

	if (m_intrq && BIT(m_control, 7))
		state = ASSERT_LINE;

	if (m_drq && BIT(m_control, 6))
		state = ASSERT_LINE;

	m_slot->nmi_w(state);
}

//**************************************************************************
//  IMPLEMENTATION  spectrum_d80v2_device
//**************************************************************************

void spectrum_d80v2_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0000:
		case 0x0008:
		case 0x0066:
			m_romcs = 1;
			nmi_check();
			break;
		case 0x1700:
			m_romcs = 0;
			nmi_check();
			break;
		}
	}
}

uint8_t spectrum_d80v2_device::fdc0_r(offs_t offset)
{
	return BIT(offset, 2) ? m_fdc->fifo_r() : m_fdc->msr_r();
}

uint8_t spectrum_d80v2_device::fdc1_r(offs_t offset)
{
	return m_fdc->dor_r();
}

void spectrum_d80v2_device::fdc0_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 2))
		m_fdc->fifo_w(data);
}

void spectrum_d80v2_device::fdc1_w(offs_t offset, uint8_t data)
{
	m_fdc->dor_w(data);
}

uint8_t spectrum_d80v2_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		if (offset < 0x3800)
			data = m_rom->base()[offset];
		else
			data = m_ram[offset & 0x7ff];
	}

	return data;
}
