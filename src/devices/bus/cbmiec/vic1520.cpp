// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1520 Plotter emulation

**********************************************************************/

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
	ROM_DEFAULT_BIOS("r03")
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
	M6500_1(config, m_mcu, XTAL(2'000'000));
	m_mcu->pa_in_cb().set(FUNC(vic1520_device::pa_r));
	m_mcu->pa_out_cb().set(FUNC(vic1520_device::pa_w));
	m_mcu->pb_in_cb().set(FUNC(vic1520_device::pb_r));
	m_mcu->pb_out_cb().set(FUNC(vic1520_device::pb_w));
	m_mcu->pc_in_cb().set(FUNC(vic1520_device::pc_r));
	m_mcu->pc_out_cb().set(m_plotter, FUNC(alps_dpg23_device::pen_w));
	m_mcu->pd_out_cb().set(m_plotter, FUNC(alps_dpg23_device::motor_w));

	ALPS_DPG23(config, m_plotter);
}


//-------------------------------------------------
//  INPUT_PORTS( vic1520 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1520 )
	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PEN CHANGE")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("COLOR CHANGE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PAPER FEED")
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

vic1520_device::vic1520_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, VIC1520, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_mcu(*this, "mcu"),
	m_plotter(*this, "plotter"),
	m_pb(*this, "PB"),
	m_led(*this, "led")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1520_device::device_start()
{
	// HACK: board-local workaround for the firmware copying low IEC inputs
	// back into the PA latch. Keep the 74LS14-driven PA0/PA1/PA7 released.
	// This forces the latch bits high rather than modelling pin contention.
	m_mcu->space(AS_PROGRAM).install_write_tap(0x0080, 0x0080, "pa_iec_inputs",
		[] (offs_t offset, u8 &data, u8 mem_mask) { data |= 0x83; });

	save_item(NAME(m_attn_ack));
	save_item(NAME(m_nrfd));
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void vic1520_device::cbm_iec_atn(int state)
{
	update_iec_data();
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void vic1520_device::cbm_iec_reset(int state)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  pa_r -
//-------------------------------------------------

uint8_t vic1520_device::pa_r()
{
	/*

		bit		description

		PA0		ATTN
		PA1		_CLK
		PA2	
		PA3
		PA4
		PA5
		PA6
		PA7		_DATA IN
	
	*/

	u8 data = 0x7c;

	data |= !m_bus->atn_r();
	data |= !m_bus->clk_r() << 1;
	data |= !m_bus->data_r() << 7;

	return data;
}


//-------------------------------------------------
//  pa_w -
//-------------------------------------------------

void vic1520_device::pa_w(uint8_t data)
{
	/*

		bit		description

		PA0		
		PA1
		PA2
		PA3
		PA4
		PA5		ATTN ACK
		PA6		NRFD
		PA7
	
	*/

	m_attn_ack = BIT(data, 5);
	m_nrfd = BIT(data, 6);

	update_iec_data();
}


//-------------------------------------------------
//  update_iec_data -
//-------------------------------------------------

void vic1520_device::update_iec_data()
{
	m_bus->data_w(this, !(!m_bus->atn_r() && !m_attn_ack) && !m_nrfd);
}


//-------------------------------------------------
//  pb_r -
//-------------------------------------------------

uint8_t vic1520_device::pb_r()
{
	/*

		bit		description

		PB0		IEEE SELECT E1
		PB1		IEEE SELECT E2
		PB2		IEEE SELECT E3
		PB3
		PB4
		PB5		REMOVE
		PB6		CHANGE
		PB7		FEED
	
	*/

	u8 data = 0;

	data |= ((m_slot->get_address() - 4) & 0x07);
	data |= m_pb->read() << 5;

	return data;
}


//-------------------------------------------------
//  pb_w -
//-------------------------------------------------

void vic1520_device::pb_w(uint8_t data)
{
	/*
	
		bit		description

		PB0		
		PB1
		PB2
		PB3
		PB4		LED
		PB5
		PB6
		PB7
	
	*/

	m_led = BIT(data, 4);
}


//-------------------------------------------------
//  pc_r -
//-------------------------------------------------

uint8_t vic1520_device::pc_r()
{
	/*

		bit		description

		PC0
		PC1
		PC2
		PC3
		PC4
		PC5
		PC6
		PC7		COLOR SENSOR SW

	*/

	return 0x7f | (m_plotter->sensor_r() << 7);
}
