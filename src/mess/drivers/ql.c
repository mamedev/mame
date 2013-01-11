/*

    Sinclair QL

    MESS Driver by Curt Coder

*/

/*

    TODO:

    - microdrive
    - ZX8301 memory access slowdown
    - use resnet.h to create palette
    - Tyche bios is broken
    - several disk interfaces (720K/1.44MB/3.2MB)
    - Gold Card (68000 @ 16MHz, 2MB RAM)
    - Super Gold Card (68020 @ 24MHz, 4MB RAM)
    - QLToolkit II ROM
    - GST 68kOS ROM
    - natural keyboard SHIFT does not work, it causes characters to be skipped altogether
    - get modified Danish version to boot (e.g. MD_DESEL was patched to jump to 0x1cd5e)
    - ICL One Per Desk / British Telecom Merlin Tonto / Telecom Australia Computerphone / Mega OPD (US)
    - OPD: 68008P8 @ 7.5MHz, ZX8301 @ 12MHz, CDP65C51E1 @ 1.8432MHz, 128K RAM, 2K NVRAM, 128K ROM (4x32), 16K speech ROM, ADM7910 modem, TMS5220 speech, 2x100K microdrives, RTC 1/1/1970, 1xRS-423
    - Sandy Q-XT 640 (original motherboard, Super QBoard 512KB, Centronics, Super Toolkit II, FDC, 1 or 2 3.5" 1MB drives, 3 expansion slots)
    - CST Thor PC 1F (720K 3.5")
    - CST Thor PC 2F (2x 720k 3.5")
    - CST Thor PC 2FW (20MB Winchester + 720K 3.5")
    - CST Thor 20 (68020)
    - CST Thor 21 (68020+68881)
    - CST Thor XVI CF    (Workstation, 68000 @ 8 MHz, M6802, 68682 DUART)
    - CST Thor XVI IF    (Single Floppy)
    - CST Thor XVI FF    (Dual Floppy)
    - CST Thor XVI W20F  (20MB Winchester, 1 Floppy)
    - CST Thor XVI W20FF (20MB Winchester, 2 Floppies)
    - CST Thor XVI W40F  (40MB Winchester, 1 Floppy)
    - CST Thor XVI W40FF (40MB Winchester, 2 Floppies)


    2011-02-22, P.Harvey-Smith,
        Implemented Miracle Trump Card disk and ram interface.

        The trump card has some interesting memory mapping and re-mapping during initialisation which goes like this

        On rest the card ram is mapped into the entire $40000-$fffff area, this is the official 512K expansion ram area
        plus the area set aside for addon card roms. The trump card onboard rom is also mapped in at $10000-$17fff.

        The main bios then performs a series of loops that initialise and size the ram, this will find either
        0K, 256K, 512K or 768K of expansion ram depending on how many banks of ram are populated.

        After the ram test, the main bios looks for the signature longword of a cartridge rom at $0c000, this read
        triggers the Trump Card to also map it's rom in at $c0000-$c7fff, the first addon rom area. If a cartridge
        rom is found it is first initialised.

        The main bios goes on to search the addon card rom area from $c0000 by searching for the signature word for
        an addon rom at $c0000 and upwards in 16K steps (there is a bug in JM and before that means they only find
        the first addon, but this does not matter).

        The main bios finds the trump card addon rom at $c0000, and calls it's initialisation routine at $c011e, this
        contains a jump instruction to jump to $10124 (in the lower mapped copy of the trumpcard rom).

        The first memory access in the $10000-$17fff range triggers the trumcard to page out the rom at $c0000, and
        page the ram back in. Once it arrives at this state the ram will remain paged in until the next reset.

        I have implemented this with a pair of read handlers at $0c000-$0cfff and $10000-$17fff which do the apropreate
        re-map of the upper area and then remove themselves from the address map and return it to simple rom handlers.

        On the Trump card the above is implemented with a pair of 16L8 PAL chips, the logic of these was decoded with
        the assistance of Malcolm Lear, without who's help working all this out would have been a much harder process.

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
#include "imagedev/cartslot.h"
#include "imagedev/flopdrv.h"
#include "imagedev/printer.h"
#include "machine/ram.h"
#include "devices/microdrv.h"
#include "formats/basicdsk.h"
#include "machine/zx8302.h"
#include "sound/speaker.h"
#include "video/zx8301.h"
#include "includes/ql.h"
#include "machine/wd17xx.h"
#include "debugger.h"

#define LOG_DISK_WRITE  0
#define LOG_DISK_READ   0

//**************************************************************************
//  INTELLIGENT PERIPHERAL CONTROLLER
//**************************************************************************

//-------------------------------------------------
//  ipc_w -
//-------------------------------------------------

WRITE8_MEMBER( ql_state::ipc_w )
{
	m_zx8302->comctl_w(0);
	m_zx8302->comctl_w(1);
}


//-------------------------------------------------
//  ipc_port1_w -
//-------------------------------------------------

WRITE8_MEMBER( ql_state::ipc_port1_w )
{
	/*

	    bit     description

	    0       Keyboard column output (KBO0)
	    1       Keyboard column output (KBO1)
	    2       Keyboard column output (KBO2)
	    3       Keyboard column output (KBO3)
	    4       Keyboard column output (KBO4)
	    5       Keyboard column output (KBO5)
	    6       Keyboard column output (KBO6)
	    7       Keyboard column output (KBO7)

	*/

	m_keylatch = data;
}


