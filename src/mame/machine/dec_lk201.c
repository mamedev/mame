// license:BSD-3-Clause
// copyright-holders:R. Belmont, M. Burke
/*
    DEC LK-201 keyboard
    Emulation by R. Belmont & M. Burke

    This is the later "cost-reduced" 6805 version; there's also an 8048 version.
    The LK-201 mechanical elements are described in US Patent 4,467,150
*/

/* LK201-AA keyboard matrix (8048 version with updates)
   Source: VCB02 Technical Reference.

   KBD controller scan matrix (PORT 1): 8 x BCD IN => 18 DECIMAL OUT

   Keyboard itself:
   18 x IN (KEYBOARD DRIVE) KBD 17... KBD 0 =>
   8 OUT (keyboard data @ D7..D0)

   to => PORT 0 @ KBD controller.

________|D7  |D6  |D5  |D4 |D3 |D2 |D1 |D0
..KBD17:|[R] |F19 |[R] |F20|PF4|N- | N,| Enter
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
  NOTE 1) N0-N9, N-, N. and N, refer to numeric keypad
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

#define LK201_CPU_TAG   "lk201_cpu"
#define LK201_SPK_TAG   "beeper"

//-------------------------------------------------
//  SERIAL COMMUNICATIONS INTERFACE
//-------------------------------------------------

#define SCI_BAUD        0                               // Baud rate register
#define BAUD_SCR        0x07                            // SCI baud rate select
#define BAUD_SCP        0x30                            // SCI prescaler select

#define SCI_SCCR1       1                               // Control register 1
#define SCCR1_WAKE      0x08                            // Wakeup method
#define SCCR1_M         0x10                            // Character length
#define SCCR1_T8        0x40                            // Transmit bit 8
#define SCCR1_R8        0x80                            // Receive bit 8

#define SCI_SCCR2       2                               // Control register 2
#define SCCR2_SBK       0x01                            // Send break
#define SCCR2_RWU       0x02                            // Receiver wakeup enable
#define SCCR2_RE        0x04                            // Receiver enable
#define SCCR2_TE        0x08                            // Transmitter enable
#define SCCR2_ILIE      0x10                            // Idle line interrupt enable
#define SCCR2_RIE       0x20                            // Receiver interrupt enable
#define SCCR2_TCIE      0x40                            // Transmit complete interrupt enable
#define SCCR2_TIE       0x80                            // Transmit interrupt enable

#define SCI_SCSR        3                               // Status register
#define SCSR_FE         0x02                            // Receiver framing error
#define SCSR_NF         0x04                            // Receiver noise
#define SCSR_OR         0x08                            // Receiver overrun
#define SCSR_IDLE       0x10                            // Receiver idle
#define SCSR_RDRF       0x20                            // Receive data register full
#define SCSR_TC         0x40                            // Transmit complete
#define SCSR_TDRE       0x80                            // Transmit data register empty
#define SCSR_INT        (SCSR_IDLE | SCSR_RDRF| \
							SCSR_TC | SCSR_TDRE)        // Interrupt sources

#define SCI_SCDR        4                               // Data register

//-------------------------------------------------
//  SERIAL PERIPHERAL INTERFACE
//-------------------------------------------------

#define SPI_SPCR        0                               // Control register
#define SPCR_SPR        0x03                            // SPI clock rate select
#define SPCR_CPHA       0x04                            // Clock phase
#define SPCR_CPOL       0x08                            // Clock polarity
#define SPCR_MSTR       0x10                            // Master mode select
#define SPCR_DWOM       0x20                            // Port D wire-or mode option
#define SPCR_SPE        0x40                            // Serial peripheral system enable
#define SPCR_SPIE       0x80                            // Serial peripheral interrupt enable

#define SPI_SPSR        1                               // Status register
#define SPSR_MODF       0x10                            // Mode fault flag
#define SPSR_WCOL       0x40                            // Write collision
#define SPSR_SPIF       0x80                            // SPI transfer complete

#define SPI_SPDR        2                               // Data I/O Register

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

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(LK201_SPK_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
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

//-------------------------------------------------
//  INPUT_PORTS( lk201 )
//-------------------------------------------------

/* [Foreign language caps] are referenced in software titles. Please leave them in.

   DEC omitted terms like 'Interrupt', 'Break' and 'Data / Talk' on some keyboards,
   so Fn numbers are definitely important for end users.

   === CURRENT SPECIAL KEYS ===
   [PC-AT] ......=> [DEC]
   LEFT CONTROL..=> Control
   LEFT ALT .....=> Compose

   RIGHT ALT ....=> Help
   RIGHT CONTROL => Do
   ==============================================================================================
   === (PC - AT ) keys above cursor block ===
   * KEYCODE_INSERT * KEYCODE_HOME * KEYCODE_PGUP
   * KEYCODE_DEL... * KEYCODE_END  * KEYCODE_PGDN

   === (DEC LK 201 layout above cursor) ===
   * Find   ........| Insert Here  | Remove
   * Select.........| Prev   ..... | Next
   ==============================================================================================
   === CURRENT NUM PAD ASSIGNMENTS ===
   [PF1] to [PF4] are mapped to NUM LOCK, SLASH etc. (=> 4 keys on top on num pad).
   Num pad '+' gives         ',' on the DEC.
           ',' translates to '.' (=> more or less the layout of model 'LK-201-AG')

   Switch between 'full' and 'partial keyboard emulation' with Scroll Lock.
*/

