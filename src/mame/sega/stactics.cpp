// license:BSD-3-Clause
// copyright-holders: Frank Palazzolo

/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

Master processor - Intel 8080A

Memory Map:

0000-2fff ROM                       R
4000-47ff RAM                       R/W
5000-5fff switches/status           R
6000-6fff dips sw                   R
6000-600f Coin rjct/palette select  W
6010-601f sound triggers            W
6020-602f lamp latch                W
6030-603f speed latch               W
6040-605f shot related              W
6060-606f score display             W
60a0-60e0 sound triggers2           W
7000-7fff RNG/swit                  R     LS Nibble are a VBlank counter
                                          used as a RNG
8000-8fff swit/stat                 R
8000-8fff offset RAM                W
9000-9fff V pos reg.                R     Reads counter from an encoder wheel
a000-afff H pos reg.                R     Reads counter from an encoder wheel
b000-bfff VRAM B                    R/W   alphanumerics, bases, barrier,
                                          enemy bombs
d000-dfff VRAM D                    R/W   furthest aliens (scrolling)
e000-efff VRAM E                    R/W   middle aliens (scrolling)
f000-ffff VRAM F                    R/W   closest aliens (scrolling)

--------------------------------------------------------------------

At this time, emulation is missing:

Sound (all discrete and a 76477)
Verify Color PROM resistor values (Last 8 colors)

****************************************************************************/

#include "emu.h"

#include "stactics.lh"

#include "cpu/i8085/i8085.h"
#include "machine/74259.h"

#include "emupal.h"
#include "screen.h"


namespace {

class stactics_state : public driver_device
{
public:
	stactics_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_display_buffer(*this, "display_buffer"),
		m_videoram_b(*this, "videoram_b"),
		m_videoram_d(*this, "videoram_d"),
		m_videoram_e(*this, "videoram_e"),
		m_videoram_f(*this, "videoram_f"),
		m_beam_region(*this, "beam"),
		m_base_lamps(*this, "base_lamp%u", 0U),
		m_beam_leds_left(*this, "beam_led_left%u", 0U),
		m_beam_leds_right(*this, "beam_led_right%u", 0U),
		m_score_digits(*this, "digit%u", 0U),
		m_credit_leds(*this, "credit_led%u", 0U),
		m_barrier_leds(*this, "barrier_led%u", 0U),
		m_round_leds(*this, "round_led%u", 0U),
		m_barrier_lamp(*this, "barrier_lamp"),
		m_start_lamp(*this, "start_lamp"),
		m_sight_led(*this, "sight_led"),
		m_in3(*this, "IN3"),
		m_fake(*this, "FAKE")
	{ }

	void stactics(machine_config &config);

	int frame_count_d3_r();
	int shot_standby_r();
	int not_shot_arrive_r();
	int motor_not_ready_r();
	ioport_value get_rng();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t vert_pos_r();
	uint8_t horiz_pos_r();
	template <uint8_t Which> void coin_lockout_w(int state);
	void palette_bank_w(int state);
	void scroll_ram_w(offs_t offset, uint8_t data);
	void speed_latch_w(uint8_t data);
	void shot_trigger_w(uint8_t data);
	void shot_flag_clear_w(uint8_t data);
	void motor_w(int state);

	INTERRUPT_GEN_MEMBER(interrupt);

	void barrier_lamp_w(int state);
	void start_lamp_w(int state);
	template <unsigned N> void base_lamp_w(int state) { m_base_lamps[N] = state; }

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_beam();
	inline int get_pixel_on_plane(uint8_t *videoram, uint8_t y, uint8_t x, uint8_t y_scroll);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	template <unsigned N> void set_indicator_leds(unsigned offset, output_finder<N> &outputs, int base_index);
	void update_artwork();
	void move_motor();

	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;

	required_shared_ptr<uint8_t> m_display_buffer;
	required_shared_ptr<uint8_t> m_videoram_b;
	required_shared_ptr<uint8_t> m_videoram_d;
	required_shared_ptr<uint8_t> m_videoram_e;
	required_shared_ptr<uint8_t> m_videoram_f;
	required_region_ptr<uint8_t> m_beam_region;

