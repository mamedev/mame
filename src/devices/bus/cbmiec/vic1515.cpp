// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1515 Printer emulation

	(Seikosha GP 80 with modified controller PCB)

**********************************************************************/

#include "emu.h"
#include "vic1515.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MB8881_TAG "p6"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1515, vic1515_device, "vic1515", "VIC-1515 Graphic Printer")


//-------------------------------------------------
//  ROM( vic1515 )
//-------------------------------------------------

ROM_START( vic1515 )
	ROM_REGION( 0x1000, MB8881_TAG, 0 )
	ROM_LOAD( "805-5.p4", 0x0000, 0x1000, CRC(05a99a5a) SHA1(035c23dc83923eea34feea260445356a909fbd98) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vic1515_device::device_rom_region() const
{
	return ROM_NAME( vic1515 );
}


//-------------------------------------------------
//  ADDRESS_MAP( mem_map )
//-------------------------------------------------

void vic1515_device::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(MB8881_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic1515_device::device_add_mconfig(machine_config &config)
{
	MB8881(config, m_maincpu, XTAL(6'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vic1515_device::mem_map);
	m_maincpu->p1_out_cb().set(FUNC(vic1515_device::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(vic1515_device::p2_r));
	m_maincpu->t0_in_cb().set(FUNC(vic1515_device::t0_r));
	m_maincpu->t1_in_cb().set(m_printer, FUNC(unihammer_printer_device::dot_r));

	UNIHAMMER_PRINTER(config, m_printer);
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

void vic1515_device::p1_w(uint8_t data)
{
	/*

		bit		description

		P10		_PIN
		P11		_LFC
		P12		_HC
		P13		_MOT
		P14
		P15		_ER
		P16		_DOF
		P17		_DPL

	*/

	m_printer->pin_w(BIT(data, 0));
	m_printer->lfc_w(BIT(data, 1));
	m_printer->hc_w(BIT(data, 2));
	m_printer->mot_w(BIT(data, 3));

	m_er = BIT(data, 5);

	if (!BIT(data, 7))
		m_data_ff = 1;
	else if (!BIT(data, 6))
		m_data_ff = 0;

	update_iec();
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

uint8_t vic1515_device::p2_r()
{
	/*

		bit		description

		P20		A8
		P21		A9
		P22		A10
		P23		A11
		P24		HOME
		P25		_TEST
		P26		4/5
		P27		ATN

	*/

	u8 data = 0x00;
	
	data |= m_printer->home_r() << 4;
	
	u8 sds = m_sds->read();
	bool const test_mode = (sds == 2);
	bool const addr5 = (sds == 1);
	data |= (!test_mode) << 5;
	data |= (!addr5) << 6;

	data |= (!m_bus->atn_r()) << 7;

	return data;
}


//-------------------------------------------------
//  t0_r -
//-------------------------------------------------

int vic1515_device::t0_r()
{
	return !m_bus->clk_r();
}


//-------------------------------------------------
//  INPUT_PORTS( vic1515 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1515 )
	PORT_START("SDS")
	PORT_DIPNAME( 0x03, 0x00, "Self-Diagnostic Switch" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "T" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vic1515_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vic1515 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1515_device - constructor
//-------------------------------------------------

vic1515_device::vic1515_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC1515, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, MB8881_TAG),
	m_printer(*this, "printer"),
	m_sds(*this, "SDS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1515_device::device_start()
{
	// save state
	save_item(NAME(m_er));
	save_item(NAME(m_data_ff));
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void vic1515_device::cbm_iec_atn(int state)
{
	if (!state)
	{
		m_data_ff = 1;
	}

	update_iec();
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void vic1515_device::cbm_iec_data(int state)
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, m_bus->data_r());
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void vic1515_device::cbm_iec_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, !state);
}


//-------------------------------------------------
//  update_iec -
//-------------------------------------------------

void vic1515_device::update_iec()
{
	m_bus->data_w(this, !(m_er && m_data_ff));
}