//-------------------------------------------------
//  ipc_port2_w -
//-------------------------------------------------

WRITE8_MEMBER( ql_state::ipc_port2_w )
{
	/*

	    bit     description

	    0       Serial data input (SER2 RxD, SER1 TxD)
	    1       Speaker output
	    2       Interrupt output (IPL0-2)
	    3       Interrupt output (IPL1)
	    4       Serial Clear-to-Send output (SER1 CTS)
	    5       Serial Data Terminal Ready output (SER2 DTR)
	    6       not connected
	    7       ZX8302 serial link input/output (COMDATA)

	*/

	// speaker
	speaker_level_w(m_speaker, BIT(data, 1));

	// interrupts
	int ipl = (BIT(data, 2) << 1) | BIT(data, 3);

	if (ipl != m_ipl)
	{
		switch (ipl)
		{
		case 0: m_maincpu->set_input_line(M68K_IRQ_7, ASSERT_LINE); break;
		case 1: m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE); break; // CTRL-ALT-7 pressed
		case 2: m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE); break;
		case 3: m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);  break;
		}

		m_ipl = ipl;
	}

	// clear to send
	//rs232_cts_w(m_ser1, !BIT(data, 4));

	// data terminal ready
	//rs232_dtr_w(m_ser2, !BIT(data, 5));

	// COMDATA
	m_comdata = BIT(data, 7);

	m_zx8302->comdata_w(BIT(data, 7));
}


//-------------------------------------------------
//  ipc_port2_r -
//-------------------------------------------------

READ8_MEMBER( ql_state::ipc_port2_r )
{
	/*

	    bit     description

	    0       Serial data input (SER2 RxD, SER1 TxD)
	    1       Speaker output
	    2       Interrupt output (IPL0-2)
	    3       Interrupt output (IPL1)
	    4       Serial Clear-to-Send output (SER2 CTS)
	    5       Serial Data Terminal Ready output (SER1 DTR)
	    6       not connected
	    7       ZX8302 serial link input/output (COMDATA)

	*/

//  int irq = (m_ser2_rxd | m_ser1_txd);

//  m_ipc->execute().set_input_line(INPUT_LINE_IRQ0, irq);

	return (m_comdata << 7);
}


//-------------------------------------------------
//  ipc_t1_r -
//-------------------------------------------------

READ8_MEMBER( ql_state::ipc_t1_r )
{
	return m_baudx4;
}


//-------------------------------------------------
//  ipc_bus_r -
//-------------------------------------------------

READ8_MEMBER( ql_state::ipc_bus_r )
{
	/*

	    bit     description

	    0       Keyboard row input (KBI0)
	    1       Keyboard row input (KBI1)
	    2       Keyboard row input (KBI2)
	    3       Keyboard row input (KBI3)
	    4       Keyboard row input (KBI4)
	    5       Keyboard row input (KBI5)
	    6       Keyboard row input (KBI6)
	    7       Keyboard row input (KBI7)

	*/

	UINT8 data = 0;

	if (BIT(m_keylatch, 0)) data |= ioport("ROW0")->read() | ioport("JOY0")->read();
	if (BIT(m_keylatch, 1)) data |= ioport("ROW1")->read() | ioport("JOY1")->read();
	if (BIT(m_keylatch, 2)) data |= ioport("ROW2")->read();
	if (BIT(m_keylatch, 3)) data |= ioport("ROW3")->read();
	if (BIT(m_keylatch, 4)) data |= ioport("ROW4")->read();
	if (BIT(m_keylatch, 5)) data |= ioport("ROW5")->read();
	if (BIT(m_keylatch, 6)) data |= ioport("ROW6")->read();
	if (BIT(m_keylatch, 7)) data |= ioport("ROW7")->read();

	return data;
}

READ8_MEMBER( ql_state::disk_io_r )
{
	UINT8   result = 0;

	if(LOG_DISK_READ)
		logerror("%s DiskIO:Read of %08X\n",machine().describe_context(),m_disk_io_base+offset);

	switch (offset)
	{
		case 0x0000 : result=wd17xx_r(m_fdc, space, offset); break;
		case 0x0001 : result=wd17xx_r(m_fdc, space, offset); break;
		case 0x0002 : result=wd17xx_r(m_fdc, space, offset); break;
		case 0x0003 : result=wd17xx_r(m_fdc, space, offset); break;
		default     : logerror("%s DiskIO undefined read : from %08X\n",machine().describe_context(),m_disk_io_base+offset); break;
	}

	return result;
}

WRITE8_MEMBER( ql_state::disk_io_w )
{
	if(LOG_DISK_WRITE)
		logerror("%s DiskIO:Write %02X to %08X\n",machine().describe_context(),data,m_disk_io_base+offset);

	switch (offset)
	{
		case 0x0000 : wd17xx_w(m_fdc, space, offset, data); break;
		case 0x0001 : wd17xx_w(m_fdc, space, offset, data); break;
		case 0x0002 : wd17xx_w(m_fdc, space, offset, data); break;
		case 0x0003 : wd17xx_w(m_fdc, space, offset, data); break;
		case 0x0004 : if(m_disk_type==DISK_TYPE_SANDY)
						sandy_set_control(data);break;
		case 0x0008 : if(m_disk_type==DISK_TYPE_SANDY)
						m_printer_char=data;
		case 0x2000 : if(m_disk_type==DISK_TYPE_TRUMP)
						trump_card_set_control(data);break;
		default     : logerror("%s DiskIO undefined write : %02X to %08X\n",machine().describe_context(),data,m_disk_io_base+offset); break;
	}
}

