// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

      Camputers Lynx

      2009-05 Skeleton driver.
      2015-10-23 Working

      The Lynx was an 8-bit British home computer that was first released
      in early 1983 as a 48 kB model.
      The designer of the Lynx was John Shireff and several models were
      available with 48 kB, 96 kB (from Sep 1983) or 128 kB RAM (from Dec
      1983). It was possible reach 192 kB with RAM expansions on-board.

      The machine was based around a Z80A CPU clocked at 4 MHz, and featured
      a Motorola 6845 as video controller. It was possible to run CP/M with
      the optional 5.25" floppy disk-drive on the 96 kB and 128 kB models.
      Approximately 30,000 Lynx units were sold world-wide.

      Camputers ceased trading in June 1984. Anston Technology took over in
      November the same year and a re-launch was planned but never happened.

      In June 1986, Anston sold everything - hardware, design rights and
      thousands of cassettes - to the National Lynx User Group. The group
      planned to produce a Super-Lynx but was too busy supplying spares and
      technical information to owners of existing models, and the project never
      came into being.

      Hardware info:
       - CPU: Zilog Z80A 4 MHz
       - CO-PROCESSOR: Motorola 6845 (CRT controller)
       - RAM: 48 kb, 96 kb or 128 kb depending on models (max. 192 kb)
       - ROM: 16 kb (48K version), 24 KB (96K and 128K versions)
       - TEXT MODES: 40 x 24, 80 x 24
       - GRAPHIC MODES: 256 x 248, 512 x 480
       - SOUND: one voice beeper

      Lynx 128 Memory Map

              | 0000    2000    4000    6000    8000    a000    c000     e000
              | 1fff    3fff    5fff    7fff    9fff    bfff    dfff     ffff
     -------------------------------------------------------------------------
              |                      |                        |       |
      Bank 0  |      BASIC ROM       |    Not Available       |  Ext  |  Ext
              |                      |                        |  ROM1 |  ROM2
     -------------------------------------------------------------------------
              |                      |
      Bank 1  |        STORE         |           Workspace RAM
              |                      |
     -------------------------------------------------------------------------
              |               |               |               |
      Bank 2  |       RED     |     BLUE      |     GREEN     |     Alt
              |               |               |               |    Green
     -------------------------------------------------------------------------
              |
      Bank 3  |              Available for Video Expansion
              |
     -------------------------------------------------------------------------
              |
      Bank 4  |              Available for User RAM Expansion
              |



    48k and 96k are basically the same machine. 128k is different. Most software for
    one won't work properly on the other, due to major hardware differences.

    This computer is weird, because it allows reads and writes from multiple banks
    at the same time. We can write to multiple banks, but we must limit ourselves
    to reading from the lowest bank selected.

    Notes:
    - The screen doesn't scroll. This is normal.
    - Variable names are case-sensitive.
    - Break key is the Escape key.
    - Typing a line number doesn't delete the line, you are in a calculator mode
      (for example, typing 67-5 will print 62)
    - DEL line-number deletes the line.
    - To edit a line, press Ctrl-E, it asks for line number, type it in,
      then arrows to move left-right, backspace to delete, press keys to insert them
    - If you entered a line and got a syntax error, press Ctrl-Q to edit it.
    - Cassette tapes made on 128k are a different speed to 48k tapes. To load a 128k
      tape on a 48k system, enter TAPE 3 before loading. (2,3,4,5 all seem to work).
    - When loading, there's no wildcard; you must specify the name.
    - INT should be activated by the MC6845 CURS pin, but this doesn't happen. Using
      VSYNC instead. Required for CP/M to boot.
    - Info about disks:
      - You must firstly do XROM to initialise the DOS ROM.
      - Then do EXT DIR with a Lynx-formatted disk, or EXT BOOT with a CP/M disk.
      - Then, on a Lynx-formatted disk, do EXT LOAD "name" or EXT MLOAD "name".

    To Do:
    - printer
    - joysticks
    - UART type COM8017 (48k,96k only) (not used by any programs)
    - There's a few games that are not perfectly perfect, but it runs at least
      as well as any other Lynx emulator.

    Game bugs (reproducible in Jynx):
    - 3D Monster Craze: When attacked, garbage on screen
    - 3D Monster Craze: When you find the key, the game freezes
    - YNXVADERS: Colours of top 2 rows of invaders should be white and yellow
      but they show as magenta and red. After game ends, title screen has wrong
      colours.

      Game Hints:
      Most games have instructions or are quite obvious.
      - Power Blaster. Using debug.exe, change byte 1965 from FE to F2 to fix
        the loading. Then, arrows to turn, Shift to clean out a whole row of dots.
        You can then move into the vacated areas. Even though it says you have
        3 lives, you actually only have 1.
      - Backgammon. This is just the instructions. The game is missing.
      - LogiChess. The page at http://www.nascomhomepage.com/games/logichess.html
        should provide enough clues to enable you to work out how to play.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/dac.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "formats/camplynx_cas.h"
