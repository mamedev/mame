// license:BSD-3-Clause
// copyright-holders:Twisted Tom
/**********************************************************************

    MGT Plus D Disc and Printer Interface

**********************************************************************/


#include "emu.h"
#include "plusd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_PLUSD, spectrum_plusd_device, "spectrum_plusd", "Plus D")


//-------------------------------------------------
//  INPUT_PORTS( plusd )
//-------------------------------------------------

INPUT_PORTS_START(plusd)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Snapshot Button") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, spectrum_plusd_device, snapshot_button, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_plusd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(plusd);
}

//-------------------------------------------------
//  SLOT_INTERFACE( plusd_floppies )
//-------------------------------------------------

static void plusd_floppies(device_slot_interface &device)
{
	//device.option_add("35ssdd", FLOPPY_35_SSDD);
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_plusd_device::floppy_formats)
	FLOPPY_MGT_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( plusd )
//-------------------------------------------------

ROM_START(plusd)
	ROM_REGION(0x2000, "rom", 0)
	ROM_DEFAULT_BIOS("gdos")
	ROM_SYSTEM_BIOS(0, "gdos", "G+DOS v1a")  // g+dos v1a
	ROMX_LOAD("plusd_g.rom", 0x0000, 0x2000, CRC(569f7e55) SHA1(6b841dc5797ef7eb219ad455cd1e434ca3b9d30d), ROM_BIOS(0))
	ROM_FILL(0x6c8, 1, 0x18)
	ROM_FILL(0x6c9, 1, 0x11)  // jr $06db
	
	ROM_SYSTEM_BIOS(1, "unidos", "UNI-DOS v?")  // uni
	ROMX_LOAD("plusd_uni.rom", 0x0000, 0x2000, CRC(60920496) SHA1(399c8c7c8335bc59849a2182c32347603fd0288a), ROM_BIOS(1))
	
	ROM_REGION(0x2000, "sys", 0)  // g+dos sys v2a
	ROM_LOAD("plusd_g_sys.ram", 0x0000, 0x2000, CRC(93af6301) SHA1(e2a80af90bbd42012cd0cea667c7d76d6c16bb69))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_plusd_device::device_add_mconfig(machine_config &config)
{
	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, "fdc:0", plusd_floppies, "35dd", spectrum_plusd_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", plusd_floppies, "35dd", spectrum_plusd_device::floppy_formats).enable_sound(true);

	/* passthru */
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

const tiny_rom_entry *spectrum_plusd_device::device_rom_region() const
{
	return ROM_NAME(plusd);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_plusd_device - constructor
//-------------------------------------------------

spectrum_plusd_device::spectrum_plusd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_PLUSD, tag, owner, clock)
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

void spectrum_plusd_device::device_start()
{
	memset(m_ram, 0, sizeof(m_ram));
	//memcpy(m_ram, memregion("sys")->base(), 0x2000);

	save_item(NAME(m_romcs));
	save_item(NAME(m_ram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_plusd_device::device_reset()
{
	m_romcs = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_plusd_device::romcs)
{
	return m_romcs | m_exp->romcs();
}

void spectrum_plusd_device::pre_opcode_fetch(offs_t offset)
{
	m_exp->pre_opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		switch (offset)
		{
		case 0x0000:
		case 0x0008:
		case 0x003a:
		case 0x0066:
			m_romcs = 1;
			break;
		}
	}
}

uint8_t spectrum_plusd_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0xe3: // fdc status reg
			data = m_fdc->read(0);
			break;
		case 0xeb: // fdc track reg
			data = m_fdc->read(1);
			break;
		case 0xf3: // fdc sector reg
			data = m_fdc->read(2);
			break;
		case 0xfb: // fdc data reg
			data = m_fdc->read(3);
			break;
		case 0xe7: // page in
			m_romcs = 1;
			break;
		case 0xf7: // bit 7 PRN BUSY
			//
			break;
		}
	}
	return data;
}

void spectrum_plusd_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0xe3: // fdc command reg
			m_fdc->write(0, data);
			break;
		case 0xeb: // fdc track reg
			m_fdc->write(1, data);
			break;
		case 0xf3: // fdc sector reg
			m_fdc->write(2, data);
			break;
		case 0xfb: // fdc data reg
			m_fdc->write(3, data);
			break;
		case 0xef: // bit 0-1: drive select, 5: ext. select(?), 6: printer strobe, 7: side select
			{
			uint8_t drive = data & 3;
			floppy_image_device* floppy = m_floppy[drive > 0 ? drive-1 : drive]->get_device();
			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 7));
			}
			break;
		case 0xe7: // page out
			m_romcs = 0;
			break;
		case 0xf7: // printer data
			//
			break;
		}
	}
	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_plusd_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x0000:
			data = m_rom->base()[offset & 0x1fff];
			break;
		case 0x2000:
			data = m_ram[offset & 0x1fff];
			break;
		}
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_plusd_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xe000)
		{
		case 0x2000:
			m_ram[offset & 0x1fff] = data;
			break;
		}
	}

	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}

INPUT_CHANGED_MEMBER(spectrum_plusd_device::snapshot_button)
{
	if (newval && !oldval)
	{
		m_slot->nmi_w(ASSERT_LINE);
	}
	else
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
}