// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Tomasz Slanina
/********************************************************************

Enigma 2 (C) Zilec Electronics / Game Plan

driver by Pierpaolo Prazzoli and Tomasz Slanina


enigma2 (1981)
 2xZ80 + AY8910
 Original dedicated board

enigma2a (1984?)
 Conversion applied to a Taito Space Invaders Part II board set with 1984 copyright. hack / Bootleg ?

enigma2b (1981)
 Conversion like enigma2a, but boots with 1981 copyright and Phantoms II title

TODO:
 - enigma2  - Star blinking frequency
 - enigma2  - Sound is incorrectly emulated (see MT07777)
 - enigma2a + enigma2b - bad sound ROM?


*********************************************************************
Enigma II, Game Plan, 1981
PCB hardware notes by Guru

GAME PLAN (C) 1981
|------------------------------------------------------------------------------------------------------------|
|     18   17   16    15    14    13    12   11    10    9     8     7     6     5     4     3    2     1    |
|   LS157 LS32 LS283 LS86  LS86  LS86  LS86       LS27  LS32  LS02  LS161 LS161 LS161 LS161 LS74 LS107 LS04 A|
|                                                                                                       10MHz|
|                                                                                            |---|           |
|   LS165 LS10 LS157 LS157 LS157 LS157 LS42 2114  2114  2114  2114  2114  2114  2114  2114   |   |          B|
|                                                                                            |Z80|     LS161 |
|                                                                                            |   |           |
|         LS08                              2114  2114  2114  2114  2114  2114  2114  2114   |   |          C|
|   LS165                                                                                    |---|     LS161 |
|                                                                                                            |
|         LS74                      6.13D   5.11D   4.10D    3.8D     2.7D     1.5D   LS138   LS244    LS04 D|
|             LS245                                                                                          |
|                                                                                                            |
|         LS42                                                                |---|                    LS32 E|
|                                                                  DIPSW(8)   | A |                          |
|                                                                             | Y |              9.2F        |
|2  LS240 LS21    LS04    LS08      8.13F   7.11F                  LS244      | 3 |                         F|
|2                                                                            | 8 |              2114        |
|W                                                  LS175    LS04             | 9 |                          |
|A  LS240            LS07  LS08  LS157 LS08 LS174                             | 1 |              2114       G|
|Y                                                                            | 0 |                    LS42  |
|                                                                             |---|          |----------|    |
|                                                                           LS164            |   Z80    |   H|
|        LM380N        VOLUME                                      LS02     4040             |----------|    |
|                                                                                                            |
|------------------------------------------------------------------------------------------------------------|
Notes: (all IC's shown)
           Z80 - Clock 2.5MHz (both) [10/4]
      AY3-8910 - Clock 1.25MHz [10/8]
        LM380N - 2.5w Audio Power Amp IC (DIP14)
          2114 - 1kx4-bit SRAM
  1.xx to 8.xx - 2716 EPROM
          9.2F - 2532 EPROM
         HSync - 15.0539kHz
         VSync - measured between 52Hz and 53Hz (moving)


Edge Connector
--------------

             Parts   Solder
             ------|------
             +5V 1 | A +5V
             +5V 2 | B +5V
               - 3 | C -
       2P THRUST 4 | D -
               - 5 | E -
               - 6 | F 2P RIGHT
         2P LEFT 7 | H 2P FIRE
       1P THRUST 8 | J 1P START
        2P START 9 | K COIN
              - 10 | L 1P RIGHT
        1P LEFT 11 | M 1P FIRE
           BLUE 12 | N GREEN
           SYNC 13 | P RED
COMPOSITE VIDEO 14 | R -
              - 15 | S -
              - 16 | T -
           SLAM 17 | U -
              - 18 | V -
       SPEAKER+ 19 | W SPEAKER-
           +12V 20 | X +12V
            GND 21 | Y GND
            GND 22 | Z GND

DIP Switches
------------

+-----------------+---+---+---+---+---+---+---+---+
| Default=*       | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
+-----------------+---+---+---+---+---+---+---+---+
|Coinage   1C 1P* |           |ON |               |
|          2C 1P  |           |OFF|               |
+-----------------+-------+---+---+---------------+
|Cabinet Upright* |       |ON |                   |
|        Cocktail |       |OFF|                   |
+-----------------+---+---+---+-------------------+
|Lives         3* |ON |ON |                       |
|              4  |OFF|ON |                       |
|              5  |ON |OFF|                       |
|              6  |OFF|OFF|                       |
+-----------------+---+---+-----------+---+---+---+
|Difficulty    1  |                   |OFF|ON |OFF|
|              2  |                   |OFF|ON |ON |
|              3* |                   |OFF|OFF|OFF|
|              4  |                   |OFF|OFF|ON |
|              5  |                   |ON |OFF|OFF|
|              6  |                   |ON |OFF|ON |
+-----------------+---------------+---+---+---+---+
|# OF INVADERS 16*|               |OFF|           |
|              32 |               |ON |           |
+-----------------+---------------+---+-----------+

*********************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_PROT    (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

/* these values provide a fairly low refresh rate of around 53Hz, but
   they were derived from the schematics.  The horizontal synch chain
   counts from 0x0c0-0x1ff and the vertical one from 0x0d8-0x1ff.  */