READ8_MEMBER( ql_state::trump_card_rom_r )
{
	// If we have more than 640K them map extra ram into top 256K
	// else just map to unmap.
	if (m_ram->size()>(640*1024))
		space.install_ram(0x0c0000, 0x0fffff, NULL);
	else
		space.unmap_readwrite(0x0c0000, 0x0fffff);

	// Setup trumcard rom mapped to rom so unlink us
	space.install_rom(0x010000, 0x018000, &machine().root_device().memregion(M68008_TAG)->base()[TRUMP_ROM_BASE]);

	return memregion(M68008_TAG)->base()[TRUMP_ROM_BASE+offset];
}

READ8_MEMBER( ql_state::cart_rom_r )
{
	// Setup trumcard rom mapped in at $c0000
	space.install_rom(0x0c0000, 0x0c8000, &machine().root_device().memregion(M68008_TAG)->base()[TRUMP_ROM_BASE]);

	// Setup cart rom to rom handler, so unlink us
	space.install_rom(0x0c000, 0x0ffff, &machine().root_device().memregion(M68008_TAG)->base()[CART_ROM_BASE]);

	return memregion(M68008_TAG)->base()[CART_ROM_BASE+offset];
}

void ql_state::trump_card_set_control(UINT8 data)
{
	if(data & TRUMP_DRIVE0_MASK)
		wd17xx_set_drive(m_fdc,0);

	if(data & TRUMP_DRIVE1_MASK)
		wd17xx_set_drive(m_fdc,1);

	wd17xx_set_side(m_fdc,(data & TRUMP_SIDE_MASK) >> TRUMP_SIDE_SHIFT);
}

void ql_state::sandy_set_control(UINT8 data)
{
	//logerror("sandy_set_control:%02X\n",data);

	m_disk_io_byte=data;

	if(data & SANDY_DRIVE0_MASK)
		wd17xx_set_drive(m_fdc,0);

	if(data & SANDY_DRIVE1_MASK)
		wd17xx_set_drive(m_fdc,1);

	wd17xx_set_side(m_fdc,(data & SANDY_SIDE_MASK) >> SANDY_SIDE_SHIFT);
	if (data & SANDY_SIDE_MASK)
	{
		logerror("Accessing side 1\n");
	}

	if (m_printer->is_ready())
	{
		if(data & SANDY_PRINTER_STROBE)
			m_printer->output(m_printer_char);

		if(data & SANDY_PRINTER_INTMASK)
			m_zx8302->extint_w(ASSERT_LINE);
	}
}

READ_LINE_MEMBER(ql_state::disk_io_dden_r)
{
	if(m_disk_type==DISK_TYPE_SANDY)
		return ((m_disk_io_byte & SANDY_DDEN_MASK) >> SANDY_DDEN_SHIFT);
	else
		return 0;
}

WRITE_LINE_MEMBER(ql_state::disk_io_intrq_w)
{
	//logerror("DiskIO:intrq = %d\n",state);
}

