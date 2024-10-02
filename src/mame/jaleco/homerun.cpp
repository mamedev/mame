// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina

/*
 Moero!! Pro Yakyuu Homerun Kyousou - (c) 1988 Jaleco
 Dynamic Shoot Kyousou - (c) 1988 Jaleco
 Ganbare Jajamaru Saisho wa Goo / Ganbare Jajamaru Hop Step & Jump - (c) 1990 Jaleco
 Driver by Tomasz Slanina

 They're gambling games (seems to be aimed at kids), with a little skill involved.
 All games have a medal hopper to reward the player.

 *weird* hardware - based on NES version
 (gfx bank changed in the middle of screen,
  sprites in NES format etc)

 homerun, dynashot and ganjaja use an extra soundchip for playing voice/samples

Todo :
 - Dump homerun and dynashot sample ROM
 - Improve controls/DIP switches
 - Fix sprite glitches in ganjaja Hop Step & Jump
 - Fix sample playing in ganjaja Saisho wa Goo. The words 'rock', 'paper', scissors' are not played?
 - Fix small gfx glitch on right side of screen in homerun. On the real PCB there is nothing on the right side.
 - Fix coinage; Extra credits are added on the first coin-up so with 5C 1C, only 4 coins are required for a credit.
   After that first credit, coinage works as expected and additional crediting requires the correct number of coins.
   When X-Coins Y-Credits is active (where X=* and Y>1), coinage works as expected.
   When starting the game with 1C 1C active, all games start with 1 credit.


------------------------------------------------------------------------------------

Hardware info by Guru

All these games run on the same PCB.
All games use a NEC uPD7756 for speech.


PCB Layout
----------

JALECO HR-8847 MADE IN JAPAN
|---------------------------------|
|C1230 4558 YM3014 Z80B 6264      |
|VOL 358 YM2203 DSW(8) 1.43       |
|       D7756C      6264          |
|      640kHz                     |
|J                 3.60           |
|A 2018                           |
|M                            8255|
|M          2018                  |
|A                  2018          |
|              2018               |
|                              555|
| 1.120                      20MHz|
|---------------------------------|
Notes:
      Z80    - Clock 5.000MHz [20/4]
      2018   - 2k x8-bit SRAM
      6264   - 8k x8-bit SRAM
      YM2203 - Clock 2.500MHz [20/8]
      YM3014 - Yamaha YM3014 DAC
      D7756  - NEC uPD7756 ADPCM Speech Processor with internal 256k-bit ROM (32k x8-bit). Clock input 640kHz
      358    - NEC C358 Dual Op AMP
      4558   - NEC C4558 Dual Op AMP
      C1230  - NEC uPC1230H 23W Power AMP
      1,2,3  - EPROMs
      VSync  - 59.something. Measured value moves between 59Hz and 60Hz
      HSync  - 15.21kHz

Edge Connector is JAMMA but with some pins re-used for other functions.
Controls are up, down, left, right, button 1 and start.
Coin 1 is JAMMA pin 16
Coin 2 is JAMMA pin 24. Note this is equivalent to JAMMA button 3
For Coin 1 (JAMMA pin 16)...
1C 1C - DSW 1 OFF & DSW 2 OFF
2C 1C - DSW 1 ON & DSW 2 OFF
3C 1C - DSW 1 OFF & DSW 2 ON
5C 1C - DSW 1 ON & DSW 2 ON
For Coin 2 (JAMMA pin 24)
1C 1C - DSW 1 OFF & DSW 2 OFF
2C 1C - DSW 1 ON & DSW 2 OFF
1C 3C - DSW 1 OFF & DSW 2 ON
1C 5C - DSW 1 ON & DSW 2 ON
Above coinage for Coin 1 and Coin 2 is confirmed correct on the real PCB.

JAMMA pin 8 is Hopper Solenoid (5VDC)
JAMMA pin 25 is Hopper Switch (SPST On/Off; when active pin 25 is tied to ground)
Hopper operates on 100VAC through a solid-state relay connected to the Hopper Solenoid signal

The cabinet has a digital number display somewhere because the manual mentions a 2nd chance
given if the digital counter reads 777. It may be on-screen or 3x 7-segment LEDs on
the control panel somewhere or maybe there's a DIP setting to turn it on.

At the title screen the sample played is 'bat hitting ball'.
When the game play starts a voice says 'play ball' but this is not played in the emulation.
When the ball is hit the sample played is 'bat hitting ball', then 'crowd cheers'.
After several strikes in a row the sample played is 'aho'
If a foul ball goes behind and is caught by the crowd the sample that plays is 'yah'.

Homerun reference video: https://www.youtube.com/watch?v=80jtlNKZtEE&t=519
This appears to be a different version with Japanese text for 'credit' and 'game over' and
the team selection screen shows the number of credits in the game.
This version shows the 2nd-chance number countdown on screen that is mentioned in the manual.
777 2nd chance video reference: https://www.youtube.com/watch?v=FBCD2tXG4Jo&t=392
This version of Homerun is not dumped.


------------------------------------------------------------------------------------
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/samples.h"
#include "sound/upd7759.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class homerun_state : public driver_device
{
public:
	homerun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_d7756(*this, "d7756"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_mainbank(*this, "mainbank")
	{ }

	void ganjaja(machine_config &config);
	void dynashot(machine_config &config);
	void homerun(machine_config &config);

	int sprite0_r();
	int homerun_d7756_busy_r();
	ioport_value ganjaja_hopper_status_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_spriteram;
	required_device<upd7756_device> m_d7756;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_memory_bank m_mainbank;

	u8 m_control = 0;
	u8 m_sample = 0;

	tilemap_t *m_tilemap = nullptr;
	u8 m_gfx_ctrl = 0;
	u16 m_scrollx = 0;
	u16 m_scrolly = 0;

	void control_w(u8 data);
	void d7756_sample_w(u8 data);
	void videoram_w(offs_t offset, u8 data);
	void scrollhi_w(u8 data);
	void scrolly_w(u8 data);
	void scrollx_w(u8 data);

	static rgb_t homerun_RGB332(u32 raw);
	TILE_GET_INFO_MEMBER(get_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void banking_w(u8 data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};


/**************************************************************************/