#define MASTER_CLOCK        (10000000)
#define CPU_CLOCK           (MASTER_CLOCK / 4)
#define PIXEL_CLOCK         (MASTER_CLOCK / 2)
#define AY8910_CLOCK        (MASTER_CLOCK / 8)
#define HCOUNTER_START      (0x0c0)
#define HCOUNTER_END        (0x1ff)
#define HTOTAL              (HCOUNTER_END + 1 - HCOUNTER_START)
#define HBEND               (0x000)
#define HBSTART             (0x100)
#define VCOUNTER_START      (0x0d8)
#define VCOUNTER_END        (0x1ff)
#define VTOTAL              (VCOUNTER_END + 1 - VCOUNTER_START)
#define VBEND               (0x048)
#define VBSTART             (VTOTAL)

/* the IRQ line is cleared (active LO) at these vertical sync counter
   values and raised one scan line later */
#define INT_TRIGGER_COUNT_1 (0x10f)
#define INT_TRIGGER_COUNT_2 (0x18f)


class enigma2_state : public driver_device
{
public:
	enigma2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_colors(*this, "colors"),
		m_stars(*this, "stars"),
		m_videoram(*this, "videoram")
	{ }

	void enigma2(machine_config &config);
	void enigma2a(machine_config &config);

	void init_enigma2();

	ioport_value p1_controls_r();
	ioport_value p2_controls_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_region_ptr<uint8_t> m_colors;
	optional_region_ptr<uint8_t> m_stars;
	required_shared_ptr<uint8_t> m_videoram;

	int m_blink_count = 0;
	uint8_t m_sound_latch = 0;
	uint8_t m_last_sound_data = 0;
	uint8_t m_protection_data = 0;
	uint8_t m_flip_screen = 0;

	emu_timer *m_interrupt_clear_timer = nullptr;
	emu_timer *m_interrupt_assert_timer = nullptr;

	uint8_t dip_switch_r(offs_t offset);
	void sound_data_w(uint8_t data);
	void enigma2_flip_screen_w(uint8_t data);
	uint8_t sound_latch_r();
	void protection_data_w(uint8_t data);
	uint32_t screen_update_enigma2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_enigma2a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_clear_callback);
	TIMER_CALLBACK_MEMBER(interrupt_assert_callback);
	inline uint16_t vpos_to_vysnc_chain_counter( int vpos );
	inline int vysnc_chain_counter_to_vpos( uint16_t counter );
	void create_interrupt_timers();
	void start_interrupt_timers();

	void enigma2_audio_cpu_map(address_map &map) ATTR_COLD;
	void enigma2_main_cpu_map(address_map &map) ATTR_COLD;
	void enigma2a_main_cpu_io_map(address_map &map) ATTR_COLD;
	void enigma2a_main_cpu_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/


uint16_t enigma2_state::vpos_to_vysnc_chain_counter( int vpos )
{
	return vpos + VCOUNTER_START;
}


int enigma2_state::vysnc_chain_counter_to_vpos( uint16_t counter )
{
	return counter - VCOUNTER_START;
}