INPUT_PORTS_START( lk201 )

#ifndef KEYBOARD_WORKAROUND
	PORT_START("KBD0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KBD1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Compose") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("KBD2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tilde") PORT_CODE(KEYCODE_TILDE) // E00
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("<") PORT_CODE(KEYCODE_BACKSLASH2) // B00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Hold Screen (F1)") PORT_CODE(KEYCODE_F1)

	PORT_START("KBD4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Setup (F3)") PORT_CODE(KEYCODE_PAUSE) // SET UP = Pause on PC
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Print Screen (F2)") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Break (F5)") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Data / Talk (F4)") PORT_CODE(KEYCODE_F4)

	PORT_START("KBD6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Interrupt (F6) [X]") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Resume (F7) [Fortsetzen]") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cancel (F8) [Zuruecknehmen]") PORT_CODE(KEYCODE_F8)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Exit (F10) [Fertig]") PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Main Screen (F9) [Hauptbild]") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC (F11)") PORT_CODE(KEYCODE_F11)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LF (F13)") PORT_CODE(KEYCODE_F13)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BS (F12)") PORT_CODE(KEYCODE_F12)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KBD11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // FIXME - duplicate "Return"
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Delete <X") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Additional Options (F14) [Zusaetze]") PORT_CODE(KEYCODE_PRTSCR)

	PORT_START("KBD12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Select") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help (F15)") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Find") PORT_CODE(KEYCODE_INSERT)

	PORT_START("KBD13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Previous [^]") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Do (F16) [Ausfuehren]") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Insert Here") PORT_CODE(KEYCODE_HOME)

	PORT_START("KBD14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Remove") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Next [v]") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_NUMLOCK)

	PORT_START("KBD15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // FIXME - duplicate "Num 0"
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F17")

	PORT_START("KBD16")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD)  // "." on num.pad
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F18")

	PORT_START("KBD17")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num ,") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // PORT_NAME("Num -") = duplicate...see KBD13
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F20")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F19")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

