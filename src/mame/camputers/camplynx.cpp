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

    TODO:
    - printer
    - joysticks
    - UART type COM8017 (48k,96k only) (not used by any programs)
    - There's a few games that are not perfectly perfect, but it runs at least
      as well as any other Lynx emulator.
    - Racer - Sideways scrolling incorrect.
    - Nuclear Invaders - User defined characters fail to be defined.
    - Card Index - Not enough RAM detected.

    Game bugs (reproducible in Jynx):
    - 3D Monster Craze: When attacked, garbage on screen
    - 3D Monster Craze: When you find the key, the game freezes
    - Ynxvaders: Colours of top 2 rows of invaders should be white and yellow
      but they show as magenta and red. After game ends, title screen has wrong
      colours.

    Game Hints:
      Most games have instructions or are quite obvious.
    - Power Blaster: Using debug.exe, change byte 1865 from FE to F2 to fix
      the loading. Then, arrows to turn, Shift to clean out a whole row of dots.
      You can then move into the vacated areas. Even though it says you have
      3 lives, you actually only have 1.
    - Treasure Island: Doesn't auto-run after loading main program, type RUN.
    - Backgammon: This is just the instructions. The game is missing.
    - LogiChess: The page at http://www.nascomhomepage.com/games/logichess.html
      should provide enough clues to enable you to work out how to play.

    Alternate ROMs for Lynx 96k:
    - Scorpion EXTensions
        OR
        XOR       - create new patterns and colours
        SON/SOFF  - scroll the screen
        SCROLL
        MCOPY
        VAR       - print values of variables used
        DIM       - print size of dimensioned arrays used
        LSTR$     - print length of all strings used
        OLD       - bring back NEWed programs
        ZERODIM   - zero all arrays
        ALTGREEN  - easily access the ALTernate GREEN BANK
        GREEN
        CLEAR     - clear all variables
        UMEM      - display amount of memory used
        VERSION   - display version
        FAST      - faster screen printing
        FTEXT     - fast 8*8 text
        FPRINT
        VAL
        BLOCK
        INSTR
        WSWAP
    - Danish EXTensions
        PAINT
        CAT
        FAST
        MULTI
        VARS
        RECOVER
        MSAVE
        ALARM
        TIMER
        WRUL
        RULON/RULOFF

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "sound/dac.h"
#include "video/mc6845.h"
#include "machine/ram.h"
#include "emupal.h"

#include "formats/camplynx_cas.h"
#include "formats/camplynx_dsk.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class camplynx_state : public driver_device
{
public:
	camplynx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_bankr(*this, "bankr%d", 0U)
		, m_cass(*this, "cassette")
		//, m_printer(*this, "centronics")
		, m_crtc(*this, "crtc")
		, m_dac(*this, "dac")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	void lynx_common(machine_config &config);
	void lynx_disk(machine_config &config);
	void lynx128k(machine_config &config);
	void lynx48k(machine_config &config);
	void lynx96k(machine_config &config);
	void init_lynx48k();
	void init_lynx128k();
	DECLARE_INPUT_CHANGED_MEMBER(brk_key);

private:
	void bank1_w(offs_t offset, u8 data);
	void bank6_w(offs_t offset, u8 data);
	void port58_w(u8 data); // drive select etc
	void port7f_w(u8 data); // banking 48k
	u8 port80_r(); // cassin for 48k
	void port80_w(u8 data); // control port 48k
	u8 port82_r(); // cassin for 128k
	void port82_w(u8 data); // banking 128k
	void port84_w(u8 data); // dac port 48k
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	static void camplynx_floppy_formats(format_registration &fr);
	MC6845_UPDATE_ROW(lynx48k_update_row);
	MC6845_UPDATE_ROW(lynx128k_update_row);
	void lynx128k_io(address_map &map) ATTR_COLD;
	void lynx128k_mem(address_map &map) ATTR_COLD;
	void lynx48k_io(address_map &map) ATTR_COLD;
	void lynx48k_mem(address_map &map) ATTR_COLD;
	void lynx96k_io(address_map &map) ATTR_COLD;
	u8 m_port58 = 0U;
	u8 m_port80 = 0U;
	u8 m_bankdata = 0U;
	u8 m_wbyte = 0U;
	u8 *m_p_ram = nullptr;
	bool m_is_128k = 0;
	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_bank_array<8> m_bankr;
	required_device<cassette_image_device> m_cass;
	//required_device<> m_printer;
	required_device<mc6845_device> m_crtc;
	required_device<dac_byte_interface> m_dac;
	optional_device<fd1793_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};