TIMER_CALLBACK_MEMBER(enigma2_state::interrupt_clear_callback)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(enigma2_state::interrupt_assert_callback)
{
	uint16_t next_counter;
	int next_vpos;

	/* compute vector and set the interrupt line */
	int vpos = m_screen->vpos();
	uint16_t counter = vpos_to_vysnc_chain_counter(vpos);
	uint8_t vector = 0xc7 | ((counter & 0x80) >> 3) | ((~counter & 0x80) >> 4);
	m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, vector); // Z80

	/* set up for next interrupt */
	if (counter == INT_TRIGGER_COUNT_1)
		next_counter = INT_TRIGGER_COUNT_2;
	else
		next_counter = INT_TRIGGER_COUNT_1;

	next_vpos = vysnc_chain_counter_to_vpos(next_counter);
	m_interrupt_assert_timer->adjust(m_screen->time_until_pos(next_vpos));
	m_interrupt_clear_timer->adjust(m_screen->time_until_pos(vpos + 1));
}


void enigma2_state::create_interrupt_timers(  )
{
	m_interrupt_clear_timer = timer_alloc(FUNC(enigma2_state::interrupt_clear_callback), this);
	m_interrupt_assert_timer = timer_alloc(FUNC(enigma2_state::interrupt_assert_callback), this);
}


void enigma2_state::start_interrupt_timers(  )
{
	int vpos = vysnc_chain_counter_to_vpos(INT_TRIGGER_COUNT_1);
	m_interrupt_assert_timer->adjust(m_screen->time_until_pos(vpos));
}



void enigma2_state::machine_start()
{
	create_interrupt_timers();

	save_item(NAME(m_blink_count));
	save_item(NAME(m_sound_latch));
	save_item(NAME(m_last_sound_data));
	save_item(NAME(m_protection_data));
	save_item(NAME(m_flip_screen));
}


void enigma2_state::machine_reset()
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_last_sound_data = 0;
	m_flip_screen = 0;
	m_sound_latch = 0;
	m_blink_count = 0;

	start_interrupt_timers();
}


/*************************************
 *
 *  Video system
 *
 *************************************/

uint32_t enigma2_state::screen_update_enigma2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();

	uint8_t x = 0;
	uint16_t bitmap_y = visarea.min_y;
	uint8_t y = (uint8_t)vpos_to_vysnc_chain_counter(bitmap_y);
	uint8_t video_data = 0;
	uint8_t fore_color = 0;
	uint8_t star_color = 0;

	while (1)
	{
		uint8_t bit;
		uint8_t color;

		/* read the video RAM */
		if ((x & 0x07) == 0x00)
		{
			offs_t color_map_address = (y >> 3 << 5) | (x >> 3);

			/* the schematics shows it like this, but it doesn't work as this would
			   produce no stars, due to the contents of the PROM -- maybe there is
			   a star disabled bit somewhere that's connected here instead of flip_screen() */
			/* star_map_address = (y >> 4 << 6) | (enigma2_flip_screen_get() << 5) | (x >> 3); */
			offs_t star_map_address = (y >> 4 << 6) | 0x20 | (x >> 3);
			if (m_blink_count & 0x08)
				star_map_address |= 0x400;

			offs_t videoram_address = (y << 5) | (x >> 3);

			/* when the screen is flipped, all the video address bits are inverted,
			   and the adder at 16A is activated */
			if (m_flip_screen)
			{
				color_map_address |= 0x400;
				videoram_address = (~videoram_address + 0x0400) & 0x1fff;
			}

			video_data = m_videoram[videoram_address];

			fore_color = m_colors[color_map_address] & 0x07;
			star_color = m_stars[star_map_address] & 0x07;
		}

		/* plot the current pixel */
		if (m_flip_screen)
		{
			bit = video_data & 0x80;
			video_data = video_data << 1;
		}
		else
		{
			bit = video_data & 0x01;
			video_data = video_data >> 1;
		}

		if (bit)
			color = fore_color;
		else
			/* stars only appear at certain positions */
			color = ((x & y & 0x0f) == 0x0f) ? star_color : 0;

		bitmap.pix(bitmap_y, x) = m_palette->pen_color(color);

		/* next pixel */
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* end of screen? */
			if (bitmap_y == visarea.max_y)
				break;

			/* next row */
			y = y + 1;
			bitmap_y = bitmap_y + 1;
		}
	}

	m_blink_count++;

	return 0;
}