int homerun_state::sprite0_r()
{
	// sprite-0 vs background collision status, similar to NES
	return (m_screen->vpos() > (m_spriteram[0] - 16 + 1)) ? 1 : 0;
}

void homerun_state::scrollhi_w(u8 data)
{
	// d0: scroll y high bit
	// d1: scroll x high bit
	// other bits: ?
	m_scrolly = (m_scrolly & 0xff) | (data << 8 & 0x100);
	m_scrollx = (m_scrollx & 0xff) | (data << 7 & 0x100);
}

void homerun_state::scrolly_w(u8 data)
{
	m_scrolly = (m_scrolly & 0xff00) | data;
}

void homerun_state::scrollx_w(u8 data)
{
	m_scrollx = (m_scrollx & 0xff00) | data;
}

void homerun_state::banking_w(u8 data)
{
	u8 const old = m_gfx_ctrl;
	if (old ^ data)
	{
		if ((old ^ data) & 3)
		{
			// games do mid-screen gfx bank switching
			int vpos = m_screen->vpos();
			m_screen->update_partial(vpos);
		}

		// d0-d1: gfx bank
		// d2-d4: ?
		// d5-d7: prg bank
		m_gfx_ctrl = data;
		if ((old ^ m_gfx_ctrl) & 1)
			m_tilemap->mark_all_dirty();

		if ((old ^ m_gfx_ctrl) >> 5 & 7)
			m_mainbank->set_entry(m_gfx_ctrl >> 5 & 7);

	}
}

void homerun_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0xfff);
}

rgb_t homerun_state::homerun_RGB332(u32 raw)
{
	/* from PCB photo:
	    bit 7:  470 ohm resistor \
	    bit 6:  220 ohm resistor -  --> 470 ohm resistor  --> blue
	    bit 5:  470 ohm resistor \
	    bit 4:  220 ohm resistor -  --> 470 ohm resistor  --> green
	    bit 3:  1  kohm resistor /
	    bit 2:  470 ohm resistor \
	    bit 1:  220 ohm resistor -  --> 470 ohm resistor  --> red
	    bit 0:  1  kohm resistor /
	*/

	// let's implement it the old fashioned way until it's found out how exactly the resnet is hooked up
	u8 bit0, bit1, bit2;

	bit0 = (raw >> 0) & 0x01;
	bit1 = (raw >> 1) & 0x01;
	bit2 = (raw >> 2) & 0x01;
	int r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = (raw >> 3) & 0x01;
	bit1 = (raw >> 4) & 0x01;
	bit2 = (raw >> 5) & 0x01;
	int g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = 0;
	bit1 = (raw >> 6) & 0x01;
	bit2 = (raw >> 7) & 0x01;
	int b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	return rgb_t(r, g, b);
}

/**************************************************************************/

