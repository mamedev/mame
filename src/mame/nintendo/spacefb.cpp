// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Space Firebird hardware

    Memory Map figured out by Chris Hardy, Paul Johnson and Andy Clark
    MAME driver by Chris Hardy

    Schematics scanned and provided by James Twine
    Thanks to Gary Walton for lending me his REAL Space Firebird

    There is an undumped bootleg of this named 'Fire Condor', see
    http://solarfox.triluminary.net/arc_spacefirebird.php for info

    Known issues/to-do's:
        * Bullet colors are incorrect.  The schematics cannot be right, so
          I am using pure red for now

          "MAME has the bullets and missiles as red, the real pcb shows them as
           yellow with red tinges, they are overall yellow/orange in appearance."
           (Andrew Welburn 24/12/10)

        * Analog sounds


    0000-3FFF ROM       Code
    8000-83FF RAM       Sprite RAM
    C000-C7FF RAM       Game RAM

    IO Ports

    IN:
    Port 0

       bit 0 = Player 1 Right
       bit 1 = Player 1 Left
       bit 2 = unused
       bit 3 = unused
       bit 4 = Player 1 Warp / Escape
       bit 5 = unused
       bit 6 = unused
       bit 7 = Player 1 Fire

    Port 1

       bit 0 = Player 2 Right
       bit 1 = Player 2 Left
       bit 2 = unused
       bit 3 = unused
       bit 4 = Player 2 Warp / Escape
       bit 5 = unused
       bit 6 = unused
       bit 7 = Player 2 Fire

    Port 2

       bit 0 = unused
       bit 1 = unused
       bit 2 = Start 1 Player game
       bit 3 = Start 2 Players game
       bit 4 = unused
       bit 5 = unused
       bit 6 = Test switch
       bit 7 = Coin and Service switch

    Port 3

       bit 0 = Dipswitch 1
       bit 1 = Dipswitch 2
       bit 2 = Dipswitch 3
       bit 3 = Dipswitch 4
       bit 4 = Dipswitch 5
       bit 5 = Dipswitch 6
       bit 6 = unused (Debug switch - Code jumps to $3800 on reset if on)
       bit 7 = unused

   OUT:
   Port 0 - Video

       bit 0 = Screen flip. (RV)
       bit 1 = unused
       bit 2 = unused
       bit 3 = unused
       bit 4 = unused
       bit 5 = Char/Sprite Bank switch (VREF)
       bit 6 = Turns on Bit 2 of the color PROM. Used to change the bird colors. (CREF)
       bit 7 = unused

   Port 1
       bit 0 = discrete sound (Enemy death)
       bit 1 = INT to 8035
       bit 2 = T1 input to 8035
       bit 3 = PB4 input to 8035
       bit 4 = PB5 input to 8035
       bit 5 = T0 input to 8035
       bit 6 = discrete sound (Ship fire)
       bit 7 = discrete sound (Explosion noise)

   Port 2 - Video control

      These are passed to the sound board and are used to produce a
      red flash effect when you die.

      bit 0 = CONT R       Changes contrast of the red/green/blue part of the stars. This is used to make the starfield flicker
      bit 1 = CONT G
      bit 2 = CONT B
      bit 3 = ALRD         Turns background red on
      bit 4 = ALBU         Turns background blue on
      bit 5 = unused
      bit 6 = unused
      bit 7 = ALBA         Turns off star field (no star field)


****************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "video/resnet.h"

#include "screen.h"
#include "speaker.h"


namespace {

class spacefb_state : public driver_device
{
public:
	spacefb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_samples(*this, "samples"),
		m_screen(*this, "screen"),
		m_videoram(*this, "videoram") { }

