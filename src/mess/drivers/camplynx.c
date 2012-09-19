/***************************************************************************

      Camputers Lynx

      05/2009 Skeleton driver.

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



    48k and 96k are basically the same machine. 128k is different.

    The work so far is without the benefit of manuals, schematic etc [Robbbert]

    48k, 96k:
    Cassette operation is strange indeed. Save takes the dac output and directs
    it to the tape. Load takes over line 0 of the keyboard and uses bits 0 and 5.

    To Do:
    - find Break key
    - banking (it is incomplete atm)
    - devices (cassette, disk, printer, joysticks)
    - find out the mc6845 clock frequency

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/dac.h"

class camplynx_state : public driver_device
{
public:
	camplynx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	//m_cass(*this, CASSETTE_TAG),
	//m_wave(*this, WAVE_TAG),
	//m_printer(*this, "centronics"),
	m_crtc(*this, "crtc")
	//m_fdc(*this, "fdc")
	{ }

	required_device<cpu_device> m_maincpu;
	//required_device<cassette_image_device> m_cass;
	//required_device<device_t> m_wave;
	//required_device<device_t> m_printer;
	required_device<mc6845_device> m_crtc;
	//optional_device<device_t> m_fdc;
	DECLARE_WRITE8_MEMBER(lynx48k_bank_w);
	DECLARE_WRITE8_MEMBER(lynx128k_bank_w);
	DECLARE_WRITE8_MEMBER(lynx128k_irq);
	DECLARE_DRIVER_INIT(lynx48k);
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_RESET(lynx128k);
};

/* These bankswitch handlers are very incomplete, just enough to get the
    computer working. Also, as it happens 6 times for every scanline
    of every character, it causes a huge slowdown. */

WRITE8_MEMBER( camplynx_state::lynx48k_bank_w )
{
	if (!data)
		membank("bank1")->set_entry(0);
	else
	if (data & 2)
		membank("bank1")->set_entry(1);
	else
	if (data & 4)
		membank("bank1")->set_entry(2);
	else
		logerror("%04X: Cannot understand bankswitch command %X\n",m_maincpu->pc(), data);
}

WRITE8_MEMBER( camplynx_state::lynx128k_bank_w )
{
	/* get address space */
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	UINT8 *base = mem.machine().root_device().memregion("maincpu")->base();

	/* Set read banks */
	UINT8 bank = data & 0x0f;

	if (!bank)
	{
		membank("bank1")->set_base(base + 0x00000);
		membank("bank2")->set_base(base + 0x02000);
		membank("bank3")->set_base(base + 0x04000);
		membank("bank4")->set_base(base + 0x16000);
		membank("bank5")->set_base(base + 0x18000);
		membank("bank6")->set_base(base + 0x1a000);
		membank("bank7")->set_base(base + 0x0c000);
		membank("bank8")->set_base(base + 0x0e000);
	}
	else
	if (bank == 0x0e)
	{
		membank("bank1")->set_base(base + 0x20000);
		membank("bank2")->set_base(base + 0x22000);
		membank("bank3")->set_base(base + 0x24000);
		membank("bank4")->set_base(base + 0x26000);
		membank("bank5")->set_base(base + 0x28000);
		membank("bank6")->set_base(base + 0x2a000);
		membank("bank7")->set_base(base + 0x2c000);
		membank("bank8")->set_base(base + 0x2e000);
	}
	else
		logerror("%04X: Cannot understand bankswitch command %X\n",m_maincpu->pc(), data);

	/* Set write banks */
	bank = data & 0xd0;

	if (!bank)
	{
		membank("bank11")->set_base(base + 0x10000);
		membank("bank12")->set_base(base + 0x12000);
		membank("bank13")->set_base(base + 0x14000);
		membank("bank14")->set_base(base + 0x16000);
		membank("bank15")->set_base(base + 0x18000);
		membank("bank16")->set_base(base + 0x1a000);
		membank("bank17")->set_base(base + 0x1c000);
		membank("bank18")->set_base(base + 0x1e000);
	}
	else
	if (bank == 0xc0)
	{
		membank("bank11")->set_base(base + 0x20000);
		membank("bank12")->set_base(base + 0x22000);
		membank("bank13")->set_base(base + 0x24000);
		membank("bank14")->set_base(base + 0x26000);
		membank("bank15")->set_base(base + 0x28000);
		membank("bank16")->set_base(base + 0x2a000);
		membank("bank17")->set_base(base + 0x2c000);
		membank("bank18")->set_base(base + 0x2e000);
	}
	else
		logerror("%04X: Cannot understand bankswitch command %X\n",m_maincpu->pc(), data);
}