void camplynx_state::port7f_w(u8 data)
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
//printf("%s:%X\n", machine().describe_context().c_str(), data);
	// do writes
	m_wbyte = (data & 0x0f) | ((m_port80 & 0x0c) << 3);
	// do reads
	u8 rbyte = (data & 0x70) | (m_port80 & 0x0c);
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
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(3);
			m_bankr[4]->set_entry(0);
			m_bankr[5]->set_entry(1);
			m_bankr[6]->set_entry(2);
			m_bankr[7]->set_entry(7);
			break;
		case 0x20:
		case 0x24:
		case 0x28:
		case 0x2c:
			m_bankr[0]->set_entry(8);
			m_bankr[1]->set_entry(9);
			m_bankr[2]->set_entry(10);
			m_bankr[3]->set_entry(11);
			m_bankr[4]->set_entry(12);
			m_bankr[5]->set_entry(13);
			m_bankr[6]->set_entry(14);
			m_bankr[7]->set_entry(BIT(m_port58, 4) ? 15 : 7);
			break;
		case 0x30:
		case 0x34:
		case 0x38:
		case 0x3c:
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(11);
			m_bankr[4]->set_entry(12);
			m_bankr[5]->set_entry(13);
			m_bankr[6]->set_entry(14);
			m_bankr[7]->set_entry(BIT(m_port58, 4) ? 15 : 7);
			break;
		case 0x44:
		case 0x64:
			m_bankr[0]->set_entry(24);
			m_bankr[1]->set_entry(25);
			m_bankr[2]->set_entry(26);
			m_bankr[3]->set_entry(27);
			m_bankr[4]->set_entry(28);
			m_bankr[5]->set_entry(29);
			m_bankr[6]->set_entry(30);
			m_bankr[7]->set_entry(31);
			break;
		case 0x40:
		case 0x60:
		case 0x48:
		case 0x68:
		case 0x4c:
		case 0x6c:
			m_bankr[0]->set_entry(16);
			m_bankr[1]->set_entry(17);
			m_bankr[2]->set_entry(18);
			m_bankr[3]->set_entry(19);
			m_bankr[4]->set_entry(20);
			m_bankr[5]->set_entry(21);
			m_bankr[6]->set_entry(22);
			m_bankr[7]->set_entry(23);
			break;
		case 0x54:
		case 0x74:
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(27);
			m_bankr[4]->set_entry(28);
			m_bankr[5]->set_entry(29);
			m_bankr[6]->set_entry(30);
			m_bankr[7]->set_entry(31);
			break;
		case 0x50:
		case 0x70:
		case 0x58:
		case 0x78:
		case 0x5c:
		case 0x7c:
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(19);
			m_bankr[4]->set_entry(20);
			m_bankr[5]->set_entry(21);
			m_bankr[6]->set_entry(22);
			m_bankr[7]->set_entry(23);
			break;
		default:
			printf("Banking code %X not handled\n", m_bankdata);
	}
}