#endif
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor lk201_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( lk201 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  lk201_device - constructor
//-------------------------------------------------

lk201_device::lk201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LK201, "DEC LK201 keyboard", tag, owner, clock, "lk201", __FILE__),
	device_serial_interface(mconfig, *this),
	m_maincpu(*this, LK201_CPU_TAG),
	m_speaker(*this, LK201_SPK_TAG),
	m_kbd0(*this, "KBD0"),
	m_kbd1(*this, "KBD1"),
	m_kbd2(*this, "KBD2"),
	m_kbd3(*this, "KBD3"),
	m_kbd4(*this, "KBD4"),
	m_kbd5(*this, "KBD5"),
	m_kbd6(*this, "KBD6"),
	m_kbd7(*this, "KBD7"),
	m_kbd8(*this, "KBD8"),
	m_kbd9(*this, "KBD9"),
	m_kbd10(*this, "KBD10"),
	m_kbd11(*this, "KBD11"),
	m_kbd12(*this, "KBD12"),
	m_kbd13(*this, "KBD13"),
	m_kbd14(*this, "KBD14"),
	m_kbd15(*this, "KBD15"),
	m_kbd16(*this, "KBD16"),
	m_kbd17(*this, "KBD17"),
	m_tx_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lk201_device::device_start()
{
	m_tx_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void lk201_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(4800);

	sci_status = (SCSR_TC | SCSR_TDRE);

	spi_status = 0;
	spi_data = 0;

	kbd_data = 0;
	led_data = 0;

	transmit_register_reset();
	receive_register_reset();
}

void lk201_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void lk201_device::rcv_complete()
{
	sci_status |= SCSR_RDRF;
	update_interrupts();
	receive_register_extract();
//  printf("lk201 got %02x\n", get_received_char());
}

void lk201_device::tra_complete()
{
	sci_status |= (SCSR_TC | SCSR_TDRE);
	update_interrupts();
}

void lk201_device::tra_callback()
{
	m_tx_handler(transmit_register_get_data_bit());
}

void lk201_device::update_interrupts()
{
	if (sci_ctl2 & sci_status & SCSR_INT)
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, 1);
	}
	else
	{
		m_maincpu->set_input_line(M68HC05EG_INT_CPI, 0);
	}
}

READ8_MEMBER( lk201_device::ddr_r )
{
	return ddrs[offset];
}

WRITE8_MEMBER( lk201_device::ddr_w )
{
//  printf("%02x to PORT %c DDR (PC=%x)\n", data, 'A' + offset, m_maincpu->pc());

	send_port(space, offset, ports[offset] & data);

	ddrs[offset] = data;
}