	output_finder<5> m_base_lamps;
	output_finder<0x40> m_beam_leds_left;
	output_finder<0x40> m_beam_leds_right;
	output_finder<6> m_score_digits;
	output_finder<8> m_credit_leds;
	output_finder<12> m_barrier_leds;
	output_finder<16> m_round_leds;
	output_finder<> m_barrier_lamp;
	output_finder<> m_start_lamp;
	output_finder<> m_sight_led;

	required_ioport m_in3;
	required_ioport m_fake;

	// machine state
	int32_t m_vert_pos = 0;
	int32_t m_horiz_pos = 0;
	bool    m_motor_on = false;

	// video state
	uint8_t  m_y_scroll_d = 0;
	uint8_t  m_y_scroll_e = 0;
	uint8_t  m_y_scroll_f = 0;
	uint8_t  m_frame_count = 0;
	uint8_t  m_shot_standby = 0;
	uint8_t  m_shot_arrive = 0;
	uint16_t m_beam_state = 0;
	uint16_t m_old_beam_state = 0;
	uint16_t m_beam_states_per_frame = 0;
	uint8_t  m_palette_bank = 0;
};


/****************************************************************************

The Video system used in Space Tactics is unusual.
Here are my notes on how the video system works:

There are 4, 4K pages of Video RAM. (B,D,E & F)

The first 1K of each VRAM page contains the following:
0 0 V V V V V H H H H H   offset value for each 8x8 bitmap
     (v-tile)  (h-tile)

The offset values are used to generate an access into the
last 2K of the VRAM page:
1 D D D D D D D D V V V   here we find 8x8 character data
     (offset)    (line)

In addition, in Page B, the upper nibble of the offset is
also used to select the color palette for the tile.

Page B, D, E, and F all work similarly, except that pages
D, E, and F can be offset from Page B by a given
number of scanlines, based on the contents of the offset
RAM

The offset RAM is addressed in this format:
1 0 0 0 P P P V V V V V V V V V
        (Page)   (scanline)
Page 4=D, 5=E, 6=F

Page D, E, and F are drawn offset from the top of the screen,
starting on the first scanline which contains a 1 in the
appropriate memory location

---

The composited monitor image is seen in a mirror.  It appears
to move when the player moves the handle, due to motors which
tilt the mirror up and down, and the monitor left and right.

---

***************************************************************************/


/*************************************
 *
 *  Palette
 *
 *************************************/

void stactics_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x400; i++)
	{
		int const bit0 = BIT(color_prom[i], 0);
		int const bit1 = BIT(color_prom[i], 1);
		int const bit2 = BIT(color_prom[i], 2);
		int const bit3 = BIT(color_prom[i], 3);

		// red component
		int const r = 0xff * bit0;

		// green component
		int const g = 0xff * bit1 - 0xcc * bit3;

		// blue component
		int const b = 0xff * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void stactics_state::palette_bank_w(int state)
{
	m_palette_bank = m_outlatch->q6_r() | (m_outlatch->q7_r() << 1);
}



/*************************************
 *
 *  Scrolling
 *
 *************************************/

void stactics_state::scroll_ram_w(offs_t offset, uint8_t data)
{
	if (data & 0x01)
	{
		switch (offset >> 8)
		{
			case 4: m_y_scroll_d = offset & 0xff; break;
			case 5: m_y_scroll_e = offset & 0xff; break;
			case 6: m_y_scroll_f = offset & 0xff; break;
		}
	}
}



/*************************************
 *
 *  Frane counter
 *
 *************************************/

int stactics_state::frame_count_d3_r()
{
	return (m_frame_count >> 3) & 0x01;
}



/*************************************
 *
 *  Beam handling
 *
 *************************************/

void stactics_state::speed_latch_w(uint8_t data)
{
	/* This writes to a shift register which is clocked by
	   a 555 oscillator.  This value determines the speed of
	   the LED fire beams as follows:

	   555_freq / bits_in_SR * edges_in_SR / states_in_PR67 / frame_rate
	     = num_led_states_per_frame
	     36439 / 8 * x / 32 / 60 ~= 19/8*x

	   Here, we will count the number of rising edges in the shift register */

	int num_rising_edges = 0;

	for (int i = 0; i < 8; i++)
	{
		if ((((data >> i) & 0x01) == 1) && (((data >> ((i + 1) % 8)) & 0x01) == 0))
			num_rising_edges++;
	}

	m_beam_states_per_frame = num_rising_edges * 19 / 8;
}


void stactics_state::shot_trigger_w(uint8_t data)
{
	m_shot_standby = 0;
}


void stactics_state::shot_flag_clear_w(uint8_t data)
{
	m_shot_arrive = 0;
}


int stactics_state::shot_standby_r()
{
	return m_shot_standby;
}


int stactics_state::not_shot_arrive_r()
{
	return !m_shot_arrive;
}


void stactics_state::update_beam()
{
	// first, update the firebeam state
	m_old_beam_state = m_beam_state;
	if (m_shot_standby == 0)
		m_beam_state = m_beam_state + m_beam_states_per_frame;

	/* These are thresholds for the two shots from the LED fire ROM
	   (Note: There are two more for sound triggers,
	          whenever that gets implemented)        */
	if ((m_old_beam_state < 0x8b) & (m_beam_state >= 0x8b))
		m_shot_arrive = 1;

	if ((m_old_beam_state < 0xca) & (m_beam_state >= 0xca))
		m_shot_arrive = 1;

	if (m_beam_state >= 0x100)
	{
		m_beam_state = 0;
		m_shot_standby = 1;
	}
}



/*************************************
 *
 *  Video drawing
 *
 *************************************/

inline int stactics_state::get_pixel_on_plane(uint8_t *videoram, uint8_t y, uint8_t x, uint8_t y_scroll)
{
	// compute effective row
	y = y - y_scroll;

	// get the character code at the given pixel
	uint8_t const code = videoram[((y >> 3) << 5) | (x >> 3)];

	// get the gfx byte
	uint8_t const gfx = videoram[0x800 | (code << 3) | (y & 0x07)];

	// return the appropriate pixel within the byte
	return (gfx >> (~x & 0x07)) & 0x01;
}


void stactics_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// for every row
	for (int y = 0; y < 0x100; y++)
	{
		// for every pixel on the row
		for (int x = 0; x < 0x100; x++)
		{
			// get the pixels for the four planes
			int const pixel_b = get_pixel_on_plane(m_videoram_b, y, x, 0);
			int const pixel_d = get_pixel_on_plane(m_videoram_d, y, x, m_y_scroll_d);
			int const pixel_e = get_pixel_on_plane(m_videoram_e, y, x, m_y_scroll_e);
			int const pixel_f = get_pixel_on_plane(m_videoram_f, y, x, m_y_scroll_f);

			// get the color for this pixel
			uint8_t const color = m_videoram_b[((y >> 3) << 5) | (x >> 3)] >> 4;

			// assemble the pen index
			int const pen = color |
								(pixel_b << 4) |
								(pixel_f << 5) |
								(pixel_e << 6) |
								(pixel_d << 7) |
								(m_palette_bank << 8);

			/* compute the effective pixel coordinate after adjusting for the
			   mirror movement - this is mechanical on the real machine */
			int const sy = y + m_vert_pos;
			int const sx = x - m_horiz_pos;

			// plot if visible
			if ((sy >= 0) && (sy < 0x100) && (sx >= 0) && (sx < 0x100))
				bitmap.pix(sy, sx) = pen;
		}
	}
}