	void spacefb(machine_config &config);
	void spacefb_audio(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
// SPACEFB_PIXEL_CLOCK clocks the star generator circuit.  The rest of the graphics
// use a clock half of SPACEFB_PIXEL_CLOCK, thus creating double width pixels.
	static constexpr int SPACEFB_MASTER_CLOCK        = 20160000;
	static constexpr int SPACEFB_MAIN_CPU_CLOCK      = 6000000 / 2;
	static constexpr int SPACEFB_AUDIO_CPU_CLOCK     = 6000000;   // this goes to X2, pixel clock goes to X1
	static constexpr int SPACEFB_PIXEL_CLOCK         = SPACEFB_MASTER_CLOCK / 2;
	static constexpr int SPACEFB_HTOTAL              = 0x280;
	static constexpr int SPACEFB_HBEND               = 0x000;
	static constexpr int SPACEFB_HBSTART             = 0x200;
	static constexpr int SPACEFB_VTOTAL              = 0x100;
	static constexpr int SPACEFB_VBEND               = 0x010;
	static constexpr int SPACEFB_VBSTART             = 0x0f0;
	static constexpr int SPACEFB_INT_TRIGGER_COUNT_1 = 0x080;
	static constexpr int SPACEFB_INT_TRIGGER_COUNT_2 = 0x0f0;

	static constexpr int NUM_STARFIELD_PENS          = 0x40;
	static constexpr int NUM_SPRITE_PENS             = 0x40;

	void spacefb_main_map(address_map &map) ATTR_COLD;
	void spacefb_main_io_map(address_map &map) ATTR_COLD;
	void spacefb_audio_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<i8035_device> m_audiocpu;
	required_device<samples_device> m_samples;
	required_device<screen_device> m_screen;

	required_shared_ptr<u8> m_videoram;

	u8 m_sound_latch = 0;
	emu_timer *m_interrupt_timer = nullptr;
	std::unique_ptr<u8[]> m_object_present_map;
	u8 m_port_0 = 0;
	u8 m_port_2 = 0;
	u32 m_star_shift_reg = 0;
	double m_color_weights_rg[3]{};
	double m_color_weights_b[2]{};

	void port_0_w(u8 data);
	void port_1_w(u8 data);
	void port_2_w(u8 data);
	u8 audio_p2_r();
	u8 audio_t0_r();
	u8 audio_t1_r();

	TIMER_CALLBACK_MEMBER(interrupt_callback);
	void start_interrupt_timer();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void shift_star_generator();
	void get_starfield_pens(pen_t *pens);
	void draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_sprite_pens(pen_t *pens);
	void draw_bullet(offs_t offs, pen_t pen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip);
	void draw_sprite(offs_t offs, pen_t *pens, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip);
	void draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


u8 spacefb_state::audio_p2_r()
{
	return (m_sound_latch & 0x18) << 1;
}


u8 spacefb_state::audio_t0_r()
{
	return BIT(m_sound_latch, 5);
}


u8 spacefb_state::audio_t1_r()
{
	return BIT(m_sound_latch, 2);
}


void spacefb_state::port_1_w(u8 data)
{
	m_audiocpu->set_input_line(0, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);

	// enemy killed
	if (!BIT(data, 0) && BIT(m_sound_latch, 0))  m_samples->start(0, 0);

	// ship fire
	if (!BIT(data, 6) && BIT(m_sound_latch, 6))  m_samples->start(1, 1);

	/*
	 *  Explosion Noise
	 *
	 *  Actual sample has a bit of attack at the start, but these doesn't seem to be an easy way
	 *  to play the attack part, then loop the middle bit until the sample is turned off
	 *  Fortunately it seems like the recorded sample of the spaceship death is the longest the sample plays for.
	 *  We loop it just in case it runs out
	 */
	if (BIT(data ^ m_sound_latch, 7))
	{
		if (BIT(data, 7))
			// play decaying noise
			m_samples->start(2, 3);
		else
			// start looping noise
			m_samples->start(2, 2, true);
	}

	m_sound_latch = data;
}


static const char *const spacefb_sample_names[] =
{
	"*spacefb",
	"ekilled",
	"shipfire",
	"explode1",
	"explode2",
	nullptr
};


void spacefb_state::spacefb_audio(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC

	SAMPLES(config, m_samples);
	m_samples->set_channels(3);
	m_samples->set_samples_names(spacefb_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 1.0);
}


/*************************************
 *
 *  Port setters
 *
 *************************************/

void spacefb_state::port_0_w(u8 data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_port_0 = data;
}


void spacefb_state::port_2_w(u8 data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_port_2 = data;
}



/*************************************
 *
 *  Video system start
 *
 *  The sprites use a 32 bytes palette PROM, connected to the RGB output this
 *  way:
 *
 *  bit 7 -- 220 ohm resistor  -- BLUE
 *        -- 470 ohm resistor  -- BLUE
 *        -- 220 ohm resistor  -- GREEN
 *        -- 470 ohm resistor  -- GREEN
 *        -- 1  kohm resistor  -- GREEN
 *        -- 220 ohm resistor  -- RED
 *        -- 470 ohm resistor  -- RED
 *  bit 0 -- 1  kohm resistor  -- RED
 *
 *
 *  The schematics show that the bullet color is connected this way,
 *  but this is impossible
 *
 *           860 ohm resistor  -- RED
 *            68 ohm resistor  -- GREEN
 *
 *
 *  The background color is connected this way:
 *
 *  Ra    -- 220 ohm resistor  -- BLUE
 *  Rb    -- 470 ohm resistor  -- BLUE
 *  Ga    -- 220 ohm resistor  -- GREEN
 *  Gb    -- 470 ohm resistor  -- GREEN
 *  Ba    -- 220 ohm resistor  -- RED
 *  Bb    -- 470 ohm resistor  -- RED
 *
 *************************************/

void spacefb_state::video_start()
{
	// compute the color gun weights
	static constexpr int resistances_rg[] = { 1000, 470, 220 };
	static constexpr int resistances_b [] = {       470, 220 };

	compute_resistor_weights(0, 0xff, -1.0,
								3, resistances_rg, m_color_weights_rg, 470, 0,
								2, resistances_b,  m_color_weights_b,  470, 0,
								0, nullptr, nullptr, 0, 0);

	int const width = m_screen->width();
	int const height = m_screen->height();
	m_object_present_map = std::make_unique<u8[]>(width * height);

	// this start value positions the stars to match the flyer screen shot, but most likely, the actual
	// star position is random as the hardware uses whatever value is on the shift register on power-up
	m_star_shift_reg = 0x18f89;

	save_pointer(NAME(m_object_present_map), width * height);
	save_item(NAME(m_port_0));
	save_item(NAME(m_port_2));
	save_item(NAME(m_star_shift_reg));
}



/*************************************
 *
 *  Star field generator
 *
 *************************************/

inline void spacefb_state::shift_star_generator()
{
	m_star_shift_reg = ((m_star_shift_reg << 1) | (BIT(~m_star_shift_reg, 16) ^ BIT(m_star_shift_reg, 4))) & 0x1ffff;
}


void spacefb_state::get_starfield_pens(pen_t *pens)
{
	// generate the pens based on the various enable bits
	int color_contrast_r   = BIT(m_port_2, 0);
	int color_contrast_g   = BIT(m_port_2, 1);
	int color_contrast_b   = BIT(m_port_2, 2);
	int background_red     = BIT(m_port_2, 3);
	int background_blue    = BIT(m_port_2, 4);
	int disable_star_field = BIT(m_port_2, 7);

	for (int i = 0; i < NUM_STARFIELD_PENS; i++)
	{
		u8 gb =  BIT(i, 0) && color_contrast_g && !disable_star_field;
		u8 ga =  BIT(i, 1) && !disable_star_field;
		u8 bb =  BIT(i, 2) && color_contrast_b && !disable_star_field;
		u8 ba = (BIT(i, 3) || background_blue) && !disable_star_field;
		u8 ra = (BIT(i, 4) || background_red) && !disable_star_field;
		u8 rb =  BIT(i, 5) && color_contrast_r && !disable_star_field;

		u8 r = combine_weights(m_color_weights_rg, 0, rb, ra);
		u8 g = combine_weights(m_color_weights_rg, 0, gb, ga);
		u8 b = combine_weights(m_color_weights_b,     bb, ba);

		pens[i] = rgb_t(r, g, b);
	}
}


void spacefb_state::draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_STARFIELD_PENS];

