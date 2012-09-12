/*
    DEC LK-201 keyboard
    Emulation by R. Belmont

    This is currently the 6805 version; there's also an 8048 version.
*/

#include "emu.h"
#include "dec_lk201.h"
#include "cpu/m6805/m6805.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LK201_CPU_TAG	"lk201"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LK201 = &device_creator<lk201_device>;

ROM_START( lk201 )
	ROM_REGION(0x2000, LK201_CPU_TAG, 0)
    ROM_LOAD( "23-00159-00.bin", 0x0000, 0x2000, CRC(be293c51) SHA1(a11ae004d2d6055d7279da3560c3e56610a19fdb) )
ROM_END

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

static ADDRESS_MAP_START( lk201_map, AS_PROGRAM, 8, lk201_device )
	AM_RANGE(0x0000, 0x0002) AM_READWRITE(ports_r, ports_w)
	AM_RANGE(0x0004, 0x0006) AM_READWRITE(ddr_r, ddr_w)
    AM_RANGE(0x000a, 0x000c) AM_READWRITE(spi_r, spi_w)
    AM_RANGE(0x000d, 0x0011) AM_READWRITE(sci_r, sci_w)
    AM_RANGE(0x0050, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x1fff) AM_ROM AM_REGION(LK201_CPU_TAG, 0x100)
ADDRESS_MAP_END

//-------------------------------------------------
//  MACHINE_CONFIG
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( lk201 )
	MCFG_CPU_ADD(LK201_CPU_TAG, M68HC05EG, 2000000) // actually 68HC05C4
	MCFG_CPU_PROGRAM_MAP(lk201_map)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor lk201_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lk201 );
}

const rom_entry *lk201_device::device_rom_region() const
{
	return ROM_NAME( lk201 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  lk201_device - constructor
//-------------------------------------------------

lk201_device::lk201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, LK201, "DEC LK201 keyboard", tag, owner, clock),
	m_maincpu(*this, LK201_CPU_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lk201_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lk201_device::device_reset()
{
}

void lk201_device::device_config_complete()
{
    m_shortname = "lk201";
}

READ8_MEMBER( lk201_device::ddr_r )
{
	return ddrs[offset];
}

WRITE8_MEMBER( lk201_device::ddr_w )
{
//    printf("%02x to PORT %c DDR (PC=%x)\n", data, 'A' + offset, m_maincpu->pc());

	send_port(space, offset, ports[offset] & data);

	ddrs[offset] = data;
}

READ8_MEMBER( lk201_device::ports_r )
{
	UINT8 incoming = 0;

	switch (offset)
	{
		case 0: 	// port A
			break;

        case 1:		// port B
			break;

		case 2:		// port C
			break;
	}

	// apply data direction registers
	incoming &= (ddrs[offset] ^ 0xff);
	// add in ddr-masked version of port writes
	incoming |= (ports[offset] & ddrs[offset]);

//    printf("PORT %c read = %02x (DDR = %02x latch = %02x) (PC=%x)\n", 'A' + offset, ports[offset], ddrs[offset], ports[offset], m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::ports_w )
{
	send_port(space, offset, data);

	ports[offset] = data;
}

void lk201_device::send_port(address_space &space, UINT8 offset, UINT8 data)
{
//    printf("PORT %c write %02x (DDR = %02x) (PC=%x)\n", 'A' + offset, data, ddrs[offset], m_maincpu->pc());

	switch (offset)
	{
        case 0: // port A
			break;

        case 1: // port B
            break;

		case 2: // port C
			break;
	}
}

READ8_MEMBER( lk201_device::sci_r )
{
	UINT8 incoming = 0;

	switch (offset)
	{
		case 0: 	// baud rate
			break;

        case 1:		// control 1
			break;

		case 2:		// control 2
			break;

        case 3:		// status
            incoming |= 0x40;   // indicate transmit ready
            break;

        case 4:		// data
            break;
	}

//    printf("SCI read @ %x = %02x (PC=%x)\n", offset, incoming, m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::sci_w )
{
	switch (offset)
	{
		case 0: 	// baud rate
			break;

        case 1:		// control 1
			break;

		case 2:		// control 2
			break;

        case 3:		// status
            break;

        case 4:		// data
            break;
	}

//    printf("SCI %02x to %x (PC=%x)\n", data, offset, m_maincpu->pc());
}

READ8_MEMBER( lk201_device::spi_r )
{
	UINT8 incoming = 0;

	switch (offset)
	{
		case 0: 	// control
			break;

        case 1:		// status
            incoming |= 0x80;
			break;

		case 2:		// data
			break;
	}

//    printf("SPI read @ %x = %02x (PC=%x)\n", offset, incoming, m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::spi_w )
{
	switch (offset)
	{
		case 0: 	// control
			break;

        case 1:		// status
			break;

		case 2:		// data
			break;
	}

//    printf("SPI %02x to %x (PC=%x)\n", data, offset, m_maincpu->pc());
}

/*

SCI 01 to 4 (PC=24b)        firmware ID
SCI 00 to 4 (PC=cb2)        hardware jumpers ID (port C & 0x30)
SCI 00 to 4 (PC=cb2)        self-test result OK
SCI 00 to 4 (PC=cb2)        keycode if there was a self-test error

*/