TILE_GET_INFO_MEMBER(homerun_state::get_tile_info)
{
	u32 const tileno = (m_videoram[tile_index]) | ((m_videoram[tile_index | 0x1000] & 0x38) << 5) | ((m_gfx_ctrl & 1) << 11);
	u16 const palno = (m_videoram[tile_index | 0x1000] & 0x07);

	tileinfo.set(0, tileno, palno, 0);
}


void homerun_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(homerun_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	save_item(NAME(m_gfx_ctrl));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_scrollx));
}


void homerun_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		if (m_spriteram[offs + 0] == 0)
			continue;

		int const sy     = m_spriteram[offs + 0] - 16 + 1;
		int const sx     = m_spriteram[offs + 3];
		u32 const code   = (m_spriteram[offs + 1]) | ((m_spriteram[offs + 2] & 0x8) << 5) | ((m_gfx_ctrl & 3) << 9);
		u32 const color  = (m_spriteram[offs + 2] & 0x07) | 8;
		bool const flipx = (m_spriteram[offs + 2] & 0x40) >> 6;
		bool const flipy = (m_spriteram[offs + 2] & 0x80) >> 7;

		if (sy >= 0)
		{
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);

			// wraparound x
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx - 256 , sy, 0);
		}
	}
}

u32 homerun_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->set_scrolly(0, m_scrolly);
	m_tilemap->set_scrollx(0, m_scrollx);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}


/***************************************************************************

  I/O / Memory

***************************************************************************/

void homerun_state::control_w(u8 data)
{
	// d0, d1: somehow related to port $40?

	// d4: d7756 start pin
	// d5: d7756 reset pin(?)
	if (m_d7756 != nullptr)
	{
		m_d7756->reset_w(!BIT(data, 5));
		m_d7756->start_w(!BIT(data, 4));
	}
	if (m_samples != nullptr)
	{
		// play MAME sample if a dump of the internal ROM does not exist
		if (data & 0x20 & ~m_control)
			m_samples->stop(0);

		if (~data & 0x10 & m_control && !m_samples->playing(0))
		{
			samples_iterator iter(*m_samples);
			if (m_sample < iter.count())
				m_samples->start(0, m_sample);
		}
	}

	// other bits: ?
	m_control = data;
}

void homerun_state::d7756_sample_w(u8 data)
{
	m_sample = data;

	if (m_d7756 != nullptr)
		m_d7756->port_w(data);
}

void homerun_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0x9fff).ram().w(FUNC(homerun_state::videoram_w)).share(m_videoram);
	map(0xa000, 0xa0ff).ram().share(m_spriteram);
	map(0xb000, 0xb03f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xc000, 0xdfff).ram();
}

void homerun_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w(FUNC(homerun_state::d7756_sample_w));
	map(0x20, 0x20).w(FUNC(homerun_state::control_w));
	map(0x30, 0x33).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x40).portr("IN0");
	map(0x50, 0x50).portr("IN2");
	map(0x60, 0x60).portr("IN1");
	map(0x70, 0x71).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


int homerun_state::homerun_d7756_busy_r()
{
	return m_samples->playing(0) ? 0 : 1;
}