void camplynx_state::port82_w(u8 data)
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
	m_wbyte = bitswap<8>(data, 0, 0, 0, 0, 4, 5, 6, 7) & 0x0f; // rearrange to 1,2,3,4
	// do reads
	u8 rbyte = bitswap<8>(data, 0, 0, 0, 0, 0, 1, 2, 3) & 0x0f; // rearrange to 0,1,2,4
	if (BIT(rbyte, 1))
		rbyte &= 0x07; // remove 4 if 1 selected (AND gate in IC82)
//printf("%s:%X:%X:%X\n", machine().describe_context().c_str(), data, rbyte, m_wbyte);
	switch (rbyte)
	{
		case 0x00:
		case 0x01:
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(3);
			m_bankr[4]->set_entry(4);
			m_bankr[5]->set_entry(5);
			m_bankr[6]->set_entry(6);
			m_bankr[7]->set_entry(7);
			break;
		case 0x02:
		case 0x0a:
			m_bankr[0]->set_entry(8);
			m_bankr[1]->set_entry(9);
			m_bankr[2]->set_entry(10);
			m_bankr[3]->set_entry(11);
			m_bankr[4]->set_entry(12);
			m_bankr[5]->set_entry(13);
			m_bankr[6]->set_entry(14);
			m_bankr[7]->set_entry(15);
			break;
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(11);
			m_bankr[4]->set_entry(12);
			m_bankr[5]->set_entry(13);
			m_bankr[6]->set_entry(14);
			m_bankr[7]->set_entry(BIT(m_port58, 4) ? 15 : 7);
			break;
		case 0x04:
		case 0x06:
		case 0x0c:
		case 0x0e:
			m_bankr[0]->set_entry(16);
			m_bankr[1]->set_entry(17);
			m_bankr[2]->set_entry(18);
			m_bankr[3]->set_entry(19);
			m_bankr[4]->set_entry(20);
			m_bankr[5]->set_entry(21);
			m_bankr[6]->set_entry(22);
			m_bankr[7]->set_entry(23);
			break;
		case 0x05:
		case 0x0d:
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(19);
			m_bankr[4]->set_entry(20);
			m_bankr[5]->set_entry(21);
			m_bankr[6]->set_entry(22);
			m_bankr[7]->set_entry(23);
			break;
		case 0x08:
			m_bankr[0]->set_entry(32);
			m_bankr[1]->set_entry(33);
			m_bankr[2]->set_entry(34);
			m_bankr[3]->set_entry(35);
			m_bankr[4]->set_entry(36);
			m_bankr[5]->set_entry(37);
			m_bankr[6]->set_entry(38);
			m_bankr[7]->set_entry(39);
			break;
		case 0x09:
			m_bankr[0]->set_entry(0);
			m_bankr[1]->set_entry(1);
			m_bankr[2]->set_entry(2);
			m_bankr[3]->set_entry(35);
			m_bankr[4]->set_entry(36);
			m_bankr[5]->set_entry(37);
			m_bankr[6]->set_entry(38);
			m_bankr[7]->set_entry(39);
			break;
		default:
			printf("Banking code %X not handled\n", m_bankdata);
	}
}

void camplynx_state::lynx48k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).bankr("bankr0");
	map(0x2000, 0x3fff).bankr("bankr1");
	map(0x4000, 0x5fff).bankr("bankr2");
	map(0x6000, 0x7fff).bankr("bankr3");
	map(0x8000, 0x9fff).bankr("bankr4");
	map(0xa000, 0xbfff).bankr("bankr5");
	map(0xc000, 0xdfff).bankr("bankr6");
	map(0xe000, 0xffff).bankr("bankr7");
	map(0x0000, 0xffff).w(FUNC(camplynx_state::bank6_w));
}

void camplynx_state::lynx128k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).bankr("bankr0");
	map(0x2000, 0x3fff).bankr("bankr1");
	map(0x4000, 0x5fff).bankr("bankr2");
	map(0x6000, 0x7fff).bankr("bankr3");
	map(0x8000, 0x9fff).bankr("bankr4");
	map(0xa000, 0xbfff).bankr("bankr5");
	map(0xc000, 0xdfff).bankr("bankr6");
	map(0xe000, 0xffff).bankr("bankr7");
	map(0x0000, 0xffff).w(FUNC(camplynx_state::bank1_w));
}

