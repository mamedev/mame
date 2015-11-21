// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
/***************************************************************************

  Slot Carnival, (1985, Wing Co,Ltd)

  Driver by David Haywood & Roberto Fresca.


  Notes:

  - Very similar to merit.c
  - We're using the MC6845 drawing code from merit.c, but it will need
    modifications to support the reels and proper colors.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "video/mc6845.h"

#define MASTER_CLOCK            (XTAL_10MHz)
#define CPU_CLOCK               (MASTER_CLOCK / 4)
#define PIXEL_CLOCK             (MASTER_CLOCK / 1)
#define CRTC_CLOCK              (MASTER_CLOCK / 8)
#define SND_CLOCK               (MASTER_CLOCK / 8)

#define NUM_PENS                (16)
#define RAM_PALETTE_SIZE        (1024)


class slotcarn_state : public driver_device
{
public:
	slotcarn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_backup_ram(*this, "backup_ram"),
		m_ram_attr(*this, "raattr"),
		m_ram_video(*this, "ravideo"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen") { }

	pen_t m_pens[NUM_PENS];
	required_shared_ptr<UINT8> m_backup_ram;
	required_shared_ptr<UINT8> m_ram_attr;
	required_shared_ptr<UINT8> m_ram_video;
	UINT8 *m_ram_palette;
	DECLARE_READ8_MEMBER(palette_r);
	DECLARE_WRITE8_MEMBER(palette_w);
	DECLARE_WRITE_LINE_MEMBER(hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(vsync_changed);
	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);
	virtual void machine_start();
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};


/*

  b800-b803 = PPI (9b) III mode0 all.
  ba00-ba03 = PPI (90) IOO mode0 all.
  bc00-bc03 = PPI (92) IIO mode0 all.

  6000 = RW

*/

READ8_MEMBER(slotcarn_state::palette_r)
{
	int co;

	co = ((m_ram_attr[offset] & 0x7F) << 3) | (offset & 0x07);
	return m_ram_palette[co];
}

WRITE8_MEMBER(slotcarn_state::palette_w)
{
	int co;

//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	data &= 0x0f;

	co = ((m_ram_attr[offset] & 0x7F) << 3) | (offset & 0x07);
	m_ram_palette[co] = data;

}


MC6845_BEGIN_UPDATE( slotcarn_state::crtc_begin_update )
{
	int dim, bit0, bit1, bit2;

	for (int i=0; i < NUM_PENS; i++)
	{
		dim = BIT(i,3) ? 255 : 127;
		bit0 = BIT(i,0);
		bit1 = BIT(i,1);
		bit2 = BIT(i,2);
		m_pens[i] = rgb_t(dim*bit0, dim*bit1, dim*bit2);
	}
}

MC6845_UPDATE_ROW( slotcarn_state::crtc_update_row )
{
	int extra_video_bank_bit = 0; // not used?
	int lscnblk = 0; // not used?

	UINT8 *gfx[2];
	UINT16 x = 0;
	int rlen;

	gfx[0] = memregion("gfx1")->base();
	gfx[1] = memregion("gfx2")->base();
	rlen = memregion("gfx2")->bytes();

	//ma = ma ^ 0x7ff;
	for (UINT8 cx = 0; cx < x_count; cx++)
	{
		int i;
		int attr = m_ram_attr[ma & 0x7ff];
		int region = (attr & 0x40) >> 6;
		int addr = ((m_ram_video[ma & 0x7ff] | ((attr & 0x80) << 1) | (extra_video_bank_bit)) << 4) | (ra & 0x0f);
		int colour = (attr & 0x7f) << 3;
		UINT8   *data;

		addr &= (rlen-1);
		data = gfx[region];

		for (i = 7; i>=0; i--)
		{
			int col = colour;

			col |= (BIT(data[0x0000 | addr],i)<<2);
			if (region==0)
			{
				col |= (BIT(data[rlen | addr],i)<<1);
				col |= (BIT(data[rlen<<1 | addr],i)<<0);
			}
			else
				col |= 0x03;

			col = m_ram_palette[col & 0x3ff];
			bitmap.pix32(y, x) = m_pens[col ? col & (NUM_PENS-1) : (lscnblk ? 8 : 0)];

			x++;
		}
		ma++;
	}
}