	get_starfield_pens(pens);

	// the shift register is always shifting -- do the portion in the top VBLANK
	if (cliprect.min_y == screen.visible_area().min_y)
	{
		// one cycle delay introduced by IC10 D flip-flop at the end of the VBLANK
		constexpr int clock_count = (SPACEFB_HBSTART - SPACEFB_HBEND) * SPACEFB_VBEND - 1;

		for (int i = 0; i < clock_count; i++)
			shift_star_generator();
	}

	// visible region of the screen
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = SPACEFB_HBEND; x < SPACEFB_HBSTART; x++)
		{
			if (m_object_present_map[(y * bitmap.width()) + x] == 0)
			{
				// draw the star - the 4 possible values come from the effect of the two XOR gates
				switch (m_star_shift_reg & 0x1c0ff)
				{
					case 0x0c0b7:
					case 0x0c0bb:
					case 0x0c0d7:
					case 0x0c0db:
						bitmap.pix(y, x) = pens[BIT(m_star_shift_reg, 8, 6)];
						break;
					default:
						bitmap.pix(y, x) = pens[0];
						break;
				}
			}

			shift_star_generator();
		}
	}

	// do the shifting in the bottom VBLANK
	if (cliprect.max_y == screen.visible_area().max_y)
	{
		constexpr int clock_count = (SPACEFB_HBSTART - SPACEFB_HBEND) * (SPACEFB_VTOTAL - SPACEFB_VBSTART);

		for (int i = 0; i < clock_count; i++)
			shift_star_generator();
	}
}



/*************************************
 *
 *  Sprite/Bullet hardware
 *
 *  Sprites are opaque wrt. each other,
 *  but transparent wrt. to the
 *  star field.
 *
 *************************************/

void spacefb_state::get_sprite_pens(pen_t *pens)
{
	static constexpr double fade_weights[] = { 1.0, 1.5, 2.5, 4.0 };
	const u8 *prom = memregion("proms")->base();

	for (int i = 0; i < NUM_SPRITE_PENS; i++)
	{
		u8 const data = prom[((m_port_0 & 0x40) >> 2) | (i & 0x0f)];

		u8 r = combine_weights(m_color_weights_rg, BIT(data, 0), BIT(data, 1), BIT(data, 2));
		u8 g = combine_weights(m_color_weights_rg, BIT(data, 3), BIT(data, 4), BIT(data, 5));
		u8 b = combine_weights(m_color_weights_b,                BIT(data, 6), BIT(data, 7));

		if (i >> 4)
		{
			double fade_weight = fade_weights[i >> 4];

			// faded pens
			r = (r / fade_weight) + 0.5;
			g = (g / fade_weight) + 0.5;
			b = (b / fade_weight) + 0.5;
		}

		pens[i] = rgb_t(r, g, b);
	}
}