static ADDRESS_MAP_START( lynx48k_mem, AS_PROGRAM, 8, camplynx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x5fff) AM_ROM
	AM_RANGE(0x6000,0x7fff) AM_RAM
	AM_RANGE(0x8000,0xffff) AM_RAMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( lynx128k_mem, AS_PROGRAM, 8, camplynx_state )
ADDRESS_MAP_END

static ADDRESS_MAP_START( lynx48k_io , AS_IO, 8, camplynx_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x007f,0x007f) AM_MIRROR(0xff00) AM_WRITE(lynx48k_bank_w)
	AM_RANGE(0x0080,0x0080) AM_MIRROR(0xff00) AM_WRITENOP		/* to be emulated */
	AM_RANGE(0x0080,0x0080) AM_READ_PORT("LINE0")
	AM_RANGE(0x0180,0x0180) AM_READ_PORT("LINE1")
	AM_RANGE(0x0280,0x0280) AM_READ_PORT("LINE2")
	AM_RANGE(0x0380,0x0380) AM_READ_PORT("LINE3")
	AM_RANGE(0x0480,0x0480) AM_READ_PORT("LINE4")
	AM_RANGE(0x0580,0x0580) AM_READ_PORT("LINE5")
	AM_RANGE(0x0680,0x0680) AM_READ_PORT("LINE6")
	AM_RANGE(0x0780,0x0780) AM_READ_PORT("LINE7")
	AM_RANGE(0x0880,0x0880) AM_READ_PORT("LINE8")
	AM_RANGE(0x0980,0x0980) AM_READ_PORT("LINE9")
	AM_RANGE(0x0084,0x0084) AM_MIRROR(0xff00) AM_DEVWRITE("dac", dac_device, write_unsigned8)	/* 6-bit dac */
	AM_RANGE(0x0086,0x0086) AM_MIRROR(0xff00) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x0087,0x0087) AM_MIRROR(0xff00) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lynx128k_io , AS_IO, 8, camplynx_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x0050,0x0053) AM_MIRROR(0xff80) AM_READ(wd179x_r) // uses a 1793
//  AM_RANGE(0x0054,0x0057) AM_MIRROR(0xff80) AM_WRITE(wd179x_w)
//  AM_RANGE(0x0058,0x0058) AM_MIRROR(0xff80) AM_WRITE(lynx128k_disk_w)
//  AM_RANGE(0x007a,0x007b) AM_MIRROR(0xff80) AM_READ(lynx128k_joysticks_r)
//  AM_RANGE(0x007c,0x007c) AM_MIRROR(0xff80) AM_READ(lynx128k_printer_r)
//  AM_RANGE(0x007d,0x007d) AM_MIRROR(0xff80) AM_WRITE(lynx128k_printer_init_w) // this is rw
//  AM_RANGE(0x007e,0x007e) AM_MIRROR(0xff80) AM_WRITE(lynx128k_printer_w)
	AM_RANGE(0x0080,0x0080) AM_MIRROR(0xff00) AM_WRITENOP		/* to be emulated */
	AM_RANGE(0x0080,0x0080) AM_READ_PORT("LINE0")
	AM_RANGE(0x0180,0x0180) AM_READ_PORT("LINE1")
	AM_RANGE(0x0280,0x0280) AM_READ_PORT("LINE2")
	AM_RANGE(0x0380,0x0380) AM_READ_PORT("LINE3")
	AM_RANGE(0x0480,0x0480) AM_READ_PORT("LINE4")
	AM_RANGE(0x0580,0x0580) AM_READ_PORT("LINE5")
	AM_RANGE(0x0680,0x0680) AM_READ_PORT("LINE6")
	AM_RANGE(0x0780,0x0780) AM_READ_PORT("LINE7")
	AM_RANGE(0x0880,0x0880) AM_READ_PORT("LINE8")
	AM_RANGE(0x0980,0x0980) AM_READ_PORT("LINE9")
	AM_RANGE(0x0082,0x0082) AM_MIRROR(0xff00) AM_WRITE(lynx128k_bank_w)	// read=serial buffer
	AM_RANGE(0x0084,0x0084) AM_MIRROR(0xff00) AM_DEVWRITE("dac", dac_device, write_unsigned8)	/* 6-bit dac. Read="single-step", causes a NMI */
	AM_RANGE(0x0086,0x0086) AM_MIRROR(0xff00) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x0087,0x0087) AM_MIRROR(0xff00) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( lynx48k )
	PORT_START("LINE0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x0e, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_START("LINE1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C CONT") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D DEL") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X WEND") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E DATA") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_START("LINE2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A AUTO") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S STOP") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z RESTORE") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W WHILE") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q REM") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(34)
	PORT_START("LINE3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F DEFPROC") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G GOTO") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V VERIFY") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T TRACE") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R REPEAT") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_START("LINE4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B BEEP") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N NEXT") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H GOSUB") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y RUN") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_START("LINE5")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J LABEL") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M RETURN") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U UNTIL") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 \'") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_START("LINE6")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K MON") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O ENDPROC") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I INPUT") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_START("LINE7")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L LIST") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P PROC") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 _") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_START("LINE8")
	PORT_BIT(0xd0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ \\") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@') PORT_CHAR('\\')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_START("LINE9")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("End") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
INPUT_PORTS_END


MACHINE_RESET_MEMBER(camplynx_state,lynx128k)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	mem.install_read_bank (0x0000, 0x1fff, "bank1");
	mem.install_read_bank (0x2000, 0x3fff, "bank2");
	mem.install_read_bank (0x4000, 0x5fff, "bank3");
	mem.install_read_bank (0x6000, 0x7fff, "bank4");
	mem.install_read_bank (0x8000, 0x9fff, "bank5");
	mem.install_read_bank (0xa000, 0xbfff, "bank6");
	mem.install_read_bank (0xc000, 0xdfff, "bank7");
	mem.install_read_bank (0xe000, 0xffff, "bank8");
	mem.install_write_bank (0x0000, 0x1fff, "bank11");
	mem.install_write_bank (0x2000, 0x3fff, "bank12");
	mem.install_write_bank (0x4000, 0x5fff, "bank13");
	mem.install_write_bank (0x6000, 0x7fff, "bank14");
	mem.install_write_bank (0x8000, 0x9fff, "bank15");
	mem.install_write_bank (0xa000, 0xbfff, "bank16");
	mem.install_write_bank (0xc000, 0xdfff, "bank17");
	mem.install_write_bank (0xe000, 0xffff, "bank18");

	lynx128k_bank_w(mem, 0, 0);
}