/*************************************
 *
 *  Non-video artwork
 *
 *************************************/

// from 7448 datasheet
static constexpr int to_7seg[0x10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07,
	0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00
};


template <unsigned N> void stactics_state::set_indicator_leds(unsigned offset, output_finder<N> &outputs, int base_index)
{
	// decode the data
	int const data = to_7seg[~m_display_buffer[offset] & 0x0f];

	// set the 4 LEDs
	outputs[base_index + 0] = BIT(data, 2);
	outputs[base_index + 1] = BIT(data, 6);
	outputs[base_index + 2] = BIT(data, 5);
	outputs[base_index + 3] = BIT(data, 4);
}


void stactics_state::barrier_lamp_w(int state)
{
	// this needs to flash on/off, not implemented
	m_barrier_lamp = state;
}


void stactics_state::start_lamp_w(int state)
{
	m_start_lamp = state;
}


void stactics_state::update_artwork()
{
	// laser beam - loop for each LED
	for (int i = 0; i < 0x40; i++)
	{
		offs_t const beam_data_offs = ((i & 0x08) << 7) | ((i & 0x30) << 4) | m_beam_state;
		uint8_t const beam_data = m_beam_region[beam_data_offs];
		int const on = BIT(beam_data, i & 0x07);

		m_beam_leds_left[i] = on;
		m_beam_leds_right[i] = on;
	}

	// sight LED
	m_sight_led = m_motor_on;

	// score display
	for (int i = 0x01; i < 0x07; i++)
		m_score_digits[i - 1] = to_7seg[~m_display_buffer[i] & 0x0f];

	// credits indicator
	set_indicator_leds(0x07, m_credit_leds, 0x00);
	set_indicator_leds(0x08, m_credit_leds, 0x04);

	// barriers indicator
	set_indicator_leds(0x09, m_barrier_leds, 0x00);
	set_indicator_leds(0x0a, m_barrier_leds, 0x04);
	set_indicator_leds(0x0b, m_barrier_leds, 0x08);

	// rounds indicator
	set_indicator_leds(0x0c, m_round_leds, 0x00);
	set_indicator_leds(0x0d, m_round_leds, 0x04);
	set_indicator_leds(0x0e, m_round_leds, 0x08);
	set_indicator_leds(0x0f, m_round_leds, 0x0c);
}



