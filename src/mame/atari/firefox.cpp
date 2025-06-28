// license:BSD-3-Clause
// copyright-holders:smf, Aaron Giles, Chris Hardy
// thanks-to:Scott Waye
/***************************************************************************

    Atari Fire Fox hardware

    driver by smf, Aaron Giles, Chris Hardy & Scott Waye

TODO:
- add option to centre joystick to enter test menu


it uses a quad pokey package 137323-1221-406???
the laser disc is a philips lvp 22vp931
(but maybe this works too... Special Drive: Laser Disc Player - Philips VP-832A)


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
#include "machine/74259.h"
#include "machine/adc0808.h"
#include "machine/ldvp931.h"
#include "machine/gen_latch.h"
#include "machine/mos6530.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "machine/x2212.h"

#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class firefox_state : public driver_device
{
public:
	firefox_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_laserdisc(*this, "laserdisc") ,
		m_tileram(*this, "tileram"),
		m_spriteram(*this, "spriteram"),
		m_sprite_palette(*this, "sprite_palette"),
		m_tile_palette(*this, "tile_palette"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_riot(*this, "riot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		soundlatch(*this, "soundlatch%u", 1U),
		m_pokey(*this, "pokey%u", 1U),
		m_tms(*this, "tms"),
		m_nvram_1c(*this, "nvram_1c"),
		m_nvram_1d(*this, "nvram_1d"),
		m_mainbank(*this, "mainbank")
	{ }

	void firefox(machine_config &config);

private:
	uint8_t firefox_disc_status_r();
	uint8_t firefox_disc_data_r();
	void firefox_disc_read_w(uint8_t data);
	void firefox_disc_lock_w(int state);
	void audio_enable_left_w(int state);
	void audio_enable_right_w(int state);
	void firefox_disc_reset_w(int state);
	void firefox_disc_write_w(int state);
	void firefox_disc_data_w(uint8_t data);
	void tileram_w(offs_t offset, uint8_t data);
	void tile_palette_w(offs_t offset, uint8_t data);
	void sprite_palette_w(offs_t offset, uint8_t data);
	void firefox_objram_bank_w(uint8_t data);
	void sound_reset_w(int state);
	uint8_t adc_r();
	void adc_select_w(uint8_t data);
	void nvram_w(offs_t offset, uint8_t data);
	uint8_t nvram_r(address_space &space, offs_t offset);
	void rom_bank_w(uint8_t data);
	void main_irq_clear_w(uint8_t data);
	void main_firq_clear_w(uint8_t data);
	void self_reset_w(uint8_t data);
	void coin_counter_right_w(int state);
	void coin_counter_left_w(int state);
	TILE_GET_INFO_MEMBER(bgtile_get_info);
	uint32_t screen_update_firefox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(video_timer_callback);
	void set_rgba( int start, int index, unsigned char *palette_ram );
	void firq_gen(philips_22vp931_device &laserdisc, int state);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	required_device<philips_22vp931_device> m_laserdisc;
	required_shared_ptr<unsigned char> m_tileram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<unsigned char> m_sprite_palette;
	required_shared_ptr<unsigned char> m_tile_palette;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<mos6532_device> m_riot;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<generic_latch_8_device, 2> soundlatch;
	required_device_array<pokey_device, 4> m_pokey;
	required_device<tms5220_device> m_tms;
	required_device<x2212_device> m_nvram_1c;
	required_device<x2212_device> m_nvram_1d;

	required_memory_bank m_mainbank;

	int m_n_disc_lock = 0;
	int m_n_disc_data = 0;
	int m_n_disc_read_data = 0;
	tilemap_t *m_bgtiles;
	int m_sprite_bank = 0;
};



#define MASTER_XTAL     XTAL(14'318'181)


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
uint8_t firefox_state::firefox_disc_status_r()
{
	uint8_t result = 0xff;

	result ^= 0x20;
	if (!m_laserdisc->ready_r())
		result ^= 0x40;
	if (m_laserdisc->data_available_r())
		result ^= 0x80;

	return result;
}

/* 4105 - DREAD */
/* this reset RDDSK (&DSKRD) */
uint8_t firefox_state::firefox_disc_data_r()
{
	return m_n_disc_read_data;
}

/* DISK READ ENABLE */
/* 4218 - DSKREAD, set RDDSK */
void firefox_state::firefox_disc_read_w(uint8_t data)
{
	m_n_disc_read_data = m_laserdisc->data_r();
}

void firefox_state::firefox_disc_lock_w(int state)
{
	m_n_disc_lock = state;
}

void firefox_state::audio_enable_left_w(int state)
{
	m_laserdisc->set_output_gain(0, state ? 1.0 : 0.0);
}