WRITE8_MEMBER( camplynx_state::lynx128k_irq )
{
	machine().device("maincpu")->execute().set_input_line(0, data);
}


static const UINT8 lynx48k_palette[8*3] =
{
	0x00, 0x00, 0x00,	/*  0 Black     */
	0x00, 0x00, 0xff,	/*  1 Blue      */
	0xff, 0x00, 0x00,	/*  2 Red       */
	0xff, 0x00, 0xff,	/*  3 Magenta       */
	0x00, 0xff, 0x00,	/*  4 Green     */
	0x00, 0xff, 0xff,	/*  5 Cyan      */
	0xff, 0xff, 0x00,	/*  6 Yellow        */
	0xff, 0xff, 0xff,	/*  7 White     */
};

void camplynx_state::palette_init()
{
	UINT8 r, b, g, i=0, color_count = 8;

	while (color_count--)
	{
		r = lynx48k_palette[i++]; g = lynx48k_palette[i++]; b = lynx48k_palette[i++];
		palette_set_color(machine(), 7-color_count, MAKE_RGB(r, g, b));
	}
}

static MC6845_UPDATE_ROW( lynx48k_update_row )
{
	UINT8 *RAM = device->machine().root_device().memregion("maincpu")->base();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 r,g,b;
	UINT32 x, *p = &bitmap.pix32(y);

	for (x = (y << 5); x < x_count + (y << 5); x++)
	{
		r = RAM[0x14000+x];
		g = RAM[0x1c000+x];
		b = RAM[0x10000+x];

		*p++ = palette[(BIT(r, 7) << 1) | (BIT(g, 7) << 2) | (BIT(b, 7))];
		*p++ = palette[(BIT(r, 6) << 1) | (BIT(g, 6) << 2) | (BIT(b, 6))];
		*p++ = palette[(BIT(r, 5) << 1) | (BIT(g, 5) << 2) | (BIT(b, 5))];
		*p++ = palette[(BIT(r, 4) << 1) | (BIT(g, 4) << 2) | (BIT(b, 4))];
		*p++ = palette[(BIT(r, 3) << 1) | (BIT(g, 3) << 2) | (BIT(b, 3))];
		*p++ = palette[(BIT(r, 2) << 1) | (BIT(g, 2) << 2) | (BIT(b, 2))];
		*p++ = palette[(BIT(r, 1) << 1) | (BIT(g, 1) << 2) | (BIT(b, 1))];
		*p++ = palette[(BIT(r, 0) << 1) | (BIT(g, 0) << 2) | (BIT(b, 0))];
	}
}