void spacefb_state::draw_bullet(offs_t offs, pen_t pen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip)
{
	u8 const *const gfx = memregion("bullets")->base();

	u8 const code = m_videoram[offs + 0x0200] & 0x3f;
	u8 y = ~m_videoram[offs + 0x0100] - 2;

	for (u8 sy = 0; sy < 4; sy++)
	{
		u8 data = gfx[(code << 2) | sy];
		u8 x = m_videoram[offs + 0x0000];

		u8 const dy = (flip) ? ~y : y;

		if ((dy > cliprect.min_y) && (dy < cliprect.max_y))
		{
			for (u8 sx = 0; sx < 4; sx++)
			{
				if (data & 0x01)
				{
					u16 const dx = 2 * ((flip) ? u8(~x) : x);

					bitmap.pix(dy, dx + 0) = pen;
					bitmap.pix(dy, dx + 1) = pen;

					m_object_present_map[(dy * bitmap.width()) + dx + 0] = 1;
					m_object_present_map[(dy * bitmap.width()) + dx + 1] = 1;
				}

				x++;
				data >>= 1;
			}
		}

		y++;
	}
}


void spacefb_state::draw_sprite(offs_t offs, pen_t *pens, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip)
{
	u8 const *const gfx = memregion("sprites")->base();

	u8 const code = ~m_videoram[offs + 0x0200];
	u8 const color_base = (~m_videoram[offs + 0x0300] & 0x0f) << 2;
	u8 y = ~m_videoram[offs + 0x0100] - 2;

	for (u8 sy = 0; sy < 8; sy++)
	{
		u8 data1 = gfx[0x0000 | (code << 3) | (sy ^ 0x07)];
		u8 data2 = gfx[0x0800 | (code << 3) | (sy ^ 0x07)];

		u8 x = m_videoram[offs + 0x0000] - 3;

		u8 const dy = (flip) ? ~y : y;

		if ((dy > cliprect.min_y) && (dy < cliprect.max_y))
		{
			for (u8 sx = 0; sx < 8; sx++)
			{
				u16 const dx = 2 * ((flip) ? u8(~x) : x);

				u8 const data = ((data1 << 1) & 0x02) | (data2 & 0x01);
				pen_t const pen = pens[color_base | data];

				bitmap.pix(dy, dx + 0) = pen;
				bitmap.pix(dy, dx + 1) = pen;

				m_object_present_map[(dy * bitmap.width()) + dx + 0] = (data != 0);
				m_object_present_map[(dy * bitmap.width()) + dx + 1] = (data != 0);

				x++;
				data1 >>= 1;
				data2 >>= 1;
			}
		}

		y++;
	}
}


void spacefb_state::draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t sprite_pens[NUM_SPRITE_PENS];

	offs_t offs = (m_port_0 & 0x20) ? 0x80 : 0x00;
	int const flip = m_port_0 & 0x01;

	// since the way the schematics show the bullet color connected is impossible, just use pure red for now
	pen_t const bullet_pen = rgb_t(0xff, 0x00, 0x00);

	get_sprite_pens(sprite_pens);

	memset(m_object_present_map.get(), 0, bitmap.width() * bitmap.height());

	do
	{
		if (m_videoram[offs + 0x0300] & 0x20)
			draw_bullet(offs, bullet_pen, bitmap, cliprect, flip);
		else if (m_videoram[offs + 0x0300] & 0x40)
			draw_sprite(offs, sprite_pens, bitmap, cliprect, flip);
	} while (++offs & 0x7f); // end of bank?
}



/*************************************
 *
 *  Video update
 *
 *************************************/

u32 spacefb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	draw_objects(bitmap, cliprect);
	draw_starfield(screen, bitmap, cliprect);

	return 0;
}


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/


TIMER_CALLBACK_MEMBER(spacefb_state::interrupt_callback)
{
	int next_vpos;

	// compute vector and set the interrupt line
	int vpos = m_screen->vpos();
	u8 vector = 0xc7 | ((vpos & 0x40) >> 2) | ((~vpos & 0x40) >> 3);
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, vector); // Z80

	// set up for next interrupt
	if (vpos == SPACEFB_INT_TRIGGER_COUNT_1)
		next_vpos = SPACEFB_INT_TRIGGER_COUNT_2;
	else
		next_vpos = SPACEFB_INT_TRIGGER_COUNT_1;

	m_interrupt_timer->adjust(m_screen->time_until_pos(next_vpos));
}

void spacefb_state::start_interrupt_timer()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(SPACEFB_INT_TRIGGER_COUNT_1));
}



/*************************************
 *
 *  Machine start
 *
 *************************************/

void spacefb_state::machine_start()
{
	m_interrupt_timer = timer_alloc(FUNC(spacefb_state::interrupt_callback), this);

	save_item(NAME(m_sound_latch));
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void spacefb_state::machine_reset()
{
	// the 3 output ports are cleared on reset
	port_0_w(0);
	port_1_w(0);
	port_2_w(0);

	start_interrupt_timer();
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void spacefb_state::spacefb_main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).noprw();
	map(0x8000, 0x83ff).mirror(0x3c00).ram().share(m_videoram);
	map(0xc000, 0xc7ff).mirror(0x3000).ram();
	map(0xc800, 0xcfff).mirror(0x3000).noprw();
}


void spacefb_state::spacefb_audio_map(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x0000, 0x03ff).rom();
}



/*************************************
 *
 *  Port handlers
 *
 *************************************/