#include "machine/wd_fdc.h"
#include "formats/camplynx_dsk.h"

class camplynx_state : public driver_device
{
public:
	camplynx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		//, m_printer(*this, "centronics")
		, m_crtc(*this, "crtc")
		, m_dac(*this, "dac")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	DECLARE_WRITE8_MEMBER(bank1_w);
	DECLARE_WRITE8_MEMBER(bank6_w);
	DECLARE_WRITE8_MEMBER(port58_w); // drive select etc
	DECLARE_WRITE8_MEMBER(port7f_w); // banking 48k
	DECLARE_READ8_MEMBER(port80_r); // cassin for 48k
	DECLARE_WRITE8_MEMBER(port80_w); // control port 48k
	DECLARE_READ8_MEMBER(port82_r); // cassin for 128k
	DECLARE_WRITE8_MEMBER(port82_w); // banking 128k
	DECLARE_WRITE8_MEMBER(port84_w); // dac port 48k
	DECLARE_INPUT_CHANGED_MEMBER(brk_key);
	DECLARE_MACHINE_RESET(lynx48k);
	DECLARE_MACHINE_RESET(lynx128k);
	DECLARE_DRIVER_INIT(lynx48k);
	DECLARE_DRIVER_INIT(lynx128k);
	DECLARE_FLOPPY_FORMATS(camplynx_floppy_formats);
	MC6845_UPDATE_ROW(lynx48k_update_row);
	MC6845_UPDATE_ROW(lynx128k_update_row);
	required_device<palette_device> m_palette;
private:
	UINT8 m_port58;
	UINT8 m_port80;
	UINT8 m_bankdata;
	UINT8 m_wbyte;
	UINT8 *m_p_ram;
	bool m_is_128k;
	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	//required_device<> m_printer;
	required_device<mc6845_device> m_crtc;
	required_device<dac_device> m_dac;
	optional_device<fd1793_t> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};

WRITE8_MEMBER( camplynx_state::port7f_w )
{
/*
d0 = write to bank 1
d1 = write to bank 2
d2 = write to bank 3
d3 = write to bank 4
d4 = read from bank 0 - roms
d5 = read from bank 1 - user ram
d6 = read from banks 2 and 3 - videoram
d7 = read from bank 4 */

	m_bankdata = data;
	data ^= 0x31; // make all lines active high
//printf("%s:%X\n", machine().describe_context(), data);
	// do writes
	m_wbyte = (data & 0x0f) | ((m_port80 & 0x0c) << 3);
	// do reads
	UINT8 rbyte = (data & 0x70) | (m_port80 & 0x0c);
	switch (rbyte)
	{
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
		case 0x10:
		case 0x14:
		case 0x18:
		case 0x1c:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(3);
			membank("bankr5")->set_entry(0);
			membank("bankr6")->set_entry(1);
			membank("bankr7")->set_entry(2);
			membank("bankr8")->set_entry(7);
			break;
		case 0x20:
		case 0x24:
		case 0x28:
		case 0x2c:
			membank("bankr1")->set_entry(8);
			membank("bankr2")->set_entry(9);
			membank("bankr3")->set_entry(10);
			membank("bankr4")->set_entry(11);
			membank("bankr5")->set_entry(12);
			membank("bankr6")->set_entry(13);
			membank("bankr7")->set_entry(14);
			membank("bankr8")->set_entry(BIT(m_port58, 4) ? 15 : 7);
			break;
		case 0x30:
		case 0x34:
		case 0x38:
		case 0x3c:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(11);
			membank("bankr5")->set_entry(12);
			membank("bankr6")->set_entry(13);
			membank("bankr7")->set_entry(14);
			membank("bankr8")->set_entry(BIT(m_port58, 4) ? 15 : 7);
			break;
		case 0x44:
		case 0x64:
			membank("bankr1")->set_entry(24);
			membank("bankr2")->set_entry(25);
			membank("bankr3")->set_entry(26);
			membank("bankr4")->set_entry(27);
			membank("bankr5")->set_entry(28);
			membank("bankr6")->set_entry(29);
			membank("bankr7")->set_entry(30);
			membank("bankr8")->set_entry(31);
			break;
		case 0x40:
		case 0x60:
		case 0x48:
		case 0x68:
		case 0x4c:
		case 0x6c:
			membank("bankr1")->set_entry(16);
			membank("bankr2")->set_entry(17);
			membank("bankr3")->set_entry(18);
			membank("bankr4")->set_entry(19);
			membank("bankr5")->set_entry(20);
			membank("bankr6")->set_entry(21);
			membank("bankr7")->set_entry(22);
			membank("bankr8")->set_entry(23);
			break;
		case 0x54:
		case 0x74:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(27);
			membank("bankr5")->set_entry(28);
			membank("bankr6")->set_entry(29);
			membank("bankr7")->set_entry(30);
			membank("bankr8")->set_entry(31);
			break;
		case 0x50:
		case 0x70:
		case 0x58:
		case 0x78:
		case 0x5c:
		case 0x7c:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(19);
			membank("bankr5")->set_entry(20);
			membank("bankr6")->set_entry(21);
			membank("bankr7")->set_entry(22);
			membank("bankr8")->set_entry(23);
			break;
		default:
			printf("Banking code %X not handled\n", m_bankdata);
	}
}

