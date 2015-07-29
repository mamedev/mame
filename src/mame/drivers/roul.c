// license:BSD-3-Clause
// copyright-holders:Roberto Zandona'
/* Super Lucky Roulette?

driver by Roberto Zandona'
thanks to Angelo Salese for some precious advice

TO DO:
- check palette
- check blitter command 0x00
- screen orientation is wrong (should clearly be ROT90 or 270 with blitter mods)

Has 36 pin Cherry master looking edge connector

.u12 2764 stickered 1
.u19 27256 stickered 2
.u15 tibpal16l8-25 (checksum was 0)
.u56 tibpal16l8-25 (checksum was 0)
.u38 82s123
.u53 82s123

Z80 x2
Altera Ep1810LC-45
20.000 MHz crystal
video 464p10 x4 (board silkscreened 4416)
AY-3-8912A

ROM text showed SUPER LUCKY ROULETTE LEISURE ENT


NOTES:

blitter:
there are 4 registers:

reg[0] -> y
reg[1] -> x
reg[3] & 0x0f -> color
reg[3] & 0x10 -> y direction (to the up or to the down)
reg[3] & 0x20 -> x direction (to the right or to the left)
reg[3] & 0xc0 == 0x00 -> filled square
reg[3] & 0xc0 == 0x40 -> width used in y direction
reg[3] & 0xc0 == 0x80 -> width used in x direction
reg[3] & 0xc0 == 0xc0 -> width used in both directions
reg[2] -> width (number of pixel to draw)

with a write in reg[2] the command is executed

not handled commands with reg[3] & 0xc0 == 0x00


Stephh's notes (based on the game Z80 code and some tests) :

  - "Reset" Dip Switch :
      * OFF : no effect if NVRAM isn't corrupted
      * ON  : reset (at least) credits, bets and last numbers (clear NVRAM)
  - When "Coin Assistance" Dip Switch is ON, you can reset the number of credits
    by pressing BOTH "Service" (IN0 bit 2) and "Clear Credits" (IN1 bit 6).
  - When in "Service Mode", press "Add Fiche" (IN1 bit 0) to increase value displayed in green after "R".
  - When in "Service Mode", press RIGHT (IN1 bit 1) to clear statistics (only possible when "R00" is displayed).
  - You need at least 5 credits for outside bets

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "roul.lh"
#include "machine/nvram.h"


class roul_state : public driver_device
{
public:
	roul_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;

	UINT8 m_reg[0x10];
	UINT8 *m_videobuf;
	UINT8 m_lamp_old;

	DECLARE_READ8_MEMBER(blitter_status_r);
	DECLARE_WRITE8_MEMBER(blitter_cmd_w);
	DECLARE_WRITE8_MEMBER(sound_latch_w);
	DECLARE_WRITE8_MEMBER(ball_w);

	virtual void video_start();
	DECLARE_PALETTE_INIT(roul);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


#define VIDEOBUF_SIZE 256*256


PALETTE_INIT_MEMBER(roul_state, roul)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit6, bit7, bit0, bit1, r, g, b;
	int i;

	for (i = 0; i < 0x20; ++i)
	{
		bit7 = (color_prom[0] >> 7) & 0x01;
		bit6 = (color_prom[0] >> 6) & 0x01;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		b = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		g = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		r = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

READ8_MEMBER(roul_state::blitter_status_r)
{
/*
code check bit 6 and bit 7
bit 7 -> blitter ready
bit 6 -> ??? (after unknown blitter command : [80][80][08][02])
*/
//  return 0x80; // blitter ready
//  logerror("Read unknown port $f5 at %04x\n",space.device().safe_pc());
	return machine().rand() & 0x00c0;
}

WRITE8_MEMBER(roul_state::blitter_cmd_w)
{
	m_reg[offset] = data;
	if (offset==2)
	{
		int i,j;
		int width   = m_reg[2];
		int y       = m_reg[0];
		int x       = m_reg[1];
		int color   = m_reg[3] & 0x0f;
		int xdirection = 1, ydirection = 1;

		if (m_reg[3] & 0x10) ydirection = -1;
		if (m_reg[3] & 0x20) xdirection = -1;

		if (width == 0x00) width = 0x100;

		switch(m_reg[3] & 0xc0)
		{
			case 0x00: // m_reg[4] used?
				for (i = - width / 2; i < width / 2; i++)
					for (j = - width / 2; j < width / 2; j++)
						m_videobuf[(y + j) * 256 + x + i] = color;
				logerror("Blitter command 0 : [%02x][%02x][%02x][%02x][%02x]\n",m_reg[0],m_reg[1],m_reg[2],m_reg[3],m_reg[4]);
				break;
			case 0x40: // vertical line - m_reg[4] not used
				for (i = 0; i < width; i++ )
					m_videobuf[(y + i * ydirection) * 256 + x] = color;
				break;
			case 0x80: // horizontal line - m_reg[4] not used
				for (i = 0; i < width; i++ )
					m_videobuf[y * 256 + x + i * xdirection] = color;
				break;
			case 0xc0: // diagonal line - m_reg[4] not used
				for (i = 0; i < width; i++ )
					m_videobuf[(y + i * ydirection) * 256 + x + i * xdirection] = color;
		}
	}

}

