// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Serial/Parallel Interface emulation

    TODO:
    - AdamNet STATUS handlers ($FD36, $FD3D, $FD44) are set to $FF
    - load Centronics data latch from the data bus
    - unknown port 1 bits (mode switch, Centronics status)

**********************************************************************/

#include "emu.h"
#include "spi.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAM_SPI, adam_spi_device, "adam_spi", "Adam SPI")


//-------------------------------------------------
//  ROM( adam_spi )
//-------------------------------------------------

ROM_START( adam_spi )
	ROM_REGION( 0x800, "rom", 0 )
	ROM_LOAD( "spi.bin", 0x000, 0x800, CRC(4ba30352) SHA1(99fe5aebd505a208bea6beec5d7322b15426e9c1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *adam_spi_device::device_rom_region() const
{
	return ROM_NAME( adam_spi );
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_spi_mem )
//-------------------------------------------------

void adam_spi_device::adam_spi_mem(address_map &map)
{
	map(0x0100, 0x0103).rw(m_epci, FUNC(scn2661a_device::read), FUNC(scn2661a_device::write));
	map(0x0104, 0x01ff).ram();
	map(0xf800, 0xffff).rom().region("rom", 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void adam_spi_device::device_add_mconfig(machine_config &config)
{
	M6803(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &adam_spi_device::adam_spi_mem);
	m_maincpu->in_p1_cb().set(FUNC(adam_spi_device::p1_r));
	m_maincpu->out_p1_cb().set(FUNC(adam_spi_device::p1_w));
	m_maincpu->in_p2_cb().set(FUNC(adam_spi_device::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(adam_spi_device::p2_w));

	SCN2661A(config, m_epci, XTAL(4'915'200));
	m_epci->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_epci->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_epci->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_epci, FUNC(scn2661a_device::rxd_w));
	m_rs232->cts_handler().set(m_epci, FUNC(scn2661a_device::cts_w));
	m_rs232->dsr_handler().set(m_epci, FUNC(scn2661a_device::dsr_w));
	m_rs232->dcd_handler().set(m_epci, FUNC(scn2661a_device::dcd_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	input_buffer_device &cent_data_in(INPUT_BUFFER(config, "cent_data_in"));
	m_centronics->set_data_input_buffer(cent_data_in);
	m_centronics->busy_handler().set(FUNC(adam_spi_device::busy_w));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_spi_device - constructor
//-------------------------------------------------

adam_spi_device::adam_spi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ADAM_SPI, tag, owner, clock)
	, device_adamnet_card_interface(mconfig, *this)
	, m_maincpu(*this, "m6801")
	, m_epci(*this, "epci")
	, m_rs232(*this, "rs232")
	, m_centronics(*this, "centronics")
	, m_busy(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_spi_device::device_start()
{
	// state saving
	save_item(NAME(m_busy));
}


//-------------------------------------------------
//  adamnet_reset_w -
//-------------------------------------------------

void adam_spi_device::adamnet_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}


//-------------------------------------------------
//  busy_w -
//-------------------------------------------------

void adam_spi_device::busy_w(int state)
{
	m_busy = state;
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

uint8_t adam_spi_device::p1_r()
{
	/*

	    bit     description

	    0       STROBE
	    1
	    2
	    3
	    4       BUSY
	    5
	    6
	    7

	*/

	return 0xef | (m_busy << 4);
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

void adam_spi_device::p1_w(uint8_t data)
{
	m_centronics->write_strobe(!BIT(data, 0));
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

uint8_t adam_spi_device::p2_r()
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4

	*/

	uint8_t data = M6801_MODE_2;

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

void adam_spi_device::p2_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       NET TXD

	*/

	m_bus->txd_w(this, BIT(data, 4));
}
