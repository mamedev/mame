// license:BSD-3-Clause
// copyright-holders:Golden Child
/***************************************************************************

    Epson RX-80 Dot Matrix printer (skeleton)

 Main CPU is a UPD7810 running at 11 MHz.
     8K of mask rom (marked EPSON M64200CA)
     uses 256 bytes of ram inside upd7810, no external ram chips
     has a limited line buffer of 137 bytes maximum from ff00 to ff88,
     used for character buffer as well as during graphic print operation.
     137 bytes is approximate maximum line length during condensed print.

*****************************************************************************/

#include "emu.h"
#include "epson_rx80.h"


DEFINE_DEVICE_TYPE(EPSON_RX80, epson_rx80_device, "rx80", "Epson RX-80")


//-------------------------------------------------
//  ROM( epson_rx80 )
//-------------------------------------------------

ROM_START( epson_rx80 )
	ROM_REGION(0x2000, "maincpu", 0)  // 8K rom for upd7810
	ROM_LOAD("rx80_2764.bin", 0x0000, 0x2000, CRC(5206104a) SHA1(3e304f5331181aedb321d3db23a9387e3cfacf0c))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_rx80_device::device_rom_region() const
{
	return ROM_NAME( epson_rx80 );
}


//-------------------------------------------------
//  ADDRESS_MAP( epson_rx80_mem )
//-------------------------------------------------

void epson_rx80_device::epson_rx80_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0xd800, 0xd800).r(FUNC(epson_rx80_device::centronics_data_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_rx80_device::device_add_mconfig(machine_config &config)
{
	upd7810_device &upd(UPD7810(config, m_maincpu, 11000000)); // 11 Mhz
	upd.set_addrmap(AS_PROGRAM, &epson_rx80_device::epson_rx80_mem);
	upd.pa_in_cb().set(FUNC(epson_rx80_device::porta_r));
	upd.pa_out_cb().set(FUNC(epson_rx80_device::porta_w));
	upd.pb_in_cb().set(FUNC(epson_rx80_device::portb_r));
	upd.pb_out_cb().set(FUNC(epson_rx80_device::portb_w));
	upd.pc_in_cb().set(FUNC(epson_rx80_device::portc_r));
	upd.pc_out_cb().set(FUNC(epson_rx80_device::portc_w));
	upd.pd_in_cb().set(FUNC(epson_rx80_device::portd_r));
	upd.pd_out_cb().set(FUNC(epson_rx80_device::portd_w));
	upd.pf_in_cb().set(FUNC(epson_rx80_device::portf_r));
	upd.pf_out_cb().set(FUNC(epson_rx80_device::portf_w));

	upd.an0_func().set(FUNC(epson_rx80_device::an0_r));
	upd.an1_func().set(FUNC(epson_rx80_device::an1_r));
	upd.an2_func().set(FUNC(epson_rx80_device::an2_r));
	upd.an3_func().set(FUNC(epson_rx80_device::an3_r));
	upd.an4_func().set(FUNC(epson_rx80_device::an4_r));
	upd.an5_func().set(FUNC(epson_rx80_device::an5_r));
	upd.an6_func().set(FUNC(epson_rx80_device::an6_r));
	upd.an7_func().set(FUNC(epson_rx80_device::an7_r));
//  upd.co0_func().set(FUNC(epson_rx80_device::co0_w));
}


//-------------------------------------------------
//  INPUT_PORTS( epson_rx80 )
//-------------------------------------------------

INPUT_PORTS_START( epson_rx80 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_rx80_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_rx80 );
}


//-------------------------------------------------
//  epson_rx80_device - constructor
//-------------------------------------------------

epson_rx80_device::epson_rx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_RX80, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_rx80_device::device_start()
{

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_rx80_device::device_reset()
{

}

uint8_t epson_rx80_device::porta_r()
{
	u8 result = 0;
	return result;
}

uint8_t epson_rx80_device::portb_r()
{
	u8 result = 0;
	return result;
}

uint8_t epson_rx80_device::portc_r()
{
	u8 result = 0;
	return result;
}

uint8_t epson_rx80_device::portd_r()
{
	u8 result = 0;
	return result;
}

uint8_t epson_rx80_device::portf_r()
{
	u8 result = 0;
	return result;
}

void epson_rx80_device::porta_w(uint8_t data)
{

}

void epson_rx80_device::portb_w(uint8_t data)
{

}

void epson_rx80_device::portc_w(uint8_t data)
{

}


void epson_rx80_device::portd_w(uint8_t data)
{

}

void epson_rx80_device::portf_w(uint8_t data)
{

}

//-------------------------------------------------
//  Analog Inputs
//-------------------------------------------------

uint8_t epson_rx80_device::an0_r()
{
	return 0;
}

uint8_t epson_rx80_device::an1_r()
{
	return 0;
}

uint8_t epson_rx80_device::an2_r()
{
	return 0;
}

uint8_t epson_rx80_device::an3_r()
{
	return 0;
}

uint8_t epson_rx80_device::an4_r()
{
	return 0;
}

uint8_t epson_rx80_device::an5_r()
{
	return 0;
}

uint8_t epson_rx80_device::an6_r()
{
	return 0;
}

uint8_t epson_rx80_device::an7_r()
{
	return 0;
}


/***************************************************************************
    Centronics
***************************************************************************/

void epson_rx80_device::input_strobe(int state)
{

}


void epson_rx80_device::input_init(int state)
{

}