WRITE8_MEMBER(roul_state::sound_latch_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_soundcpu->set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(roul_state::ball_w)
{
	int lamp = data;

	output_set_lamp_value(data, 1);
	output_set_lamp_value(m_lamp_old, 0);
	m_lamp_old = lamp;
}

static ADDRESS_MAP_START( roul_map, AS_PROGRAM, 8, roul_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( roul_cpu_io_map, AS_IO, 8, roul_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf4) AM_WRITE(blitter_cmd_w)
	AM_RANGE(0xf5, 0xf5) AM_READ(blitter_status_r)
	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("DSW")
	AM_RANGE(0xf9, 0xf9) AM_WRITE(ball_w)
	AM_RANGE(0xfa, 0xfa) AM_READ_PORT("IN0")
	AM_RANGE(0xfd, 0xfd) AM_READ_PORT("IN1")
	AM_RANGE(0xfe, 0xfe) AM_WRITE(sound_latch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, roul_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_io_map, AS_IO, 8, roul_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
ADDRESS_MAP_END

void roul_state::video_start()
{
	m_videobuf = auto_alloc_array_clear(machine(), UINT8, VIDEOBUF_SIZE);

	save_item(NAME(m_reg));
	save_pointer(NAME(m_videobuf), VIDEOBUF_SIZE);
	save_item(NAME(m_lamp_old));
}

UINT32 roul_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i,j;
	for (i = 0; i < 256; i++)
		for (j = 0; j < 256; j++)
			bitmap.pix16(j, i) = m_videobuf[j * 256 + 255 - i];
	return 0;
}


/* verified from Z80 code */
static INPUT_PORTS_START( roul )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_NAME("Coin 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Service") PORT_CODE(KEYCODE_X)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Z)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Add Fiche")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Remove Fiche")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_C)    /* see notes */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	/* single - split - street - square / double street / dozen - column - 1 to 18 - 19 to 36 - red - black - odd - even */
	PORT_DIPNAME( 0x81, 0x00, "Max Bet" )
	PORT_DIPSETTING(    0x00, "10 / 30 / 30" )
	PORT_DIPSETTING(    0x80, "20 / 40 / 50" )
	PORT_DIPSETTING(    0x01, "30 / 50 / 70" )
	PORT_DIPSETTING(    0x81, "40 / 60 / 90" )
	PORT_DIPNAME( 0x0e, 0x0e, "Percentage Payout" )
	PORT_DIPSETTING(    0x00, "94%" )
	PORT_DIPSETTING(    0x02, "88%" )
	PORT_DIPSETTING(    0x04, "82%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x08, "68%" )
	PORT_DIPSETTING(    0x0a, "62%" )
	PORT_DIPSETTING(    0x0c, "56%" )
	PORT_DIPSETTING(    0x0e, "50%" )
	PORT_DIPNAME( 0x10, 0x10, "Reset Machine" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Doubble Odds" )
	PORT_DIPSETTING(    0x20, "With" )
	PORT_DIPSETTING(    0x00, "Without" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Assistance" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( roul, roul_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(roul_map)
	MCFG_CPU_IO_MAP(roul_cpu_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", roul_state, nmi_line_pulse)

	MCFG_CPU_ADD("soundcpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_cpu_io_map)

	MCFG_NVRAM_ADD_0FILL("nvram")


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(roul_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(roul_state, roul)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 1000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START(roul)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roul.u19",    0x0000, 0x8000, CRC(1ec37876) SHA1(c2877646dad9daebc55db57d513ad448b1f4c923) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("roul.u12",    0x0000, 0x1000, CRC(356fe025) SHA1(bca69e090a852454e921130afbdd28021b62c44e) )
	ROM_CONTINUE(0x0000,0x1000)

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "roul.u53",   0x0000, 0x0020, CRC(1965dfaa) SHA1(114eccd3e478902ac7dbb10b9425784231ff581e) )
	ROM_LOAD( "roul.u38",   0x0020, 0x0020, CRC(23ae22c1) SHA1(bf0383462976ec6341ffa8a173264ce820bc654a) )
ROM_END

GAMEL( 1990, roul,  0,   roul, roul, driver_device, 0, ROT0, "bootleg", "Super Lucky Roulette", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_roul )
