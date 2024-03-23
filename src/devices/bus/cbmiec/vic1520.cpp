// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1520 Plotter emulation

**********************************************************************/

/*
PA0 ATN
PA1 _CLK
PA5 ATTN ACK
PA6 NRFD
PA7 _DATA IN

PB0 IEEE SELECT
PB1 IEEE SELECT
PB2 IEEE SELECT
PB4 LED
PB5 REMOVE
PB6 CHANGE
PB7 FEED

PC0 _DN
PC1 _UP
PC7 COLOR SENSOR SW

PD0 X MOTOR COM A
PD1 X MOTOR COM B
PD2 X MOTOR COM C
PD3 X MOTOR COM D
PD4 Y MOTOR COM A
PD5 Y MOTOR COM B
PD6 Y MOTOR COM C
PD7 Y MOTOR COM D
*/

#include "emu.h"
#include "vic1520.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1520, vic1520_device, "vic1520", "VIC-1520 Color Printer Plotter")


//-------------------------------------------------
//  ROM( vic1520 )
//-------------------------------------------------

ROM_START( vic1520 )
	ROM_REGION( 0x800, "mcu", 0 )
	ROM_SYSTEM_BIOS( 0, "r01", "325340-01" )
	ROMX_LOAD( "325340-01.u1", 0x000, 0x800, CRC(3757da6f) SHA1(8ab43603f74b0f269bbe890d1939a9ae31307eb1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r03", "325340-03" )
	ROMX_LOAD( "325340-03.u1", 0x000, 0x800, CRC(f72ea2b6) SHA1(74c15b2cc1f7632bffa37439609cbdb50b82ea92), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vic1520_device::device_rom_region() const
{
	return ROM_NAME( vic1520 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic1520_device::device_add_mconfig(machine_config &config)
{
	M6500_1(config, m_mcu, 2_MHz_XTAL); // MPS 6500/1
	m_mcu->pa_out_cb().set(FUNC(vic1520_device::port_w));
	m_mcu->pb_in_cb().set(FUNC(vic1520_device::select_r));
	m_mcu->pb_out_cb().set(FUNC(vic1520_device::led_w));
	m_mcu->pc_out_cb().set(FUNC(vic1520_device::pen_w));
	m_mcu->pd_out_cb().set(FUNC(vic1520_device::motor_w));
}


//-------------------------------------------------
//  INPUT_PORTS( vic1520 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1520 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vic1520_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vic1520 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1520_device - constructor
//-------------------------------------------------

vic1520_device::vic1520_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VIC1520, tag, owner, clock)
	, device_cbm_iec_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_pa_data(0xff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1520_device::device_start()
{
	save_item(NAME(m_pa_data));
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void vic1520_device::cbm_iec_atn(int state)
{
	if (state && BIT(m_pa_data, 0))
	{
		m_pa_data &= 0xfe;
		m_mcu->pa_w(m_pa_data);
	}
	else if (!state && !BIT(m_pa_data, 0))
	{
		m_pa_data |= 0x01;
		m_mcu->pa_w(m_pa_data);
	}
}


//-------------------------------------------------
//  cbm_iec_clk -
//-------------------------------------------------

void vic1520_device::cbm_iec_clk(int state)
{
	if (state && BIT(m_pa_data, 1))
	{
		m_pa_data &= 0xfd;
		m_mcu->pa_w(m_pa_data);
	}
	else if (!state && !BIT(m_pa_data, 1))
	{
		m_pa_data |= 0x02;
		m_mcu->pa_w(m_pa_data);
	}
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void vic1520_device::cbm_iec_data(int state)
{
	if (state && BIT(m_pa_data, 7))
	{
		m_pa_data &= 0x7f;
		m_mcu->pa_w(m_pa_data);
	}
	else if (!state && !BIT(m_pa_data, 7))
	{
		m_pa_data |= 0x80;
		m_mcu->pa_w(m_pa_data);
	}
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void vic1520_device::cbm_iec_reset(int state)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  port_w - issue response on data line
//-------------------------------------------------

void vic1520_device::port_w(u8 data)
{
	m_bus->data_w(this, !BIT(data, 6) && (data & 0x21) != 0x01);
}


//-------------------------------------------------
//  select_r -
//-------------------------------------------------

u8 vic1520_device::select_r()
{
	return 0xff;
}


//-------------------------------------------------
//  led_w -
//-------------------------------------------------

void vic1520_device::led_w(u8 data)
{
}


//-------------------------------------------------
//  pen_w -
//-------------------------------------------------

void vic1520_device::pen_w(u8 data)
{
}


//-------------------------------------------------
//  motor_w -
//-------------------------------------------------

void vic1520_device::motor_w(u8 data)
{
}