WRITE_LINE_MEMBER(ql_state::disk_io_drq_w)
{
	//logerror("DiskIO:drq = %d\n",state);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( ql_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( ql_mem, AS_PROGRAM, 8, ql_state )
	AM_RANGE(0x000000, 0x00bfff) AM_ROM // 48K System ROM
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_WRITENOP                         // 16K Cartridge ROM
	AM_RANGE(0x010000, 0x017fff) AM_UNMAP                                   // Trump card ROM is mapped in here
	AM_RANGE(0x018000, 0x018003) AM_DEVREAD(ZX8302_TAG, zx8302_device, rtc_r)
	AM_RANGE(0x018000, 0x018001) AM_DEVWRITE(ZX8302_TAG, zx8302_device, rtc_w)
	AM_RANGE(0x018002, 0x018002) AM_DEVWRITE(ZX8302_TAG, zx8302_device, control_w)
	AM_RANGE(0x018003, 0x018003) AM_DEVWRITE(ZX8302_TAG, zx8302_device, ipc_command_w)
	AM_RANGE(0x018020, 0x018020) AM_DEVREADWRITE(ZX8302_TAG, zx8302_device, status_r, mdv_control_w)
	AM_RANGE(0x018021, 0x018021) AM_DEVREADWRITE(ZX8302_TAG, zx8302_device, irq_status_r, irq_acknowledge_w)
	AM_RANGE(0x018022, 0x018022) AM_DEVREADWRITE(ZX8302_TAG, zx8302_device, mdv_track_r, data_w)
	AM_RANGE(0x018023, 0x018023) AM_DEVREAD(ZX8302_TAG, zx8302_device, mdv_track_r) AM_WRITENOP
	AM_RANGE(0x018063, 0x018063) AM_DEVWRITE(ZX8301_TAG, zx8301_device, control_w)
	AM_RANGE(0x01c000, 0x01ffff) AM_UNMAP                                   // 16K Expansion I/O
	AM_RANGE(0x020000, 0x03ffff) AM_DEVREADWRITE(ZX8301_TAG, zx8301_device, data_r, data_w)
	AM_RANGE(0x040000, 0x0fffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( ipc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ipc_io, AS_IO, 8, ql_state )
	AM_RANGE(0x00, 0x7f) AM_WRITE(ipc_w)
	AM_RANGE(0x27, 0x28) AM_READNOP // IPC reads these to set P0 (bus) to Hi-Z mode
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(ipc_port1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(ipc_port2_r, ipc_port2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(ipc_t1_r)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(ipc_bus_r) AM_WRITENOP
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( ql )
//-------------------------------------------------

static INPUT_PORTS_START( ql )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("F4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("F1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("F2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("F3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("F5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC @") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_CHAR('\x1b') PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_NAME("\xc2\xa3 ~") PORT_CHAR(0xa3) PORT_CHAR('~') // ? ~
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TABULATE") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("JOY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )PORT_PLAYER(1) PORT_8WAY PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_PLAYER(1) PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )   PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_PLAYER(2) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY PORT_CODE(KEYCODE_DOWN)


	PORT_START(QL_CONFIG_PORT)
	PORT_DIPNAME( DISK_TYPE_MASK, DISK_TYPE_NONE, "Disk interface select")
	PORT_DIPSETTING(DISK_TYPE_NONE,     DEF_STR( None ))
	PORT_DIPSETTING(DISK_TYPE_TRUMP,    "Miracle Trump card")
	PORT_DIPSETTING(DISK_TYPE_SANDY,    "Sandy Superdisk")

INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_es )
//-------------------------------------------------

static INPUT_PORTS_START( ql_es )
	PORT_INCLUDE(ql)

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("] \xc3\x9c") PORT_CHAR(']') PORT_CHAR(0xfc) PORT_CHAR(0xdc)

	PORT_MODIFY("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('`') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[ \xc3\x87") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('[') PORT_CHAR(0xe7) PORT_CHAR(0xc7)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR(':')

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\x91") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0xf1) PORT_CHAR(0xd1)

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 \xc2\xa1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR(0xa1)

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 \xc2\xbf") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(0xbf)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('/')

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('?')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_de )
//-------------------------------------------------

static INPUT_PORTS_START( ql_de )
	PORT_INCLUDE(ql)

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')

	PORT_MODIFY("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\x84") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0xe4) PORT_CHAR(0xc4)

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\x9c") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0xfc) PORT_CHAR(0xdc)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('#') PORT_CHAR('\'')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\x96") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0xf6) PORT_CHAR(0xd6)

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 \xc2\xa3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(0xa7)

	PORT_MODIFY("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\x9f ?") PORT_CODE(KEYCODE_MINUS) PORT_CHAR(0xdf) PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_it )
//-------------------------------------------------

static INPUT_PORTS_START( ql_it )
	PORT_INCLUDE(ql)

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('(') PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('\'') PORT_CHAR('4')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 \xc3\xa8") PORT_CODE(KEYCODE_7) PORT_CHAR('?') PORT_CHAR('7')

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')

	PORT_MODIFY("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR(':') PORT_CHAR('/')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("* \xc2\xa7") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('*') PORT_CHAR(0xa7)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xb9 %") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0xf9) PORT_CHAR('%')

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xac =") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0xec) PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('"') PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('#') PORT_CHAR('1')

	PORT_MODIFY("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xa7 9") PORT_CODE(KEYCODE_9) PORT_CHAR(0xe7) PORT_CHAR('9')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(')') PORT_CHAR('\\')

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('^') PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xa9 2") PORT_CODE(KEYCODE_2) PORT_CHAR(0xe9) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('_') PORT_CHAR('6')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xa0 0") PORT_CODE(KEYCODE_0) PORT_CHAR(0xe0) PORT_CHAR('0')

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xc3\xb2 !") PORT_CODE(KEYCODE_SLASH) PORT_CHAR(0xf2) PORT_CHAR('!')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('.')
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_fr )
//-------------------------------------------------

static INPUT_PORTS_START( ql_fr )
	PORT_INCLUDE(ql)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_se )
//-------------------------------------------------

static INPUT_PORTS_START( ql_se )
	PORT_INCLUDE(ql)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ql_dk )
//-------------------------------------------------

static INPUT_PORTS_START( ql_dk )
	PORT_INCLUDE(ql)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ZX8301_INTERFACE( ql_zx8301_intf )
//-------------------------------------------------

static ZX8301_INTERFACE( ql_zx8301_intf )
{
	M68008_TAG,
	SCREEN_TAG,
	DEVCB_DEVICE_LINE_MEMBER(ZX8302_TAG, zx8302_device, vsync_w)
};


//-------------------------------------------------
//  ZX8302_INTERFACE( ql_zx8302_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( ql_state::ql_baudx4_w )
{
	m_baudx4 = state;
}

WRITE_LINE_MEMBER( ql_state::ql_comdata_w )
{
	m_comdata = state;
}

WRITE_LINE_MEMBER( ql_state::zx8302_mdselck_w )
{
	m_mdv2->clk_w(state);
	m_mdv1->clk_w(state);
}

WRITE_LINE_MEMBER( ql_state::zx8302_mdrdw_w )
{
	m_mdv1->read_write_w(state);
	m_mdv2->read_write_w(state);
}