WRITE8_MEMBER( camplynx_state::port82_w )
{
/* Almost the same as the 48k, except the bit order is reversed.
d7 = write to bank 1
d6 = write to bank 2
d5 = colsel - allow r/w to bank 3??
d4 = write to bank 4
d3 = read from bank 0 - roms
d2 = read from bank 1 - user ram
d1 = read from bank 2 - videoram
d0 = read from bank 4 */
	m_bankdata = data;
	data ^= 0x8c; // make all lines active high
	// do writes
	m_wbyte = BITSWAP8(data, 0, 0, 0, 0, 4, 5, 6, 7) & 0x0f; // rearrange to 1,2,3,4
	// do reads
	UINT8 rbyte = BITSWAP8(data, 0, 0, 0, 0, 0, 1, 2, 3) & 0x0f; // rearrange to 0,1,2,4
	if BIT(rbyte, 1)
		rbyte &= 0x07; // remove 4 if 1 selected (AND gate in IC82)
//printf("%s:%X:%X:%X\n", machine().describe_context(), data, rbyte, m_wbyte);
	switch (rbyte)
	{
		case 0x00:
		case 0x01:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(3);
			membank("bankr5")->set_entry(4);
			membank("bankr6")->set_entry(5);
			membank("bankr7")->set_entry(6);
			membank("bankr8")->set_entry(7);
			break;
		case 0x02:
		case 0x0a:
			membank("bankr1")->set_entry(8);
			membank("bankr2")->set_entry(9);
			membank("bankr3")->set_entry(10);
			membank("bankr4")->set_entry(11);
			membank("bankr5")->set_entry(12);
			membank("bankr6")->set_entry(13);
			membank("bankr7")->set_entry(14);
			membank("bankr8")->set_entry(15);
			break;
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(11);
			membank("bankr5")->set_entry(12);
			membank("bankr6")->set_entry(13);
			membank("bankr7")->set_entry(14);
			membank("bankr8")->set_entry(BIT(m_port58, 4) ? 15 : 7);
			break;
		case 0x04:
		case 0x06:
		case 0x0c:
		case 0x0e:
			membank("bankr1")->set_entry(16);
			membank("bankr2")->set_entry(17);
			membank("bankr3")->set_entry(18);
			membank("bankr4")->set_entry(19);
			membank("bankr5")->set_entry(20);
			membank("bankr6")->set_entry(21);
			membank("bankr7")->set_entry(22);
			membank("bankr8")->set_entry(23);
			break;
		case 0x05:
		case 0x0d:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(19);
			membank("bankr5")->set_entry(20);
			membank("bankr6")->set_entry(21);
			membank("bankr7")->set_entry(22);
			membank("bankr8")->set_entry(23);
			break;
		case 0x08:
			membank("bankr1")->set_entry(32);
			membank("bankr2")->set_entry(33);
			membank("bankr3")->set_entry(34);
			membank("bankr4")->set_entry(35);
			membank("bankr5")->set_entry(36);
			membank("bankr6")->set_entry(37);
			membank("bankr7")->set_entry(38);
			membank("bankr8")->set_entry(39);
			break;
		case 0x09:
			membank("bankr1")->set_entry(0);
			membank("bankr2")->set_entry(1);
			membank("bankr3")->set_entry(2);
			membank("bankr4")->set_entry(35);
			membank("bankr5")->set_entry(36);
			membank("bankr6")->set_entry(37);
			membank("bankr7")->set_entry(38);
			membank("bankr8")->set_entry(39);
			break;
		default:
			printf("Banking code %X not handled\n", m_bankdata);
	}
}

