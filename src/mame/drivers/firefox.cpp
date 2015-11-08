// license:BSD-3-Clause
// copyright-holders:smf, Aaron Giles, Chris Hardy
/***************************************************************************

    Atari Fire Fox hardware

    driver by smf, Aaron Giles, Chris Hardy & Scott Waye

short term:
    split driver/vidhrdw/sndhrdw/machine
    add option to centre joystick to enter test menu

it uses a quad pokey package 137323-1221-406???
the laser disc is a philips lvp 22vp931
( but maybe this works too... Special Drive: Laser Disc Player - Philips VP-832A )


AV# 60626
Atari "Firefox" V

Laser Disc - 30 minutes - Color - 1983

An interactive CAV laserdisc designed for use in the Atari video arcade game machine.
Contains over 100 visual and sound segments that include all of the branching possibilities of this game.
Each segment is two to five seconds long. This disc will play on any player,
but requires a special level III player for proper control. Video: CAV. Audio: Analog.

*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "machine/ldvp931.h"
#include "machine/6532riot.h"
#include "machine/x2212.h"


class firefox_state : public driver_device
{
public:
	firefox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_laserdisc(*this, "laserdisc") ,
		m_tileram(*this, "tileram"),
		m_spriteram(*this, "spriteram"),
		m_sprite_palette(*this, "sprite_palette"),
		m_tile_palette(*this, "tile_palette"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<phillips_22vp931_device> m_laserdisc;
	required_shared_ptr<unsigned char> m_tileram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<unsigned char> m_sprite_palette;
	required_shared_ptr<unsigned char> m_tile_palette;
	int m_n_disc_lock;
	int m_n_disc_data;
	int m_n_disc_read_data;
	x2212_device *m_nvram_1c;
	x2212_device *m_nvram_1d;
	tilemap_t *m_bgtiles;
	int m_control_num;
	UINT8 m_sound_to_main_flag;
	UINT8 m_main_to_sound_flag;
	int m_sprite_bank;
	DECLARE_READ8_MEMBER(firefox_disc_status_r);
	DECLARE_READ8_MEMBER(firefox_disc_data_r);
	DECLARE_WRITE8_MEMBER(firefox_disc_read_w);
	DECLARE_WRITE8_MEMBER(firefox_disc_lock_w);
	DECLARE_WRITE8_MEMBER(audio_enable_w);
	DECLARE_WRITE8_MEMBER(firefox_disc_reset_w);
	DECLARE_WRITE8_MEMBER(firefox_disc_write_w);
	DECLARE_WRITE8_MEMBER(firefox_disc_data_w);
	DECLARE_WRITE8_MEMBER(tileram_w);
	DECLARE_WRITE8_MEMBER(tile_palette_w);
	DECLARE_WRITE8_MEMBER(sprite_palette_w);
	DECLARE_WRITE8_MEMBER(firefox_objram_bank_w);
	DECLARE_READ8_MEMBER(sound_to_main_r);
	DECLARE_WRITE8_MEMBER(main_to_sound_w);
	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_READ8_MEMBER(main_to_sound_r);
	DECLARE_WRITE8_MEMBER(sound_to_main_w);
	DECLARE_READ8_MEMBER(adc_r);
	DECLARE_WRITE8_MEMBER(adc_select_w);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(novram_recall_w);
	DECLARE_WRITE8_MEMBER(novram_store_w);
	DECLARE_WRITE8_MEMBER(rom_bank_w);
	DECLARE_WRITE8_MEMBER(main_irq_clear_w);
	DECLARE_WRITE8_MEMBER(main_firq_clear_w);
	DECLARE_WRITE8_MEMBER(self_reset_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(firefox_coin_counter_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mainflag_r);
	DECLARE_CUSTOM_INPUT_MEMBER(soundflag_r);
	DECLARE_READ8_MEMBER(riot_porta_r);
	DECLARE_WRITE8_MEMBER(riot_porta_w);
	DECLARE_WRITE_LINE_MEMBER(riot_irq);
	TILE_GET_INFO_MEMBER(bgtile_get_info);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_firefox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(video_timer_callback);
	void set_rgba( int start, int index, unsigned char *palette_ram );
	void firq_gen(phillips_22vp931_device &laserdisc, int state);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};



#define MASTER_XTAL     XTAL_14_31818MHz


/*
fff6=firq e4a2 when dav goes active/low
fff8=irq e38f  This is through a flip-flop so goes off (high as active low) only when reset_irq is active - low.
fffa=??? e38d
fffc=??? e38d
fffe=reset e7cc
*/