/*************************************
 *
 *  Start
 *
 *************************************/

void stactics_state::video_start()
{
	m_base_lamps.resolve();
	m_beam_leds_left.resolve();
	m_beam_leds_right.resolve();
	m_score_digits.resolve();
	m_credit_leds.resolve();
	m_barrier_leds.resolve();
	m_round_leds.resolve();
	m_barrier_lamp.resolve();
	m_start_lamp.resolve();
	m_sight_led.resolve();

	m_y_scroll_d = 0;
	m_y_scroll_e = 0;
	m_y_scroll_f = 0;

	m_frame_count = 0;
	m_shot_standby = 1;
	m_shot_arrive = 0;
	m_beam_state = 0;
	m_old_beam_state = 0;

	m_palette_bank = 0;

	save_item(NAME(m_y_scroll_d));
	save_item(NAME(m_y_scroll_e));
	save_item(NAME(m_y_scroll_f));
	save_item(NAME(m_frame_count));
	save_item(NAME(m_shot_standby));
	save_item(NAME(m_shot_arrive));
	save_item(NAME(m_beam_state));
	save_item(NAME(m_old_beam_state));
	save_item(NAME(m_beam_states_per_frame));
	save_item(NAME(m_palette_bank));
}



/*************************************
 *
 *  Update
 *
 *************************************/

uint32_t stactics_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	update_beam();
	draw_background(bitmap, cliprect);
	update_artwork();

	m_frame_count = (m_frame_count + 1) & 0x0f;

	return 0;
}


/*************************************
 *
 *  Mirror motor handling
 *
 *************************************/

void stactics_state::motor_w(int state)
{
	m_motor_on = state;
}


int stactics_state::motor_not_ready_r()
{
	// if the motor is self-centering, but not centered yet
	return (!m_motor_on && (m_horiz_pos != 0 || m_vert_pos != 0));
}


uint8_t stactics_state::vert_pos_r()
{
	return 0x70 - m_vert_pos;
}


uint8_t stactics_state::horiz_pos_r()
{
	return m_horiz_pos + 0x88;
}


void stactics_state::move_motor()
{
	// monitor motor under joystick control
	if (m_motor_on)
	{
		const int in3 = m_in3->read();
		const int in4 = m_fake->read();

		// up
		if (!(in4 & 0x01) && m_vert_pos > -128)
			m_vert_pos--;

		// down
		if (!(in4 & 0x02) && m_vert_pos < 127)
			m_vert_pos++;

		// left
		if (!(in3 & 0x20)  && m_horiz_pos < 127)
			m_horiz_pos++;

		// right
		if (!(in3 & 0x40) && m_horiz_pos > -128)
			m_horiz_pos--;
	}

	// monitor motor under self-centering control
	else
	{
		if (m_horiz_pos > 0)
			m_horiz_pos--;
		else if (m_horiz_pos < 0)
			m_horiz_pos++;

		if (m_vert_pos > 0)
			m_vert_pos--;
		else if (m_vert_pos < 0)
			m_vert_pos++;
	}
}



/*************************************
 *
 *  Random number generator
 *
 *************************************/

ioport_value stactics_state::get_rng()
{
	// this is a 555 timer, but cannot read one of the resistor values
	return machine().rand() & 0x07;
}