static ADDRESS_MAP_START( lynx48k_mem, AS_PROGRAM, 8, camplynx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x1fff) AM_READ_BANK("bankr1")
	AM_RANGE(0x2000,0x3fff) AM_READ_BANK("bankr2")
	AM_RANGE(0x4000,0x5fff) AM_READ_BANK("bankr3")
	AM_RANGE(0x6000,0x7fff) AM_READ_BANK("bankr4")
	AM_RANGE(0x8000,0x9fff) AM_READ_BANK("bankr5")
	AM_RANGE(0xa000,0xbfff) AM_READ_BANK("bankr6")
	AM_RANGE(0xc000,0xdfff) AM_READ_BANK("bankr7")
	AM_RANGE(0xe000,0xffff) AM_READ_BANK("bankr8")
	AM_RANGE(0x0000,0xffff) AM_WRITE(bank6_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lynx128k_mem, AS_PROGRAM, 8, camplynx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x1fff) AM_READ_BANK("bankr1")
	AM_RANGE(0x2000,0x3fff) AM_READ_BANK("bankr2")
	AM_RANGE(0x4000,0x5fff) AM_READ_BANK("bankr3")
	AM_RANGE(0x6000,0x7fff) AM_READ_BANK("bankr4")
	AM_RANGE(0x8000,0x9fff) AM_READ_BANK("bankr5")
	AM_RANGE(0xa000,0xbfff) AM_READ_BANK("bankr6")
	AM_RANGE(0xc000,0xdfff) AM_READ_BANK("bankr7")
	AM_RANGE(0xe000,0xffff) AM_READ_BANK("bankr8")
	AM_RANGE(0x0000,0xffff) AM_WRITE(bank1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lynx48k_io, AS_IO, 8, camplynx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x007f,0x007f) AM_MIRROR(0xff80) AM_WRITE(port7f_w)
	AM_RANGE(0x0080,0x0080) AM_MIRROR(0xff00) AM_WRITE(port80_w)
	AM_RANGE(0x0080,0x0080) AM_MIRROR(0xf000) AM_READ(port80_r)
	AM_RANGE(0x0180,0x0180) AM_MIRROR(0xf000) AM_READ_PORT("LINE1")
	AM_RANGE(0x0280,0x0280) AM_MIRROR(0xf000) AM_READ_PORT("LINE2")
	AM_RANGE(0x0380,0x0380) AM_MIRROR(0xf000) AM_READ_PORT("LINE3")
	AM_RANGE(0x0480,0x0480) AM_MIRROR(0xf000) AM_READ_PORT("LINE4")
	AM_RANGE(0x0580,0x0580) AM_MIRROR(0xf000) AM_READ_PORT("LINE5")
	AM_RANGE(0x0680,0x0680) AM_MIRROR(0xf000) AM_READ_PORT("LINE6")
	AM_RANGE(0x0780,0x0780) AM_MIRROR(0xf000) AM_READ_PORT("LINE7")
	AM_RANGE(0x0880,0x0880) AM_MIRROR(0xf000) AM_READ_PORT("LINE8")
	AM_RANGE(0x0980,0x0980) AM_MIRROR(0xf000) AM_READ_PORT("LINE9")
	AM_RANGE(0x0084,0x0084) AM_MIRROR(0xff00) AM_WRITE(port84_w)
	AM_RANGE(0x0086,0x0086) AM_MIRROR(0xff00) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x0087,0x0087) AM_MIRROR(0xff00) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lynx96k_io, AS_IO, 8, camplynx_state )
	AM_IMPORT_FROM(lynx48k_io)
	AM_RANGE(0x0050,0x0053) AM_MIRROR(0xff80) AM_DEVREAD("fdc", fd1793_t, read)
	AM_RANGE(0x0054,0x0057) AM_MIRROR(0xff80) AM_DEVWRITE("fdc", fd1793_t, write)
	AM_RANGE(0x0058,0x0058) AM_MIRROR(0xff80) AM_WRITE(port58_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lynx128k_io, AS_IO, 8, camplynx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0050,0x0053) AM_MIRROR(0xff80) AM_DEVREAD("fdc", fd1793_t, read)
	AM_RANGE(0x0054,0x0057) AM_MIRROR(0xff80) AM_DEVWRITE("fdc", fd1793_t, write)
	AM_RANGE(0x0058,0x0058) AM_MIRROR(0xff80) AM_WRITE(port58_w)
