/*
    DEC LK-201 keyboard
    Emulation by R. Belmont

    This is the later "cost-reduced" 6805 version; there's also an 8048 version.
*/

/* LK201-AA keyboard matrix (8048 version with updates)
   Source: VCB02 Technical Reference.

   KBD controller scan matrix (PORT 1): 8 x BCD IN => 18 DECIMAL OUT

   Keyboard itself:
   18 x IN (KEYBOARD DRIVE) KBD 17... KBD 0 =>
   8 OUT (keyboard data @ D7..D0)

   to => PORT 0 @ KBD controller.

________|D7  |D6  |D5  |D4 |D3 |D2 |D1 |D0
..KBD17:|[R] |F19 |[R] |F20|PF4|N--- N,| Enter
........|    |    |    |   |   |   NOTE1)
........|    |G22 |    |G23|E23|D23|C23| A23
--------|----|----|----|---|---|---|---|---
..KBD16:|F18 |PF3 |[R] |N9 |C:D|N6 |N3 |N.
........|G21 |E22 |    |D22|B17|C22|B22|A22
--------|----|----|----|---|---|---|---|---
..KBD15:|F17 |PF2 |[R] |N8 |N5 |C:R| N2|N0
........|    |    |    |   |   |   |   |NOTE 2)
........|G20 |E21 |    |D21|C21|B18|B21|
--------|----|----|----|---|---|---|---|---
  KBD14:|PF1 |Next|Rem-|C:U|N7 |N4 |N1 |N0
........|    |Scrn|move|...|   |   |   |
........|E20 |D18 |E18 |C17|D20|C20|B20|A20
--------|----|----|----|---|---|---|---|---
..KBD13:|Ins.|--- |'Do'|Prev { |"  |[R]|[R]
........|Here|-   |    Scrn| [ |'  |   |
........|E17 |E11 |G16 |D17|D11|C11|   |
--------|----|----|----|---|---|---|---|---
..KBD12:|Find|+   |Help|Se-| } |Re-|C:L| |
........|    |=   |    |lect ] |turn...| \
........|E16 |E12 |G15 |D16 D12|C13|B16|C12
--------|----|----|----|---|---|---|---|---
..KBD11:Addtnl <X||[R] |)  |P  NOTE|:  | ?
.......Options Del|    |0  |   | 3)|;  | /
........|G14 | E13|....|E10|D10|...|C10|B10
--------|----|----|----|---|---|---|---|---
..KBD10:|[R] |F12 |[R] |F13| ( |O  |L  | .
........|....|(BS)|    |(LF) 9 |   |   | .
........|....|G12 |....|G13|E09|D09|C09|B09
--------|----|----|----|---|---|---|---|---
..KBD_9:|[R] |F11 |[R] |[R]|*  |I  |K  | ,
........|....|ESC |    |   |8  |   |   | ,
........|....|G11 |....|...|E08|D08|C08|B08
--------|----|----|----|---|---|---|---|---
..KBD_8:|[R] |Main|[R] Exit|&  |U  |J  |M
........|    |Scrn|    |   |7  |   |   |
........|    |G08 |    |G09|E07|D07|C07|B07
--------|----|----|----|---|---|---|---|---
..KBD_7:|[R] Cancel[R] Resu ^  |Y  |H  |N
........|....|....|.....me |6  |   |   |
........|....|G07 |....|G06|E06|D06|C06|B06
--------|----|----|----|---|---|---|---|---
..KBD_6:|[R] |[R] |[R] Inter % |T  |G  |B
........|....|....|....rupt| 5 |   |   |
........|....|....|....|G05|E05|D05|C05|B05
--------|----|----|----|---|---|---|---|---
..KBD_5: F4  |Break [R]|$  |R  |F  |V  |Space
........|....|....|....|4  |   |   |   |
........ G02 |G03 |....|E04 D04 C04 B04 A01-A09
--------|----|----|----|---|---|---|---|---
..KBD_4: [R] |Prt.|[R] |Set|#  |E  |D  |C
........|....|Scrn|....|-Up|3  |   |   |
........|....|G00 |....|G01 E03 D03 C03 B03
--------|----|----|----|---|---|---|---|---
..KBD_3: Hold| @  |[R] |Tab|W  |S  |X  |>
........|Scrn| 2  |....|   |   |   |   |<
........|G99 |E02 |....|D00|D02|C02|B02|B00
--------|----|----|----|---|---|---|---|---
..KBD_2: [R] |[R] |[R] |~  |!  |Q  |A  |Z
........|..............|...|1
........|..............|E00 E01 D01 C01 B01
--------|----|----|----|---|---|---|---|---
..KBD_1: Ctrl|Lock|Comp|[R]
........|C99 |C00 |A99 |
--------|----|----|----|---|---|---|---|---
..KBD_0: Shift
........|B99,B11

---
  [R] = Reserved
  NOTE 1) N0--N9, N---, N and N. refer to numeric keypad
  NOTE 2) N0 can be divided into 2 keys.
  Normally only the N0 keyswitch is implemented as a double-sized key.
  NOTE 3) Return key occupies 2 positions that are
  decoded as the Return (C13) key.

  C:D - Cursor down (B17)
  C:U - Cursor up (C17)
  C:R - Cursor right (B18)
  C:L - Cursor left (B16)
 */

#include "emu.h"
#include "dec_lk201.h"
#include "cpu/m6805/m6805.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LK201_CPU_TAG   "lk201"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LK201 = &device_creator<lk201_device>;

ROM_START( lk201 )
	ROM_REGION(0x2000, LK201_CPU_TAG, 0)
	ROM_LOAD( "23-001s9-00.bin", 0x0000, 0x2000, CRC(be293c51) SHA1(a11ae004d2d6055d7279da3560c3e56610a19fdb) )
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
	: device_t(mconfig, LK201, "DEC LK201 keyboard", tag, owner, clock, "lk201", __FILE__),
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
		case 0:     // port A
			break;

		case 1:     // port B
			break;

		case 2:     // port C
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
		case 0:     // baud rate
			break;

		case 1:     // control 1
			break;

		case 2:     // control 2
			break;

		case 3:     // status
			incoming |= 0x40;   // indicate transmit ready
			break;

		case 4:     // data
			break;
	}

//    printf("SCI read @ %x = %02x (PC=%x)\n", offset, incoming, m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::sci_w )
{
	switch (offset)
	{
		case 0:     // baud rate
			break;

		case 1:     // control 1
			break;

		case 2:     // control 2
			break;

		case 3:     // status
			break;

		case 4:     // data
			break;
	}

//    printf("SCI %02x to %x (PC=%x)\n", data, offset, m_maincpu->pc());
}

READ8_MEMBER( lk201_device::spi_r )
{
	UINT8 incoming = 0;

	switch (offset)
	{
		case 0:     // control
			break;

		case 1:     // status
			incoming |= 0x80;
			break;

		case 2:     // data
			break;
	}

//    printf("SPI read @ %x = %02x (PC=%x)\n", offset, incoming, m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::spi_w )
{
	switch (offset)
	{
		case 0:     // control
			break;

		case 1:     // status
			break;

		case 2:     // data
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