uint32_t enigma2_state::screen_update_enigma2a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t x = 0;
	const rectangle &visarea = screen.visible_area();
	uint16_t bitmap_y = visarea.min_y;
	uint8_t y = (uint8_t)vpos_to_vysnc_chain_counter(bitmap_y);
	uint8_t video_data = 0;

	while (1)
	{
		uint8_t bit;
		pen_t pen;

		/* read the video RAM */
		if ((x & 0x07) == 0x00)
		{
			offs_t videoram_address = (y << 5) | (x >> 3);

			/* when the screen is flipped, all the video address bits are inverted,
			   and the adder at 16A is activated */
			if (m_flip_screen)  videoram_address = (~videoram_address + 0x0400) & 0x1fff;

			video_data = m_videoram[videoram_address];
		}

		/* plot the current pixel */
		if (m_flip_screen)
		{
			bit = video_data & 0x80;
			video_data = video_data << 1;
		}
		else
		{
			bit = video_data & 0x01;
			video_data = video_data >> 1;
		}

		pen = bit ? rgb_t::white() : rgb_t::black();
		bitmap.pix(bitmap_y, x) = pen;

		/* next pixel */
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* end of screen? */
			if (bitmap_y == visarea.max_y)
				break;

			/* next row */
			y = y + 1;
			bitmap_y = bitmap_y + 1;
		}
	}

	return 0;
}



uint8_t enigma2_state::dip_switch_r(offs_t offset)
{
	uint8_t ret = 0x00;

	LOGMASKED(LOG_PROT, "DIP SW Read: %x at %x (prot data %x)\n", offset, m_maincpu->pc(), m_protection_data);
	switch (offset)
	{
	case 0x01:
		/* For the DIP switches to be read, protection_data must be
		   0xff on reset. The AY8910 reset ensures this. */
		if (m_protection_data != 0xff)
			ret = m_protection_data ^ 0x88;
		else
			ret = ioport("DSW")->read();
		break;

	case 0x02:
		if (m_maincpu->pc() == 0x07e5)
			ret = 0xaa;
		else
			ret = 0xf4;
		break;

	case 0x35:  ret = 0x38; break;
	case 0x51:  ret = 0xaa; break;
	case 0x79:  ret = 0x38; break;
	}

	return ret;
}


void enigma2_state::sound_data_w(uint8_t data)
{
	/* clock sound latch shift register on rising edge of D2 */
	if (!(data & 0x04) && (m_last_sound_data & 0x04))
		m_sound_latch = (m_sound_latch << 1) | (~data & 0x01);

	m_audiocpu->set_input_line(INPUT_LINE_NMI, (data & 0x02) ? ASSERT_LINE : CLEAR_LINE);

	m_last_sound_data = data;
}


uint8_t enigma2_state::sound_latch_r()
{
	return bitswap<8>(m_sound_latch,0,1,2,3,4,5,6,7);
}


void enigma2_state::protection_data_w(uint8_t data)
{
	LOGMASKED(LOG_PROT, "%s: Protection Data Write: %x\n", machine().describe_context(), data);
	m_protection_data = data;
}


void enigma2_state::enigma2_flip_screen_w(uint8_t data)
{
	m_flip_screen = ((data >> 5) & 0x01) && ((ioport("DSW")->read() & 0x20) == 0x20);
}


ioport_value enigma2_state::p1_controls_r()
{
	return ioport("P1CONTROLS")->read();
}


ioport_value enigma2_state::p2_controls_r()
{
	if (m_flip_screen)
		return ioport("P2CONTROLS")->read();
	else
		return ioport("P1CONTROLS")->read();
}