void spacefb_state::spacefb_main_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).portr("P1");
	map(0x01, 0x01).portr("P2");
	map(0x02, 0x02).portr("SYSTEM");
	map(0x03, 0x03).portr("DSW");
	map(0x04, 0x07).nopr();  // yes, this is correct (1-of-8 decoder)

	map(0x00, 0x00).mirror(0x04).w(FUNC(spacefb_state::port_0_w));
	map(0x01, 0x01).mirror(0x04).w(FUNC(spacefb_state::port_1_w));
	map(0x02, 0x02).mirror(0x04).w(FUNC(spacefb_state::port_2_w));
	map(0x03, 0x03).mirror(0x04).nopw();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( spacefb )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) // Test ?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


// same as Space Firebird, except for the difficulty switch (replacing 5/6 lives) and 1C_3C (rather than 3C_1C)
static INPUT_PORTS_START( spacedem )
	PORT_INCLUDE( spacefb )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void spacefb_state::spacefb(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, SPACEFB_MAIN_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &spacefb_state::spacefb_main_map);
	m_maincpu->set_addrmap(AS_IO, &spacefb_state::spacefb_main_io_map);

	I8035(config, m_audiocpu, SPACEFB_AUDIO_CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &spacefb_state::spacefb_audio_map);
	m_audiocpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_audiocpu->p2_in_cb().set(FUNC(spacefb_state::audio_p2_r));
	m_audiocpu->t0_in_cb().set(FUNC(spacefb_state::audio_t0_r));
	m_audiocpu->t1_in_cb().set(FUNC(spacefb_state::audio_t1_r));

	config.set_maximum_quantum(attotime::from_hz(180));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(SPACEFB_PIXEL_CLOCK, SPACEFB_HTOTAL, SPACEFB_HBEND, SPACEFB_HBSTART, SPACEFB_VTOTAL, SPACEFB_VBEND, SPACEFB_VBSTART);
	m_screen->set_screen_update(FUNC(spacefb_state::screen_update));

	// audio hardware
	spacefb_audio(config);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

