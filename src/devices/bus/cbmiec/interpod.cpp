// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Oxford Computer Systems Interpod IEC to IEEE interface emulation

*********************************************************************/

/*

PCB Layout
----------

INTERPOD 1000 ISS.3

|---------------------------------------|
|                                       |
|       ACIA        ROM         CPU     |
|                                       |
|CN1    75188   LS73    LS04    RIOT    |
|       75189                           |
|                       7417    VIA     |
|CN2                                    |
|                   3446  3446  3446    |
|       CN3 CN4         LD1             |
|---------------------------|   CN5   |-|
                            |---------|

Notes:
    All IC's shown.

    ROM     - 2716 "1.4"
    CPU     - Rockwell R6502P
    RIOT    - Rockwell R6532AP
    VIA     - Rockwell R6522P
    ACIA    - Thomson-CSF EF6850P
    3446    - Motorola MC3446AP
    CN1     - DB25 serial connector
    CN2     - power connector
    CN3     - DIN5 IEC connector
    CN4     - DIN5 IEC connector
    CN5     - 2x12 PCB edge IEEE-488 connector
    LD1     - LED

	http://mikenaberezny.com/hardware/c64-128/interpod-ieee-488-interface/

*/

#include "emu.h"
#include "interpod.h"
#include "bus/rs232/printer.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define R6502_TAG       "u1"
#define R6532_TAG       "u3"
#define R6522_TAG       "u4"
#define R6850_TAG       "u5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM_INTERPOD, cbm_interpod_device, "cbm_interpod", "Oxford Computer Systems Interpod")


//-------------------------------------------------
//  ROM( interpod )
//-------------------------------------------------

ROM_START( interpod )
	ROM_REGION( 0x800, R6502_TAG, 0 )
	ROM_DEFAULT_BIOS("v16")
	ROM_SYSTEM_BIOS( 0, "v14", "Version 1.4" )
	ROMX_LOAD( "1.4.u2", 0x000, 0x800, CRC(c5b71982) SHA1(614d677b7c6273f6b84fa61affaf91cfdaeed6a6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v16", "Version 1.6" )
	ROMX_LOAD( "1.6.u2", 0x000, 0x800, CRC(67bb0436) SHA1(7659c45b73f577233f7657c4da9141dcfe8b6d97), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cbm_interpod_device::device_rom_region() const
{
	return ROM_NAME( interpod );
}


//-------------------------------------------------
//  ADDRESS_MAP( interpod_mem )
//-------------------------------------------------

void cbm_interpod_device::interpod_mem(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x3b80).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x0400, 0x041f).mirror(0x3be0).m(m_riot, FUNC(mos6532_device::io_map));
	map(0x2000, 0x2001).mirror(0x9ffe).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x4000, 0x47ff).mirror(0xb800).rom().region(R6502_TAG, 0);
	map(0x8000, 0x800f).mirror(0x5ff0).m(m_via, FUNC(via6522_device::map));
}

uint8_t cbm_interpod_device::via_pa_r()
{
	/*

	    bit     description

	    PA0     NDAC sense
	    PA1
	    PA2
	    PA3     ATN sense
	    PA4
	    PA5
	    PA6     NRFD sense
	    PA7     DAV sense

	*/

	uint8_t data = 0;

	data |= m_ieee->ndac_r();
	data |= m_ieee->atn_r() << 3;
	data |= m_ieee->nrfd_r() << 6;
	data |= m_ieee->dav_r() << 7;

	return data;
}

void cbm_interpod_device::via_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0
	    PA1     NRFD drive
	    PA2     ATN drive
	    PA3
	    PA4     NDAC drive
	    PA5     DAV drive
	    PA6
	    PA7

	*/

	m_ieee->host_nrfd_w(BIT(data, 1));
	m_ieee->host_atn_w(BIT(data, 2));
	m_ieee->host_ndac_w(BIT(data, 4));
	m_ieee->host_dav_w(BIT(data, 5));
}

uint8_t cbm_interpod_device::via_pb_r()
{
	/*

	    bit     description

	    PB0     
	    PB1     
	    PB2     ATN sense
	    PB3     
	    PB4     CLK sense
	    PB5     DATA sense
	    PB6     EOI sense
	    PB7     

	*/

	uint8_t data = 0xff;
	
	if (!m_bus->atn_r()) data &= ~0x04;
	if (!m_bus->clk_r()) data &= ~0x10;
	if (!m_bus->data_r()) data &= ~0x20;
	if (!m_ieee->eoi_r()) data &= ~0x40;

	return data;
}

void cbm_interpod_device::via_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     
	    PB1     LED
	    PB2     
	    PB3     EOI drive
	    PB4     
	    PB5     
	    PB6     
	    PB7     ACIA TXCLK (T1 PB7 square wave output)

	*/

	m_led = BIT(data, 1);

	m_ieee->host_eoi_w(BIT(data, 3));

	m_acia->write_txc(BIT(data, 7));
}

void cbm_interpod_device::iec_clk_w(int state)
{
	m_bus->clk_w(this, state);
}

void cbm_interpod_device::iec_data_w(int state)
{
	m_bus->data_w(this, state);
}

static void interpod_rs232_devices(device_slot_interface &device)
{
	device.option_add("printer", SERIAL_PRINTER);
}

static DEVICE_INPUT_DEFAULTS_START(interpod_printer)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",   0x00ff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0x00ff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0x00ff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0x00ff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cbm_interpod_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, 1000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cbm_interpod_device::interpod_mem);

	MOS6522(config, m_via, 1000000);
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);
	m_via->readpa_handler().set(FUNC(cbm_interpod_device::via_pa_r));
	m_via->readpb_handler().set(FUNC(cbm_interpod_device::via_pb_r));
	m_via->writepa_handler().set(FUNC(cbm_interpod_device::via_pa_w));
	m_via->writepb_handler().set(FUNC(cbm_interpod_device::via_pb_w));
	m_via->ca2_handler().set(FUNC(cbm_interpod_device::iec_clk_w));
	m_via->cb2_handler().set(FUNC(cbm_interpod_device::iec_data_w));

	MOS6532(config, m_riot, 1000000);
	m_riot->pa_rd_callback().set(m_ieee, FUNC(ieee488_device::dio_r));
	m_riot->pb_wr_callback().set(m_ieee, FUNC(ieee488_device::host_dio_w));

	ACIA6850(config, m_acia);
	m_acia->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));

	ieee488_device::add_cbm_devices(config, nullptr);

	RS232_PORT(config, m_rs232, interpod_rs232_devices, nullptr);
	m_rs232->set_option_device_input_defaults("printer", DEVICE_INPUT_DEFAULTS_NAME(interpod_printer));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm_interpod_device - constructor
//-------------------------------------------------

cbm_interpod_device::cbm_interpod_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CBM_INTERPOD, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, R6502_TAG),
	m_via(*this, R6522_TAG),
	m_riot(*this, R6532_TAG),
	m_acia(*this, R6850_TAG),
	m_ieee(*this, IEEE488_TAG),
	m_rs232(*this, "rs232"),
	m_led(*this, "led0")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm_interpod_device::device_start()
{
	m_via->write_cb1(1);
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void cbm_interpod_device::cbm_iec_atn(int state)
{
	m_via->write_cb1(state);
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void cbm_interpod_device::cbm_iec_reset(int state)
{
	m_ieee->host_ifc_w(state);
	
	if (!state)
	{
		reset();
	}
}