WRITE_LINE_MEMBER( ql_state::zx8302_erase_w )
{
	m_mdv1->erase_w(state);
	m_mdv2->erase_w(state);
}

WRITE_LINE_MEMBER( ql_state::zx8302_raw1_w )
{
	m_mdv1->data1_w(state);
	m_mdv2->data1_w(state);
}

READ_LINE_MEMBER( ql_state::zx8302_raw1_r )
{
	return m_mdv1->data1_r() | m_mdv2->data1_r();
}

WRITE_LINE_MEMBER( ql_state::zx8302_raw2_w )
{
	m_mdv1->data2_w(state);
	m_mdv2->data2_w(state);
}

READ_LINE_MEMBER( ql_state::zx8302_raw2_r )
{
	return m_mdv1->data2_r() | m_mdv2->data2_r();
}

static ZX8302_INTERFACE( ql_zx8302_intf )
{
	X2,
	DEVCB_CPU_INPUT_LINE(M68008_TAG, M68K_IRQ_2),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, ql_baudx4_w),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, ql_comdata_w),
	DEVCB_NULL, // TXD1
	DEVCB_NULL, // TXD2
	DEVCB_NULL, // DTR1
	DEVCB_NULL, // CTS2
	DEVCB_NULL, // NETOUT
	DEVCB_NULL, // NETIN
	DEVCB_DRIVER_LINE_MEMBER(ql_state, zx8302_mdselck_w),
	DEVCB_DEVICE_LINE_MEMBER(MDV_1, microdrive_image_device, comms_in_w),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, zx8302_mdrdw_w),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, zx8302_erase_w),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, zx8302_raw1_w),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, zx8302_raw1_r),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, zx8302_raw2_w),
	DEVCB_DRIVER_LINE_MEMBER(ql_state, zx8302_raw2_r)
};