/*************************************
 *
 *  Coin lockout
 *
 *************************************/

template <uint8_t Which>
void stactics_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(Which, !state);
}



/*************************************
 *
 *  Interrupt system
 *
 *************************************/

INTERRUPT_GEN_MEMBER(stactics_state::interrupt)
{
	move_motor();

	device.execute().set_input_line(0, HOLD_LINE);
}



/*************************************
 *
 *  Data CPU memory handlers
 *
 *************************************/

void stactics_state::main_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x40ff).mirror(0x0700).ram();
	map(0x5000, 0x5000).mirror(0x0fff).portr("IN0");
	map(0x6000, 0x6000).mirror(0x0fff).portr("IN1");
	map(0x6000, 0x6007).mirror(0x0f08).w(m_outlatch, FUNC(ls259_device::write_d0));
	map(0x6010, 0x6017).mirror(0x0f08).w("audiolatch", FUNC(ls259_device::write_d0));
	map(0x6020, 0x6027).mirror(0x0f08).w("lamplatch", FUNC(ls259_device::write_d0));
	map(0x6030, 0x6030).mirror(0x0f0f).w(FUNC(stactics_state::speed_latch_w));
	map(0x6040, 0x6040).mirror(0x0f0f).w(FUNC(stactics_state::shot_trigger_w));
	map(0x6050, 0x6050).mirror(0x0f0f).w(FUNC(stactics_state::shot_flag_clear_w));
	map(0x6060, 0x606f).mirror(0x0f00).writeonly().share(m_display_buffer);
	map(0x6070, 0x609f).mirror(0x0f00).nopw();
	// map(0x60a0, 0x60ef).mirror(0x0f00).w(FUNC(stactics_state::sound2_w));
	map(0x60f0, 0x60ff).mirror(0x0f00).nopw();
	map(0x7000, 0x7000).mirror(0x0fff).portr("IN2");
	map(0x8000, 0x8000).mirror(0x0fff).portr("IN3");
	map(0x8000, 0x87ff).mirror(0x0800).w(FUNC(stactics_state::scroll_ram_w));
	map(0x9000, 0x9000).mirror(0x0fff).r(FUNC(stactics_state::vert_pos_r));
	map(0xa000, 0xa000).mirror(0x0fff).r(FUNC(stactics_state::horiz_pos_r));
	map(0xb000, 0xbfff).ram().share(m_videoram_b);
	map(0xc000, 0xcfff).noprw();
	map(0xd000, 0xdfff).ram().share(m_videoram_d);
	map(0xe000, 0xefff).ram().share(m_videoram_e);
	map(0xf000, 0xffff).ram().share(m_videoram_f);
}



/*************************************
 *
 *  Input port definitions
 *
 *************************************/