void firefox_state::audio_enable_right_w(int state)
{
	m_laserdisc->set_output_gain(1, state ? 1.0 : 0.0);
}

void firefox_state::firefox_disc_reset_w(int state)
{
	m_laserdisc->reset_w(state ? CLEAR_LINE : ASSERT_LINE);
}

/* active low on dbb7 */
void firefox_state::firefox_disc_write_w(int state)
{
	if (state == 0)
		m_laserdisc->data_w(m_n_disc_data);
}

/* latch the data */
void firefox_state::firefox_disc_data_w(uint8_t data)
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
	tileinfo.set(0, m_tileram[tile_index], 0, 0);
}


void firefox_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_bgtiles->mark_tile_dirty(offset);
}


void firefox_state::video_start()
{
	m_bgtiles = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(firefox_state::bgtile_get_info)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_bgtiles->set_transparent_pen(0);
	m_bgtiles->set_scrolldy(m_screen->visible_area().top(), 0);
}


uint32_t firefox_state::screen_update_firefox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int gfxtop = screen.visible_area().top();

	bitmap.fill(m_palette->pen_color(256), cliprect);

	for (int sprite = 0; sprite < 32; sprite++)
	{
		uint8_t *sprite_data = m_spriteram + (0x200 * m_sprite_bank) + (sprite * 16);
		int flags = sprite_data[0];
		int y = sprite_data[1] + (256 * ((flags >> 0) & 1));
		int x = sprite_data[2] + (256 * ((flags >> 1) & 1));

		if (x != 0)
		{
			for (int row = 0; row < 8; row++)
			{
				int color = (flags >> 2) & 0x03;
				int flipy = flags & 0x10;
				int flipx = flags & 0x20;
				int code = sprite_data[15 - row] + (256 * ((flags >> 6) & 3));

				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, x + 8, gfxtop + 500 - y - (row * 16), 0);
			}
		}
	}

	m_bgtiles->draw(screen, bitmap, cliprect, 0, 0);

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

void firefox_state::tile_palette_w(offs_t offset, uint8_t data)
{
	m_tile_palette[ offset ] = data;
	set_rgba( 0, offset & 0xff, m_tile_palette );
}

void firefox_state::sprite_palette_w(offs_t offset, uint8_t data)
{
	m_sprite_palette[ offset ] = data;
	set_rgba( 256, offset & 0xff, m_sprite_palette );
}

void firefox_state::firefox_objram_bank_w(uint8_t data)
{
	m_sprite_bank = data & 0x03;
}


/*************************************
 *
 *  Main <-> sound communication
 *
 *************************************/

void firefox_state::sound_reset_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
	if (state != 0)
	{
		soundlatch[0]->reset();
		soundlatch[1]->reset();
	}
}


/*************************************
 *
 *  Non-Volatile RAM (NOVRAM)
 *
 *************************************/

void firefox_state::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram_1c->write(offset, data >> 4);
	m_nvram_1d->write(offset, data & 0xf);
}

uint8_t firefox_state::nvram_r(address_space &space, offs_t offset)
{
	return (m_nvram_1c->read(space, offset) << 4) | (m_nvram_1d->read(space, offset) & 0x0f);
}


/*************************************
 *
 *  Main cpu
 *
 *************************************/

void firefox_state::rom_bank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x1f);
}

void firefox_state::main_irq_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE );
}

void firefox_state::main_firq_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE );
}

void firefox_state::self_reset_w(uint8_t data)
{
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


/*************************************
 *
 *  I/O
 *
 *************************************/

void firefox_state::coin_counter_right_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void firefox_state::coin_counter_left_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


void firefox_state::firq_gen(philips_22vp931_device &laserdisc, int state)
{
	if (state)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE );
}