//-------------------------------------------------
//  floppy_interface ql_floppy_interface
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( ql )
	LEGACY_FLOPPY_OPTION(ql, "dsk", "SSDD 40 track disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(ql, "dsk", "DSDD 40 track disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([40])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(ql, "dsk", "DSDD 80 track disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(ql, "dsk", "DSHD 80 track disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([18])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(ql, "dsk", "DSED disk 80 track image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([40])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface ql_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(ql),
	NULL,
	NULL
};

wd17xx_interface ql_wd17xx_interface =
{
	DEVCB_DRIVER_LINE_MEMBER(ql_state,disk_io_dden_r),
	DEVCB_DRIVER_LINE_MEMBER(ql_state,disk_io_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(ql_state,disk_io_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};


//-------------------------------------------------
//  MICRODRIVE_CONFIG( mdv1_config )
//-------------------------------------------------

static MICRODRIVE_CONFIG( mdv1_config )
{
	DEVCB_DEVICE_LINE_MEMBER(MDV_2, microdrive_image_device, comms_in_w),
	NULL,
	NULL,
};


//-------------------------------------------------
//  MICRODRIVE_CONFIG( mdv2_config )
//-------------------------------------------------

static MICRODRIVE_CONFIG( mdv2_config )
{
	DEVCB_NULL,
	NULL,
	NULL
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( ql )
//-------------------------------------------------

//
// The 896K option is dealt with once the machine is up and running as
// the Trump Card ROM is initally mapped in at $C0000-$C8000, as this is
// officially the expansion card rom area. Once the Trump Card has initialised
// it maps the last 256K of ram into this area (asuming that it has 768K
// onboard).
//

void ql_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_keylatch));
	save_item(NAME(m_ipl));
	save_item(NAME(m_comdata));
	save_item(NAME(m_baudx4));
	save_item(NAME(m_printer_char));
	save_item(NAME(m_disk_io_byte));
}

void ql_state::machine_reset()
{
	address_space   &program    = m_maincpu->space(AS_PROGRAM);

	m_disk_type=ioport(QL_CONFIG_PORT)->read() & DISK_TYPE_MASK;
	logerror("disktype=%d\n",m_disk_type);

	m_printer_char=0;
	m_disk_io_byte=0;

	logerror("Configuring RAM\n");
	// configure RAM
	switch (m_ram->size())
	{
		case 128*1024:
			program.unmap_readwrite(0x040000, 0x0fffff);
			break;

		case 192*1024:
			program.unmap_readwrite(0x050000, 0x0fffff);
			break;

		case 256*1024:
			program.unmap_readwrite(0x060000, 0x0fffff);
			break;

		case 384*1024:
			program.unmap_readwrite(0x080000, 0x0fffff);
			break;

		case 640*1024:
			program.unmap_readwrite(0x0c0000, 0x0fffff);
			break;
	}

	switch (m_disk_type)
	{
		case DISK_TYPE_SANDY :
			logerror("Configuring SandySuperDisk\n");
			program.install_rom(0x0c0000, 0x0c3fff, &machine().root_device().memregion(M68008_TAG)->base()[SANDY_ROM_BASE]);
			program.install_read_handler(SANDY_IO_BASE, SANDY_IO_END, 0, 0, read8_delegate(FUNC(ql_state::disk_io_r), this));
			program.install_write_handler(SANDY_IO_BASE, SANDY_IO_END, 0, 0, write8_delegate(FUNC(ql_state::disk_io_w), this));
			m_disk_io_base=SANDY_IO_BASE;
			break;
		case DISK_TYPE_TRUMP :
			logerror("Configuring TrumpCard\n");
			program.install_read_handler(CART_ROM_BASE, CART_ROM_END, 0, 0, read8_delegate(FUNC(ql_state::cart_rom_r), this));
			program.install_read_handler(TRUMP_ROM_BASE, TRUMP_ROM_END, 0, 0, read8_delegate(FUNC(ql_state::trump_card_rom_r), this));
			program.install_read_handler(TRUMP_IO_BASE, TRUMP_IO_END, 0, 0, read8_delegate(FUNC(ql_state::disk_io_r), this));
			program.install_write_handler(TRUMP_IO_BASE, TRUMP_IO_END, 0, 0, write8_delegate(FUNC(ql_state::disk_io_w), this));
			m_disk_io_base=TRUMP_IO_BASE;
			break;
	}
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( ql )
//-------------------------------------------------

static MACHINE_CONFIG_START( ql, ql_state )
	// basic machine hardware
	MCFG_CPU_ADD(M68008_TAG, M68008, X1/2)
	MCFG_CPU_PROGRAM_MAP(ql_mem)

	MCFG_CPU_ADD(I8749_TAG, I8749, X4)
	MCFG_CPU_IO_MAP(ipc_io)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50.08)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_UPDATE_DEVICE(ZX8301_TAG, zx8301_device, screen_update)
	MCFG_SCREEN_SIZE(960, 312)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_ZX8301_ADD(ZX8301_TAG, X1, ql_zx8301_intf)
	MCFG_ZX8302_ADD(ZX8302_TAG, X1, ql_zx8302_intf)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(ql_floppy_interface)
	MCFG_WD1772_ADD(WD1772_TAG,ql_wd17xx_interface)

	MCFG_MICRODRIVE_ADD(MDV_1, mdv1_config)
	MCFG_MICRODRIVE_ADD(MDV_2, mdv2_config)

	// cartridge
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("ql_cart")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cass_list", "ql_cass")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "ql")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("192K,256K,384K,640K,896K")

	// Parallel printer port on Sandy disk interface
	MCFG_PRINTER_ADD(PRINTER_TAG)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( ql_ntsc )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( ql_ntsc, ql )
	// video hardware
	MCFG_SCREEN_MODIFY(SCREEN_TAG)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(960, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
MACHINE_CONFIG_END

/*
//-------------------------------------------------
//  MACHINE_CONFIG( opd )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( opd, ql )
    // internal ram
    MCFG_RAM_MODIFY(RAM_TAG)
    MCFG_RAM_DEFAULT_SIZE("128K")
    MCFG_RAM_EXTRA_OPTIONS("256K")
MACHINE_CONFIG_END
*/

/*
//-------------------------------------------------
//  MACHINE_CONFIG( megaopd )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( megaopd, ql )
    // internal ram
    MCFG_RAM_MODIFY(RAM_TAG)
    MCFG_RAM_DEFAULT_SIZE("256K")
MACHINE_CONFIG_END
*/


//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( ql )
//-------------------------------------------------

ROM_START( ql )
	ROM_REGION( 0x1C000, M68008_TAG, 0 )
	ROM_DEFAULT_BIOS("js")
	ROM_SYSTEM_BIOS( 0, "fb", "v1.00 (FB)" )
	ROMX_LOAD( "fb.ic33", 0x0000, 0x8000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "fb.ic34", 0x8000, 0x4000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "pm", "v1.01 (PM)" )
	ROMX_LOAD( "pm.ic33", 0x0000, 0x8000, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "pm.ic34", 0x8000, 0x4000, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "ah", "v1.02 (AH)" )
	ROMX_LOAD( "ah.ic33.1", 0x0000, 0x4000, BAD_DUMP CRC(a9b4d2df) SHA1(142d6f01a9621aff5e0ad678bd3cbf5cde0db801), ROM_BIOS(3) )
	ROMX_LOAD( "ah.ic33.2", 0x4000, 0x4000, BAD_DUMP CRC(36488e4e) SHA1(ff6f597b30ea03ce480a3d6728fd1d858da34d6a), ROM_BIOS(3) )
	ROMX_LOAD( "ah.ic34",   0x8000, 0x4000, BAD_DUMP CRC(61259d4c) SHA1(bdd10d111e7ba488551a27c8d3b2743917ff1307), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "jm", "v1.03 (JM)" )
	ROMX_LOAD( "ql.jm 0000.ic33", 0x0000, 0x8000, CRC(1f8e840a) SHA1(7929e716dfe88318bbe99e34f47d039957fe3cc0), ROM_BIOS(4) )
	ROMX_LOAD( "ql.jm 8000.ic34", 0x8000, 0x4000, CRC(9168a2e9) SHA1(1e7c47a59fc40bd96dfefc2f4d86827c15f0199e), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "tb", "v1.0? (TB)" )
	ROMX_LOAD( "tb.ic33", 0x0000, 0x8000, BAD_DUMP CRC(1c86d688) SHA1(7df8028e6671afc4ebd5f65bf6c2d6019181f239), ROM_BIOS(5) )
	ROMX_LOAD( "tb.ic34", 0x8000, 0x4000, BAD_DUMP CRC(de7f9669) SHA1(9d6bc0b794541a4cec2203256ae92c7e68d1011d), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "js", "v1.10 (JS)" )
	ROMX_LOAD( "ql.js 0000.ic33", 0x0000, 0x8000, CRC(1bbad3b8) SHA1(59fd4372771a630967ee102760f4652904d7d5fa), ROM_BIOS(6) )
	ROMX_LOAD( "ql.js 8000.ic34", 0x8000, 0x4000, CRC(c970800e) SHA1(b8c9203026a7de6a44bd0942ec9343e8b222cb41), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 6, "tyche", "v2.05 (Tyche)" )
	ROMX_LOAD( "tyche.rom", 0x0000, 0x010000, CRC(8724b495) SHA1(5f33a1bc3f23fd09c31844b65bc3aca7616f180a), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 7, "min189", "Minerva v1.89" )
	ROMX_LOAD( "minerva.rom", 0x0000, 0x00c000, BAD_DUMP CRC(930befe3) SHA1(84a99c4df13b97f90baf1ec8cb6c2e52e3e1bb4d), ROM_BIOS(8) )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_LOAD( "trumpcard-125.rom", TRUMP_ROM_BASE, 0x08000, CRC(938eaa46) SHA1(9b3458cf3a279ed86ba395dc45c8f26939d6c44d))
	ROM_LOAD( "sandysuperdisk.rom", SANDY_ROM_BASE, 0x04000, CRC(b52077da) SHA1(bf531758145ffd083e01c1cf9c45d0e9264a3b53))

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_us )
//-------------------------------------------------

ROM_START( ql_us )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_LOAD( "jsu.ic33", 0x0000, 0x8000, BAD_DUMP CRC(e397f49f) SHA1(c06f92eabaf3e6dd298c51cb7f7535d8ef0ef9c5) )
	ROM_LOAD( "jsu.ic34", 0x8000, 0x4000, BAD_DUMP CRC(3debbacc) SHA1(9fbc3e42ec463fa42f9c535d63780ff53a9313ec) )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_es )
//-------------------------------------------------

ROM_START( ql_es )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_LOAD( "mge.ic33", 0x0000, 0x8000, BAD_DUMP CRC(d5293bde) SHA1(bf5af7e53a472d4e9871f182210787d601db0634) )
	ROM_LOAD( "mge.ic34", 0x8000, 0x4000, BAD_DUMP CRC(a694f8d7) SHA1(bd2868656008de85d7c191598588017ae8aa3339) )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_fr )