static INPUT_PORTS_START( stactics )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, motor_not_ready_r)

	PORT_START("IN1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x00, "High Score Initial Entry" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(stactics_state, get_rng)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, frame_count_d3_r)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, shot_standby_r)
	PORT_DIPNAME( 0x04, 0x04, "Number of Barriers" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Barriers" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, not_shot_arrive_r)

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
INPUT_PORTS_END



/*************************************
 *
 *  Start
 *
 *************************************/

void stactics_state::machine_start()
{
	m_vert_pos = 0;
	m_horiz_pos = 0;
	m_motor_on = false;

	save_item(NAME(m_vert_pos));
	save_item(NAME(m_horiz_pos));
	save_item(NAME(m_motor_on));
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void stactics_state::stactics(machine_config &config)
{
	// basic machine hardware
	I8080(config, m_maincpu, 15.46848_MHz_XTAL / 8); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &stactics_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(stactics_state::interrupt));

	LS259(config, m_outlatch); // 50
	m_outlatch->q_out_cb<0>().set(FUNC(stactics_state::coin_lockout_w<0>)); // COIN REJECT 1
	m_outlatch->q_out_cb<1>().set(FUNC(stactics_state::coin_lockout_w<1>)); // COIN REJECT 2
	m_outlatch->q_out_cb<6>().set(FUNC(stactics_state::palette_bank_w)); // FLM COL 0
	m_outlatch->q_out_cb<7>().set(FUNC(stactics_state::palette_bank_w)); // FLM COL 1

	ls259_device &audiolatch(LS259(config, "audiolatch")); // 58 - TODO: implement these switches
	audiolatch.q_out_cb<0>().set_nop(); // MUTE
	audiolatch.q_out_cb<1>().set_nop(); // INV. DISTANCE A
	audiolatch.q_out_cb<2>().set_nop(); // INV. DISTANCE B
	audiolatch.q_out_cb<3>().set_nop(); // UFO
	audiolatch.q_out_cb<4>().set_nop(); // INVADER
	audiolatch.q_out_cb<5>().set_nop(); // EMEGENCY (sic)
	audiolatch.q_out_cb<6>().set(FUNC(stactics_state::motor_w)); // overlaps rocket sound
	audiolatch.q_out_cb<7>().set_nop(); // SOUND ON

	ls259_device &lamplatch(LS259(config, "lamplatch")); // 96
	lamplatch.q_out_cb<0>().set(FUNC(stactics_state::base_lamp_w<4>));
	lamplatch.q_out_cb<1>().set(FUNC(stactics_state::base_lamp_w<3>));
	lamplatch.q_out_cb<2>().set(FUNC(stactics_state::base_lamp_w<2>));
	lamplatch.q_out_cb<3>().set(FUNC(stactics_state::base_lamp_w<1>));
	lamplatch.q_out_cb<4>().set(FUNC(stactics_state::base_lamp_w<0>));
	lamplatch.q_out_cb<5>().set(FUNC(stactics_state::start_lamp_w));
	lamplatch.q_out_cb<6>().set(FUNC(stactics_state::barrier_lamp_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_ALWAYS_UPDATE);
	screen.set_raw(15.46848_MHz_XTAL / 3, 328, 0, 256, 262, 0, 232);
	screen.set_screen_update(FUNC(stactics_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(stactics_state::palette), 0x400);

	// audio hardware
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( stactics )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-218x",     0x0000, 0x0800, CRC(b1186ad2) SHA1(88929a183ac0499619b3e07241f3b5a0c89bdab1) )
	ROM_LOAD( "epr-219x",     0x0800, 0x0800, CRC(3b86036d) SHA1(6ad5e14dcfdbc6d2a0a32ae7f18ce41ab4b51eec) )
	ROM_LOAD( "epr-220x",     0x1000, 0x0800, CRC(c58702da) SHA1(93936c46810722d435f9ddb0641defb741743dee) )
	ROM_LOAD( "epr-221x",     0x1800, 0x0800, CRC(e327639e) SHA1(024929b65c71eaeb6d234a14d7535a7d5b98b8d3) )
	ROM_LOAD( "epr-222y",     0x2000, 0x0800, CRC(24dd2bcc) SHA1(f77c59beccc1a77e3bfc2928ff532d6e221ff42d) )
	ROM_LOAD( "epr-223x",     0x2800, 0x0800, CRC(7fef0940) SHA1(5b2af55f75ef0130f9202b6a916a96dbd601fcfa) )

	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "pr54",         0x0000, 0x0800, CRC(9640bd6e) SHA1(dd12952a6591f2056ac1b5688dca0a3a2ef69f2d) )      // color/priority PROM
	ROM_LOAD( "pr55",         0x0800, 0x0800, CRC(f162673b) SHA1(83743780b6c1f8014df24fa0650000b7cb137d92) )      // timing PROM (unused)
	ROM_LOAD( "pr65",         0x1000, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )      // timing PROM (unused)
	ROM_LOAD( "pr66",         0x1020, 0x0020, CRC(78dcf300) SHA1(37034cc0cfa4a8ec47937a2a34b77ec56b387a9b) )      // timing PROM (unused)

	ROM_REGION( 0x0820, "beam", 0 )
	ROM_LOAD( "epr-217",      0x0000, 0x0800, CRC(38259f5f) SHA1(1f4182ffc2d78fca22711526bb2ae2cfe040173c) )      // LED fire beam data
	ROM_LOAD( "pr67",         0x0800, 0x0020, CRC(b27874e7) SHA1(c24bc78c4b2ae01aaed5d994ce2e7c5e0f2eece8) )      // LED timing ROM (unused)
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAMEL( 1981, stactics, 0, stactics, stactics, stactics_state, empty_init, ORIENTATION_FLIP_X, "Sega", "Space Tactics", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_stactics )