static MC6845_UPDATE_ROW( lynx128k_update_row )
{
	UINT8 *RAM = device->machine().root_device().memregion("maincpu")->base();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 r,g,b;
	UINT32 x, *p = &bitmap.pix32(y);

	for (x = (y << 6); x < x_count + (y << 6); x++)
	{
		r = RAM[0x20100+x];
		g = RAM[0x28100+x];
		b = RAM[0x24100+x];

		*p++ = palette[(BIT(r, 7) << 1) | (BIT(g, 7) << 2) | (BIT(b, 7))];
		*p++ = palette[(BIT(r, 6) << 1) | (BIT(g, 6) << 2) | (BIT(b, 6))];
		*p++ = palette[(BIT(r, 5) << 1) | (BIT(g, 5) << 2) | (BIT(b, 5))];
		*p++ = palette[(BIT(r, 4) << 1) | (BIT(g, 4) << 2) | (BIT(b, 4))];
		*p++ = palette[(BIT(r, 3) << 1) | (BIT(g, 3) << 2) | (BIT(b, 3))];
		*p++ = palette[(BIT(r, 2) << 1) | (BIT(g, 2) << 2) | (BIT(b, 2))];
		*p++ = palette[(BIT(r, 1) << 1) | (BIT(g, 1) << 2) | (BIT(b, 1))];
		*p++ = palette[(BIT(r, 0) << 1) | (BIT(g, 0) << 2) | (BIT(b, 0))];
	}
}

void camplynx_state::video_start()
{
}

static const mc6845_interface lynx48k_crtc6845_interface = {
	"screen",
	8,
	NULL,
	lynx48k_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static const mc6845_interface lynx128k_crtc6845_interface = {
	"screen",			/* screen name */
	8,				/* dots per character */
	NULL,
	lynx128k_update_row,		/* callback to display one scanline */
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(camplynx_state, lynx128k_irq),	/* callback when cursor pin changes state */
	DEVCB_NULL,
	NULL
};


static MACHINE_CONFIG_START( lynx48k, camplynx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(lynx48k_mem)
	MCFG_CPU_IO_MAP(lynx48k_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 479)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, XTAL_12MHz / 8 /*? dot clock divided by dots per char */, lynx48k_crtc6845_interface)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( lynx128k, camplynx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(lynx128k_mem)
	MCFG_CPU_IO_MAP(lynx128k_io)

	MCFG_MACHINE_RESET_OVERRIDE(camplynx_state,lynx128k)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 479)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, XTAL_12MHz / 8 /*? dot clock divided by dots per char */, lynx128k_crtc6845_interface)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(camplynx_state,lynx48k)
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 3, &RAM[0x8000],  0x8000);
}


/* ROM definition */
ROM_START( lynx48k )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "1", "Set1")
	ROMX_LOAD( "lynx48-1.rom", 0x0000, 0x2000, CRC(56feec44) SHA1(7ded5184561168e159a30fa8e9d3fde5e52aa91a), ROM_BIOS(1) )
	ROMX_LOAD( "lynx48-2.rom", 0x2000, 0x2000, CRC(d894562e) SHA1(c08a78ecb4eb05baa4c52488fce3648cd2688744), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "2", "Set2")
	ROMX_LOAD( "lynx4811.rom", 0x0000, 0x2000, CRC(a933e577) SHA1(c7b30a28d99b38dbe63a1314c78e3e614287143b), ROM_BIOS(2) )
	ROMX_LOAD( "lynx4812.rom", 0x2000, 0x2000, CRC(3d3fdd0e) SHA1(259d124f05367a96f891790f9418cc9c7798e2f8), ROM_BIOS(2) )
ROM_END

ROM_START( lynx96k )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
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
/*    YEAR  NAME       PARENT     COMPAT   MACHINE    INPUT     INIT         COMPANY     FULLNAME     FLAGS */
COMP( 1983, lynx48k,   0,         0,       lynx48k,   lynx48k, camplynx_state,  lynx48k,  "Camputers",  "Lynx 48k",   GAME_NOT_WORKING)
COMP( 1983, lynx96k,   lynx48k,   0,       lynx48k,   lynx48k, camplynx_state,  lynx48k,  "Camputers",  "Lynx 96k",   GAME_NOT_WORKING)
COMP( 1983, lynx128k,  lynx48k,   0,       lynx128k,  lynx48k, driver_device,  0,        "Camputers",  "Lynx 128k",  GAME_NOT_WORKING)