//-------------------------------------------------

ROM_START( ql_fr )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_LOAD( "mgf.ic33", 0x0000, 0x8000, NO_DUMP )
	ROM_LOAD( "mgf.ic34", 0x8000, 0x4000, NO_DUMP )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_de )
//-------------------------------------------------

ROM_START( ql_de )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "mg", "v1.10 (MG)" )
	ROMX_LOAD( "mgg.ic33", 0x0000, 0x8000, BAD_DUMP CRC(b4e468fd) SHA1(cd02a3cd79af90d48b65077d0571efc2f12f146e), ROM_BIOS(1) )
	ROMX_LOAD( "mgg.ic34", 0x8000, 0x4000, BAD_DUMP CRC(54959d40) SHA1(ffc0be9649f26019d7be82925c18dc699259877f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "mf", "v1.14 (MF)" )
	ROMX_LOAD( "mf.ic33", 0x0000, 0x8000, BAD_DUMP CRC(49c40563) SHA1(d3bcd0614cf9b52e9d7fc2832e11463e5030476b), ROM_BIOS(2) )
	ROMX_LOAD( "mf.ic34", 0x8000, 0x4000, BAD_DUMP CRC(5974616b) SHA1(c3603768c08535c25f077eed02fb80128aff13d9), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "ultramg", "Ultrasoft" )
	ROMX_LOAD( "ultramg.rom", 0x0000, 0x00c000, BAD_DUMP CRC(ad12463b) SHA1(0561b3bc7ce090f3101b2142ee957c18c250eefa), ROM_BIOS(3) )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_it )
//-------------------------------------------------

ROM_START( ql_it )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_LOAD( "mgi.ic33", 0x0000, 0x8000, BAD_DUMP CRC(d5293bde) SHA1(bf5af7e53a472d4e9871f182210787d601db0634) )
	ROM_LOAD( "mgi.ic34", 0x8000, 0x4000, BAD_DUMP CRC(a2fdfb83) SHA1(162b1052737500f3c13497cdf0f813ba006bdae9) )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_se )
//-------------------------------------------------

ROM_START( ql_se )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_LOAD( "mgs.ic33", 0x0000, 0x8000, NO_DUMP )
	ROM_LOAD( "mgs.ic34", 0x8000, 0x4000, NO_DUMP )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_gr )
//-------------------------------------------------

ROM_START( ql_gr )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_LOAD( "efp.ic33", 0x0000, 0x8000, BAD_DUMP CRC(eb181641) SHA1(43c1e0215cf540cbbda240b1048910ff55681059) )
	ROM_LOAD( "efp.ic34", 0x8000, 0x4000, BAD_DUMP CRC(4c3b34b7) SHA1(f9dc571d2d4f68520b306ecc7516acaeea69ec0d) )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( ql_dk )
//-------------------------------------------------