void enigma2_state::enigma2_main_cpu_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).rom().nopw();
	map(0x2000, 0x3fff).mirror(0x4000).ram().share("videoram");
	map(0x4000, 0x4fff).rom().nopw();
	map(0x5000, 0x57ff).r(FUNC(enigma2_state::dip_switch_r)).nopw();
	map(0x5800, 0x5800).mirror(0x07f8).noprw();
	map(0x5801, 0x5801).mirror(0x07f8).portr("IN0").nopw();
	map(0x5802, 0x5802).mirror(0x07f8).portr("IN1").nopw();
	map(0x5803, 0x5803).mirror(0x07f8).nopr().w(FUNC(enigma2_state::sound_data_w));
	map(0x5804, 0x5804).mirror(0x07f8).noprw();
	map(0x5805, 0x5805).mirror(0x07f8).nopr().w(FUNC(enigma2_state::enigma2_flip_screen_w));
	map(0x5806, 0x5807).mirror(0x07f8).noprw();
}


void enigma2_state::enigma2a_main_cpu_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().nopw();
	map(0x2000, 0x3fff).mirror(0x4000).ram().share("videoram");
	map(0x4000, 0x4fff).rom().nopw();
	map(0x5000, 0x57ff).r(FUNC(enigma2_state::dip_switch_r)).nopw();
	map(0x5800, 0x5fff).noprw();
}


void enigma2_state::enigma2a_main_cpu_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).noprw();
	map(0x01, 0x01).portr("IN0").nopw();
	map(0x02, 0x02).portr("IN1").nopw();
	map(0x03, 0x03).nopr().w(FUNC(enigma2_state::sound_data_w));
	map(0x04, 0x04).noprw();
	map(0x05, 0x05).nopr().w(FUNC(enigma2_state::enigma2_flip_screen_w));
	map(0x06, 0x07).noprw();
}


void enigma2_state::enigma2_audio_cpu_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x1000).rom().nopw();
	map(0x2000, 0x7fff).noprw();
	map(0x8000, 0x83ff).mirror(0x1c00).ram();
	map(0xa000, 0xa001).mirror(0x1ffc).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xa002, 0xa002).mirror(0x1ffc).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xa003, 0xa003).mirror(0x1ffc).noprw();
	map(0xc000, 0xffff).noprw();
}