void camplynx_state::lynx48k_io(address_map &map)
{
	map.unmap_value_high();
	map(0x007f, 0x007f).mirror(0xff80).w(FUNC(camplynx_state::port7f_w));
	map(0x0080, 0x0080).mirror(0xff00).w(FUNC(camplynx_state::port80_w));
	map(0x0080, 0x0080).mirror(0xf000).r(FUNC(camplynx_state::port80_r));
	map(0x0180, 0x0180).mirror(0xf000).portr("LINE1");
	map(0x0280, 0x0280).mirror(0xf000).portr("LINE2");
	map(0x0380, 0x0380).mirror(0xf000).portr("LINE3");
	map(0x0480, 0x0480).mirror(0xf000).portr("LINE4");
	map(0x0580, 0x0580).mirror(0xf000).portr("LINE5");
	map(0x0680, 0x0680).mirror(0xf000).portr("LINE6");
	map(0x0780, 0x0780).mirror(0xf000).portr("LINE7");
	map(0x0880, 0x0880).mirror(0xf000).portr("LINE8");
	map(0x0980, 0x0980).mirror(0xf000).portr("LINE9");
	map(0x0084, 0x0084).mirror(0xff00).w(FUNC(camplynx_state::port84_w));
	map(0x0086, 0x0086).mirror(0xff00).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0087, 0x0087).mirror(0xff00).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

void camplynx_state::lynx96k_io(address_map &map)
{
	lynx48k_io(map);
	map(0x0050, 0x0053).mirror(0xff80).r("fdc", FUNC(fd1793_device::read));
	map(0x0054, 0x0057).mirror(0xff80).w("fdc", FUNC(fd1793_device::write));
	map(0x0058, 0x0058).mirror(0xff80).w(FUNC(camplynx_state::port58_w));
}

void camplynx_state::lynx128k_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0050, 0x0053).mirror(0xff80).r("fdc", FUNC(fd1793_device::read));
	map(0x0054, 0x0057).mirror(0xff80).w("fdc", FUNC(fd1793_device::write));
	map(0x0058, 0x0058).mirror(0xff80).w(FUNC(camplynx_state::port58_w));
//  map(0x007a, 0x007b).mirror(0xff80).r(FUNC(camplynx_state::lynx128k_joysticks_r));
//  map(0x007c, 0x007c).mirror(0xff80).r(FUNC(camplynx_state::lynx128k_printer_r));
//  map(0x007d, 0x007d).mirror(0xff80).w(FUNC(camplynx_state::lynx128k_printer_init_w)); // this is rw
//  map(0x007e, 0x007e).mirror(0xff80).w(FUNC(camplynx_state::lynx128k_printer_w));
	map(0x0080, 0x0080).mirror(0xff00).w(FUNC(camplynx_state::port80_w));
	map(0x0080, 0x0080).mirror(0xf000).portr("LINE0");
	map(0x0180, 0x0180).mirror(0xf000).portr("LINE1");
	map(0x0280, 0x0280).mirror(0xf000).portr("LINE2");
	map(0x0380, 0x0380).mirror(0xf000).portr("LINE3");
	map(0x0480, 0x0480).mirror(0xf000).portr("LINE4");
	map(0x0580, 0x0580).mirror(0xf000).portr("LINE5");
	map(0x0680, 0x0680).mirror(0xf000).portr("LINE6");
	map(0x0780, 0x0780).mirror(0xf000).portr("LINE7");
	map(0x0880, 0x0880).mirror(0xf000).portr("LINE8");
	map(0x0980, 0x0980).mirror(0xf000).portr("LINE9");
	map(0x0082, 0x0082).mirror(0xff00).rw(FUNC(camplynx_state::port82_r), FUNC(camplynx_state::port82_w)); // read=serial buffer
	map(0x0084, 0x0084).mirror(0xff00).w(FUNC(camplynx_state::port84_w));
	map(0x0086, 0x0086).mirror(0xff00).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0087, 0x0087).mirror(0xff00).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

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