ROM_START( ql_dk )
	ROM_REGION( 0x18000, M68008_TAG, 0 )
	ROM_LOAD( "mgd.ic33",  0x0000, 0x8000, BAD_DUMP CRC(f57755eb) SHA1(dc57939ffb8741e17967a1d2479c339750ec7ff6) )
	ROM_LOAD( "mgd.ic34",  0x8000, 0x4000, BAD_DUMP CRC(1892465a) SHA1(0ff3046b5276da6639d3fe79b22ae25cc265d540) )
	ROM_CART_LOAD("cart", 0xc000, 0x4000, ROM_MIRROR | ROM_OPTIONAL)

	ROM_REGION( 0x4000, "extra", 0 )
	ROM_LOAD( "extra.rom", 0x0000, 0x4000, NO_DUMP ) // located at 0x1c000 in M68008 memory map

	ROM_REGION( 0x800, I8749_TAG, 0 )
	ROM_LOAD( "ipc8049.ic24", 0x000, 0x800, CRC(6a0d1f20) SHA1(fcb1c97ee7c66e5b6d8fbb57c06fd2f6509f2e1b) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "hal16l8.ic38", 0x0000, 0x0400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( tonto )
//-------------------------------------------------

ROM_START( tonto )
	ROM_REGION( 0x400000, M68008_TAG, 0 )
	ROM_LOAD( "bios-1.rom", 0x000000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-2.rom", 0x008000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-3.rom", 0x010000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-4.rom", 0x018000, 0x008000, NO_DUMP )

	ROM_REGION( 0x10000, I8051_TAG, 0 )
	ROM_LOAD( "8051-1.rom", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x10000, "tms5220", 0 )
	ROM_LOAD( "tms5220.rom", 0x000000, 0x004000, NO_DUMP )

	ROM_REGION( 0x400000, "rompack", 0 )
	ROM_LOAD( "rompack-1.rom", 0x000000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-2.rom", 0x008000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-3.rom", 0x010000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-4.rom", 0x018000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-5.rom", 0x020000, 0x008000, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( megaopd )
//-------------------------------------------------

ROM_START( megaopd )
	ROM_REGION( 0x400000, M68008_TAG, 0 )
	ROM_LOAD( "bios-1.rom", 0x000000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-2.rom", 0x008000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-3.rom", 0x010000, 0x008000, NO_DUMP )
	ROM_LOAD( "bios-4.rom", 0x018000, 0x008000, NO_DUMP )

	ROM_REGION( 0x10000, I8051_TAG, 0 )
	ROM_LOAD( "8051-1.rom", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x10000, "tms5220", 0 )
	ROM_LOAD( "tms5220.rom", 0x000000, 0x004000, NO_DUMP )

	ROM_REGION( 0x400000, "rompack", 0 )
	ROM_LOAD( "rompack-1.rom", 0x000000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-2.rom", 0x008000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-3.rom", 0x010000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-4.rom", 0x018000, 0x008000, NO_DUMP )
	ROM_LOAD( "rompack-5.rom", 0x020000, 0x008000, NO_DUMP )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   INIT    COMPANY                     FULLNAME        FLAGS
COMP( 1984, ql,     0,      0,      ql,         ql, driver_device,     0,      "Sinclair Research Ltd",    "QL (UK)",      GAME_SUPPORTS_SAVE )
COMP( 1985, ql_us,  ql,     0,      ql_ntsc,    ql, driver_device,     0,      "Sinclair Research Ltd",    "QL (USA)",     GAME_SUPPORTS_SAVE )
COMP( 1985, ql_es,  ql,     0,      ql,         ql_es, driver_device,  0,      "Sinclair Research Ltd",    "QL (Spain)",   GAME_SUPPORTS_SAVE )
COMP( 1985, ql_fr,  ql,     0,      ql,         ql_fr, driver_device,  0,      "Sinclair Research Ltd",    "QL (France)",  GAME_NOT_WORKING )
COMP( 1985, ql_de,  ql,     0,      ql,         ql_de, driver_device,  0,      "Sinclair Research Ltd",    "QL (Germany)", GAME_SUPPORTS_SAVE )
COMP( 1985, ql_it,  ql,     0,      ql,         ql_it, driver_device,  0,      "Sinclair Research Ltd",    "QL (Italy)",   GAME_SUPPORTS_SAVE )
COMP( 1985, ql_se,  ql,     0,      ql,         ql_se, driver_device,  0,      "Sinclair Research Ltd",    "QL (Sweden)",  GAME_NOT_WORKING )
COMP( 1985, ql_dk,  ql,     0,      ql,         ql_dk, driver_device,  0,      "Sinclair Research Ltd",    "QL (Denmark)", GAME_NOT_WORKING )
COMP( 1985, ql_gr,  ql,     0,      ql,         ql, driver_device,     0,      "Sinclair Research Ltd",    "QL (Greece)",  GAME_SUPPORTS_SAVE )
//COMP( 1984, tonto,  0,        0,      opd,        ql, driver_device,     0,      "British Telecom Business Systems", "Merlin M1800 Tonto", GAME_NOT_WORKING )
//COMP( 1986, megaopd,tonto,    0,      megaopd,    ql, driver_device,     0,      "International Computer Limited", "MegaOPD (USA)", GAME_NOT_WORKING )
