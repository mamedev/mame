// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

    Bull (Originally R2E) Micral 80-22G

    2015-10-01 Skeleton [Robbbert]

    http://www.ti99.com/exelvision/website/index.php?page=r2e-micral-8022-g

    This expensive, futuristic-looking design featured a motherboard and slots,
    much like an ancient pc. The known chip complement is:
    Z80A, 4MHz; 64KB RAM, 2KB BIOS ROM, 256x4 prom (7611);
    CRT8002, TMS9937 (=CRT5037), 4KB video RAM, 256x4 prom (7611);
    2x 5.25 inch floppy drives, one ST506 5MB hard drive;
    CDP6402 UART. Sound is a beeper.
    The keyboard has a uPD780C (=Z80) and 1KB of ROM.

    The FDC and HDC are unknown.
    No manuals, schematic or circuit description have been found.

*********************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9927.h"
#include "sound/beep.h"


class micral_state : public driver_device
{
public:
	micral_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_beep(*this, "beeper")
	{ }

	DECLARE_DRIVER_INIT(micral);
	DECLARE_MACHINE_RESET(micral);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
};


static ADDRESS_MAP_START( micral_mem, AS_PROGRAM, 8, micral_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xfeff) AM_ROM
	AM_RANGE(0xff00, 0xffff) AM_RAM // FFF0-F seems to be devices
ADDRESS_MAP_END

static ADDRESS_MAP_START( micral_kbd_mem, AS_PROGRAM, 8, micral_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_RAM // byte returned to main cpu after receiving irq
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("X0")
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("X1")
	AM_RANGE(0x8004, 0x8004) AM_READ_PORT("X2")
	AM_RANGE(0x8008, 0x8008) AM_READ_PORT("X3")
	AM_RANGE(0x8010, 0x8010) AM_READ_PORT("X4")
	AM_RANGE(0x8020, 0x8020) AM_READ_PORT("X5")
	AM_RANGE(0x8040, 0x8040) AM_READ_PORT("X6")
	AM_RANGE(0x8080, 0x8080) AM_READ_PORT("X7")
	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("X8")
	AM_RANGE(0x8200, 0x8200) AM_READ_PORT("X9")
	AM_RANGE(0x8400, 0x8400) AM_READ_PORT("X10")
	AM_RANGE(0x8800, 0x8800) AM_READ_PORT("X11")
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("X12")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("X13")
ADDRESS_MAP_END

static ADDRESS_MAP_START( micral_kbd_io, AS_IO, 8, micral_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("X14")
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( micral )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 01
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 03
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) // 2A
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) // 22
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) // 28
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) // 94
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) // 90
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 29

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 3E, 3C
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '^'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) // 5B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // OB
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) // 3F
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // ':','/'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 08
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 06

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 02
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) // 91
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) // 27
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) // '-'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) // '_'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) // 8E
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) // '+'

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '@', '#'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9C, '%'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 05
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 7F
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // ';','.'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '!','&'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 0A
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 95,FE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 97,FC
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9D,'$'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 96,'\'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 99,84
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9A,92

	PORT_START("X13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X14")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL) // ?? don't look for a new keypress
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) // ??
INPUT_PORTS_END

UINT32 micral_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
//  for (int y = 0; y < 32*8; y++)
//  {
//      offs_t offset = (y / 8) * 128;

//      for (int sx = 0; sx < 64; sx++)
//      {
//          UINT8 code = m_video_ram[offset++];
//          UINT8 attr = m_video_ram[offset++];

//          offs_t char_offs = ((code & 0x7f) << 3) | (y & 0x07);
//          if (BIT(code, 7)) char_offs = ((code & 0x7f) << 3) | ((y >> 1) & 0x07);

//          UINT8 data = m_char_rom->base()[char_offs];

//          rgb_t fg = m_palette->pen_color(attr & 0x07);
//          rgb_t bg = m_palette->pen_color((attr >> 3) & 0x07);

//          for (int x = 0; x < 6; x++)
//          {
//              bitmap.pix32(y, (sx * 6) + x) = BIT(data, 7) ? fg : bg;

//              data <<= 1;
//          }
//      }
//  }

	return 0;
}

DRIVER_INIT_MEMBER( micral_state, micral )
{
	//UINT8 *main = memregion("maincpu")->base();

	//membank("bankr0")->configure_entry(1, &main[0xf800]);
	//membank("bankr0")->configure_entry(0, &main[0x10000]);
	//membank("bankw0")->configure_entry(0, &main[0xf800]);
}

MACHINE_RESET_MEMBER( micral_state, micral )
{
	//membank("bankr0")->set_entry(0); // point at rom
	//membank("bankw0")->set_entry(0); // always write to ram
	m_maincpu->set_state_int(Z80_PC, 0xf800);
}

static MACHINE_CONFIG_START( micral, micral_state )
	// basic machine hardware
	MCFG_CPU_ADD( "maincpu", Z80, XTAL_4MHz )
	MCFG_CPU_PROGRAM_MAP(micral_mem)
	// no i/o ports on main cpu
	MCFG_CPU_ADD( "keyboard", Z80, XTAL_1MHz ) // freq unknown
	MCFG_CPU_PROGRAM_MAP(micral_kbd_mem)
	MCFG_CPU_IO_MAP(micral_kbd_io)

	MCFG_MACHINE_RESET_OVERRIDE(micral_state, micral)

	// video hardware
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(micral_state, screen_update)
	MCFG_SCREEN_SIZE(64*6, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*6-1, 0, 32*8-1)
	MCFG_PALETTE_ADD_MONOCHROME("palette")
	//MCFG_GFXDECODE_ADD("gfxdecode", "palette", micral)

	MCFG_DEVICE_ADD("crtc", CRT5037, XTAL_17_9712MHz/2)  // xtal freq unknown
	MCFG_TMS9927_CHAR_WIDTH(6)  // unknown
	//MCFG_TMS9927_VSYN_CALLBACK(DEVWRITELINE(TMS5501_TAG, tms5501_device, sens_w))
	MCFG_VIDEO_SET_SCREEN("screen")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 2000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( micral )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8022g.rom", 0xf800, 0x0800, CRC(882732a9) SHA1(3f37b82c450a54aedec209bd46fcbcf131c86313) )

	ROM_REGION( 0x400, "keyboard", 0 )
	ROM_LOAD( "2010221.rom", 0x000, 0x400, CRC(65123378) SHA1(401f0a648b78bf1662a1cd2546e83ba8e3cb7a42) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT     COMPANY       FULLNAME       FLAGS */
COMP( 1981, micral,  0,      0,      micral,    micral,  micral_state,  micral,  "Bull R2E", "Micral 80-22G", MACHINE_IS_SKELETON )