/* 0x50-52 Used as a copy of the status
   0x59 = 6-length of laser disc return code
   0x53 = pointer to laser disc return
   ( LaserDiscBits & 0x80 ) != 0 when return code available
   DSKREAD = acknowledge
   ReadDiscData = return code
*/

/* FXXXXX for first field
   AXXXXX for second field */


/* 20 = DISKOPR - Active low
   40 = DISKFULL - Active low
   80 = DISKDAV - Active low data available
   */
READ8_MEMBER(firefox_state::firefox_disc_status_r)
{
	UINT8 result = 0xff;

	result ^= 0x20;
	if (!m_laserdisc->ready_r())
		result ^= 0x40;
	if (m_laserdisc->data_available_r())
		result ^= 0x80;

	return result;
}

/* 4105 - DREAD */
/* this reset RDDSK (&DSKRD) */
READ8_MEMBER(firefox_state::firefox_disc_data_r)
{
	return m_n_disc_read_data;
}

/* DISK READ ENABLE */
/* 4218 - DSKREAD, set RDDSK */
WRITE8_MEMBER(firefox_state::firefox_disc_read_w)
{
	m_n_disc_read_data = m_laserdisc->data_r();
}

WRITE8_MEMBER(firefox_state::firefox_disc_lock_w)
{
	m_n_disc_lock = data & 0x80;
}

WRITE8_MEMBER(firefox_state::audio_enable_w)
{
	m_laserdisc->set_output_gain(~offset & 1, (data & 0x80) ? 1.0 : 0.0);
}

WRITE8_MEMBER(firefox_state::firefox_disc_reset_w)
{
	m_laserdisc->reset_w((data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

/* active low on dbb7 */
WRITE8_MEMBER(firefox_state::firefox_disc_write_w)
{
	if ( ( data & 0x80 ) == 0 )
		m_laserdisc->data_w(m_n_disc_data);
}

/* latch the data */
WRITE8_MEMBER(firefox_state::firefox_disc_data_w)
{
	m_n_disc_data = data;
}




/*************************************
 *
 *  Video
 *
 *************************************/

TILE_GET_INFO_MEMBER(firefox_state::bgtile_get_info)
{
	SET_TILE_INFO_MEMBER(0, m_tileram[tile_index], 0, 0);
}


WRITE8_MEMBER(firefox_state::tileram_w)
{
	m_tileram[offset] = data;
	m_bgtiles->mark_tile_dirty(offset);
}


void firefox_state::video_start()
{
	m_bgtiles = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(firefox_state::bgtile_get_info),this), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_bgtiles->set_transparent_pen(0);
	m_bgtiles->set_scrolldy(m_screen->visible_area().min_y, 0);
}


UINT32 firefox_state::screen_update_firefox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int sprite;
	int gfxtop = screen.visible_area().min_y;

	bitmap.fill(m_palette->pen_color(256), cliprect);

	for( sprite = 0; sprite < 32; sprite++ )
	{
		UINT8 *sprite_data = m_spriteram + ( 0x200 * m_sprite_bank ) + ( sprite * 16 );
		int flags = sprite_data[ 0 ];
		int y = sprite_data[ 1 ] + ( 256 * ( ( flags >> 0 ) & 1 ) );
		int x = sprite_data[ 2 ] + ( 256 * ( ( flags >> 1 ) & 1 ) );

		if( x != 0 )
		{
			int row;

			for( row = 0; row < 8; row++ )
			{
				int color = ( flags >> 2 ) & 0x03;
				int flipy = flags & 0x10;
				int flipx = flags & 0x20;
				int code = sprite_data[ 15 - row ] + ( 256 * ( ( flags >> 6 ) & 3 ) );

				m_gfxdecode->gfx( 1 )->transpen(bitmap,cliprect, code, color, flipx, flipy, x + 8, gfxtop + 500 - y - ( row * 16 ), 0 );
			}
		}
	}

	m_bgtiles->draw(screen, bitmap, cliprect, 0, 0 );

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(firefox_state::video_timer_callback)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE );
}