//  AM_RANGE(0x007a,0x007b) AM_MIRROR(0xff80) AM_READ(lynx128k_joysticks_r)
//  AM_RANGE(0x007c,0x007c) AM_MIRROR(0xff80) AM_READ(lynx128k_printer_r)
//  AM_RANGE(0x007d,0x007d) AM_MIRROR(0xff80) AM_WRITE(lynx128k_printer_init_w) // this is rw
//  AM_RANGE(0x007e,0x007e) AM_MIRROR(0xff80) AM_WRITE(lynx128k_printer_w)
	AM_RANGE(0x0080,0x0080) AM_MIRROR(0xff00) AM_WRITE(port80_w)
	AM_RANGE(0x0080,0x0080) AM_MIRROR(0xf000) AM_READ_PORT("LINE0")
	AM_RANGE(0x0180,0x0180) AM_MIRROR(0xf000) AM_READ_PORT("LINE1")
	AM_RANGE(0x0280,0x0280) AM_MIRROR(0xf000) AM_READ_PORT("LINE2")
	AM_RANGE(0x0380,0x0380) AM_MIRROR(0xf000) AM_READ_PORT("LINE3")
	AM_RANGE(0x0480,0x0480) AM_MIRROR(0xf000) AM_READ_PORT("LINE4")
	AM_RANGE(0x0580,0x0580) AM_MIRROR(0xf000) AM_READ_PORT("LINE5")
	AM_RANGE(0x0680,0x0680) AM_MIRROR(0xf000) AM_READ_PORT("LINE6")
	AM_RANGE(0x0780,0x0780) AM_MIRROR(0xf000) AM_READ_PORT("LINE7")
	AM_RANGE(0x0880,0x0880) AM_MIRROR(0xf000) AM_READ_PORT("LINE8")
	AM_RANGE(0x0980,0x0980) AM_MIRROR(0xf000) AM_READ_PORT("LINE9")
	AM_RANGE(0x0082,0x0082) AM_MIRROR(0xff00) AM_READWRITE(port82_r,port82_w) // read=serial buffer
	AM_RANGE(0x0084,0x0084) AM_MIRROR(0xff00) AM_WRITE(port84_w)
	AM_RANGE(0x0086,0x0086) AM_MIRROR(0xff00) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x0087,0x0087) AM_MIRROR(0xff00) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( lynx48k )
	PORT_START("LINE0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27) PORT_CHANGED_MEMBER(DEVICE_SELF, camplynx_state, brk_key, 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x0e, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_START("LINE1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C CONT") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D DEL") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X WEND") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E DATA") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_START("LINE2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A AUTO") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S STOP") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z RESTORE") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W WHILE") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q REM") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(34)
	PORT_START("LINE3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F DEFPROC") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G GOTO") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V VERIFY") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T TRACE") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R REPEAT") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_START("LINE4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B BEEP") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N NEXT") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H GOSUB") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y RUN") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_START("LINE5")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J LABEL") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M RETURN") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U UNTIL") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 \'") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_START("LINE6")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K MON") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O ENDPROC") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I INPUT") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_START("LINE7")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L LIST") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P PROC") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 _") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_START("LINE8")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(0x5e)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ \\") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@') PORT_CHAR('\\')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_START("LINE9")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR(0x7f)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( camplynx_state::brk_key )
{
	m_maincpu->set_input_line(0, newval ? CLEAR_LINE : ASSERT_LINE);
}

WRITE8_MEMBER( camplynx_state::bank1_w )
{
	if BIT(m_wbyte, 0)
		m_p_ram[offset+0x10000] = data;
	if ((m_wbyte & 0x22) == 0x02)
		m_p_ram[offset+0x20000] = data;
	if ((m_wbyte & 0x44) == 0x04)
		m_p_ram[offset+0x30000] = data;
	if (m_is_128k && BIT(m_wbyte, 3))
		m_p_ram[offset+0x40000] = data;
}

