// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Serial/Parallel Interface emulation

**********************************************************************/

#include "spi.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "m6801"
#define MC2661_TAG      "mc2661"
#define RS232_TAG       "rs232"
#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ADAM_SPI = &device_creator<adam_spi_device>;


//-------------------------------------------------
//  ROM( adam_spi )
//-------------------------------------------------

ROM_START( adam_spi )
	ROM_REGION( 0x800, M6801_TAG, 0 )
	ROM_LOAD( "spi.bin", 0x000, 0x800, CRC(4ba30352) SHA1(99fe5aebd505a208bea6beec5d7322b15426e9c1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *adam_spi_device::device_rom_region() const
{
	return ROM_NAME( adam_spi );
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_spi_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_spi_mem, AS_PROGRAM, 8, adam_spi_device )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE(M6801_TAG, m6801_cpu_device, m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6801_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( adam_spi_io )
//-------------------------------------------------

static ADDRESS_MAP_START( adam_spi_io, AS_IO, 8, adam_spi_device )
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(p2_r, p2_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( adam_spi )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( adam_spi )
	MCFG_CPU_ADD(M6801_TAG, M6801, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(adam_spi_mem)
	MCFG_CPU_IO_MAP(adam_spi_io)
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD(MC2661_TAG, MC2661, XTAL_4_9152MHz)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor adam_spi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( adam_spi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_spi_device - constructor
//-------------------------------------------------

adam_spi_device::adam_spi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADAM_SPI, "Adam SPI", tag, owner, clock, "adam_spi", __FILE__),
		device_adamnet_card_interface(mconfig, *this),
		m_maincpu(*this, M6801_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_spi_device::device_start()
{
}


//-------------------------------------------------
//  adamnet_reset_w -
//-------------------------------------------------

void adam_spi_device::adamnet_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_spi_device::p2_r )
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4

	*/

	UINT8 data = M6801_MODE_7;

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_spi_device::p2_w )
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