void firefox_state::set_rgba( int start, int index, unsigned char *palette_ram )
{
	int r = palette_ram[ index ];
	int g = palette_ram[ index + 256 ];
	int b = palette_ram[ index + 512 ];
	int a = ( b & 3 ) * 0x55;

	m_palette->set_pen_color( start + index, rgb_t( a, r, g, b ) );
}

WRITE8_MEMBER(firefox_state::tile_palette_w)
{
	m_tile_palette[ offset ] = data;
	set_rgba( 0, offset & 0xff, m_tile_palette );
}

WRITE8_MEMBER(firefox_state::sprite_palette_w)
{
	m_sprite_palette[ offset ] = data;
	set_rgba( 256, offset & 0xff, m_sprite_palette );
}

WRITE8_MEMBER(firefox_state::firefox_objram_bank_w)
{
	m_sprite_bank = data & 0x03;
}



/*************************************
 *
 *  Main <-> sound communication
 *
 *************************************/

CUSTOM_INPUT_MEMBER(firefox_state::mainflag_r)
{
	return m_main_to_sound_flag;
}

CUSTOM_INPUT_MEMBER(firefox_state::soundflag_r)
{
	return m_sound_to_main_flag;
}

READ8_MEMBER(firefox_state::sound_to_main_r)
{
	m_sound_to_main_flag = 0;
	return soundlatch2_byte_r(space, 0);
}

WRITE8_MEMBER(firefox_state::main_to_sound_w)
{
	m_main_to_sound_flag = 1;
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(firefox_state::sound_reset_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
	if ((data & 0x80) != 0)
		m_sound_to_main_flag = m_main_to_sound_flag = 0;
}

READ8_MEMBER(firefox_state::main_to_sound_r)
{
	m_main_to_sound_flag = 0;
	return soundlatch_byte_r(space, 0);
}

WRITE8_MEMBER(firefox_state::sound_to_main_w)
{
	m_sound_to_main_flag = 1;
	soundlatch2_byte_w(space, 0, data);
}



/*************************************
 *
 *  6532 RIOT handlers
 *
 *************************************/

READ8_MEMBER(firefox_state::riot_porta_r)
{
	tms5220_device *tms5220 = machine().device<tms5220_device>("tms");

	/* bit 7 = MAINFLAG */
	/* bit 6 = SOUNDFLAG */
	/* bit 5 = PA5 */
	/* bit 4 = TEST */
	/* bit 3 = n/c */
	/* bit 2 = TMS /ready */
	/* bit 1 = TMS /read */
	/* bit 0 = TMS /write */

	return (m_main_to_sound_flag << 7) | (m_sound_to_main_flag << 6) | 0x10 | (tms5220->readyq_r() << 2);
}

WRITE8_MEMBER(firefox_state::riot_porta_w)
{
	tms5220_device *tms5220 = machine().device<tms5220_device>("tms");

	/* handle 5220 read */
	tms5220->rsq_w((data>>1) & 1);

	/* handle 5220 write */
	tms5220->wsq_w(data & 1);
}

WRITE_LINE_MEMBER(firefox_state::riot_irq)
{
	m_audiocpu->set_input_line(M6502_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  ADC input and control
 *
 *************************************/

READ8_MEMBER(firefox_state::adc_r)
{
	if( m_control_num == 0 )
	{
		return ioport( "PITCH" )->read();
	}

	return ioport( "YAW" )->read();
}

WRITE8_MEMBER(firefox_state::adc_select_w)
{
	m_control_num = offset;
}



/*************************************
 *
 *  Non-Volatile RAM (NOVRAM)
 *
 *************************************/

WRITE8_MEMBER(firefox_state::nvram_w)
{
	m_nvram_1c->write(space, offset, data >> 4);
	m_nvram_1d->write(space, offset, data & 0xf);
}

READ8_MEMBER(firefox_state::nvram_r)
{
	return (m_nvram_1c->read(space, offset) << 4) | (m_nvram_1d->read(space, offset) & 0x0f);
}

WRITE8_MEMBER(firefox_state::novram_recall_w)
{
	m_nvram_1c->recall(data & 0x80);
	m_nvram_1d->recall(data & 0x80);
}

WRITE8_MEMBER(firefox_state::novram_store_w)
{
	m_nvram_1c->store(data & 0x80);
	m_nvram_1d->store(data & 0x80);
}



/*************************************
 *
 *  Main cpu
 *
 *************************************/

WRITE8_MEMBER(firefox_state::rom_bank_w)
{
	membank("bank1")->set_entry(data & 0x1f);
}

WRITE8_MEMBER(firefox_state::main_irq_clear_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE );
}

WRITE8_MEMBER(firefox_state::main_firq_clear_w)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE );
}