WRITE8_MEMBER( camplynx_state::bank6_w )
{
	if BIT(m_wbyte, 0)
		m_p_ram[offset+0x10000] = data;

	offset &= 0x5fff;

	if ((m_wbyte & 0x22) == 0x02)
	{
		m_p_ram[offset | 0x20000] = data;
		m_p_ram[offset | 0x22000] = data;
		m_p_ram[offset | 0x28000] = data;
		m_p_ram[offset | 0x2a000] = data;
	}

	if ((m_wbyte & 0x44) == 0x04)
	{
		m_p_ram[offset | 0x30000] = data;
		m_p_ram[offset | 0x32000] = data;
		m_p_ram[offset | 0x38000] = data;
		m_p_ram[offset | 0x3a000] = data;
	}
}

READ8_MEMBER( camplynx_state::port80_r )
{
	UINT8 data = ioport("LINE0")->read();
	// when reading tape, bit 0 becomes cass-in signal
	if BIT(m_port80, 1)
	{
		data &= 0xfe;
		data |= (m_cass->input() > +0.02) ? 0 : 1;
	}
	return data;
}

/* 128k:
d7 = serial data out
d6 = interline control
d5 = cpu access
d4 = alt green or normal green (afaik this only affects the display not the banking)
d3 = cass motor on
d2 = cass enable
d1 = serial h/s out
d0 = speaker */
WRITE8_MEMBER( camplynx_state::port80_w )
{
	m_port80 = data;
	m_cass->change_state( BIT(data, (m_is_128k) ? 3 : 1) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	if (!m_is_128k)
		port7f_w(space, 0, m_bankdata);
}

/* DAC port (6-bit). If writing cassette, output goes to tape as a sine wave, otherwise it goes to speaker.
   There is code below to write as a sine wave or a square wave, both work and can be loaded successfully.
   However the PALE emulator cannot load either of them, although it loads its own output.
   MESS can load PALE's wav files though.
   Currently square wave output is selected. */

WRITE8_MEMBER( camplynx_state::port84_w )
{
	if BIT(m_port80, (m_is_128k) ? 3 : 1) // for 128k, bit 2 might be ok too
	{
		// Sine wave output
		//float t = (float)(unsigned)data - 32.0f;
		//m_cass->output(t/31);

		// Square wave output
		m_cass->output(BIT(data, 5) ? -1.0 : +1.0);
	}
	else    // speaker output
		m_dac->write_unsigned8(space, 0, data);
}

/*
d7 = clock
d2 = cass-in
d1 = serial data in
d0 = serial h/s in */
READ8_MEMBER( camplynx_state::port82_r )
{
	UINT8 data = 0xfb; // guess
	data |= (m_cass->input() > +0.02) ? 4 : 0;
	return data;
}

MACHINE_RESET_MEMBER(camplynx_state, lynx48k)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	m_port58 = 0;
	m_port80 = 0;
	port7f_w( mem, 0, 0 );
	m_maincpu->reset();
}

MACHINE_RESET_MEMBER(camplynx_state, lynx128k)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	m_port58 = 0;
	m_port80 = 0;
	port82_w( mem, 0, 0 );
	m_maincpu->reset();
}

MC6845_UPDATE_ROW( camplynx_state::lynx48k_update_row )
{
	UINT8 r,g,b,x;
	UINT32 green_bank, *p = &bitmap.pix32(y);
	UINT16 mem = ((ma << 2) + (ra << 5)) & 0x1fff;

	// determine green bank
	if BIT(m_port80, 4)
		green_bank = 0x38000+mem; // alt green
	else
		green_bank = 0x3c000+mem; // normal green

	for (x = 0; x < x_count; x++)
	{
		r = m_p_ram[0x2c000+mem+x];
		b = m_p_ram[0x28000+mem+x];
		g = m_p_ram[green_bank+x];

		*p++ = m_palette->pen_color((BIT(b, 7) << 2) | (BIT(g, 7) << 1) | (BIT(r, 7)));
		*p++ = m_palette->pen_color((BIT(b, 6) << 2) | (BIT(g, 6) << 1) | (BIT(r, 6)));
		*p++ = m_palette->pen_color((BIT(b, 5) << 2) | (BIT(g, 5) << 1) | (BIT(r, 5)));
		*p++ = m_palette->pen_color((BIT(b, 4) << 2) | (BIT(g, 4) << 1) | (BIT(r, 4)));
		*p++ = m_palette->pen_color((BIT(b, 3) << 2) | (BIT(g, 3) << 1) | (BIT(r, 3)));
		*p++ = m_palette->pen_color((BIT(b, 2) << 2) | (BIT(g, 2) << 1) | (BIT(r, 2)));
		*p++ = m_palette->pen_color((BIT(b, 1) << 2) | (BIT(g, 1) << 1) | (BIT(r, 1)));
		*p++ = m_palette->pen_color((BIT(b, 0) << 2) | (BIT(g, 0) << 1) | (BIT(r, 0)));
	}
}