ioport_value homerun_state::ganjaja_hopper_status_r()
{
	// gives hopper error if not 0
	return 0;
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( homerun )
	PORT_START("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(homerun_state, sprite0_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(homerun_state, homerun_d7756_busy_r)
	PORT_BIT( 0x37, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )         PORT_DIPLOCATION("DIPSW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )  // game boots with 2 permanent credits which is correct
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DIPSW:3" )  // manual shows blank so assumed to be unused
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )         PORT_DIPLOCATION("DIPSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )         PORT_DIPLOCATION("DIPSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )         PORT_DIPLOCATION("DIPSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )         PORT_DIPLOCATION("DIPSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )          PORT_DIPLOCATION("DIPSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )

// The manual shows the following DIPs but they don't appear to do anything
// so this could be for a different version which is not dumped.
//..PORT_DIPNAME( 0x70, 0x70, "Difficulty" )               PORT_DIPLOCATION("DIPSW:5,6,7")
//..PORT_DIPSETTING(    0x70, "1" )
//..PORT_DIPSETTING(    0x10, "2" )
//..PORT_DIPSETTING(    0x20, "3" )  // manual only shows difficulty 1-5 but using 3 DIP switches
//..PORT_DIPSETTING(    0x30, "4" )
//..PORT_DIPSETTING(    0x40, "5" )
//..PORT_DIPSETTING(    0x50, "?" )
//..PORT_DIPSETTING(    0x60, "?" )
//..PORT_DIPSETTING(    0x00, "?" )
//..PORT_DIPNAME( 0x88, 0x88, "Payout" )                   PORT_DIPLOCATION("DIPSW:8,4")
//..PORT_DIPSETTING(    0x88, "1 Medal" )    // after number is a Japanese symbol 枚 which is used for counting flat thin objects
//..PORT_DIPSETTING(    0x08, "2 Medals" )   // so it must be referring to 'medals' since the wiring diagram shows a 'medal hopper'
//..PORT_DIPSETTING(    0x80, "3 Medals" )
//..PORT_DIPSETTING(    0x00, "Unused" )

INPUT_PORTS_END

static INPUT_PORTS_START( dynashot )
	PORT_START("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(homerun_state, sprite0_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  // ... actually does have a D7756 on the PCB
	PORT_BIT( 0x37, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xdf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )          PORT_DIPLOCATION("DIPSW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )  // game boots with 1 credit inserted - wrong
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DIPSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DIPSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DIPSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIPSW:6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )          PORT_DIPLOCATION("DIPSW:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )  // game boots with 1 credit inserted - wrong
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ganjaja )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(homerun_state, sprite0_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("d7756", upd7756_device, busy_r)
	PORT_BIT( 0x36, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_NAME("P1 Up / Rock")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_NAME("P1 Down / Paper")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Right / Scissors")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) // unused?
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(homerun_state, ganjaja_hopper_status_r)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // bit masked with IN0 IPT_COIN1, maybe coin lockout?
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xcf, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	// Starts game with coin in if 1C_1C
	// With 2C_1C only 1 coin is needed to start the game because 1 extra credit is incorrectly given
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DIPSW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) ) // game boots with 1 credit inserted - wrong
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DIPSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DIPSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DIPSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DIPSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIPSW:6" ) // chance to win?
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIPSW:7" ) // "
	PORT_DIPNAME( 0x80, 0x80, "Game" )                  PORT_DIPLOCATION("DIPSW:8")
	PORT_DIPSETTING(    0x80, "Saisho wa Goo" )
	PORT_DIPSETTING(    0x00, "Hop Step & Jump" )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

// homerun samples, taken from the Famicom version of Moero!! Pro Yakyuu
// note that this is the complete ROM contents; not all samples are used in this game
static const char *const homerun_sample_names[] =
{
	"*homerun",
	"00", // strike (not used)
	"01", // ball (not used)
	"02", // time (ask for time out) (not used)
	"03", // out (not used)
	"04", // safe (not used)
	"05", // foul (not used)
	"06", // yah (person in crowd catching a foul ball going behind)(used)
	"07", // batter out (batter out after 3 strikes) (not used)
	"08", // play ball (based on video reference this should be played but isn't)
	"09", // ball four (not used)
	"10", // home run (used)
	"11", // new pitcher (choosing new pitcher in time out) (not used)
	"12", // ouch (batter gets hit by pitcher) (not used)
	"13", // aho (be called a fool by supervisor)(used)
	"14", // bat hits ball(used)
	"15", // crowd cheers(used)
	nullptr
};

/**************************************************************************/

static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 8*8,0},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8*2
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 8*8,0 },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8), STEP8(8*8*2*2,8) },
	8*8*2*4
};

static GFXDECODE_START( gfx_homerun )
	GFXDECODE_ENTRY( "tiles",   0, gfxlayout,    0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0, 16 )
GFXDECODE_END


/**************************************************************************/

void homerun_state::machine_start()
{
	u8 *ROM = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 8, &ROM[0x00000], 0x4000);

	save_item(NAME(m_control));
	save_item(NAME(m_sample));
}

void homerun_state::machine_reset()
{
	control_w(0);
	d7756_sample_w(0);
	banking_w(0);
	m_scrolly = 0;
	m_scrollx = 0;
}

/**************************************************************************/

void homerun_state::dynashot(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(20'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &homerun_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &homerun_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(homerun_state::irq0_line_hold));

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.out_pa_callback().set(FUNC(homerun_state::scrollhi_w));
	ppi.out_pb_callback().set(FUNC(homerun_state::scrolly_w));
	ppi.out_pc_callback().set(FUNC(homerun_state::scrollx_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(20'000'000) / 4, 328, 0, 256, 253, 0, 240);
	m_screen->set_screen_update(FUNC(homerun_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_homerun);
	PALETTE(config, m_palette).set_format(1, &homerun_state::homerun_RGB332, 16 * 4);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(20'000'000) / 8));
	ymsnd.port_a_read_callback().set_ioport("DSW");
	ymsnd.port_b_write_callback().set(FUNC(homerun_state::banking_w));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.50);

	UPD7756(config, m_d7756);
	m_d7756->add_route(ALL_OUTPUTS, "mono", 0.75);
}

