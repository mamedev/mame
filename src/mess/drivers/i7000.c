// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

    Itautec I7000

    driver by Felipe C. da S. Sanches <juca@members.fsf.org>
    with tech info provided by Alexandre Souza (a.k.a. Tabajara).

    The portuguese Wikipedia article available at
    http://pt.wikipedia.org/wiki/Itautec_I-7000
    also provides a technical overview of this machine:

    The I-7000 was the first computer manufactured by Itautec
    (http://www.itautec.com.br/pt-br/produtos). It was originally an 8 bit CP/M
    computer that became an IBM PC-XT clone in later hardware revisions which
    took the "I-7000 PC-XT" name.

    * Released in 1982
    * Operating System: SIM/M / BASIC
    * CPU: National NSC800 D-4 at 4,00 MHz
    * Memory: 64KB to 128KB
    * keyboards: 80 keys (with a reduced numerical keypad and function keys)
    * display:
     - 40 X 25 text
     - 80 X 25 text
     - 160 X 100 (8 colors)
     - 640 X 200 (monochrome, with an expansion board)
     - 320 X 200 (16 colors, with an expansion board)
    * Expansion slots:
     - 1 frontal cart slot
     - 4 internal expansion slots
    * Ports:
     - 1 composite video output for a color monitor
     - 2 cassete interfaces
     - 1 RS-232C serial port
     - 1 parallel interface
    * Storage:
     - Cassetes recorder
     - Up to 4 external floppy drives: 8" (FD/DD, 1,1MB) or 5" 1/4
     - Up to 1 external 10 MB hard-drive

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h" //CPU was actually a NSC800 (Z80 compatible)
#include "bus/generic/carts.h"
#include "machine/pit8253.h"
#include "machine/i8279.h"
#include "sound/speaker.h"
#include "video/mc6845.h"

class i7000_state : public driver_device
{
public:
	i7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
          m_maincpu(*this, "maincpu"),
          m_card(*this, "cardslot"),
          m_gfxdecode(*this, "gfxdecode"),
          m_videoram(*this, "videoram")
    { }

	void video_start();
	void machine_start();

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_card;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<UINT8> m_videoram;
	UINT32 screen_update_i7000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT8 *m_char_rom;
	UINT8 m_row;
	tilemap_t *m_bg_tilemap;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	DECLARE_DRIVER_INIT(i7000);
	DECLARE_PALETTE_INIT(i7000);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( i7000_card );

	DECLARE_READ8_MEMBER(i7000_kbd_r);
	DECLARE_WRITE8_MEMBER(i7000_scanlines_w);
};

WRITE8_MEMBER( i7000_state::i7000_scanlines_w )
{
	m_row = data;
}

READ8_MEMBER( i7000_state::i7000_kbd_r )
{
	UINT8 data = 0xff;

    for (int i=0; i<40*25; i++){
    	m_bg_tilemap->mark_tile_dirty(i);
    }

	if (m_row < 8)
	{
		char kbdrow[6];
		sprintf(kbdrow,"X%X",m_row);
		data = ioport(kbdrow)->read();
	}
	return data;
}

/* Input ports */
static INPUT_PORTS_START( i7000 )
	PORT_START("X0")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("!") PORT_CHAR('!')
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)

	PORT_START("X1")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0x9D")
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0x8F")
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^R DC2") //0x12

	PORT_START("X2")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("$ ^") PORT_CHAR('$') PORT_CHAR('^')
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0xA0")
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS)
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X3")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("% +") PORT_CHAR('%') PORT_CHAR('+')
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0x9C")
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CHAR('@')

	PORT_START("X4")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 *") PORT_CODE(KEYCODE_5)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^U NAK") //0x15
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("| <") PORT_CHAR('<')
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", ;") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^E ENQ") //0x05

	PORT_START("X5")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^O SI") //0x0F
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("#") PORT_CHAR('#')
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{") PORT_CHAR('{')

	PORT_START("X6")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^L FF") //0x0C
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^T DC4") //0x14

	PORT_START("X7")
	    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">") PORT_CHAR('>')
	    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACEBAR") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("}") PORT_CHAR('}')

	PORT_START("DSW") /* DP01 */
	    PORT_DIPNAME( 0x80, 0x80, "1")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	    PORT_DIPNAME( 0x40, 0x40, "2")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	    PORT_DIPNAME( 0x20, 0x00, "3")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	    PORT_DIPNAME( 0x10, 0x10, "4")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	    PORT_DIPNAME( 0x08, 0x08, "5")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	    PORT_DIPNAME( 0x04, 0x04, "6")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	    PORT_DIPNAME( 0x02, 0x00, "7")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	    PORT_DIPNAME( 0x01, 0x01, "8")
	    PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	    PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