READ8_MEMBER( lk201_device::ports_r )
{
	UINT8 incoming = 0;

	// apply data direction registers
	incoming &= (ddrs[offset] ^ 0xff);
	// add in ddr-masked version of port writes
	incoming |= (ports[offset] & ddrs[offset]);

//  printf("PORT %c read = %02x (DDR = %02x latch = %02x) (PC=%x)\n", 'A' + offset, ports[offset], ddrs[offset], ports[offset], m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::ports_w )
{
	send_port(space, offset, data);

	ports[offset] = data;
}

void lk201_device::send_port(address_space &space, UINT8 offset, UINT8 data)
{
//  printf("PORT %c write %02x (DDR = %02x) (PC=%x)\n", 'A' + offset, data, ddrs[offset], m_maincpu->pc());

	switch (offset)
	{
		case 0: // port A
			break;

		case 1: // port B
			break;

		case 2: // port C
			// Check for keyboard read strobe
			if (((data & 0x40) == 0) && (ports[offset] & 0x40))
			{
#ifndef KEYBOARD_WORKAROUND
				if (ports[0] & 0x1) kbd_data = m_kbd0->read();
				if (ports[0] & 0x2) kbd_data = m_kbd1->read();
				if (ports[0] & 0x4) kbd_data = m_kbd2->read();
				if (ports[0] & 0x8) kbd_data = m_kbd3->read();
				if (ports[0] & 0x10) kbd_data = m_kbd4->read();
				if (ports[0] & 0x20) kbd_data = m_kbd5->read();
				if (ports[0] & 0x40) kbd_data = m_kbd6->read();
				if (ports[0] & 0x80) kbd_data = m_kbd7->read();
				if (ports[1] & 0x1) kbd_data = m_kbd8->read();
				if (ports[1] & 0x2) kbd_data = m_kbd9->read();
				if (ports[1] & 0x4) kbd_data = m_kbd10->read();
				if (ports[1] & 0x8) kbd_data = m_kbd11->read();
				if (ports[1] & 0x10) kbd_data = m_kbd12->read();
				if (ports[1] & 0x20) kbd_data = m_kbd13->read();
				if (ports[1] & 0x40) kbd_data = m_kbd14->read();
				if (ports[1] & 0x80) kbd_data = m_kbd15->read();
				if (ports[2] & 0x1) kbd_data = m_kbd16->read();
				if (ports[2] & 0x2) kbd_data = m_kbd17->read();
			}
			// Check for LED update strobe
			if (((data & 0x80) == 0) && (ports[offset] & 0x80))
			{
				// Lower nibble contains the LED values (1 = on, 0 = off)
				output_set_value("led_wait"   , (led_data & 0x1) == 0);
				output_set_value("led_compose", (led_data & 0x2) == 0);
				output_set_value("led_hold"   , (led_data & 0x4) == 0);
				output_set_value("led_lock"   , (led_data & 0x8) == 0);
			}
#endif

			break;
	}
}

READ8_MEMBER( lk201_device::sci_r )
{
	UINT8 incoming = 0;

	switch (offset)
	{
		case SCI_BAUD:  // Baud rate
			break;

		case SCI_SCCR1: // Control 1
			break;

		case SCI_SCCR2: // Control 2
			incoming = sci_ctl2;
			break;

		case SCI_SCSR:  // Status
			incoming = sci_status;
			break;

		case SCI_SCDR:  // Data
			incoming = get_received_char();
			sci_status &= ~SCSR_RDRF;
			m_maincpu->set_input_line(M68HC05EG_INT_CPI, 0);
			update_interrupts();
			break;
	}

//  printf("SCI read @ %x = %02x (PC=%x)\n", offset, incoming, m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::sci_w )
{
	switch (offset)
	{
		case SCI_BAUD:  // Baud rate
			break;

		case SCI_SCCR1: // Control 1
			break;

		case SCI_SCCR2: // Control 2
			sci_ctl2 = data;
			update_interrupts();
			break;

		case SCI_SCSR:  // Status
			break;

		case SCI_SCDR:  // Data
//          printf("LK201: sending %02x\n", data);
			transmit_register_setup(data);
			sci_status &= ~(SCSR_TC | SCSR_TDRE);
			m_maincpu->set_input_line(M68HC05EG_INT_CPI, 0);
			update_interrupts();
			break;
	}

//  printf("SCI %02x to %x (PC=%x)\n", data, offset, m_maincpu->pc());
}

READ8_MEMBER( lk201_device::spi_r )
{
	UINT8 incoming = 0;

	switch (offset)
	{
		case SPI_SPCR:  // Control
			break;

		case SPI_SPSR:  // Status
			incoming = spi_status;
			spi_status &= ~SPSR_SPIF;
			break;

		case SPI_SPDR:  // Data I/O
			incoming = spi_data;
			break;
	}

//  printf("SPI read @ %x = %02x (PC=%x)\n", offset, incoming, m_maincpu->pc());

	return incoming;
}

WRITE8_MEMBER( lk201_device::spi_w )
{
	switch (offset)
	{
		case SPI_SPCR:  // Control
			break;

		case SPI_SPSR:  // Status (read only)
			break;

		case SPI_SPDR:  // Data I/O
			spi_data = data;

			// Transfer only allowed if transfer complete flag has been acknowleged
			if ((spi_status & SPSR_SPIF) == 0)
			{
				// Data out
				led_data = data;

				// Data in
				spi_data = kbd_data;

				// Indicate transfer complete
				spi_status |= SPSR_SPIF;
			}
			break;
	}

//  printf("SPI %02x to %x (PC=%x)\n", data, offset, m_maincpu->pc());
}