MC6845_UPDATE_ROW( camplynx_state::lynx128k_update_row )
{
	UINT8 r,g,b,x;
	UINT32 green_bank, *p = &bitmap.pix32(y);
	UINT16 mem = ((ma << 2) + (ra << 6)) & 0x3fff;
	// determine green bank
	if BIT(m_port80, 4)
		green_bank = 0x2c000+mem; // alt green
	else
		green_bank = 0x28000+mem; // normal green

	for (x = 0; x < x_count; x++)
	{
		r = m_p_ram[0x20000+mem+x];
		b = m_p_ram[0x24000+mem+x];
		g = m_p_ram[green_bank+x];

		*p++ = m_palette->pen_color((BIT(b, 7) << 2) | (BIT(g, 7) << 1) | (BIT(r, 7)));
		*p++ = m_palette->pen_color((BIT(b, 6) << 2) | (BIT(g, 6) << 1) | (BIT(r, 6)));
		*p++ = m_palette->pen_color((BIT(b, 5) << 2) | (BIT(g, 5) << 1) | (BIT(r, 5)));
		*p++ = m_palette->pen_color((BIT(b, 4) << 2) | (BIT(g, 4) << 1) | (BIT(r, 4)));
		*p++ = m_palette->pen_color((BIT(b, 3) << 2) | (BIT(g, 3) << 1) | (BIT(r, 3)));
		*p++ = m_palette->pen_color((BIT(b, 2) << 2) | (BIT(g, 2) << 1) | (BIT(r, 2)));
		*p++ = m_palette->pen_color((BIT(b, 1) << 2) | (BIT(g, 1) << 1) | (BIT(r, 1)));
		*p++ = m_palette->pen_color((BIT(b, 0) << 2) | (BIT(g, 0) << 1) | (BIT(r, 0)));
	}
}

WRITE8_MEMBER( camplynx_state::port58_w )
{
/*
d0,d1 = drive select
d2 = side
d3 = motor
d4 = eprom
d5 = not used
d6 = no precomp
d7 = 125ns or 250ns */

	if (BIT(m_port58, 4) ^ BIT(data, 4))
	{
		m_port58 = data;
		if (m_is_128k)
			port82_w(space, 0, m_bankdata);
		else
			port7f_w(space, 0, m_bankdata);
	}

	floppy_image_device *floppy = nullptr;
	if ((data & 3) == 0) floppy = m_floppy0->get_device();
	else
	if ((data & 3) == 1) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(BIT(data, 2));

	m_floppy0->get_device()->mon_w(BIT(data, 3));
	m_floppy1->get_device()->mon_w(BIT(data, 3));
}