static INPUT_PORTS_START( enigma2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(enigma2_state, p1_controls_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(enigma2_state, p2_controls_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )      PORT_DIPLOCATION("DSW:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, "Skill Level" )         PORT_DIPLOCATION("DSW:!6,!7,!8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )    PORT_DIPLOCATION("DSW:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Number of Invaders" )  PORT_DIPLOCATION("DSW:!5")
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )    PORT_DIPLOCATION("DSW:!4")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("P1CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( enigma2a )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(enigma2_state, p1_controls_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(enigma2_state, p2_controls_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )      PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, "Skill Level" )         PORT_DIPLOCATION("DSW:6,7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )    PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )    PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("P1CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void enigma2_state::enigma2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &enigma2_state::enigma2_main_cpu_map);

	Z80(config, m_audiocpu, 2500000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &enigma2_state::enigma2_audio_cpu_map);
	m_audiocpu->set_periodic_int(FUNC(enigma2_state::irq0_line_hold), attotime::from_hz(8*52));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(enigma2_state::screen_update_enigma2));

	PALETTE(config, m_palette, palette_device::GBR_3BIT);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", AY8910_CLOCK));
	aysnd.port_a_read_callback().set(FUNC(enigma2_state::sound_latch_r));
	aysnd.port_b_write_callback().set(FUNC(enigma2_state::protection_data_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


void enigma2_state::enigma2a(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &enigma2_state::enigma2a_main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &enigma2_state::enigma2a_main_cpu_io_map);

	Z80(config, m_audiocpu, 2500000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &enigma2_state::enigma2_audio_cpu_map);
	m_audiocpu->set_periodic_int(FUNC(enigma2_state::irq0_line_hold), attotime::from_hz(8*52));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(enigma2_state::screen_update_enigma2a));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", AY8910_CLOCK));
	aysnd.port_a_read_callback().set(FUNC(enigma2_state::sound_latch_r));
	aysnd.port_b_write_callback().set(FUNC(enigma2_state::protection_data_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( enigma2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5d",         0x0000, 0x0800, CRC(499749de) SHA1(401928ff41d3b4cbb68e6ad3bf3be4a10ae1781f) )
	ROM_LOAD( "2.7d",         0x0800, 0x0800, CRC(173c1329) SHA1(3f1ad46d0e58ab236e4ff2b385d09fbf113627da) )
	ROM_LOAD( "3.8d",         0x1000, 0x0800, CRC(c7d3e6b1) SHA1(43f7c3a02b46747998260d5469248f21714fe12b) )
	ROM_LOAD( "4.10d",        0x1800, 0x0800, CRC(c6a7428c) SHA1(3503f09856655c5973fb89f60d1045fe41012aa9) )
	ROM_LOAD( "5.11d",        0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "6.13d",        0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "9.2f",    0x0000, 0x1000, CRC(68fd8c54) SHA1(69996d5dfd996f0aacb26e397bef314204a2a88a) )

	ROM_REGION( 0x0800, "colors", 0 )
	ROM_LOAD( "7.11f",        0x0000, 0x0800, CRC(409b5aad) SHA1(1b774a70f725637458ed68df9ed42476291b0e43) )

	ROM_REGION( 0x0800, "stars", 0 )
	ROM_LOAD( "8.13f",        0x0000, 0x0800, CRC(e9cb116d) SHA1(41da4f46c5614ec3345c233467ebad022c6b0bf5) )
ROM_END


ROM_START( enigma2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "36_en1.bin",   0x0000, 0x0800, CRC(15f44806) SHA1(4a2f7bc91d4edf7a069e0865d964371c97af0a0a) )
	ROM_LOAD( "35_en2.bin",   0x0800, 0x0800, CRC(e841072f) SHA1(6ab02fd9fdeac5ab887cd25eee3d6b70ab01f849) )
	ROM_LOAD( "34_en3.bin",   0x1000, 0x0800, CRC(43d06cf4) SHA1(495af05d54c0325efb67347f691e64d194645d85) )
	ROM_LOAD( "33_en4.bin",   0x1800, 0x0800, CRC(8879a430) SHA1(c97f44bef3741eef74e137d2459e79f1b3a90457) )
	ROM_LOAD( "5.11d",        0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "6.13d",        0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.bin",    0x0000, 0x0800, BAD_DUMP CRC(5f092d3c) SHA1(17c70f6af1b5560a45e6b1bdb330a98b27570fe9) )
ROM_END

ROM_START( enigma2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic36.bin",   0x0000, 0x0800, CRC(71dc9ecc) SHA1(a41260259cf0a36b01b5e8ad35cf968e920d22d9) )
	ROM_LOAD( "ic35.bin",   0x0800, 0x0800, CRC(e841072f) SHA1(6ab02fd9fdeac5ab887cd25eee3d6b70ab01f849) )
	ROM_LOAD( "ic34.bin",   0x1000, 0x0800, CRC(1384073d) SHA1(7a3a910c0431e680cc952a10a040b02f3df0532a) )
	ROM_LOAD( "ic33.bin",   0x1800, 0x0800, CRC(ac6c2410) SHA1(d35565a5ffe795d0c36970bd9c2f948bf79e0ed8) )
	ROM_LOAD( "ic32.bin",   0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "ic42.bin",   0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	/* this rom was completely broken on this pcb.. */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.bin",    0x0000, 0x0800, BAD_DUMP CRC(5f092d3c) SHA1(17c70f6af1b5560a45e6b1bdb330a98b27570fe9) )
ROM_END


void enigma2_state::init_enigma2()
{
	uint8_t *rom = memregion("audiocpu")->base();
	for (offs_t i = 0; i < 0x2000; i++)
	{
		rom[i] = bitswap<8>(rom[i],4,5,6,0,7,1,3,2);
	}
}

} // Anonymous namespace


GAME( 1981, enigma2,  0,       enigma2,  enigma2,  enigma2_state, init_enigma2, ROT270, "Zilec Electronics (Game Plan license)", "Enigma II",                             MACHINE_SUPPORTS_SAVE )
GAME( 1984, enigma2a, enigma2, enigma2a, enigma2a, enigma2_state, init_enigma2, ROT270, "Zilec Electronics",                     "Enigma II (Space Invaders hardware)",   MACHINE_SUPPORTS_SAVE )
GAME( 1981, enigma2b, enigma2, enigma2a, enigma2a, enigma2_state, init_enigma2, ROT270, "Zilec Electronics",                     "Phantoms II (Space Invaders hardware)", MACHINE_SUPPORTS_SAVE )