INPUT_PORTS_END

DRIVER_INIT_MEMBER(i7000_state, i7000)
{
}

void i7000_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (m_card->exists())
	{
		// 0x4000 - 0xbfff   32KB ROM
		program.install_read_handler(0x4000, 0xbfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_card));
	}
}

PALETTE_INIT_MEMBER(i7000_state, i7000)
{
	palette.set_pen_color(0, rgb_t(0x33, 0x33, 0x33));
	palette.set_pen_color(1, rgb_t(0xBB, 0xBB, 0xBB));
}

/*FIXME: we still need to figure out the proper memory map
         for the maincpu and where the cartridge slot maps to. */
static ADDRESS_MAP_START(i7000_mem, AS_PROGRAM, 8, i7000_state)
    AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("boot", 0)
    AM_RANGE(0x2000, 0x2fff) AM_RAM AM_SHARE("videoram")
    AM_RANGE(0x4000, 0xffff) AM_RAM
//  AM_RANGE(0x4000, 0xbfff) AM_ROM AM_REGION("cardslot", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i7000_io , AS_IO, 8, i7000_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK (0xff)
//	AM_RANGE(0x06, 0x06) AM_WRITE(i7000_io_?_w)
//	AM_RANGE(0x08, 0x09) AM_WRITE(i7000_io_?_w) //printer perhaps?
//	AM_RANGE(0x0c, 0x0c) AM_WRITE(i7000_io_?_w) //0x0C and 0x10 may be related to mem page swapping. (self-test "4. PAG")
//	AM_RANGE(0x10, 0x10) AM_WRITE(i7000_io_?_w)
//	AM_RANGE(0x14, 0x15) AM_WRITE(i7000_io_?_w)

	AM_RANGE(0x18, 0x1b) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)

//	AM_RANGE(0x1c, 0x1c) AM_WRITE(i7000_io_printer_data_w) //ASCII data
	AM_RANGE(0x1d, 0x1d) AM_READ_PORT("DSW")
//	AM_RANGE(0x1e, 0x1e) AM_READWRITE(i7000_io_printer_status_r, i7000_io_?_w)
//	AM_RANGE(0x1f, 0x1f) AM_WRITE(i7000_io_printer_strobe_w) //self-test routine writes 0x08 and 0x09 (it seems that bit 0 is the strobe and bit 3 is an enable signal)
//	AM_RANGE(0x20, 0x21) AM_READWRITE(i7000_io_keyboard_r, i7000_io_keyboard_w)

	AM_RANGE( 0x20, 0x20 ) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w)
	AM_RANGE( 0x21, 0x21 ) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)

//	AM_RANGE(0x24, 0x24) AM_READ(i7000_io_?_r)
//	AM_RANGE(0x25, 0x25) AM_WRITE(i7000_io_?_w)

//	AM_RANGE(0x28, 0x2d) AM_READWRITE(i7000_io_joystick_r, i7000_io_joystick_w)

//	AM_RANGE(0x3b, 0x3b) AM_WRITE(i7000_io_?_w)
//	AM_RANGE(0x66, 0x67) AM_WRITE(i7000_io_?_w)
//	AM_RANGE(0xbb, 0xbb) AM_WRITE(i7000_io_?_w) //may be related to page-swapping...
ADDRESS_MAP_END