WRITE8_MEMBER(firefox_state::self_reset_w)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE );
}



/*************************************
 *
 *  I/O
 *
 *************************************/

WRITE8_MEMBER(firefox_state::led_w)
{
	set_led_status( machine(), offset, ( data & 0x80 ) == 0 );
}

WRITE8_MEMBER(firefox_state::firefox_coin_counter_w)
{
	coin_counter_w( machine(), offset, data & 0x80 );
}



void firefox_state::firq_gen(phillips_22vp931_device &laserdisc, int state)
{
	if (state)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE );
}


void firefox_state::machine_start()
{
	membank("bank1")->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x1000);
	m_nvram_1c = machine().device<x2212_device>("nvram_1c");
	m_nvram_1d = machine().device<x2212_device>("nvram_1d");

	m_laserdisc->set_data_ready_callback(phillips_22vp931_device::data_ready_delegate(FUNC(firefox_state::firq_gen), this));

	m_control_num = 0;
	m_sprite_bank = 0;
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, firefox_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_RAM_WRITE(tileram_w) AM_SHARE("tileram")
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x2800, 0x2aff) AM_RAM_WRITE(sprite_palette_w) AM_SHARE("sprite_palette")
	AM_RANGE(0x2b00, 0x2b00) AM_MIRROR(0x04ff) AM_WRITE(firefox_objram_bank_w)
	AM_RANGE(0x2c00, 0x2eff) AM_RAM_WRITE(tile_palette_w) AM_SHARE("tile_palette")
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x40ff) AM_READWRITE(nvram_r, nvram_w)                     /* NOVRAM */
	AM_RANGE(0x4100, 0x4100) AM_MIRROR(0x00f8) AM_READ_PORT("rdin0")            /* RDIN0 */
	AM_RANGE(0x4101, 0x4101) AM_MIRROR(0x00f8) AM_READ_PORT("rdin1")            /* RDIN1 */
	AM_RANGE(0x4102, 0x4102) AM_MIRROR(0x00f8) AM_READ(firefox_disc_status_r)   /* RDIN2 */
	AM_RANGE(0x4103, 0x4103) AM_MIRROR(0x00f8) AM_READ_PORT("opt0")             /* OPT0 */
	AM_RANGE(0x4104, 0x4104) AM_MIRROR(0x00f8) AM_READ_PORT("opt1")             /* OPT1 */
	AM_RANGE(0x4105, 0x4105) AM_MIRROR(0x00f8) AM_READ(firefox_disc_data_r)     /* DREAD */
	AM_RANGE(0x4106, 0x4106) AM_MIRROR(0x00f8) AM_READ(sound_to_main_r)         /* RDSOUND */
	AM_RANGE(0x4107, 0x4107) AM_MIRROR(0x00f8) AM_READ(adc_r)                   /* ADC */
	AM_RANGE(0x4200, 0x4200) AM_MIRROR(0x0047) AM_WRITE(main_irq_clear_w)       /* RSTIRQ */
	AM_RANGE(0x4208, 0x4208) AM_MIRROR(0x0047) AM_WRITE(main_firq_clear_w)      /* RSTFIRQ */
	AM_RANGE(0x4210, 0x4210) AM_MIRROR(0x0047) AM_WRITE(watchdog_reset_w)       /* WDCLK */
	AM_RANGE(0x4218, 0x4218) AM_MIRROR(0x0047) AM_WRITE(firefox_disc_read_w)    /* DSKREAD */
	AM_RANGE(0x4220, 0x4223) AM_MIRROR(0x0044) AM_WRITE(adc_select_w)           /* ADCSTART */
	AM_RANGE(0x4230, 0x4230) AM_MIRROR(0x0047) AM_WRITE(self_reset_w)           /* AMUCK */
	AM_RANGE(0x4280, 0x4280) AM_MIRROR(0x0040) AM_WRITE(novram_recall_w)        /* LATCH0 -> NVRECALL */
	AM_RANGE(0x4281, 0x4281) AM_MIRROR(0x0040) AM_WRITE(sound_reset_w)          /* LATCH0 -> RSTSOUND */
	AM_RANGE(0x4282, 0x4282) AM_MIRROR(0x0040) AM_WRITE(novram_store_w)         /* LATCH0 -> NVRSTORE */
	AM_RANGE(0x4283, 0x4283) AM_MIRROR(0x0040) AM_WRITE(firefox_disc_lock_w)    /* LATCH0 -> LOCK */
	AM_RANGE(0x4284, 0x4285) AM_MIRROR(0x0040) AM_WRITE(audio_enable_w)         /* LATCH0 -> SWDSKR, SWDSKL */
	AM_RANGE(0x4286, 0x4286) AM_MIRROR(0x0040) AM_WRITE(firefox_disc_reset_w)   /* LATCH0 -> RSTDSK */
	AM_RANGE(0x4287, 0x4287) AM_MIRROR(0x0040) AM_WRITE(firefox_disc_write_w)   /* LATCH0 -> WRDSK */
	AM_RANGE(0x4288, 0x4289) AM_MIRROR(0x0040) AM_WRITE(firefox_coin_counter_w) /* LATCH1 -> COIN COUNTERR, COUNTERL */
	AM_RANGE(0x428c, 0x428f) AM_MIRROR(0x0040) AM_WRITE(led_w)                  /* LATCH1 -> LEDs */
	AM_RANGE(0x4290, 0x4290) AM_MIRROR(0x0047) AM_WRITE(rom_bank_w)             /* WRTREG */
	AM_RANGE(0x4298, 0x4298) AM_MIRROR(0x0047) AM_WRITE(main_to_sound_w)        /* WRSOUND */
	AM_RANGE(0x42a0, 0x42a0) AM_MIRROR(0x0047) AM_WRITE(firefox_disc_data_w)    /* DSKLATCH */
	AM_RANGE(0x4400, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, firefox_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x087f) AM_MIRROR(0x0700) AM_RAM /* RIOT ram */
	AM_RANGE(0x0880, 0x089f) AM_MIRROR(0x07e0) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x1000, 0x1000) AM_READ(main_to_sound_r)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(sound_to_main_w)
	AM_RANGE(0x2000, 0x200f) AM_DEVREADWRITE("pokey1", pokey_device, read, write)
	AM_RANGE(0x2800, 0x280f) AM_DEVREADWRITE("pokey2", pokey_device, read, write)
	AM_RANGE(0x3000, 0x300f) AM_DEVREADWRITE("pokey3", pokey_device, read, write)
	AM_RANGE(0x3800, 0x380f) AM_DEVREADWRITE("pokey4", pokey_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( firefox )
	PORT_START("rdin0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("rdin1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firefox_state,mainflag_r, NULL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firefox_state,soundflag_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("opt0")
	PORT_DIPNAME(    0x03, 0x00, "Coins Per Credit" )
	PORT_DIPSETTING( 0x00, "1 Coin 1 Credit" )
	PORT_DIPSETTING( 0x01, "2 Coins 1 Credit" )
	PORT_DIPSETTING( 0x02, "3 Coins 1 Credit" )
	PORT_DIPSETTING( 0x03, "4 Coins 1 Credit" )
	PORT_DIPNAME(    0x0c, 0x00, "Right Coin" )
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x04, "1 Coin for 4 Coin Units" )
	PORT_DIPSETTING( 0x08, "1 Coin for 5 Coin Units" )
	PORT_DIPSETTING( 0x0c, "1 Coin for 6 Coin Units" )
	PORT_DIPNAME(    0x10, 0x00, "Left Coin" )
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x10, "1 Coin for 2 Coin Units" )
	PORT_DIPNAME(    0xe0, 0x00, "Bonus Adder" )
	PORT_DIPSETTING( 0x00, DEF_STR( None ) )
	PORT_DIPSETTING( 0x20, "1 Credit for 2 Coin Units" )
	PORT_DIPSETTING( 0xa0, "1 Credit for 3 Coin Units" )
	PORT_DIPSETTING( 0x40, "1 Credit for 4 Coin Units" )
	PORT_DIPSETTING( 0x80, "1 Credit for 5 Coin Units" )
	PORT_DIPSETTING( 0x60, "2 Credits for 4 Coin Units" )
	PORT_DIPSETTING( 0xe0, DEF_STR( Free_Play ) )

	PORT_START("opt1")
	PORT_DIPNAME( 0x01, 0x00, "Missions" )
	PORT_DIPSETTING(    0x00, "All .50" )
	PORT_DIPSETTING(    0x01, ".50 .75" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, "Moderate" )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x00, "Gas Usage" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, "Moderate" )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x00, "Bonus Gas" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, "Moderate" )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, "Pro Limit" )
	PORT_DIPSETTING(    0x00, "Moderate" )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )

	PORT_START("PITCH")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("YAW")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(0,6), RGN_FRAC(1,6), RGN_FRAC(2,6), RGN_FRAC(3,6), RGN_FRAC(4,6), RGN_FRAC(5,6) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	32*8
};