WRITE_LINE_MEMBER(slotcarn_state::hsync_changed)
{
	/* update any video up to the current scanline */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
}

WRITE_LINE_MEMBER(slotcarn_state::vsync_changed)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

/*******************************
*          Memory Map          *
*******************************/

static ADDRESS_MAP_START( slotcarn_map, AS_PROGRAM, 8, slotcarn_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("backup_ram")
	AM_RANGE(0x6800, 0x6fff) AM_RAM // spielbud
	AM_RANGE(0x7000, 0xafff) AM_ROM // spielbud


	AM_RANGE(0xb000, 0xb000) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0xb100, 0xb100) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, data_w)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)    /* Input Ports */
	AM_RANGE(0xba00, 0xba03) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)    /* Input Ports */
	AM_RANGE(0xbc00, 0xbc03) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)    /* Input/Output Ports */

	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("DSW3")
	AM_RANGE(0xc400, 0xc400) AM_READ_PORT("DSW4")

	AM_RANGE(0xd800, 0xd81f) AM_RAM // column scroll for reels?

	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_device, register_w)

	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

// spielbud - is the ay mirrored, or are there now 2?
static ADDRESS_MAP_START( spielbud_io_map, AS_IO, 8, slotcarn_state )
	AM_RANGE(0xb000, 0xb000) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0xb100, 0xb100) AM_DEVWRITE("aysnd", ay8910_device, data_w)
ADDRESS_MAP_END

/********************************
*          Input Ports          *
********************************/

static INPUT_PORTS_START( slotcarn )
	PORT_START("IN0")   /* b800 (ppi8255) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_Q) PORT_NAME("Key In")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_M) PORT_NAME("Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Select")

	PORT_START("IN1")   /* b801 (ppi8255) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1)    PORT_IMPULSE(2)       /* Coin A */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2)    PORT_IMPULSE(2)       /* Coin B */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Stats")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Settings")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3)    PORT_IMPULSE(2)       /* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN4)    PORT_IMPULSE(2)       /* Coin D */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* b802 (ppi8255) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* bc00 (ppi8255) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_W) PORT_NAME("Payout")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_H) PORT_NAME("Empty Hopper")

	PORT_START("IN4")   /* bc01 (ppi8255) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* ba00 (ppi8255) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "D-UP Pay Rate" )     PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0xc0, "80%" )
	PORT_DIPSETTING(    0x80, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )

	PORT_START("DSW2")  /* ay8910, port B */
	PORT_DIPNAME( 0x01, 0x01, "FIVE LINE Pay Rate" )    PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x00, "85%" )
	PORT_DIPNAME( 0x02, 0x02, "SUPER CONTI Pay Rate" )  PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x02, "75%" )
	PORT_DIPSETTING(    0x00, "85%" )
	PORT_DIPNAME( 0x04, 0x04, "LUCKY BAR Pay Rate" )    PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x00, "85%" )
	PORT_DIPNAME( 0x08, 0x08, "BONUS LINE Pay Rate" )   PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x00, "85%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* c000 direct */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")  /* c400 direct */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spielbud )
	PORT_START("IN0")   /* b800 (ppi8255) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1 / Deal (BJ)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3 / Bet 1 / Split (BJ)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4 / Stand (BJ)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5 / Hit (BJ)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Select")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   /* b801 (ppi8255) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(2)   /* Coin A */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2) PORT_IMPULSE(2)   /* Coin B */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3) PORT_IMPULSE(2)   /* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN4) PORT_IMPULSE(2)   /* Coin D */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   /* b802 (ppi8255) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* bc00 (ppi8255) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Tief (Low)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Hoch (High)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")   /* bc01 (ppi8255) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* ba00 (ppi8255) */
	PORT_DIPNAME( 0x01, 0x01, "Game STRATEGIE" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Game MITTE TRIFFT" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Game KNOBELN" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game BLACK JACK" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game CONTINENTAL" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Game 5 LINIEN SPIEL" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet Settings" )
	PORT_DIPSETTING(    0xc0, "Play 1 to 10 bets" )
	PORT_DIPSETTING(    0x80, "Play 1 to 20 bets" )
	PORT_DIPSETTING(    0x40, "Play 1 bet by hand" )
	PORT_DIPSETTING(    0x00, "Play 1 to 50 bets" )

	PORT_START("DSW2")  /* ay8910, port B */
	PORT_DIPNAME( 0x07, 0x07, "Main Game rate" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x07, "85%" )
	PORT_DIPSETTING(    0x03, "90%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Hold Buttons Behaviour" )
	PORT_DIPSETTING(    0x00, "Hold" )
	PORT_DIPSETTING(    0x80, "Discard" )

	PORT_START("DSW3")  /* c000 direct */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")  /* c400 direct */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