// PCB is revision 04 and has ROMs with suffix 'u'
ROM_START( spacefb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tst-c-u.5e",  0x0000, 0x0800, CRC(79c3527e) SHA1(5f2d9f3a8b573333e40e78222996f556ed6686ea) )
	ROM_LOAD( "tst-c-u.5f",  0x0800, 0x0800, CRC(c0973965) SHA1(f0dcd820c0e0766368ef5d58c29ef090fc5cfdef) )
	ROM_LOAD( "tst-c-u.5h",  0x1000, 0x0800, CRC(02c60ec5) SHA1(43b8553076c7c0e22e0708797c8f3d30fccf82ec) )
	ROM_LOAD( "tst-c-u.5i",  0x1800, 0x0800, CRC(76fd18c7) SHA1(2db271269b8f810eb93e5e86d59251fe1f43769a) )
	ROM_LOAD( "tst-c-u.5j",  0x2000, 0x0800, CRC(df52c97c) SHA1(54032c1ed694911079ffa45545b1e63ec8107fc4) )
	ROM_LOAD( "tst-c-u.5k",  0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "tst-c-u.5m",  0x3000, 0x0800, CRC(6286f534) SHA1(c47d0df85a52c774a4bc26351fdae18795062b6e) )
	ROM_LOAD( "tst-c-u.5n",  0x3800, 0x0800, CRC(1c9f91ee) SHA1(481a309fe9aa9ce6fd18d7d908c18790f594057d) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "tst-v-a.5k",  0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "tst-v-a.6k",  0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

// PCB is revision 03 and has ROMs with suffix 'e'
ROM_START( spacefbe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tst-c-e.5e",  0x0000, 0x0800, CRC(77dda05b) SHA1(b8a42632587260509ba023c7e05d252972f90363) )
	ROM_LOAD( "tst-c-e.5f",  0x0800, 0x0800, CRC(89f0c34a) SHA1(4d8652fb7c4f22ddbac8c2d7ca7df675eaa2a447) )
	ROM_LOAD( "tst-c-e.5h",  0x1000, 0x0800, CRC(c4bcac3e) SHA1(5364d6fc9d3402b2def163dee7c39fe3fe57eea3) )
	ROM_LOAD( "tst-c-e.5i",  0x1800, 0x0800, CRC(61c00a65) SHA1(afc93e320478c70b3ddca8375fd648c9f2572dab) )
	ROM_LOAD( "tst-c-e.5j",  0x2000, 0x0800, CRC(598420b9) SHA1(92ea695177c7297699d1d18f166e98392ef0e0f9) )
	ROM_LOAD( "tst-c-e.5k",  0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "tst-c-e.5m",  0x3000, 0x0800, CRC(6286f534) SHA1(c47d0df85a52c774a4bc26351fdae18795062b6e) )
	ROM_LOAD( "tst-c-e.5n",  0x3800, 0x0800, CRC(1c9f91ee) SHA1(481a309fe9aa9ce6fd18d7d908c18790f594057d) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "tst-v-a.5k",  0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "tst-v-a.6k",  0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

ROM_START( spacefbe2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5e.cpu",      0x0000, 0x0800, CRC(2d406678) SHA1(9dff1980fc5267313f99f9f67d2d83eda8aae00e) ) // only 5e differs to above set, by 2 bytes.
	ROM_LOAD( "tst-c-e.5f",  0x0800, 0x0800, CRC(89f0c34a) SHA1(4d8652fb7c4f22ddbac8c2d7ca7df675eaa2a447) )
	ROM_LOAD( "tst-c-e.5h",  0x1000, 0x0800, CRC(c4bcac3e) SHA1(5364d6fc9d3402b2def163dee7c39fe3fe57eea3) )
	ROM_LOAD( "tst-c-e.5i",  0x1800, 0x0800, CRC(61c00a65) SHA1(afc93e320478c70b3ddca8375fd648c9f2572dab) )
	ROM_LOAD( "tst-c-e.5j",  0x2000, 0x0800, CRC(598420b9) SHA1(92ea695177c7297699d1d18f166e98392ef0e0f9) )
	ROM_LOAD( "tst-c-e.5k",  0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "tst-c-e.5m",  0x3000, 0x0800, CRC(6286f534) SHA1(c47d0df85a52c774a4bc26351fdae18795062b6e) )
	ROM_LOAD( "tst-c-e.5n",  0x3800, 0x0800, CRC(1c9f91ee) SHA1(481a309fe9aa9ce6fd18d7d908c18790f594057d) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "tst-v-a.5k",  0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "tst-v-a.6k",  0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

// CPU PCB is revision 02, video PCB is revision 03. ROMs with handwritten suffix 'a'
ROM_START( spacefba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tst-c-a.5e",  0x0000, 0x0800, CRC(5657bd2f) SHA1(0e615a7dd5efbbf6f543480bc150f45089c41d32) )
	ROM_LOAD( "tst-c-a.5f",  0x0800, 0x0800, CRC(303b0294) SHA1(a2f5637e201739b440e7ea0868d2d5745fbb4f5b) )
	ROM_LOAD( "tst-c-a.5h",  0x1000, 0x0800, CRC(49a26fe5) SHA1(851f62df651aa180b6fa236f4c54ed7791d92a21) )
	ROM_LOAD( "tst-c-a.5i",  0x1800, 0x0800, CRC(c23025da) SHA1(ccc73ca9754b04e49733661cbd9e788b13163100) )
	ROM_LOAD( "tst-c-a.5j",  0x2000, 0x0800, CRC(946bee5d) SHA1(6e668cec5986af3d319bf9aa8962a3d9008d0156) )
	ROM_LOAD( "tst-c-a.5k",  0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "tst-c-a.5m",  0x3000, 0x0800, CRC(4cbe92fc) SHA1(903b617e42f740e94a6edb6a973dc0d57ac0abee) )
	ROM_LOAD( "tst-c-a.5n",  0x3800, 0x0800, CRC(1a798fbf) SHA1(65ff2fe91c2037378314c4a68b2bd21fd167c64a) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "tst-e-20.bin",0x0000, 0x0400, CRC(f7a59492) SHA1(22bdc02c72086c38acd9d9675da54ce6ba3f80a3) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "tst-v-a.5k",  0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "tst-v-a.6k",  0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "mb7052-a.4i", 0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051-a.3n", 0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END


ROM_START( spacefbg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tst-c.5e",    0x0000, 0x0800, CRC(07949110) SHA1(b090e629203c54fc0937d82b0cfe355153a65d6b) )
	ROM_LOAD( "tst-c.5f",    0x0800, 0x0800, CRC(ce591929) SHA1(c9cf7b8a77c108e004e8863b6a08392204e9d434) )
	ROM_LOAD( "tst-c.5h",    0x1000, 0x0800, CRC(55d34ea5) SHA1(d98125e4a33c00285a14cb6cc9880d215b4c29d2) )
	ROM_LOAD( "tst-c.5i",    0x1800, 0x0800, CRC(a11e2881) SHA1(c084a0975b88a981f23a52baa6b8c239dae00e5c) )
	ROM_LOAD( "tst-c.5j",    0x2000, 0x0800, CRC(a6aff352) SHA1(a7fd6b5fe5c76aad726d599142b4cca88109fa10) )
	ROM_LOAD( "tst-c.5k",    0x2800, 0x0800, CRC(f4213603) SHA1(cf39027f2a77cab02d1117025a8eccb868f6a1b0) )
	ROM_LOAD( "5m.cpu",      0x3000, 0x0800, CRC(6286f534) SHA1(c47d0df85a52c774a4bc26351fdae18795062b6e) )
	ROM_LOAD( "5n.cpu",      0x3800, 0x0800, CRC(1c9f91ee) SHA1(481a309fe9aa9ce6fd18d7d908c18790f594057d) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "tst-v.5k",    0x0000, 0x0800, CRC(bacc780d) SHA1(fe498b477bbf7f03fd256de2f799483383a7e819) )
	ROM_LOAD( "tst-v.6k",    0x0800, 0x0800, CRC(1645ff26) SHA1(34cfa0e6221bf53b1bda8609eb14fbcc5fb5bdcd) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

ROM_START( spacebrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sb5e.cpu",    0x0000, 0x0800, CRC(232d66b8) SHA1(d443651819007828a40cea05b6936b762375c48f) )
	ROM_LOAD( "sb5f.cpu",    0x0800, 0x0800, CRC(99504327) SHA1(043182097680b5d6164157055a1a5b95759ca64d) )
	ROM_LOAD( "sb5h.cpu",    0x1000, 0x0800, CRC(49a26fe5) SHA1(851f62df651aa180b6fa236f4c54ed7791d92a21) )
	ROM_LOAD( "sb5i.cpu",    0x1800, 0x0800, CRC(c23025da) SHA1(ccc73ca9754b04e49733661cbd9e788b13163100) )
	ROM_LOAD( "sb5j.cpu",    0x2000, 0x0800, CRC(5e97baf0) SHA1(5e1985b8e3354a0c3454a5e43f80e69f1e1f77c0) )
	ROM_LOAD( "5k.cpu",      0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "sb5m.cpu",    0x3000, 0x0800, CRC(4cbe92fc) SHA1(903b617e42f740e94a6edb6a973dc0d57ac0abee) )
	ROM_LOAD( "sb5n.cpu",    0x3800, 0x0800, CRC(1a798fbf) SHA1(65ff2fe91c2037378314c4a68b2bd21fd167c64a) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "5k.vid",      0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "6k.vid",      0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "spcbird.clr", 0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END

// only a few bytes are different between this and spacebrd above
ROM_START( spacefbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fc51",        0x0000, 0x0800, CRC(5657bd2f) SHA1(0e615a7dd5efbbf6f543480bc150f45089c41d32) )
	ROM_LOAD( "fc52",        0x0800, 0x0800, CRC(303b0294) SHA1(a2f5637e201739b440e7ea0868d2d5745fbb4f5b) )
	ROM_LOAD( "sb5h.cpu",    0x1000, 0x0800, CRC(49a26fe5) SHA1(851f62df651aa180b6fa236f4c54ed7791d92a21) )
	ROM_LOAD( "sb5i.cpu",    0x1800, 0x0800, CRC(c23025da) SHA1(ccc73ca9754b04e49733661cbd9e788b13163100) )
	ROM_LOAD( "fc55",        0x2000, 0x0800, CRC(946bee5d) SHA1(6e668cec5986af3d319bf9aa8962a3d9008d0156) )
	ROM_LOAD( "5k.cpu",      0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "sb5m.cpu",    0x3000, 0x0800, CRC(4cbe92fc) SHA1(903b617e42f740e94a6edb6a973dc0d57ac0abee) )
	ROM_LOAD( "sb5n.cpu",    0x3800, 0x0800, CRC(1a798fbf) SHA1(65ff2fe91c2037378314c4a68b2bd21fd167c64a) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "fb.snd",      0x0000, 0x0400, CRC(f7a59492) SHA1(22bdc02c72086c38acd9d9675da54ce6ba3f80a3) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "fc59",        0x0000, 0x0800, CRC(a00ad16c) SHA1(6130b2250b492b56e3ea94e44f7b2ddf45908d00) )
	ROM_LOAD( "6k.vid",      0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

ROM_START( spacedem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdm-c-5e",    0x0000, 0x0800, CRC(be4b9cbb) SHA1(345ea1e56754e0c8300148b53346dbec50b3608e) )
	ROM_LOAD( "sdm-c-5f",    0x0800, 0x0800, CRC(0814f964) SHA1(0186d11ca98f4b2e4c2572db9d440456370275e7) )
	ROM_LOAD( "sdm-c-5h",    0x1000, 0x0800, CRC(ebfff682) SHA1(e060627de302a9ce125d939d9890739d2154a507) )
	ROM_LOAD( "sdm-c-5i",    0x1800, 0x0800, CRC(dd7e1378) SHA1(94a756036e7d03c42ee896b794cb1f8753a67b91) )
	ROM_LOAD( "sdm-c-5j",    0x2000, 0x0800, CRC(98334fda) SHA1(9990bbfb2aa4d953e531bb49eab1c3a999b78b9c) )
	ROM_LOAD( "sdm-c-5k",    0x2800, 0x0800, CRC(ba4933b2) SHA1(9e5003849185ea35b5929c9a8ae188a87bb522cc) )
	ROM_LOAD( "sdm-c-5m",    0x3000, 0x0800, CRC(14d3c656) SHA1(55522df8c2e484b8d5d4a32bf7cfb2b30dcdab4a) )
	ROM_LOAD( "sdm-c-5n",    0x3800, 0x0800, CRC(7e0e41b0) SHA1(e7dd509ab36e0f9be6350b5fa9de4694224477db) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "sdm-e-20",    0x0000, 0x0400, CRC(55f40a0b) SHA1(8dff27b636f7f1831f71816505e451cf97fc3f98) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "sdm-v-5k",    0x0000, 0x0800, CRC(55758e4d) SHA1(1338b45f76f5a31a5350c953eac36cc543fbe62e) )
	ROM_LOAD( "sdm-v-6k",    0x0800, 0x0800, CRC(3fcbb20c) SHA1(674de509f7b6c5d7c41112881b0c3093b9b176a0) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdm-v-3n",    0x0000, 0x0020, CRC(6d8ad169) SHA1(6ccc931774183e14e28bb9b93223d366fd596f30) )
ROM_END

ROM_START( starwarr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw51.5e",     0x0000, 0x0800, CRC(a0f5e690) SHA1(03b81d88ef6c3eaf2d23f1526f02d4ae5ba569a1) )
	ROM_LOAD( "sw52.5f",     0x0800, 0x0800, CRC(303b0294) SHA1(a2f5637e201739b440e7ea0868d2d5745fbb4f5b) )
	ROM_LOAD( "sw53.5h",     0x1000, 0x0800, CRC(49a26fe5) SHA1(851f62df651aa180b6fa236f4c54ed7791d92a21) )
	ROM_LOAD( "sw54.5i",     0x1800, 0x0800, CRC(c23025da) SHA1(ccc73ca9754b04e49733661cbd9e788b13163100) )
	ROM_LOAD( "sw55.5j",     0x2000, 0x0800, CRC(946bee5d) SHA1(6e668cec5986af3d319bf9aa8962a3d9008d0156) )
	ROM_LOAD( "sw56.5k",     0x2800, 0x0800, CRC(8a2de5f0) SHA1(6e824332be7047bf830545d88f13b30938b40cdb) )
	ROM_LOAD( "sw57.5m",     0x3000, 0x0800, CRC(4cbe92fc) SHA1(903b617e42f740e94a6edb6a973dc0d57ac0abee) )
	ROM_LOAD( "sw58.5n",     0x3800, 0x0800, CRC(1a798fbf) SHA1(65ff2fe91c2037378314c4a68b2bd21fd167c64a) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "sw00.snd",    0x0000, 0x0400, CRC(f7a59492) SHA1(22bdc02c72086c38acd9d9675da54ce6ba3f80a3) )

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "sw59.5k",     0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "sw60.6k",     0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "mb7052.4i",   0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

// Found on Niemer PCB
ROM_START( redbird )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1721.e5",     0x0000, 0x0800, CRC(12926811) SHA1(6c8ea1dac9286aefcd07a9886eaf8143c9aa267d) )
	ROM_LOAD( "1722.fg5",    0x0800, 0x0800, CRC(30ab3872) SHA1(fe78314090012ecd937e25bfdd4dddf557cbfb6c) )
	ROM_LOAD( "1723.gh5",    0x1000, 0x0800, CRC(89740d35) SHA1(1f34ad07b526d08f88bd5cb87df1755f9c18176b) )
	ROM_LOAD( "1724.i5",     0x1800, 0x0800, CRC(61c00a65) SHA1(afc93e320478c70b3ddca8375fd648c9f2572dab) )
	ROM_LOAD( "2725.j5",     0x2000, 0x0800, CRC(ac3fc251) SHA1(27313028bf453981c7605be6e4568b4ffe3edee0) )
	ROM_LOAD( "3726.k5",     0x2800, 0x0800, CRC(cd6ced71) SHA1(b3bd6c7cbd8a05920f06b515499943bbb76ba563) )
	ROM_LOAD( "2727.lm5",    0x3000, 0x0800, CRC(6286f534) SHA1(c47d0df85a52c774a4bc26351fdae18795062b6e) )
	ROM_LOAD( "2728.mn5",    0x3800, 0x0800, CRC(1c9f91ee) SHA1(481a309fe9aa9ce6fd18d7d908c18790f594057d) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "ic20.snd",    0x0000, 0x0400, BAD_DUMP CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) ) // Not dumped on this set

	ROM_REGION( 0x1000, "sprites", 0 )
	ROM_LOAD( "4711.k5",     0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "4712.k6",     0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "bullets", 0 )
	ROM_LOAD( "7052.i4",     0x0000, 0x0100, CRC(feccde96) SHA1(4b2b38e89d8d4e035cddeed4c91bd23e13fe23e8) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331.m3",     0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

} // anonymouse namespace

/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR  NAME       PARENT   MACHINE  INPUT     CLASS          INIT        ROT     COMPANY                       FULLNAME                                FLAGS
GAME( 1980, spacefb,   0,       spacefb, spacefb,  spacefb_state, empty_init, ROT270, "Nintendo",                   "Space Firebird (rev. 04-u)",           MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacefbe,  spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "Nintendo",                   "Space Firebird (rev. 03-e set 1)",     MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacefbe2, spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "Nintendo",                   "Space Firebird (rev. 03-e set 2)",     MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacefba,  spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "Nintendo",                   "Space Firebird (rev. 02-a)",           MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacefbg,  spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "Nintendo (Gremlin license)", "Space Firebird (Gremlin)",             MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacebrd,  spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "bootleg (Karateco)",         "Space Bird (bootleg)",                 MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacefbb,  spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "bootleg",                    "Space Firebird (bootleg)",             MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacedem,  spacefb, spacefb, spacedem, spacefb_state, empty_init, ROT270, "Nintendo (Fortrek license)", "Space Demon",                          MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, starwarr,  spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "bootleg (Potomac Mortgage)", "Star Warrior",                         MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, redbird,   spacefb, spacefb, spacefb,  spacefb_state, empty_init, ROT270, "bootleg (Codematic)",        "Red Bird (bootleg of Space Firebird)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