void firefox_state::machine_start()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x1000);

	m_laserdisc->set_data_ready_callback(philips_22vp931_device::data_ready_delegate(&firefox_state::firq_gen, this));
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void firefox_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().w(FUNC(firefox_state::tileram_w)).share("tileram");
	map(0x2000, 0x27ff).ram().share("spriteram");
	map(0x2800, 0x2aff).ram().w(FUNC(firefox_state::sprite_palette_w)).share("sprite_palette");
	map(0x2b00, 0x2b00).mirror(0x04ff).w(FUNC(firefox_state::firefox_objram_bank_w));
	map(0x2c00, 0x2eff).ram().w(FUNC(firefox_state::tile_palette_w)).share("tile_palette");
	map(0x3000, 0x3fff).bankr("mainbank");
	map(0x4000, 0x40ff).rw(FUNC(firefox_state::nvram_r), FUNC(firefox_state::nvram_w));                     /* NOVRAM */
	map(0x4100, 0x4100).mirror(0x00f8).portr("rdin0");            /* RDIN0 */
	map(0x4101, 0x4101).mirror(0x00f8).portr("rdin1");            /* RDIN1 */
	map(0x4102, 0x4102).mirror(0x00f8).r(FUNC(firefox_state::firefox_disc_status_r));   /* RDIN2 */
	map(0x4103, 0x4103).mirror(0x00f8).portr("opt0");             /* OPT0 */
	map(0x4104, 0x4104).mirror(0x00f8).portr("opt1");             /* OPT1 */
	map(0x4105, 0x4105).mirror(0x00f8).r(FUNC(firefox_state::firefox_disc_data_r));     /* DREAD */
	map(0x4106, 0x4106).mirror(0x00f8).r(soundlatch[1], FUNC(generic_latch_8_device::read));         /* RDSOUND */
	map(0x4107, 0x4107).mirror(0x00f8).r("adc", FUNC(adc0808_device::data_r));               /* ADC */
	map(0x4200, 0x4200).mirror(0x0047).w(FUNC(firefox_state::main_irq_clear_w));       /* RSTIRQ */
	map(0x4208, 0x4208).mirror(0x0047).w(FUNC(firefox_state::main_firq_clear_w));      /* RSTFIRQ */
	map(0x4210, 0x4210).mirror(0x0047).w("watchdog", FUNC(watchdog_timer_device::reset_w));       /* WDCLK */
	map(0x4218, 0x4218).mirror(0x0047).w(FUNC(firefox_state::firefox_disc_read_w));    /* DSKREAD */
	map(0x4220, 0x4223).mirror(0x0044).w("adc", FUNC(adc0808_device::address_offset_start_w));       /* ADCSTART */
	map(0x4230, 0x4230).mirror(0x0047).w(FUNC(firefox_state::self_reset_w));           /* AMUCK */
	map(0x4280, 0x4287).mirror(0x0040).w("latch0", FUNC(ls259_device::write_d7));
	map(0x4288, 0x428f).mirror(0x0040).w("latch1", FUNC(ls259_device::write_d7));
	map(0x4290, 0x4290).mirror(0x0047).w(FUNC(firefox_state::rom_bank_w));             /* WRTREG */
	map(0x4298, 0x4298).mirror(0x0047).w(soundlatch[0], FUNC(generic_latch_8_device::write));        /* WRSOUND */
	map(0x42a0, 0x42a0).mirror(0x0047).w(FUNC(firefox_state::firefox_disc_data_w));    /* DSKLATCH */
	map(0x4400, 0xffff).rom();
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void firefox_state::audio_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x087f).mirror(0x0700).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x0880, 0x089f).mirror(0x0760).m(m_riot, FUNC(mos6532_device::io_map));
	map(0x1000, 0x1000).r(soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0x1800, 0x1800).w(soundlatch[1], FUNC(generic_latch_8_device::write));
	map(0x2000, 0x200f).rw(m_pokey[0], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x2800, 0x280f).rw(m_pokey[1], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x3000, 0x300f).rw(m_pokey[2], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x3800, 0x380f).rw(m_pokey[3], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x8000, 0xffff).rom();
}



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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch1", FUNC(generic_latch_8_device::pending_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch2", FUNC(generic_latch_8_device::pending_r))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Diagnostic Step") // mentioned in schematics, but N/C?
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