void homerun_state::homerun(machine_config &config)
{
	dynashot(config);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(homerun_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void homerun_state::ganjaja(machine_config &config)
{
	dynashot(config);

	// basic machine hardware
	m_maincpu->set_periodic_int(FUNC(homerun_state::irq0_line_hold), attotime::from_hz(4 * 60)); // ?
}



/**************************************************************************/

ROM_START( homerun )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "homerun.ic43",    0x00000, 0x20000, CRC(e759e476) SHA1(ad4f356ff26209033320a3e6353e4d4d9beb59c1) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "homerun.ic60",    0x00000, 0x10000, CRC(69a720d1) SHA1(0f0a4877578f358e9e829ece8c31e23f01adcf83) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "homerun.ic120",   0x00000, 0x20000, CRC(52f0709b) SHA1(19e675bcccadb774f60ec5929fc1fb5cf0d3f617) )

	ROM_REGION( 0x08000, "d7756", ROMREGION_ERASE00 )
	ROM_LOAD( "d7756c_146.ic98", 0x00000, 0x08000, NO_DUMP ) // D7756C built-in rom - same maskrom serial as Moero!! Pro Yakyuu (Black/Red) on Famicom
ROM_END

ROM_START( nhomerun )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",   0x00000, 0x20000, CRC(aed96d6d) SHA1(5cb3932f4cfa3f6c0134ac20a1747c562db31a65) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "3.ic60",   0x00000, 0x10000, CRC(69a720d1) SHA1(0f0a4877578f358e9e829ece8c31e23f01adcf83) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "2.ic120",  0x00000, 0x20000, CRC(57e9b757) SHA1(8190d690721005407a5b06d13d64e70301d1e925) )

	ROM_REGION( 0x08000, "d7756", ROMREGION_ERASE00 )
	ROM_LOAD( "d7756c_146.ic98", 0x00000, 0x08000, NO_DUMP )
ROM_END

ROM_START( dynashot )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",          0x00000, 0x20000, CRC(bf3c9586) SHA1(439effbda305f5fa265e5897c81dc1447e5d867d) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "3.ic60",          0x00000, 0x10000, CRC(77d6a608) SHA1(a31ff343a5d4d6f20301c030ecc2e252149bcf9d) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "2.ic120",         0x00000, 0x20000, CRC(bedf7b98) SHA1(cb6c5fcaf8df5f5c7636c3c8f79b9dda78e30c2e) )

	ROM_REGION( 0x08000, "d7756", ROMREGION_ERASE00 )
	ROM_LOAD( "d7756c_146.ic98", 0x00000, 0x08000, NO_DUMP ) // unused?
ROM_END


ROM_START( ganjaja )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.ic43",         0x00000, 0x20000, CRC(dad57543) SHA1(dbd8b5cee33756ee5e3c41bf84c0f7141d3466dc) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "ic60",           0x00000, 0x10000, CRC(855f6b28) SHA1(386411e88cf9bed54fe2073f0828d579cb1d04ee) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "2.ic120",        0x00000, 0x20000, CRC(e65d4d57) SHA1(2ec9e5bdaa94b808573313b6eca657d798004b53) )

	ROM_REGION( 0x08000, "d7756", 0 )
	ROM_LOAD( "d77p56cr.ic98",  0x00000, 0x08000, CRC(06a234ac) SHA1(b4ceff3f9f78551cf4a085642e162e33b266f067) ) // D77P56CR OTP rom (One-Time Programmable, note the extra P)
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT    MACHINE   INPUT     STATE          INIT        ROT    COMPANY   FULLNAME                                                            FLAGS
GAME( 1988, nhomerun, 0,        homerun,  homerun,  homerun_state, empty_init, ROT0, "Jaleco", "NEW Moero!! Pro Yakyuu Homerun Kyousou",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // same as below but harder?
GAME( 1988, homerun,  nhomerun, homerun,  homerun,  homerun_state, empty_init, ROT0, "Jaleco", "Moero!! Pro Yakyuu Homerun Kyousou",                                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1988, dynashot, 0,        dynashot, dynashot, homerun_state, empty_init, ROT0, "Jaleco", "Dynamic Shoot Kyousou",                                             MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1990, ganjaja,  0,        ganjaja,  ganjaja,  homerun_state, empty_init, ROT0, "Jaleco", "Ganbare Jajamaru Saisho wa Goo / Ganbare Jajamaru Hop Step & Jump", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