DEVICE_IMAGE_LOAD_MEMBER( i7000_state, i7000_card )
{
	UINT32 size = m_card->common_get_size("rom");

	m_card->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_BIG);
	m_card->common_load_rom(m_card->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

static const gfx_layout i7000_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                 /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( i7000 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, i7000_charlayout, 0, 8 )
GFXDECODE_END

/****************************
* Video/Character functions *
****************************/

TILE_GET_INFO_MEMBER(i7000_state::get_bg_tile_info)
{
	SET_TILE_INFO_MEMBER(0, /*code:*/ m_videoram[tile_index], /*color:*/ 0, 0);
}

void i7000_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(i7000_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 40, 25);
}

UINT32 i7000_state::screen_update_i7000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/* ROCKWELL 6545 - Transparent Memory Addressing */
MC6845_ON_UPDATE_ADDR_CHANGED(i7000_state::crtc_addr)
{
	/* What is this mandatory function meant to do ? */
}


static MACHINE_CONFIG_START( i7000, i7000_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NSC800, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(i7000_mem)
	MCFG_CPU_IO_MAP(i7000_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 200) /* 40x25 8x8 chars */
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)

	MCFG_SCREEN_UPDATE_DRIVER(i7000_state, screen_update_i7000)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i7000)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(i7000_state, i7000)

	MCFG_MC6845_ADD("crtc", R6545_1, "screen", XTAL_20MHz) /* (?) */
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_ADDR_CHANGED_CB(i7000_state, crtc_addr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

    /* Programmable timer */
	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
//	MCFG_PIT8253_CLK0(XTAL_4MHz / 2) /* TODO: verify on PCB */
//	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(i7000_state,i7000_pit_out0))
//	MCFG_PIT8253_CLK1(XTAL_4MHz / 2) /* TODO: verify on PCB */
//	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(i7000_state,i7000_pit_out1))
	MCFG_PIT8253_CLK2(XTAL_4MHz / 2) /* TODO: verify on PCB */
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("speaker", speaker_sound_device, level_w))

	/* Keyboard interface */
	MCFG_DEVICE_ADD("i8279", I8279, 4000000) /* guessed value. TODO: verify on PCB */
	MCFG_I8279_OUT_SL_CB(WRITE8(i7000_state, i7000_scanlines_w))          // scan SL lines
	MCFG_I8279_IN_RL_CB(READ8(i7000_state, i7000_kbd_r))                  // kbd RL lines
	MCFG_I8279_IN_SHIFT_CB(VCC) // TODO: Shift key
	MCFG_I8279_IN_CTRL_CB(VCC) // TODO: Ctrl key

	/* Cartridge slot */
	MCFG_GENERIC_CARTSLOT_ADD("cardslot", generic_romram_plain_slot, "i7000_card")
	MCFG_GENERIC_EXTENSIONS("rom")
	MCFG_GENERIC_LOAD(i7000_state, i7000_card)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("card_list", "i7000_card")
MACHINE_CONFIG_END

ROM_START( i7000 )
	ROM_REGION( 0x1000, "boot", 0 )
	ROM_LOAD( "i7000_boot_v1_4r02_15_10_85_d52d.rom",  0x0000, 0x1000, CRC(622412e5) SHA1(bf187a095600fd46a739c35132a85b5f39b2f867) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "i7000_chargen.rom", 0x0000, 0x0800, CRC(7ba75183) SHA1(4af799f4a8bd385e1e4e5ece378df93e1133dc12) )

	ROM_REGION( 0x1000, "drive", 0 )
	ROM_LOAD( "i7000_drive_ci01.rom", 0x0000, 0x1000, CRC(d8d6e5c1) SHA1(93e7db42fbfaa8243973321c7fc8c51ed80780be) )

	ROM_REGION( 0x1000, "telex", 0 )
	ROM_LOAD( "i7000_telex_ci09.rom", 0x0000, 0x1000, CRC(c1c8fcc8) SHA1(cbf5fb600e587b998f190a9e3fb398a51d8a5e87) )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT                COMPANY    FULLNAME    FLAGS */
COMP( 1982, i7000,  0,      0,       i7000,     i7000,   i7000_state, i7000, "Itautec", "I-7000",   GAME_NOT_WORKING)