*          Graphics Layouts          *
*************************************/

static const gfx_layout slotcarntiles8x8x3_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8 // the flipped tiles are mixed with normal tiles, so skip a tile each time
};

static const gfx_layout slotcarntiles8x8x1_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8 // the flipped tiles are mixed with normal tiles, so skip a tile each time
};


/************************************
*          Graphics Decode          *
************************************/

static GFXDECODE_START( slotcarn )
	GFXDECODE_ENTRY( "gfx1", 0, slotcarntiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 8, slotcarntiles8x8x3_layout, 0, 16 ) // flipped
	GFXDECODE_ENTRY( "gfx2", 0, slotcarntiles8x8x1_layout, 0, 4 )
	GFXDECODE_ENTRY( "gfx2", 8, slotcarntiles8x8x1_layout, 0, 4 ) // flipped
GFXDECODE_END



void slotcarn_state::machine_start()
{
	m_ram_palette = auto_alloc_array(machine(), UINT8, RAM_PALETTE_SIZE);
	save_pointer(NAME(m_ram_palette), RAM_PALETTE_SIZE);
}

/***********************************
*          Machine Driver          *
***********************************/

static MACHINE_CONFIG_START( slotcarn, slotcarn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK) // 2.5 Mhz?
	MCFG_CPU_PROGRAM_MAP(slotcarn_map)
	MCFG_CPU_IO_MAP(spielbud_io_map)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))

	MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN3"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN4"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 512, 0, 512, 256, 0, 256)   /* temporary, CRTC will configure screen */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_BEGIN_UPDATE_CB(slotcarn_state, crtc_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(slotcarn_state, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(slotcarn_state, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(slotcarn_state, vsync_changed))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", slotcarn)
	MCFG_PALETTE_ADD("palette", 0x400)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd",AY8910, SND_CLOCK)
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/******************************
*          ROMs Load          *
******************************/
/*

Slot Carnival (c) WING 1985

roms 1 - 3 M5L2764K
roms 4 - 7 HN4827128G-30
proms 1 X MB7051

xtal 1 x 10mhz

cpus

1 x NEC D780C-1  (Z80)
3 x NEC  D8255AC-5
1 x AY-3-8910 (SOUND)
1 X HD46505SP-1  (VIDEO?)

ram

2x 2114
3x 58725
1x 6116

sound amp

HA1366W

(has 23 pots for volume)

4 banks of dipswitches 8 switches each
set to

- off + on

bank1
++------
bank2
+---++++
bank3
-+------
bank4
------++

board has a ton is transistors on it presumably for driving the coin hopper?
and a battery 3.5v 50ma

rom3 has mention of coin hopper and coin jam.

*/

ROM_START( slotcarn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "rom1.e10", 0x0000, 0x2000, CRC(a7ea6420) SHA1(4dd88f1bcaf354da93c3e88979a5e1a026105598) )
	ROM_LOAD( "rom2.e9",  0x2000, 0x2000, CRC(8156a603) SHA1(92618ac2ac908d24adb75eb705dc2f84eef12211) )
	ROM_LOAD( "rom3.e8",  0x4000, 0x2000, CRC(bf74ccad) SHA1(7f5049693de236790671b16dd1e1d0d2ac120e1a) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rom5.a4", 0x0000, 0x4000, CRC(99235fc7) SHA1(e93be11ee139f0845272a63ef2d50962e85e155a) )
	ROM_LOAD( "rom6.a3", 0x4000, 0x4000, CRC(2a86ce1f) SHA1(6dadbed41ae4b6e6e9efdb3a9d9d1f52dc76fe13) )
	ROM_LOAD( "rom7.a2", 0x8000, 0x4000, CRC(c8196687) SHA1(68233c3f039a01e4e25113c929f6a1b8d60af177) )

	ROM_REGION( 0x04000, "gfx2", 0 ) // 1bpp (both halves identical)
	ROM_LOAD( "rom4.a5", 0x0000, 0x4000, CRC(1428c46c) SHA1(ea30eeebcc2cc825f33e1ffeb590b047e3072b9c) )
ROM_END

/*

Lucky - Wing?
--------------

CPU:      1x Z80 (NEC D780C)
Video:    1x 6845 (HD46505SP HD6845SP)
Sound:    1x AY-3-8910
I/O:      3x 8255 (M5L8255AP-5)

ROM:      6x 2764 (0, 1, 2, 3, 4, 9)
          3x 27128 (6, 7, 8)

RAM:      1x HM6116LP-3 near ROMs 0 to 4
          1x M58725P near ROMs 0 to 4
          2x M58725P near ROMs 6 to 9

Xtal:     10 MHz.

DIP SW:   4x 8 DSW banks.


---------------------------------

Not from Wing.
It's a multi cards game produced in 1983, called "Spiel Bud" (Play Booth)
Language: german.

Roms have stickers with only a number, so were renamed from lwing to spielbud.

*/

ROM_START( spielbud )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "spielbud.00", 0x0000, 0x2000, CRC(201c7f19) SHA1(fb902824d1f6cfdf7ba124fca4c680af099ca4e1) )
	ROM_LOAD( "spielbud.01", 0x2000, 0x2000, CRC(16339de8) SHA1(00b14c6bca268b98bfb6ac9840d59b9a64d43b92) )
	ROM_LOAD( "spielbud.02", 0x4000, 0x2000, CRC(c791e75b) SHA1(b0276a82302a194fc1ab2608440a5bf1efe99e64) )
	ROM_LOAD( "spielbud.03", 0x7000, 0x2000, CRC(ddc60075) SHA1(592dc1f9ff00e703ed1c06fd037aa32569b19143) )
	ROM_LOAD( "spielbud.04", 0x9000, 0x2000, CRC(74f1ca60) SHA1(17a67beb00d9c4b3310930ad5c5f566b801634d7) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "spielbud.08", 0x0000, 0x4000, CRC(4a4ebe62) SHA1(5d789f2eea61864cd55f20a7697304daaff105c4) )
	ROM_LOAD( "spielbud.07", 0x4000, 0x4000, CRC(33c0064e) SHA1(bb4b80359b36b0b34e0d31ee1f18fd64b92ed342) )
	ROM_LOAD( "spielbud.06", 0x8000, 0x4000, CRC(bb2b6fd3) SHA1(5adb543c1d10aaede08609463f2ce82c5a68aca8) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "spielbud.09", 0x0000, 0x2000, CRC(d88c72f2) SHA1(bb015685f4c2cc7723c24880c11cb6d31f71e73f) )
ROM_END


/*********************************************
*                Game Drivers                *
**********************************************

      YEAR  NAME      PARENT   MACHINE   INPUT     INIT   ROT    COMPANY           FULLNAME               FLAGS  */
GAME( 1985, slotcarn, 0,       slotcarn, slotcarn, driver_device, 0,     ROT0, "Wing Co., Ltd.", "Slot Carnival",        MACHINE_NOT_WORKING )
GAME( 1985, spielbud, 0,       slotcarn, spielbud, driver_device, 0,     ROT0, "ADP",            "Spiel Bude (German)",  MACHINE_NOT_WORKING )