static GFXDECODE_START( gfx_firefox )
	GFXDECODE_ENTRY("tiles",   0, gfx_8x8x4_packed_msb,   0,   1)
	GFXDECODE_ENTRY("sprites", 0, spritelayout,           256, 4)
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void firefox_state::firefox(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, MASTER_XTAL/8); // 68B09E
	m_maincpu->set_addrmap(AS_PROGRAM, &firefox_state::main_map);
	/* interrupts count starting at end of VBLANK, which is 44, so add 44 */
	TIMER(config, "32v").configure_scanline(FUNC(firefox_state::video_timer_callback), "screen", 96+44, 128);

	M6502(config, m_audiocpu, MASTER_XTAL/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &firefox_state::audio_map);

	config.set_maximum_quantum(attotime::from_hz(60000));

	adc0809_device &adc(ADC0809(config, "adc", MASTER_XTAL/16)); // nominally 900 kHz
	adc.in_callback<0>().set_ioport("PITCH");
	adc.in_callback<1>().set_ioport("YAW");

	ls259_device &latch0(LS259(config, "latch0")); // 7F
	latch0.q_out_cb<0>().set(m_nvram_1c, FUNC(x2212_device::recall));      // NVRECALL
	latch0.q_out_cb<0>().append(m_nvram_1d, FUNC(x2212_device::recall));
	latch0.q_out_cb<1>().set(FUNC(firefox_state::sound_reset_w));          // RSTSOUND
	latch0.q_out_cb<2>().set(m_nvram_1c, FUNC(x2212_device::store));       // NVRSTORE
	latch0.q_out_cb<2>().append(m_nvram_1d, FUNC(x2212_device::store));
	latch0.q_out_cb<3>().set(FUNC(firefox_state::firefox_disc_lock_w));    // LOCK
	latch0.q_out_cb<4>().set(FUNC(firefox_state::audio_enable_right_w));   // SWDSKR
	latch0.q_out_cb<5>().set(FUNC(firefox_state::audio_enable_left_w));    // SWDSKL
	latch0.q_out_cb<6>().set(FUNC(firefox_state::firefox_disc_reset_w));   // RSTDSK
	latch0.q_out_cb<7>().set(FUNC(firefox_state::firefox_disc_write_w));   // WRDSK

	ls259_device &latch1(LS259(config, "latch1")); // 1F
	latch1.q_out_cb<0>().set(FUNC(firefox_state::coin_counter_right_w));   // COIN COUNTERR
	latch1.q_out_cb<1>().set(FUNC(firefox_state::coin_counter_left_w));    // COIN COUNTERL
	latch1.q_out_cb<4>().set_output("led0").invert();
	latch1.q_out_cb<5>().set_output("led1").invert();
	latch1.q_out_cb<6>().set_output("led2").invert();
	latch1.q_out_cb<7>().set_output("led3").invert();

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_hz(MASTER_XTAL/8/16/16/16/16));

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_firefox);
	PALETTE(config, m_palette).set_entries(512);

	PHILIPS_22VP931(config, m_laserdisc, 0);
	m_laserdisc->set_overlay(64*8, 525, FUNC(firefox_state::screen_update_firefox));
	m_laserdisc->set_overlay_clip(7*8, 53*8-1, 44, 480+44);
	m_laserdisc->add_route(0, "speaker", 0.50, 0);
	m_laserdisc->add_route(1, "speaker", 0.50, 1);
	m_laserdisc->add_ntsc_screen(config, "screen");

	X2212(config, "nvram_1c").set_auto_save(true);
	X2212(config, "nvram_1d").set_auto_save(true);

	MOS6532(config, m_riot, MASTER_XTAL/8);
	m_riot->pa_wr_callback<0>().set(m_tms, FUNC(tms5220_device::wsq_w));
	m_riot->pa_wr_callback<1>().set(m_tms, FUNC(tms5220_device::rsq_w));
	m_riot->pa_rd_callback<2>().set(m_tms, FUNC(tms5220_device::readyq_r));
	m_riot->pa_rd_callback<4>().set_constant(1); // TEST
	m_riot->pa_wr_callback<5>().set_nop(); // TMS5220 VDD
	m_riot->pb_rd_callback().set(m_tms, FUNC(tms5220_device::status_r));
	m_riot->pb_wr_callback().set(m_tms, FUNC(tms5220_device::data_w));
	m_riot->irq_wr_callback().set_inputline(m_audiocpu, M6502_IRQ_LINE);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, soundlatch[0]);
	soundlatch[0]->data_pending_callback().set(m_riot, FUNC(mos6532_device::pa_bit_w<7>)); // MAINFLAG
	soundlatch[0]->data_pending_callback().append_inputline(m_audiocpu, INPUT_LINE_NMI);
	soundlatch[0]->data_pending_callback().append([this](int state) { if (state) machine().scheduler().perfect_quantum(attotime::from_usec(100)); });

	GENERIC_LATCH_8(config, soundlatch[1]);
	soundlatch[1]->data_pending_callback().set(m_riot, FUNC(mos6532_device::pa_bit_w<6>)); // SOUNDFLAG

	for (int i = 0; i < 4; i++)
	{
		POKEY(config, m_pokey[i], MASTER_XTAL/8);
		m_pokey[i]->add_route(ALL_OUTPUTS, "speaker", 0.30, 0);
		m_pokey[i]->add_route(ALL_OUTPUTS, "speaker", 0.30, 1);
	}

	TMS5220(config, m_tms, MASTER_XTAL/2/11);
	m_tms->add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	m_tms->add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
}



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

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, firefox,  0,       firefox, firefox, firefox_state, empty_init, ROT0, "Atari", "Fire Fox (set 1)", 0 )
GAME( 1984, firefoxa, firefox, firefox, firefox, firefox_state, empty_init, ROT0, "Atari", "Fire Fox (set 2)", 0 )
