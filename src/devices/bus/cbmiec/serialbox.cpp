// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Serial Box 64K Serial Port Buffer emulation

**********************************************************************/

#include "emu.h"
#include "serialbox.h"

#include "serialbox.lh"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG "maincpu"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM_SERIAL_BOX, cbm_serial_box_device, "cbm_serbox", "Serial Box")


//-------------------------------------------------
//  ROM( serial_box )
//-------------------------------------------------

ROM_START( serial_box )
	ROM_REGION( 0x1000, M6502_TAG, 0 )
	ROM_LOAD( "serialbx.bin", 0x0000, 0x1000, CRC(d0e0218c) SHA1(9b922f1e9f9b71e771361c52d4df2aa5695488a5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cbm_serial_box_device::device_rom_region() const
{
	return ROM_NAME( serial_box );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( serialbox )
	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pause")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset")
INPUT_PORTS_END

ioport_constructor cbm_serial_box_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( serialbox );
}


//-------------------------------------------------
//  ADDRESS_MAP( serial_box_mem )
//-------------------------------------------------

void cbm_serial_box_device::serial_box_mem(address_map &map)
{
	map(0x0000, 0xefff).ram();
	map(0xf000, 0xf000).mirror(0xff).rw(FUNC(cbm_serial_box_device::io_r), FUNC(cbm_serial_box_device::io_w));
	map(0xf400, 0xffff).rom().region(M6502_TAG, 0x400);
}

uint8_t cbm_serial_box_device::io_r()
{
	/*

		bit		description

		0		PAUSE
		1		RESET
		2		DIAG
		3		ATN IN
		4		PRINTER DATA IN
		5
		6		CLK IN
		7		DATA IN

	*/

	u8 data = 0x04;

	data |= m_pb->read();

	data |= m_bus->atn_r() << 3;
	data |= m_bus->clk_r() << 6;
	data |= m_bus->data_r() << 7;

	data |= m_iec->data_r() << 4;

	return data;
}

void cbm_serial_box_device::io_w(uint8_t data)
{
	/*

		bit		description

		0
		1		DATA OUT
		2		PRINTER ATN OUT
		3		PRINTER CLK OUT
		4		PRINTER DATA OUT
		5		RUN LED
		6		PAUSE LED
		7		FULL LED

	*/

	m_bus->data_w(this, !BIT(data, 1));

	m_iec->host_atn_w(BIT(data, 2));
	m_iec->host_clk_w(BIT(data, 3));
	m_iec->host_data_w(!BIT(data, 4));

	m_leds[LED_RUN] = !BIT(data, 5);
	m_leds[LED_PAUSE] = !BIT(data, 6);
	m_leds[LED_FULL] = !BIT(data, 7);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cbm_serial_box_device::device_add_mconfig(machine_config &config)
{
	W65C02(config, m_maincpu, XTAL(4'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &cbm_serial_box_device::serial_box_mem);

	CBM_IEC_SLOT(config, "iec4", 4, cbm_iec_devices, nullptr);
	CBM_IEC_SLOT(config, "iec5", 5, cbm_iec_devices, nullptr);
	CBM_IEC_SLOT(config, "iec6", 6, cbm_iec_devices, nullptr);
	CBM_IEC_SLOT(config, "iec7", 7, cbm_iec_devices, nullptr);
	CBM_IEC(config, m_iec, 0);

	config.set_default_layout(layout_serialbox);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm_serial_box_device - constructor
//-------------------------------------------------

cbm_serial_box_device::cbm_serial_box_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CBM_SERIAL_BOX, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_iec(*this, CBM_IEC_TAG),
	m_leds(*this, "led%u", 0U),
	m_pb(*this, "PB")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm_serial_box_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm_serial_box_device::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void cbm_serial_box_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		reset();
	}
}