FLOPPY_FORMATS_MEMBER( camplynx_state::camplynx_floppy_formats )
	FLOPPY_CAMPLYNX_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( camplynx_floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_525_QD )
	SLOT_INTERFACE( "drive1", FLOPPY_525_QD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( lynx_common )
	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.5)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.02)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( lynx_disk )
	MCFG_FD1793_ADD("fdc", XTAL_24MHz / 24)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", camplynx_floppies, "drive0", camplynx_state::camplynx_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", camplynx_floppies, "drive1", camplynx_state::camplynx_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( lynx48k, camplynx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_24MHz / 6)
	MCFG_CPU_PROGRAM_MAP(lynx48k_mem)
	MCFG_CPU_IO_MAP(lynx48k_io)

	MCFG_MACHINE_RESET_OVERRIDE(camplynx_state, lynx48k)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 479)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_FRAGMENT_ADD(lynx_common)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(lynx48k_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED)
	//MCFG_CASSETTE_INTERFACE("camplynx_cass")

	/* devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_12MHz / 8 )
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(camplynx_state, lynx48k_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE("maincpu", z80_device, irq_line))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lynx96k, lynx48k )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(lynx96k_io)

	MCFG_FRAGMENT_ADD(lynx_disk)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( lynx128k, camplynx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_24MHz / 4)
	MCFG_CPU_PROGRAM_MAP(lynx128k_mem)
	MCFG_CPU_IO_MAP(lynx128k_io)

	MCFG_MACHINE_RESET_OVERRIDE(camplynx_state, lynx128k)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 479)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_FRAGMENT_ADD(lynx_common)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(lynx128k_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED)
	//MCFG_CASSETTE_INTERFACE("camplynx_cass")

	/* devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_12MHz / 8 )
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(camplynx_state, lynx128k_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE("maincpu", z80_device, irq_line))

	MCFG_FRAGMENT_ADD(lynx_disk)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(camplynx_state, lynx48k)
{
	m_is_128k = false;
	m_p_ram = memregion("maincpu")->base();
	membank("bankr1")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
	membank("bankr2")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
	membank("bankr3")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
	membank("bankr4")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
	membank("bankr5")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
	membank("bankr6")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
	membank("bankr7")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
	membank("bankr8")->configure_entries(0, 32, &m_p_ram[0], 0x2000);
}

DRIVER_INIT_MEMBER(camplynx_state, lynx128k)
{
	m_is_128k = true;
	m_p_ram = memregion("maincpu")->base();
	membank("bankr1")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
	membank("bankr2")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
	membank("bankr3")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
	membank("bankr4")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
	membank("bankr5")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
	membank("bankr6")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
	membank("bankr7")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
	membank("bankr8")->configure_entries(0, 40, &m_p_ram[0], 0x2000);
}


/* ROM definition */
ROM_START( lynx48k )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "1", "Set1")
	ROMX_LOAD( "lynx48-1.rom", 0x0000, 0x2000, CRC(56feec44) SHA1(7ded5184561168e159a30fa8e9d3fde5e52aa91a), ROM_BIOS(1) )
	ROMX_LOAD( "lynx48-2.rom", 0x2000, 0x2000, CRC(d894562e) SHA1(c08a78ecb4eb05baa4c52488fce3648cd2688744), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "2", "Set2")
	ROMX_LOAD( "lynx4811.rom", 0x0000, 0x2000, CRC(a933e577) SHA1(c7b30a28d99b38dbe63a1314c78e3e614287143b), ROM_BIOS(2) )
	ROMX_LOAD( "lynx4812.rom", 0x2000, 0x2000, CRC(3d3fdd0e) SHA1(259d124f05367a96f891790f9418cc9c7798e2f8), ROM_BIOS(2) )
ROM_END

ROM_START( lynx96k )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lynx9646.rom",  0x0000, 0x2000, CRC(f86c5514) SHA1(77a4af7557382003d697d08f364839e2dc28f063) )
	ROM_LOAD( "lynx9645.rom",  0x2000, 0x2000, CRC(f596b9a3) SHA1(3fca46bd68422d34c6cd801dd904507e52bd8846) )
	ROM_LOAD( "lynx9644.rom",  0x4000, 0x1000, CRC(4b96b0de) SHA1(c372a8d26399b9b45e615b674d61ccda76491b8b) )
	ROM_LOAD( "dosrom.rom",    0xe000, 0x2000, CRC(011e106a) SHA1(e77f0ca99790551a7122945f3194516b2390fb69) )
ROM_END

ROM_START( lynx128k )
	ROM_REGION( 0x50000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lynx128-1.rom", 0x0000, 0x2000, CRC(65d292ce) SHA1(36567c2fbd9cf72f758e8cb80c21cb4d82040752) )
	ROM_LOAD( "lynx128-2.rom", 0x2000, 0x2000, CRC(23288773) SHA1(e12a7ebea3fae5eb375c03e848dbb81070d9d189) )
	ROM_LOAD( "lynx128-3.rom", 0x4000, 0x2000, CRC(9827b9e9) SHA1(1092367b2af51c72ce9be367179240d692aeb131) )
	ROM_LOAD( "dosrom.rom",    0xe000, 0x2000, CRC(011e106a) SHA1(e77f0ca99790551a7122945f3194516b2390fb69) )
ROM_END


/* Driver */
/*    YEAR  NAME       PARENT     COMPAT   MACHINE    INPUT    CLASS            INIT         COMPANY     FULLNAME     FLAGS */
COMP( 1983, lynx48k,   0,         0,       lynx48k,   lynx48k, camplynx_state,  lynx48k,  "Camputers",  "Lynx 48k", 0 )
COMP( 1983, lynx96k,   lynx48k,   0,       lynx96k,   lynx48k, camplynx_state,  lynx48k,  "Camputers",  "Lynx 96k", 0 )
COMP( 1983, lynx128k,  lynx48k,   0,       lynx128k,  lynx48k, camplynx_state,  lynx128k, "Camputers",  "Lynx 128k", 0 )