void camplynx_state::bank1_w(offs_t offset, u8 data)
{
	if (BIT(m_wbyte, 0))
		m_p_ram[offset+0x10000] = data;
	if ((m_wbyte & 0x22) == 0x02)
		m_p_ram[offset+0x20000] = data;
	if ((m_wbyte & 0x44) == 0x04)
		m_p_ram[offset+0x30000] = data;
	if (m_is_128k && BIT(m_wbyte, 3))
		m_p_ram[offset+0x40000] = data;
}

void camplynx_state::bank6_w(offs_t offset, u8 data)
{
	if (BIT(m_wbyte, 0))
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

u8 camplynx_state::port80_r()
{
	u8 data = ioport("LINE0")->read();
	// when reading tape, bit 0 becomes cass-in signal
	if (BIT(m_port80, 1))
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
void camplynx_state::port80_w(u8 data)
{
	m_port80 = data;
	m_cass->change_state( BIT(data, (m_is_128k) ? 3 : 1) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	if (!m_is_128k)
		port7f_w(m_bankdata);
}

/* DAC port (6-bit). If writing cassette, output goes to tape as a sine wave, otherwise it goes to speaker.
   There is code below to write as a sine wave or a square wave, both work and can be loaded successfully.
   However the PALE emulator cannot load either of them, although it loads its own output.
   MAME can load PALE's wav files though.
   Currently square wave output is selected. */

void camplynx_state::port84_w(u8 data)
{
	if (BIT(m_port80, (m_is_128k) ? 3 : 1)) // for 128k, bit 2 might be ok too
	{
		// Sine wave output
		//float t = (float)(unsigned)data - 32.0f;
		//m_cass->output(t/31);

		// Square wave output
		m_cass->output(BIT(data, 5) ? -1.0 : +1.0);
	}
	else    // speaker output
		m_dac->write(data);
}

/*
d7 = clock
d2 = cass-in
d1 = serial data in
d0 = serial h/s in */
u8 camplynx_state::port82_r()
{
	u8 data = 0xfb; // guess
	data |= (m_cass->input() > +0.02) ? 4 : 0;
	return data;
}

void camplynx_state::machine_reset()
{
	m_port58 = 0;
	m_port80 = 0;
	if (m_is_128k)
		port82_w( 0 );
	else
		port7f_w( 0 );
	m_maincpu->reset();
}

MC6845_UPDATE_ROW( camplynx_state::lynx48k_update_row )
{
	uint32_t green_bank, *p = &bitmap.pix(y);
	uint16_t const mem = ((ma << 2) + (ra << 5)) & 0x1fff;

	// determine green bank
	if (BIT(m_port80, 4))
		green_bank = 0x38000 + mem; // alt green
	else
		green_bank = 0x3c000 + mem; // normal green

	for (u8 x = 0; x < x_count; x++)
	{
		u8 const r = m_p_ram[0x2c000 + mem + x];
		u8 const b = m_p_ram[0x28000 + mem + x];
		u8 const g = m_p_ram[green_bank + x];

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
	uint32_t green_bank, *p = &bitmap.pix(y);
	uint16_t const mem = ((ma << 2) + (ra << 6)) & 0x3fff;

	// determine green bank
	if (BIT(m_port80, 4))
		green_bank = 0x2c000 + mem; // alt green
	else
		green_bank = 0x28000 + mem; // normal green

	for (u8 x = 0; x < x_count; x++)
	{
		u8 const r = m_p_ram[0x20000 + mem + x];
		u8 const b = m_p_ram[0x24000 + mem + x];
		u8 const g = m_p_ram[green_bank + x];

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

void camplynx_state::port58_w(u8 data)
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
			port82_w(m_bankdata);
		else
			port7f_w(m_bankdata);
	}

	floppy_image_device *floppy = nullptr;
	if ((data & 3) == 0)
		floppy = m_floppy0->get_device();
	else if ((data & 3) == 1)
		floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(BIT(data, 2));

	m_floppy0->get_device()->mon_w(BIT(data, 3));
	m_floppy1->get_device()->mon_w(BIT(data, 3));
}

void camplynx_state::machine_start()
{
	save_item(NAME(m_port58));
	save_item(NAME(m_port80));
	save_item(NAME(m_bankdata));
	save_item(NAME(m_wbyte));
	save_item(NAME(m_is_128k));
}

void camplynx_state::camplynx_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_CAMPLYNX_FORMAT);
}

static void camplynx_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void camplynx_state::lynx_common(machine_config &config)
{
	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_6BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.375); // unknown DAC
}

void camplynx_state::lynx_disk(machine_config &config)
{
	FD1793(config, m_fdc, 24_MHz_XTAL / 24);
	FLOPPY_CONNECTOR(config, m_floppy0, camplynx_floppies, "525qd", camplynx_state::camplynx_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, camplynx_floppies, "525qd", camplynx_state::camplynx_floppy_formats).enable_sound(true);
}

void camplynx_state::lynx48k(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 24_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &camplynx_state::lynx48k_mem);
	m_maincpu->set_addrmap(AS_IO, &camplynx_state::lynx48k_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 480);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	lynx_common(config);

	CASSETTE(config, m_cass);
	m_cass->set_formats(lynx48k_cassette_formats);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	m_cass->add_route(ALL_OUTPUTS, "speaker", 0.05);
	m_cass->set_interface("camplynx_cass");

	/* devices */
	MC6845(config, m_crtc, 12_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(camplynx_state::lynx48k_update_row));
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256K");

	/* software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("camplynx_cass");
}

void camplynx_state::lynx96k(machine_config &config)
{
	lynx48k(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &camplynx_state::lynx96k_io);

	lynx_disk(config);

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("camplynx_flop").set_filter("96K");
}

void camplynx_state::lynx128k(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 24_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &camplynx_state::lynx128k_mem);
	m_maincpu->set_addrmap(AS_IO, &camplynx_state::lynx128k_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 480);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	lynx_common(config);

	CASSETTE(config, m_cass);
	m_cass->set_formats(lynx128k_cassette_formats);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	m_cass->add_route(ALL_OUTPUTS, "speaker", 0.05);
	m_cass->set_interface("camplynx_cass");

	/* devices */
	MC6845(config, m_crtc, 12_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(camplynx_state::lynx128k_update_row));
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("320K");

	lynx_disk(config);

	/* software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("camplynx_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("camplynx_flop").set_filter("128K");
}

void camplynx_state::init_lynx48k()
{
	m_is_128k = false;
	u8 *m = memregion("maincpu")->base();
	m_p_ram = m_ram->pointer();
	for (auto &bank : m_bankr)
		bank->configure_entries(0, 32, m_p_ram, 0x2000);
	for (u8 i = 0; i < 8; i++)
		for (auto &bank : m_bankr)
			bank->configure_entry(i, m+i*0x2000);
}

void camplynx_state::init_lynx128k()
{
	m_is_128k = true;
	u8 *m = memregion("maincpu")->base();
	m_p_ram = m_ram->pointer();
	for (auto &bank : m_bankr)
		bank->configure_entries(0, 40, m_p_ram, 0x2000);
	for (u8 i = 0; i < 8; i++)
		for (auto &bank : m_bankr)
			bank->configure_entry(i, m+i*0x2000);
}


/* ROM definition */
ROM_START( lynx48k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("1")
	ROM_SYSTEM_BIOS(0, "1", "Set1")
	ROMX_LOAD( "lynx48-1.ic46", 0x0000, 0x2000, CRC(56feec44) SHA1(7ded5184561168e159a30fa8e9d3fde5e52aa91a), ROM_BIOS(0) )
	ROMX_LOAD( "lynx48-2.ic45", 0x2000, 0x2000, CRC(d894562e) SHA1(c08a78ecb4eb05baa4c52488fce3648cd2688744), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "2", "Set2")
	ROMX_LOAD( "lynx4811.ic46", 0x0000, 0x2000, CRC(a933e577) SHA1(c7b30a28d99b38dbe63a1314c78e3e614287143b), ROM_BIOS(1) )
	ROMX_LOAD( "lynx4812.ic45", 0x2000, 0x2000, CRC(3d3fdd0e) SHA1(259d124f05367a96f891790f9418cc9c7798e2f8), ROM_BIOS(1) )
ROM_END

ROM_START( lynx96k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lynx9646.ic46",  0x0000, 0x2000, CRC(f86c5514) SHA1(77a4af7557382003d697d08f364839e2dc28f063) )
	ROM_LOAD( "lynx9645.ic45",  0x2000, 0x2000, CRC(f596b9a3) SHA1(3fca46bd68422d34c6cd801dd904507e52bd8846) )
	ROM_DEFAULT_BIOS("orig")
	ROM_SYSTEM_BIOS(0, "orig", "Original")
	ROMX_LOAD( "lynx9644.ic44", 0x4000, 0x1000, CRC(4b96b0de) SHA1(c372a8d26399b9b45e615b674d61ccda76491b8b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "scorp", "Scorpion") /* Scorpion ROM v2.1 03/86 (Reading Lynx User Group) */
	ROMX_LOAD( "skorprom.ic44", 0x4000, 0x2000, CRC(698d3de9) SHA1(c707bdcecef79774c2a8a23d1f3e9ba382cb9304), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "danish", "Danish")
	ROMX_LOAD( "danish96k3.ic44", 0x4000, 0x2000, CRC(795c22ea) SHA1(0a57394cd986c5b338b38d514e894bace7f6e47b), ROM_BIOS(2) )
	ROM_LOAD( "dosrom.rom",    0xe000, 0x2000, CRC(011e106a) SHA1(e77f0ca99790551a7122945f3194516b2390fb69) )
ROM_END

ROM_START( lynx128k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lynx128-1.ic1", 0x0000, 0x2000, CRC(65d292ce) SHA1(36567c2fbd9cf72f758e8cb80c21cb4d82040752) )
	ROM_LOAD( "lynx128-2.ic2", 0x2000, 0x2000, CRC(23288773) SHA1(e12a7ebea3fae5eb375c03e848dbb81070d9d189) )
	ROM_LOAD( "lynx128-3.ic3", 0x4000, 0x2000, CRC(9827b9e9) SHA1(1092367b2af51c72ce9be367179240d692aeb131) )
	ROM_LOAD( "dosrom.rom",    0xe000, 0x2000, CRC(011e106a) SHA1(e77f0ca99790551a7122945f3194516b2390fb69) )
ROM_END

} // anonymous namespace


/* Driver */
/*    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS           INIT           COMPANY      FULLNAME     FLAGS */
COMP( 1983, lynx48k,  0,       0,      lynx48k,  lynx48k, camplynx_state, init_lynx48k,  "Camputers", "Lynx 48k",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, lynx96k,  lynx48k, 0,      lynx96k,  lynx48k, camplynx_state, init_lynx48k,  "Camputers", "Lynx 96k",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, lynx128k, lynx48k, 0,      lynx128k, lynx48k, camplynx_state, init_lynx128k, "Camputers", "Lynx 128k", MACHINE_SUPPORTS_SAVE )