static GFXDECODE_START( firefox )
	GFXDECODE_ENTRY("tiles",   0, tilelayout,   0,   1)
	GFXDECODE_ENTRY("sprites", 0, spritelayout, 256, 4)
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( firefox, firefox_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809E, MASTER_XTAL/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	/* interrupts count starting at end of VBLANK, which is 44, so add 44 */
	MCFG_TIMER_DRIVER_ADD_SCANLINE("32v", firefox_state, video_timer_callback, "screen", 96+44, 128)

	MCFG_CPU_ADD("audiocpu", M6502, MASTER_XTAL/8)
	MCFG_CPU_PROGRAM_MAP(audio_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000))

	MCFG_WATCHDOG_TIME_INIT(attotime::from_hz((double)MASTER_XTAL/8/16/16/16/16))

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", firefox)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_LASERDISC_22VP931_ADD("laserdisc")
	MCFG_LASERDISC_OVERLAY_DRIVER(64*8, 525, firefox_state, screen_update_firefox)
	MCFG_LASERDISC_OVERLAY_CLIP(7*8, 53*8-1, 44, 480+44)
	MCFG_LASERDISC_OVERLAY_PALETTE("palette")

	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")

	MCFG_X2212_ADD_AUTOSAVE("nvram_1c")
	MCFG_X2212_ADD_AUTOSAVE("nvram_1d")

	MCFG_DEVICE_ADD("riot", RIOT6532, MASTER_XTAL/8)
	MCFG_RIOT6532_IN_PA_CB(READ8(firefox_state, riot_porta_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(firefox_state, riot_porta_w))
	MCFG_RIOT6532_IN_PB_CB(DEVREAD8("tms", tms5220_device, status_r))
	MCFG_RIOT6532_OUT_PB_CB(DEVWRITE8("tms", tms5220_device, data_w))
	MCFG_RIOT6532_IRQ_CB(WRITELINE(firefox_state, riot_irq))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("pokey1", POKEY, MASTER_XTAL/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_SOUND_ADD("pokey2", POKEY, MASTER_XTAL/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_SOUND_ADD("pokey3", POKEY, MASTER_XTAL/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_SOUND_ADD("pokey4", POKEY, MASTER_XTAL/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_SOUND_ADD("tms", TMS5220, MASTER_XTAL/2/11)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( firefox )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + data & 128k for banked roms */
	ROM_LOAD( "136026.209",     0x04000, 0x4000, CRC(9f559f1b) SHA1(142d14cb5158ea77f6fc6d9bf0ce723842f345e2) ) /* 8b/c */
	ROM_LOAD( "136026.210",     0x08000, 0x4000, CRC(d769b40d) SHA1(2d354649a381f3399cb0161267bd1c36a8f2bb4b) ) /* 7b/c */
	ROM_LOAD( "136026.211",     0x0c000, 0x4000, CRC(7293ab03) SHA1(73d0d173da295ad59e431bab0a9814a71146cbc2) ) /* 6b/c */
	ROM_LOAD( "136026.201",     0x10000, 0x4000, CRC(c118547a) SHA1(4d3502cbde3116588ed944bf1750bab50e4c813c) ) /* 8a */
	/* empty 7a */
	/* empty 6a */
	/* empty 5a */
	ROM_LOAD( "136026.205",     0x20000, 0x4000, CRC(dc21677f) SHA1(576a96c1e07e1362a0a367e76dc369ee8a950144) ) /* 4a */
	ROM_LOAD( "136026.127",     0x24000, 0x2000, CRC(c0c765ab) SHA1(79f6c8c1d00684d7143b2d33a5669bdf5cd01e96) ) /* 3a */
	ROM_RELOAD( 0x26000, 0x2000 )
	/* empty 2a */
	/* empty 1a */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for code */
	/* empty 4k/l */
	ROM_LOAD( "136026.128",     0x08000, 0x2000, CRC(5358d870) SHA1(e8f2983a7e612e1a050a3c0b9f19b1077de4c146) ) /* 4m */
	ROM_RELOAD( 0x0a000, 0x2000 )
	ROM_LOAD( "136026.214",     0x0c000, 0x4000, CRC(92378b78) SHA1(62c7a1fee675fa3f9125f8e208b8207f0ce28bbe) ) /* 4n */

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "136026.125",     0x0000,  0x2000, CRC(8a32f9f1) SHA1(f899174f55cd4a24a3be4a0f4bb44d3e8e938586) ) /* 6p */

	ROM_REGION( 0x30000, "sprites", ROMREGION_ERASE00 )
	/* empty 6c */
	/* empty 6a */
	ROM_LOAD( "136026.124",     0x08000,  0x4000, CRC(5efe0f6c) SHA1(df35fd9267d966ab379c2f78ed418f4606741b28)) /* 5c */
	ROM_LOAD( "136026.123",     0x0c000,  0x4000, CRC(dffe48b3) SHA1(559907651bb425e26a834b467959b15092d23d27)) /* 5a */
	ROM_LOAD( "136026.118",     0x10000,  0x4000, CRC(0ed4df15) SHA1(7aa599f428112fff4bfedf63fafc22f19fa66546)) /* 4c */
	ROM_LOAD( "136026.122",     0x14000,  0x4000, CRC(8e2c6616) SHA1(59cbd585028bb634034a9dfd552275bd41f01989)) /* 4a */
	ROM_LOAD( "136026.117",     0x18000,  0x4000, CRC(79129084) SHA1(4219ff7cd444ad11e4cb9f1c30ac15fe0cfc5a17)) /* 3c */
	ROM_LOAD( "136026.121",     0x1c000,  0x4000, CRC(494972d4) SHA1(fa0e24e911b233e9644d7794ba03f76bfd39aa8c)) /* 3a */
	ROM_LOAD( "136026.116",     0x20000,  0x4000, CRC(d5282d4e) SHA1(de5fdf82a615625aa77b39e035b4206216faaf9c)) /* 2c */
	ROM_LOAD( "136026.120",     0x24000,  0x4000, CRC(e1b95923) SHA1(b6d0c0af0a8f55e728cd0f4c3222745eefd57f50)) /* 2a */
	ROM_LOAD( "136026.115",     0x28000,  0x4000, CRC(861abc82) SHA1(1845888d07162ae915364a2a91294731f1c5b3bd)) /* 1c */
	ROM_LOAD( "136026.119",     0x2c000,  0x4000, CRC(959471b1) SHA1(a032209a209f51d34360d5c7ad32ec62150158d2)) /* 1a */

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "firefox", 0, SHA1(3c4be40f55b44d0352b64c0861b6d1b650451ce7) )
ROM_END

ROM_START( firefoxa )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + data & 128k for banked roms */
	ROM_LOAD( "136026.109",     0x04000, 0x4000, CRC(7639270c) SHA1(1b8f53c516d26aecb4478ac99783a37e5b1a107f)) /* 8b/c */
	ROM_LOAD( "136026.110",     0x08000, 0x4000, CRC(f3102944) SHA1(460f18180b19b6360c99c7e70f86d745f69ba95d)) /* 7b/c */
	ROM_LOAD( "136026.111",     0x0c000, 0x4000, CRC(8a230bb5) SHA1(0cfa1e981e4a8ccaf5903b4e761a2085b5a56181)) /* 6b/c */
	ROM_LOAD( "136026.101",     0x10000, 0x4000, CRC(91bba45a) SHA1(d584a8f60bbbdbe250978b7aeb3f5e7698f94d60)) /* 8a */
	ROM_LOAD( "136026.102",     0x14000, 0x4000, CRC(5f1e423d) SHA1(c55c27600877272c1ca94eab75c1eb25ff84d36f)) /* 7a */
	/* empty 6a */
	/* empty 5a */
	ROM_LOAD( "136026.105",     0x20000, 0x4000, CRC(83f1d4ed) SHA1(ed4b22b3473f16cbcca1415f6d81be558ab10ff3)) /* 4a */
	ROM_LOAD( "136026.106",     0x24000, 0x4000, CRC(c5d8d417) SHA1(6a29595b2c091bbcf413c7213c6577eaf9c507d1)) /* 3a */
	/* empty 2a */
	/* empty 1a */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for code */
	/* empty 4k/l */
	ROM_LOAD( "136026.113",     0x08000, 0x4000, CRC(90988b3b) SHA1(7571cf6b7e9e3e22f930d9ba991b730e734edfb7)) /* 4m */
	ROM_LOAD( "136026.114",     0x0c000, 0x4000, CRC(1437ce14) SHA1(eef14172b3935a4afb3470852f93d30926b139e4)) /* 4n */

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "136026.125",     0x0000,  0x2000, CRC(8a32f9f1) SHA1(f899174f55cd4a24a3be4a0f4bb44d3e8e938586) ) /* 6p */

	ROM_REGION( 0x30000, "sprites", ROMREGION_ERASE00 )
	/* empty 6c */
	/* empty 6a */
	ROM_LOAD( "136026.124",     0x08000,  0x4000, CRC(5efe0f6c) SHA1(df35fd9267d966ab379c2f78ed418f4606741b28)) /* 5c */
	ROM_LOAD( "136026.123",     0x0c000,  0x4000, CRC(dffe48b3) SHA1(559907651bb425e26a834b467959b15092d23d27)) /* 5a */
	ROM_LOAD( "136026.118",     0x10000,  0x4000, CRC(0ed4df15) SHA1(7aa599f428112fff4bfedf63fafc22f19fa66546)) /* 4c */
	ROM_LOAD( "136026.122",     0x14000,  0x4000, CRC(8e2c6616) SHA1(59cbd585028bb634034a9dfd552275bd41f01989)) /* 4a */
	ROM_LOAD( "136026.117",     0x18000,  0x4000, CRC(79129084) SHA1(4219ff7cd444ad11e4cb9f1c30ac15fe0cfc5a17)) /* 3c */
	ROM_LOAD( "136026.121",     0x1c000,  0x4000, CRC(494972d4) SHA1(fa0e24e911b233e9644d7794ba03f76bfd39aa8c)) /* 3a */
	ROM_LOAD( "136026.116",     0x20000,  0x4000, CRC(d5282d4e) SHA1(de5fdf82a615625aa77b39e035b4206216faaf9c)) /* 2c */
	ROM_LOAD( "136026.120",     0x24000,  0x4000, CRC(e1b95923) SHA1(b6d0c0af0a8f55e728cd0f4c3222745eefd57f50)) /* 2a */
	ROM_LOAD( "136026.115",     0x28000,  0x4000, CRC(861abc82) SHA1(1845888d07162ae915364a2a91294731f1c5b3bd)) /* 1c */
	ROM_LOAD( "136026.119",     0x2c000,  0x4000, CRC(959471b1) SHA1(a032209a209f51d34360d5c7ad32ec62150158d2)) /* 1a */

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "firefox", 0, SHA1(3c4be40f55b44d0352b64c0861b6d1b650451ce7) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, firefox,  0,       firefox, firefox, driver_device, 0, ROT0, "Atari", "Fire Fox (set 1)", 0 )
GAME( 1984, firefoxa, firefox, firefox, firefox, driver_device, 0, ROT0, "Atari", "Fire Fox (set 2)", 0 )
