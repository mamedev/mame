// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/**************************************************************************

  Winners Circle.
  1980-81-82, Corona Co, LTD.

  Driver written by Roberto Fresca.
  Blitter based on roul driver from Roberto Zandona' & Angelo Salese.


  Games running in this hardware:

  * Winners Circle (81, 28*28 PCB),           1981, Corona Co, LTD.
  * Winners Circle (81, 18*22 PCB),           1981, Corona Co, LTD.
  * Winners Circle (82),                      1982, Corona Co, LTD.
  * Le Grandchamps,                           198?, Isermatic France S.A.
  * Ruleta RE-800 (earlier),                  1991, Entretenimientos GEMINIS & GENATRON.
  * Ruleta RE-800 (v1.0),                     1991, Entretenimientos GEMINIS & GENATRON.
  * Ruleta RE-800 (v3.0),                     1992, Entretenimientos GEMINIS & GENATRON.
  * Ruleta RCI (6-players, Spanish),          199?, Entretenimientos GEMINIS & GENATRON.
  * Lucky Roulette Plus (6-players, Spanish), 1990, unknown manufacturer.


  Special thanks to Rob Ragon for his invaluable cooperation.


**************************************************************************

  Game Notes:
  ----------

  * Winners Circle.
  1980-81-82 Corona Co.LTD.

  Four players 7-horses racing game.

  Very rare game, due to massive conversions to any kind of roulette games.

  This game is a milestone. If you ask any old operator about the gambling
  games history, he will say that every started with the Winners Circle.


  * Le Grandchamps.
  Isermatic France S.A.

  Four players 6-horses racing game, similar to Winner Circle
  (note that this one has 6 horses instead of 7).

  It has four independent coin slots of 10-francs each.


**************************************************************************

  Hardware Notes:
  --------------


  * Winners Circle 81 (28*28 pins PCB):

  1x Z80 @ 3 MHz. (main CPU)
  1x Z80 @ 2.4 MHz. (sound CPU)

  1x AY-3-8912A @ 1 MHz.

  16x 8116 (16K x 1bit). DRAM Video (should be set as 32 KB).
  1x 6116 (NVRAM)
  2x 2114 (Sound DRAM)

  4x 2732 (Main program ROMs).
  1x 2716 (Sound program ROM).

  1x 82s123 (Color bipolar PROM)

  1x Xtal @ 24 MHz. (main)
  1x Xtal @ 20 MHz. (video)


  * Winners Circle 82 (18*22 pins PCB):

  1x Z80 @ 2.6 MHz. (main CPU)
  1x Z80 @ 2.6 MHz. (sound CPU)

  1x AY-3-8910 @ 2 MHz? (need clk confirmation).
  1x empty socket (maybe an AY replacement?)
    wih the following pinouts:
    (28 pins)
    Vcc +5v @ Pin 19
    GND @ pins 6 & 16
    Analog Channels @ Pins 1 y 5
    Clock @ pin 15

  16x 8116 (16K x 1bit). DRAM Video (should be set as 32 KB).
  1x 6116 (NVRAM)
  2x 2114 (Sound DRAM)

  4x 2732 (Main program ROMs).
  1x 2716 (Sound program ROM).

  1x 82s123 (Color bipolar PROM)

  1x Xtal @ 18.432 MHz. (main)
  1x Xtal @ 20 MHz. (video)


  * Ruleta RE-800:

  1x Z80 @ 2 MHz. (main CPU) <--- is this correct?
  1x Z80 @ 2.4 MHz. (sound CPU) <--- is this correct?

  1x AY-3-8912A @ 2 MHz.

  16x 8116 (16K x 1bit). DRAM Video (should be set as 32 KB).
  2x 6116 or 1x 6264 (NVRAM). Need to confirm if the whole 8K are mapped/accessed.
  2x 2114 (Sound DRAM)

  1x 27128 (Main program ROMs).
  1x 2716 (Sound program ROM).

  1x 82s123 (Color bipolar PROM)

  1x Xtal @ 16 MHz. (main)
  1x Xtal @ 20 MHz. (video)

  ...

  This hardware could be either way:

  - Single PCB, with 28 pins connector.
  - Dual PCB, with 18/22 pins connectors.
  - Dual PCB, with 28/28 pins connectors.


**************************************************************************

  Pinouts:
  -------

  H(XX) and M(XX) are the molex and pin equivalent.


  * Single Corona PCB (28)

  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M01/M02           GND |01| -12V                  M11 |
  | H08               GND |02| +5V                   M05 |
  |                    -  |03|  -                        |
  |                    -  |04|  -                        |
  |                    -  |05|  -                        |
  |                    -  |06|  -                        |
  |                    -  |07|  -                        |
  |                    -  |08|  -                        |
  |                    -  |09|  -                        |
  |           Total Reset |10| Jackpot Reset             |
  |                    -  |11|  -                        |
  |                    -  |12|  -                        |
  | H11               BET |13| Cursor UP             H12 |
  | H09        Credits IN |14| Credits OUT           H10 |
  | H14      Cursor RIGHT |15| Cursor DOWN           H13 |
  |                    -  |16| Cursor LEFT           H15 |
  |                    -  |17|  -                        |
  | H07      Credits LOCK |18|  -                        |
  | H01          Player 1 |19| Player 3              H03 |
  | H05          Player 5 |20| Player 4              H04 |
  | H02          Player 2 |21| Credits IN counter    M17 |
  | H06          Player 6 |22| Credits OUT counter   M18 |
  | M16              Sync |23| Red                   M13 |
  | M14             Green |24| Blue                  M15 |
  | M06/M07           +5V |25| +5V               M08/M19 |
  | M24       Speaker (+) |26| -5V                   M10 |
  | M09              +12V |27| +12V                  M09 |
  | M03/M23           GND |28| GND               M04/M12 |
  '-----------------------+--+---------------------------'


  * Dual Corona PCB (18*22)

    Top PCB (CS)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  |                    -  |01|  -                        |
  |                    -  |02|  -                        |
  |                    -  |03|  -                        |
  |                    -  |04| Credits LOCK          H07 |
  |                    -  |05|  -                        |
  |           Total Reset |06|  -                        |
  |         Jackpot Reset |07|  -                        |
  |                    -  |08| Credits IN            H09 |
  |                    -  |09| Credits OUT           H10 |
  | H11               BET |10| Cursor LEFT           H15 |
  | H12         Cursor UP |11|  -                        |
  |                    -  |12| Cursor RIGHT          H14 |
  |                    -  |13| Cursor DOWN           H13 |
  |                    -  |14| GND Credits LOCK      H08 |
  |                    -  |15| Player 3              H03 |
  |                    -  |16| Player 6              H06 |
  |                    -  |17| Player 4              H04 |
  |                    -  |18| Player 5              H05 |
  |                    -  |19| Player 1              H01 |
  |                 Reset |20| Player 2              H02 |
  |                    -  |21| Credits IN counter    M17 |
  | M09              +12V |22| Credits OUT counter   M18 |
  '-----------------------+--+---------------------------'

    Bottom PCB (CI)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M01               GND |01| GND                       |
  | M02               GND |02| GND                       |
  | M05               +5V |03| +5V                   M07 |
  | M06               +5V |04| +5V               M08/M19 |
  |                    -  |05|  -                        |
  | M09              +12V |06| +12V                  M09 |
  | M24       Speaker (+) |07| Speaker (-)           M23 |
  |                    -  |08|  -                        |
  |                    -  |09|  -                        |
  |                    -  |10|  -                        |
  |                    -  |11|  -                        |
  |                    -  |12|  -                        |
  |                    -  |13|  -                        |
  |                    -  |14|  -                        |
  | M15              Blue |15| Red                   M13 |
  | M16              Sync |16| Green                 M14 |
  | M03               GND |17| GND                   M12 |
  | M04               GND |18| GND                       |
  '-----------------------+--+---------------------------'


  * Dual Corona PCB (28*28)

    Top PCB (CS)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M08               +5V |01|  -                        |
  |                    -  |02|  -                        |
  |                    -  |03|  -                        |
  |                    -  |04| Credits LOCK          H07 |
  |                    -  |05|  -                        |
  |           Total Reset |06|  -                        |
  |         Jackpot Reset |07|  -                        |
  |                    -  |08| Credits IN            H09 |
  |                    -  |09| Credits OUT           H10 |
  | H11               BET |10| Cursor LEFT           H15 |
  | H12         Cursor UP |11|  -                        |
  |                    -  |12| Cursor RIGHT          H14 |
  |                    -  |13| Cursor DOWN           H13 |
  |                    -  |14| GND Credits LOCK      H08 |
  |                    -  |15| Player 3              H03 |
  |                    -  |16| Player 6              H06 |
  |                    -  |17| Player 4              H04 |
  |                    -  |18| Player 5              H05 |
  |                    -  |19| Player 1              H01 |
  |                 Reset |20| Player 2              H02 |
  | M09        +12V Audio |21| Credits IN counter    M17 |
  |                    -  |22|  -                        |
  | M24       Speaker (+) |23| Credits OUT counter   M18 |
  |                    -  |24| Speaker (-)           M23 |
  | M05/M19           +5V |25| GND                   M04 |
  |                    -  |26|  -                        |
  |                    -  |27|  -                        |
  |                    -  |28|  -                        |
  '-----------------------+--+---------------------------'

    Bottom PCB (CI)
  .-----------------------+--+---------------------------.
  | Molex  -  Solder side |PN| Components side  -  Molex |
  +-----------------------+--+---------------------------+
  | M06               +5V |01| GND                   M01 |
  | M10               -5V |02| GND                   M02 |
  | M09              +12V |03|  -                        |
  | M11              -12V |04|  -                        |
  |                    -  |05|  -                        |
  |                    -  |06|  -                        |
  |                    -  |07|  -                        |
  | M16              Sync |08|  -                        |
  |                    -  |09|  -                        |
  | M15              Blue |10|  -                        |
  |                    -  |11|  -                        |
  |                    -  |12|  -                        |
  | M12         GND video |13|  -                        |
  |                    -  |14|  -                        |
  |                    -  |15|  -                        |
  | M14             Green |16|  -                        |
  | M13               Red |17|  -                        |
  |                    -  |18|  -                        |
  |                    -  |19|  -                        |
  |                    -  |20|  -                        |
  |                    -  |21|  -                        |
  |                    -  |22|  -                        |
  |                    -  |23|  -                        |
  |                    -  |24|  -                        |
  | M07               +5V |25| GND                   M03 |
  |                    -  |26|  -                        |
  |                    -  |27|  -                        |
  |                    -  |28|  -                        |
  '-----------------------+--+---------------------------'


**************************************************************************

  TODO:

  - See why the "rainbow timer" for gamble the earnings is nearly twice
    the slow against the real thing.

  - Check the sound CPU and AY clocks in Winners Circle sets.


**************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "re800.lh"
#include "luckyrlt.lh"


namespace {

#define WC81_MAIN_XTAL      XTAL(24'000'000)    // Main crystal for Winners Circle 28*28 pins PCB's
#define WC82_MAIN_XTAL      XTAL(18'432'000)    // Main crystal for Winners Circle 18*22 pins PCB's
#define RE_MAIN_XTAL        XTAL(16'000'000)    // Main for roulette boards
#define VIDEO_XTAL          XTAL(20'000'000)    // Video circuitry crystal (all)
#define AY_CLK1             1000000             // AY-3-8912 clock for WC81 (28*28 PCB), measured
#define AY_CLK2             2000000             // AY-3-8910 clock for 81b & 82 (18*22 PCB), guessed
#define VIDEOBUF_SIZE       512*512


class corona_state : public driver_device
{
public:
	corona_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void winner81(machine_config &config);
	void winner82(machine_config &config);
	void rcirulet(machine_config &config);
	void luckyrlt(machine_config &config);
	void re800(machine_config &config);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	void blitter_y_w(uint8_t data);
	void blitter_unk_w(uint8_t data);
	void blitter_x_w(uint8_t data);
	void blitter_aux_w(uint8_t data);
	uint8_t blitter_status_r();
	void blitter_trig_wdht_w(uint8_t data);
	void sound_latch_w(uint8_t data);
	uint8_t sound_latch_r();
	void ball_w(uint8_t data);
	uint8_t mux_port_r();
	void mux_port_w(uint8_t data);
	void wc_meters_w(uint8_t data);
	void blitter_execute(int x, int y, int color, int width, int flag);
	void corona_palette(palette_device &palette) const;
	uint32_t screen_update_winner(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_luckyrlt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void luckyrlt_cpu_io_map(address_map &map) ATTR_COLD;
	void luckyrlt_map(address_map &map) ATTR_COLD;
	void luckyrlt_sound_cpu_io_map(address_map &map) ATTR_COLD;
	void luckyrlt_sound_map(address_map &map) ATTR_COLD;
	void re800_cpu_io_map(address_map &map) ATTR_COLD;
	void re800_map(address_map &map) ATTR_COLD;
	void re800_sound_cpu_io_map(address_map &map) ATTR_COLD;
	void re800_sound_map(address_map &map) ATTR_COLD;
	void winner81_cpu_io_map(address_map &map) ATTR_COLD;
	void winner81_map(address_map &map) ATTR_COLD;
	void winner81_sound_cpu_io_map(address_map &map) ATTR_COLD;
	void winner81_sound_map(address_map &map) ATTR_COLD;
	void winner82_cpu_io_map(address_map &map) ATTR_COLD;
	void winner82_map(address_map &map) ATTR_COLD;
	void winner82_sound_cpu_io_map(address_map &map) ATTR_COLD;
	void winner82_sound_map(address_map &map) ATTR_COLD;

	uint8_t m_blitter_x_reg = 0;
	uint8_t m_blitter_y_reg = 0;
	uint8_t m_blitter_aux_reg = 0;
	uint8_t m_blitter_unk_reg = 0;
	std::unique_ptr<uint8_t[]> m_videobuf;
	uint8_t m_lamp = 0;
	uint8_t m_lamp_old = 0;
	uint8_t m_input_selector = 0;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	output_finder<256> m_lamps;
};


/*********************************************
*               Video Hardware               *
*********************************************/

void corona_state::corona_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1;
		int const bit7 = BIT(color_prom[i], 7);
		int const bit6 = BIT(color_prom[i], 6);

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		int const b = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 3);
		int const g = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		int const r = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void corona_state::blitter_y_w(uint8_t data)
{
	m_blitter_y_reg = data;
}

void corona_state::blitter_unk_w(uint8_t data)
{
	m_blitter_unk_reg = data;
}

void corona_state::blitter_x_w(uint8_t data)
{
	m_blitter_x_reg = data;
}

void corona_state::blitter_aux_w(uint8_t data)
{
	m_blitter_aux_reg = data;
}

uint8_t corona_state::blitter_status_r()
{
	//  Code checks bit 6 and/or bit 7
	//
	//  x--- ---- blitter busy
	//  -x-- ---- vblank

	return 0x80 | ((m_screen->vblank() & 1) << 6);
	// return machine().rand() & 0xc0;
}

void corona_state::blitter_execute(int x, int y, int color, int width, int flag)
{
	int const xdir = (flag & 0x10)    ? -1 : 1;
	int const ydir = (!(flag & 0x20)) ? -1 : 1;

	if(width == 0)  // ignored
		return;

	if((flag & 0xc0) == 0)  // square shape / layer clearance
	{
		if(x != 128 || y != 128 || width != 8)
			printf("%02x %02x %02x %02x %02x\n", x, y, color, width, flag);

		for(int yp = 0; yp < 0x100; yp++)
			for(int xp = 0; xp < 0x100; xp++)
				m_videobuf[(yp & 0x1ff) * 512 + (xp & 0x1ff)] = color;
	}
	else  // line shape
	{
		// printf("%02x %02x %02x %02x %02x\n", x, y, color, width, flag);

		for(int i = 0; i < width; i++)
		{
			m_videobuf[(y & 0x1ff) * 512 + (x & 0x1ff)] = color;

			if(flag & 0x40) { x+=xdir; }
			if(flag & 0x80) { y+=ydir; }
		}
	}
}

void corona_state::blitter_trig_wdht_w(uint8_t data)
{
	blitter_execute(m_blitter_x_reg, 0x100 - m_blitter_y_reg, m_blitter_aux_reg & 0xf, data, m_blitter_aux_reg & 0xf0);
}

void corona_state::video_start()
{
	m_videobuf = make_unique_clear<uint8_t[]>(VIDEOBUF_SIZE);

	m_lamp_old = 0;
}

uint32_t corona_state::screen_update_winner(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 256; y++)
		for (int x = 0; x < 256; x++)
			bitmap.pix(y, x) = m_videobuf[y * 512 + x];

	return 0;
}

uint32_t corona_state::screen_update_luckyrlt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 256; y++)
		for (int x = 0; x < 256; x++)
			bitmap.pix(255 - y, x) = m_videobuf[y * 512 + x];

	return 0;
}


/*******************************************
*           Read & Write Handlers          *
*******************************************/

void corona_state::sound_latch_w(uint8_t data)
{
	m_soundlatch->write(data & 0xff);
	m_soundcpu->set_input_line(0, ASSERT_LINE);
}

uint8_t corona_state::sound_latch_r()
{
	m_soundcpu->set_input_line(0, CLEAR_LINE);
	return m_soundlatch->read();
}


void corona_state::ball_w(uint8_t data)
{
	m_lamp = data;

	m_lamps[data] = 1;
	m_lamps[m_lamp_old] = 0;
	m_lamp_old = m_lamp;
}


/********  Multiplexed Inputs  ********/

uint8_t corona_state::mux_port_r()
{
	switch( m_input_selector )
	{
		case 0x01: return ioport("IN0-1")->read();
		case 0x02: return ioport("IN0-2")->read();
		case 0x04: return ioport("IN0-3")->read();
		case 0x08: return ioport("IN0-4")->read();
		case 0x10: return ioport("IN0-5")->read();
		case 0x20: return ioport("IN0-6")->read();
	}

	return 0xff;
}

void corona_state::mux_port_w(uint8_t data)
{
/*  - bits -
    7654 3210
    --xx xxxx   Input selector.
    -x-- ----   Credits In counter pulse.
    x--- ----   Credits Out counter pulse.

   Data is written inverted.

*/
	m_input_selector = (data ^ 0xff) & 0x3f;    // Input Selector

	machine().bookkeeping().coin_counter_w(0, (data ^ 0xff) & 0x40);  // Credits In (mechanical meters)
	machine().bookkeeping().coin_counter_w(1, (data ^ 0xff) & 0x80);  // Credits Out (mechanical meters)

//  logerror("muxsel: %02x \n", m_input_selector);
}

void corona_state::wc_meters_w(uint8_t data)
{
/*  - bits -
    7654 3210
    ---- ---x   Credits In counter pulse.
    ---- --x-   Credits In counter pulse, inserted through coin3 only (lower 4 lines of IN3).
    ---- -x--   Credits Out counter pulse.
    -xxx x---   Unknown (directly connected to the higher 4 lines of IN3).
    x--- ----   Unknown.

   Data is written inverted.

*/
	machine().bookkeeping().coin_counter_w(0, (data ^ 0xff) & 0x01);  // Credits In
	machine().bookkeeping().coin_counter_w(1, (data ^ 0xff) & 0x02);  // Credits In (through Coin 3)
	machine().bookkeeping().coin_counter_w(2, (data ^ 0xff) & 0x04);  // Credits Out

//  popmessage("meters: %02x", (data ^ 0xff));
}


/***************************************************
*               --- MEMORY MAPS ---                *
***************************************************/

/* Winner Circle 1981

  Blitter Ports...
  RE-800   -   Winner

    F0           70
    F1           74
    F2           72
    F3           76
    F4           71

  Writes: (begin) EF/D8 (00)
           71 (FF/00)
           DF (01)
           71 (FF/00)
           D8 (00....) --> snd_latch?
           EF: meters.

  Reads: E8/E9....EA/EB/EC/ED/EE

  RAM data is relocated to B800-B8FF

*/

void corona_state::winner81_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x8fff).ram().share("nvram");
	map(0xb800, 0xb8ff).ram(); // copied from 8000 (0x10 bytes, repeated)
}

void corona_state::winner81_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x70, 0x70).w(FUNC(corona_state::blitter_x_w));
	map(0x71, 0x71).w(FUNC(corona_state::blitter_unk_w));
	map(0x72, 0x72).w(FUNC(corona_state::blitter_trig_wdht_w));
	map(0x74, 0x74).w(FUNC(corona_state::blitter_y_w));
	map(0x75, 0x75).r(FUNC(corona_state::blitter_status_r));
	map(0x76, 0x76).w(FUNC(corona_state::blitter_aux_w));

	map(0xd8, 0xd8).nopw();          // dunno, but is writing 0's very often
	map(0xdf, 0xdf).w(FUNC(corona_state::sound_latch_w));

	map(0xe8, 0xe8).portr("IN0");    // credits for players A, B, C, D
	map(0xe9, 0xe9).portr("IN3");
	map(0xea, 0xea).portr("IN1");    // left & right for all players
	map(0xeb, 0xeb).portr("IN2");    // bet for all players
	map(0xec, 0xec).portr("IN4");
	map(0xed, 0xed).portr("DSW1");   // DIP switches bank #1
	map(0xee, 0xee).portr("DSW2");
	map(0xef, 0xef).w(FUNC(corona_state::wc_meters_w));  // meters: coin1 = bit0, coin2 = bit1, coinout = bit2
}

void corona_state::winner81_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x8000, 0x83ff).ram();
}

void corona_state::winner81_sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(corona_state::sound_latch_r));
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
}

/*  Winners Circle 1982

  Blitter F0-F4
  Status: F5

  Reads:
         F8 ---> DIP switches bank 1.
         F9 ---> Controls for players A & B.
         FA ---> Credits for players A, B, C, D.
         FB ---> Single credits for players A, B, C, D, + fix bits 3, 4, 5, 6 in meters.
         FD ---> Controls for players C & D.
         FF ---> Unknown.

  Writes:
         FC ---> Meters.
         FE ---> Sound Latch (writes 01, 02 and 03 during attract)...
*/

void corona_state::winner82_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).ram().share("nvram");
	map(0x8000, 0x80ff).ram();
}

void corona_state::winner82_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).w(FUNC(corona_state::blitter_x_w));
	map(0xf1, 0xf1).w(FUNC(corona_state::blitter_y_w));
	map(0xf2, 0xf2).w(FUNC(corona_state::blitter_trig_wdht_w));
	map(0xf3, 0xf3).w(FUNC(corona_state::blitter_aux_w));
	map(0xf4, 0xf4).w(FUNC(corona_state::blitter_unk_w));
	map(0xf5, 0xf5).r(FUNC(corona_state::blitter_status_r));

	map(0xf8, 0xf8).portr("DSW1");   // coinage DIP SW
	map(0xf9, 0xf9).portr("IN0");    // controls for players A & B
	map(0xfa, 0xfa).portr("IN1");    // credits for players A, B, C, D
	map(0xfb, 0xfb).portr("IN3");    // single credits for players A, B, C, D, + fix bits 3, 4, 5, 6 in meters
	map(0xfc, 0xfc).w(FUNC(corona_state::wc_meters_w));
	map(0xfd, 0xfd).portr("IN2");    // controls for players C & D
	map(0xfe, 0xfe).w(FUNC(corona_state::sound_latch_w));
	map(0xff, 0xff).portr("DSW2");   // no idea
}

void corona_state::winner82_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
}

void corona_state::winner82_sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(corona_state::sound_latch_r));
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x03).nopw();    // socket for another ay-8910, initialized but never played
}

/* Ruleta RE-800

  Blitter: F0-F4
  Status:  F5
  Ball:    FF

  F4       -W   (FF/00)
  F8       R-   DIP switches? (polled just at game start)
  F9       R-   DIP switches? (polled after reset, and when insert credits)

  FC       -W   mux selector & meters
  FD       R-   muxed port

  FE       -W   snd_latch... writes 02 on events, 07 when ball.

  Port:
  0 - credits
  1 - clear credits
  2 - bet
  3 - left
  4 - right
  5 - up
  6 - down
  7 - unknown

*/

void corona_state::re800_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x87ff).ram().share("nvram");  // 801a comm?
}

void corona_state::re800_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).w(FUNC(corona_state::blitter_x_w));
	map(0xf1, 0xf1).w(FUNC(corona_state::blitter_y_w));
	map(0xf2, 0xf2).w(FUNC(corona_state::blitter_trig_wdht_w));
	map(0xf3, 0xf3).w(FUNC(corona_state::blitter_aux_w));
	map(0xf4, 0xf4).w(FUNC(corona_state::blitter_unk_w));
	map(0xf5, 0xf5).r(FUNC(corona_state::blitter_status_r));
	map(0xf8, 0xf8).portr("IN1");
	map(0xf9, 0xf9).portr("DSW1");
	map(0xfc, 0xfc).w(FUNC(corona_state::mux_port_w));
	map(0xfd, 0xfd).r(FUNC(corona_state::mux_port_r));
	map(0xfe, 0xfe).w(FUNC(corona_state::sound_latch_w));
	map(0xff, 0xff).w(FUNC(corona_state::ball_w));
}

void corona_state::re800_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x8000, 0x83ff).ram();
}

void corona_state::re800_sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(corona_state::sound_latch_r));
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
}


/* Lucky Roulette

  Blitter: F0-F4
  Status:  F5
  Ball:    F9

  F8       R-   DIP switches?
  FA       R-   DIP switches?

  FC       -W   mux selector & meters
  FD       R-   muxed port

  FE       -W   snd_latch...

  Port:
  0 - bet
  1 - right
  2 - down
  3 - left
  4 - up
  5 - credits out
  6 - credits in
  7 - lock (inverted... seems a DIP switch)

*/

void corona_state::luckyrlt_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram().share("nvram");
}

void corona_state::luckyrlt_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf0).w(FUNC(corona_state::blitter_x_w));
	map(0xf1, 0xf1).w(FUNC(corona_state::blitter_y_w));
	map(0xf2, 0xf2).w(FUNC(corona_state::blitter_trig_wdht_w));
	map(0xf3, 0xf3).w(FUNC(corona_state::blitter_aux_w));
	map(0xf4, 0xf4).w(FUNC(corona_state::blitter_unk_w));
	map(0xf5, 0xf5).r(FUNC(corona_state::blitter_status_r));

	map(0xf8, 0xf8).portr("DSW2");
	map(0xf9, 0xf9).w(FUNC(corona_state::ball_w));
	map(0xfa, 0xfa).portr("DSW1");
	map(0xfc, 0xfc).w(FUNC(corona_state::mux_port_w));
	map(0xfd, 0xfd).r(FUNC(corona_state::mux_port_r));
	map(0xfe, 0xfe).w(FUNC(corona_state::sound_latch_w));
}

void corona_state::luckyrlt_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
}

void corona_state::luckyrlt_sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(corona_state::sound_latch_r));
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
}


/*********************************
*         Input Ports            *
*********************************/

static INPUT_PORTS_START( winner81 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("Player 2 - Coin 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("Player 1 - Coin 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Player 4 - Coin 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Player 3 - Coin 1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("Player 2 - Coin 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Player 1 - Coin 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("Player 4 - Coin 2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("Player 3 - Coin 2")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Player 2 - Right")  // B right
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Player 1 - Right")  // A right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)     PORT_NAME("Player 4 - Right")  // D right
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)     PORT_NAME("Player 3 - Right")  // C right
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Player 2 - Left")   // B left
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Player 1 - Left")   // A left
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)     PORT_NAME("Player 4 - Left")   // D left
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)     PORT_NAME("Player 3 - Left")   // C left

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)    PORT_NAME("Player 2 - Bet")  // B bet
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player 1 - Bet")  // A bet
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)        PORT_NAME("Player 4 - Bet")  // D bet
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)        PORT_NAME("Player 3 - Bet")  // C bet
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Player 2 - Credits Out")  // B manual credits out
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Player 1 - Credits Out")  // A manual credits out
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player 4 - Credits Out")  // D manual credits out
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Player 3 - Credits Out")  // C manual credits out

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x01, "x2" )
	PORT_DIPSETTING(    0x02, "x5" )
	PORT_DIPSETTING(    0x03, "x10" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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


static INPUT_PORTS_START( winner82 )
	PORT_START("IN0")  // players A & B controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player A - Bet/Triple")    // A: bet/triple
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)        PORT_NAME("Player A - Credits Out")   // A: credits out
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)     PORT_NAME("Player A - Cursor Left")   // A: cursor left
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)    PORT_NAME("Player A - Cursor Right")  // A: cursor right
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)    PORT_NAME("Player B - Bet/Triple")    // B: bet/triple
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)        PORT_NAME("Player B - Credits Out")   // B: credits out
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)    PORT_NAME("Player B - Cursor Left")   // B: cursor left
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)    PORT_NAME("Player B - Cursor Right")  // B: cursor right

	PORT_START("IN1")  //credits
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("Player B - Coin 2")  // 10 credits / pulse
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Player A - Coin 2")  // 10 credits / pulse
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("Player D - Coin 2")  // 10 credits / pulse
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("Player C - Coin 2")  // 10 credits / pulse
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("Player B - Coin 1")  // 1 credit / pulse
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("Player A - Coin 1")  // 1 credit / pulse
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Player D - Coin 1")  // 1 credit / pulse
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Player C - Coin 1")  // 1 credit / pulse

	PORT_START("IN2")  // players C & D controls
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("Player C - Bet/Triple")   // C: bet/triple
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Player C - Credits Out")  // C: credits out
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Player C - Cursor Right") // C: cursor right
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("Player C - Cursor Left")  // C: cursor left
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Player D - Bet/Triple")   // D: bet/triple
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player D - Credits Out")  // D: credits out
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Player D - Cursor Right") // D: cursor right
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Player D - Cursor Left")  // D: cursor left

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Player B - Coin 3")  // always 1 credit, lockable through DIP switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("Player A - Coin 3")  // always 1 credit, lockable through DIP switch
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Player D - Coin 3")  // always 1 credit, lockable through DIP switch
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Player C - Coin 3")  // always 1 credit, lockable through DIP switch
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")  // bit 3 in meters
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")  // bit 4 in meters
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")  // bit 5 in meters
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O) PORT_NAME("IN3-8")  // bit 6 in meters

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)     PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "x10" )
	PORT_DIPSETTING(    0x01, "x5" )
	PORT_DIPSETTING(    0x02, "x2" )
	PORT_DIPSETTING(    0x03, "x1" )
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
	PORT_DIPNAME( 0x80, 0x00, "Coin 3 Lockout" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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


static INPUT_PORTS_START( re800 )
	// Multiplexed from just one single port
	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1)        PORT_NAME("Player 1 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)        PORT_NAME("Player 1 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player 1 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)     PORT_NAME("Player 1 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)    PORT_NAME("Player 1 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)       PORT_NAME("Player 1 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)     PORT_NAME("Player 1 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)     PORT_NAME("Player 1 - Unknown")

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2)     PORT_NAME("Player 2 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)     PORT_NAME("Player 2 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Player 2 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Player 2 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Player 2 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Player 2 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Player 2 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Player 2 - Unknown")

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Player 3 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Player 3 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Player 3 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Player 3 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Player 3 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Player 3 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Player 3 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Player 3 - Unknown")

	PORT_START("IN0-4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Player 4 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Player 4 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player 4 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("Player 4 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Player 4 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Player 4 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("Player 4 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("Player 4 - Unknown")

	PORT_START("IN0-5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Player 5 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Player 5 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Player 5 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("Player 5 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Player 5 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Player 5 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("Player 5 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O) PORT_NAME("Player 5 - Unknown")

	PORT_START("IN0-6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6)     PORT_NAME("Player 6 - Credits")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)     PORT_NAME("Player 6 - Credits Clear")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Player 6 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DEL)   PORT_NAME("Player 6 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGDN)  PORT_NAME("Player 6 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_HOME)  PORT_NAME("Player 6 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_END)   PORT_NAME("Player 6 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Player 6 - Unknown")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_DIPNAME( 0x80, 0x00, "Credits Lock" )  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( re800v3 )
	// Multiplexed from just one single port
	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1)        PORT_NAME("Player 1 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)        PORT_NAME("Player 1 - Credits Out / Fin (end)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player 1 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)     PORT_NAME("Player 1 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)    PORT_NAME("Player 1 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)       PORT_NAME("Player 1 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)     PORT_NAME("Player 1 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)     PORT_NAME("Player 1 - Unknown")

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2)     PORT_NAME("Player 2 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)     PORT_NAME("Player 2 - Credits Out / Cambia (change)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Player 2 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Player 2 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Player 2 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Player 2 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Player 2 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Player 2 - Unknown")

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Player 3 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Player 3 - Credits Out / Sel (select)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Player 3 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Player 3 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Player 3 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Player 3 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Player 3 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("Player 3 - Unknown")

	PORT_START("IN0-4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Player 4 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Player 4 - Credits Out")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player 4 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("Player 4 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Player 4 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Player 4 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("Player 4 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("Player 4 - Unknown")

	PORT_START("IN0-5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Player 5 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Player 5 - Credits Out")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Player 5 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("Player 5 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Player 5 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Player 5 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("Player 5 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O) PORT_NAME("Player 5 - Unknown")

	PORT_START("IN0-6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6)     PORT_NAME("Player 6 - Credits In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)     PORT_NAME("Player 6 - Credits Out")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Player 6 - Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DEL)   PORT_NAME("Player 6 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGDN)  PORT_NAME("Player 6 - Right")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_HOME)  PORT_NAME("Player 6 - Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_END)   PORT_NAME("Player 6 - Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Player 6 - Unknown")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_F2) PORT_NAME("Settings (in bookkeeping mode)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_DIPNAME( 0x40, 0x40, "Bookkeeping Mode" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Credits Lock" )  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( luckyrlt )
	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Player 1 - Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)    PORT_NAME("Player 1 - Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)     PORT_NAME("Player 1 - Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)     PORT_NAME("Player 1 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)       PORT_NAME("Player 1 - Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)        PORT_NAME("Player 1 - Credits Out")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1)        PORT_NAME("Player 1 - Credits In")
	PORT_DIPNAME( 0x80, 0x00, "Player 1 Credits Lock" )  PORT_DIPLOCATION("DSW1:2")  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Player 2 - Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Player 2 - Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Player 2 - Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Player 2 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Player 2 - Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)     PORT_NAME("Player 2 - Credits Out")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2)     PORT_NAME("Player 2 - Credits In")
	PORT_DIPNAME( 0x80, 0x00, "Player 2 Credits Lock" )  PORT_DIPLOCATION("DSW1:3")  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("Player 3 - Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("Player 3 - Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Player 3 - Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Player 3 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("Player 3 - Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Player 3 - Credits Out")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Player 3 - Credits In")
	PORT_DIPNAME( 0x80, 0x00, "Player 3 Credits Lock" )  PORT_DIPLOCATION("DSW1:4")  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0-4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Player 4 - Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Player 4 - Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("Player 4 - Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("Player 4 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Player 4 - Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Player 4 - Credits Out")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Player 4 - Credits In")
	PORT_DIPNAME( 0x80, 0x00, "Player 4 Credits Lock" )  PORT_DIPLOCATION("DSW1:5")  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0-5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("Player 5 - Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("Player 5 - Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("Player 5 - Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("Player 5 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("Player 5 - Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Player 5 - Credits Out")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("Player 5 - Credits In")
	PORT_DIPNAME( 0x80, 0x00, "Player 5 Credits Lock" )  PORT_DIPLOCATION("DSW1:6")  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0-6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Player 6 - Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGDN)  PORT_NAME("Player 6 - Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_END)   PORT_NAME("Player 6 - Down")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DEL)   PORT_NAME("Player 6 - Left")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_HOME)  PORT_NAME("Player 6 - Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)     PORT_NAME("Player 6 - Credits Out")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6)     PORT_NAME("Player 6 - Credits In")
	PORT_DIPNAME( 0x80, 0x00, "Player 6 Credits Lock" )  PORT_DIPLOCATION("DSW1:7")  // lock the credits in/out
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Test / Bookkeeping" )    PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x81, 0x81, "Max Bet" )   PORT_DIPLOCATION("DSW2:1,8")
	PORT_DIPSETTING(    0x00, "Plain = 10; Line = 30; Chance = 30" )
	PORT_DIPSETTING(    0x80, "Plain = 20; Line = 40; Chance = 50" )
	PORT_DIPSETTING(    0x01, "Plain = 30; Line = 50; Chance = 70" )
	PORT_DIPSETTING(    0x81, "Plain = 40; Line = 70; Chance = 90" )
	PORT_DIPNAME( 0x0e, 0x0e, "Rate (%)" )  PORT_DIPLOCATION("DSW2:2,3,4")
	PORT_DIPSETTING(    0x00, "3.12" )
	PORT_DIPSETTING(    0x02, "6.25" )
	PORT_DIPSETTING(    0x04, "9.40" )
	PORT_DIPSETTING(    0x06, "12.5" )
	PORT_DIPSETTING(    0x08, "15.6" )
	PORT_DIPSETTING(    0x0a, "18.7" )
	PORT_DIPSETTING(    0x0c, "21.9" )
	PORT_DIPSETTING(    0x0e, "25.1" )
	PORT_DIPNAME( 0x10, 0x10, "Reset" )     PORT_DIPLOCATION("DSW2:5")  // don't know how it works
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Pago Doble (Double Pay)" )   PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Pago x5 (Pay x5)" )  PORT_DIPLOCATION("DSW2:7")  // only in PLUS 1990 model
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
INPUT_PORTS_END


/*******************************************
*             Machine Drivers              *
*******************************************/

void corona_state::winner81(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, WC81_MAIN_XTAL/8);  // measured
	m_maincpu->set_addrmap(AS_PROGRAM, &corona_state::winner81_map);
	m_maincpu->set_addrmap(AS_IO, &corona_state::winner81_cpu_io_map);

	Z80(config, m_soundcpu, WC81_MAIN_XTAL/10);  // measured
	m_soundcpu->set_addrmap(AS_PROGRAM, &corona_state::winner81_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &corona_state::winner81_sound_cpu_io_map);
	m_soundcpu->set_periodic_int(FUNC(corona_state::nmi_line_pulse), attotime::from_hz(244));  // 244 Hz (1MHz/16/16/16)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(corona_state::screen_update_winner));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, "palette", FUNC(corona_state::corona_palette), 0x100);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8912(config, "aysnd", AY_CLK1).add_route(ALL_OUTPUTS, "mono", 1.0);  // measured
}


void corona_state::winner82(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, WC82_MAIN_XTAL/8);  // measured
	m_maincpu->set_addrmap(AS_PROGRAM, &corona_state::winner82_map);
	m_maincpu->set_addrmap(AS_IO, &corona_state::winner82_cpu_io_map);

	Z80(config, m_soundcpu, WC82_MAIN_XTAL/8);  // measured
	m_soundcpu->set_addrmap(AS_PROGRAM, &corona_state::winner82_sound_map);  // IM1 instead of NMI
	m_soundcpu->set_addrmap(AS_IO, &corona_state::winner82_sound_cpu_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(corona_state::screen_update_winner));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, "palette", FUNC(corona_state::corona_palette), 0x100);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, "aysnd", AY_CLK2).add_route(ALL_OUTPUTS, "mono", 1.0);  // measured
}


void corona_state::re800(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, RE_MAIN_XTAL/8);  // measured 2MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &corona_state::re800_map);
	m_maincpu->set_addrmap(AS_IO, &corona_state::re800_cpu_io_map);

	Z80(config, m_soundcpu, RE_MAIN_XTAL/8);  // measured 2MHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &corona_state::re800_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &corona_state::re800_sound_cpu_io_map);
	m_soundcpu->set_periodic_int(FUNC(corona_state::nmi_line_pulse), attotime::from_hz(244));  // 244 Hz (1MHz/16/16/16)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 32*8-1);
	m_screen->set_screen_update(FUNC(corona_state::screen_update_winner));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, "palette", FUNC(corona_state::corona_palette), 0x100);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8912(config, "aysnd", AY_CLK2).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void corona_state::rcirulet(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, RE_MAIN_XTAL/8);  // measured 2MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &corona_state::re800_map);
	m_maincpu->set_addrmap(AS_IO, &corona_state::re800_cpu_io_map);

	Z80(config, m_soundcpu, RE_MAIN_XTAL/8);  // measured 2MHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &corona_state::winner82_sound_map);  // IM1 instead of NMI
	m_soundcpu->set_addrmap(AS_IO, &corona_state::winner82_sound_cpu_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 32*8-1);
	m_screen->set_screen_update(FUNC(corona_state::screen_update_winner));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, "palette", FUNC(corona_state::corona_palette), 0x100);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8912(config, "aysnd", AY_CLK2).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void corona_state::luckyrlt(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, RE_MAIN_XTAL/8);  // measured 2MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &corona_state::luckyrlt_map);
	m_maincpu->set_addrmap(AS_IO, &corona_state::luckyrlt_cpu_io_map);

	Z80(config, m_soundcpu, RE_MAIN_XTAL/8);  // measured 2MHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &corona_state::luckyrlt_sound_map);
	m_soundcpu->set_addrmap(AS_IO, &corona_state::luckyrlt_sound_cpu_io_map);
	m_soundcpu->set_periodic_int(FUNC(corona_state::nmi_line_pulse), attotime::from_hz(244));  // 244 Hz (1MHz/16/16/16)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); //not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 30*8-1);
	m_screen->set_screen_update(FUNC(corona_state::screen_update_luckyrlt));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, "palette", FUNC(corona_state::corona_palette), 0x100);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8912(config, "aysnd", AY_CLK1).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/**************** Corona Co,LTD. Hardware ****************
*  > ROM Loading Routines...                             *
*********************************************************/
/***************************************************

  Winners Circle
  Four players 7-horses racing game.
  1980-81-82 Corona Co.LTD.

  PCB silkscreened CRN-MK-1000

  2x Z80.
  1x AY-3-8912.

  Xtal: 1x 24 MHz. (CPU)
        1x 20 MHz. (Video)

  2x 8 DIP switches banks.

  All ICs are scratched to avoid the recognizement.

  Note:
  4_2732_80e0.bin is identical to son_2716_4070.bin,
  for whatever reason on this set

***************************************************/

ROM_START(winner81)
	ROM_REGION( 0x10000, "maincpu", 0 )  // from the 28*28 pins PCB
	ROM_LOAD("1_2732_c61e.bin", 0x0000, 0x1000, CRC(841cdbd1) SHA1(87caeec0a80460c72408ceae28480fe2f3ba3052) )
	ROM_LOAD("2_2732_eafa.bin", 0x1000, 0x1000, CRC(216f71d2) SHA1(913454bc78487c099c854e35d594454077266590) )
	ROM_LOAD("3_2732_121e.bin", 0x2000, 0x1000, CRC(b5849762) SHA1(f62ea11ae5834dd84081d8716798c2ac0b879a35) )
	ROM_LOAD("4_2732_80e0.bin", 0x3000, 0x1000, CRC(13d0d2a6) SHA1(13b102f23e559971c4728fbbe0938341aacbff11) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("son_2716_4070.bin",   0x0000, 0x0800, CRC(547068a8) SHA1(fe0e1272feb0196b14554d7c3cb043212508bfbc) )
	ROM_RELOAD(                     0x0800, 0x0800 ) //reads here during horse race

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "corona_82s123.bin",  0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END


ROM_START(winner81b)
	ROM_REGION( 0x10000, "maincpu", 0 )  // from the 18*22 pins PCB, more close to winner82
	ROM_LOAD("winner_27128_a145.bin",   0x0000, 0x4000, CRC(a9737c8f) SHA1(d1e3b3979d3ef1aa2d8c32d5d56c30165c949e50) )

	ROM_REGION( 0x10000, "soundcpu", 0 )  // IM1 instead of NMI. Identical halves
	ROM_LOAD("son_2732_8ccc.bin",   0x0000, 0x1000, CRC(c944a4ae) SHA1(989ec9f39a7761aa73d08ca39b081cb2c4c75a7c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "corona_82s123.bin",  0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END


ROM_START(winner82)
	ROM_REGION( 0x10000, "maincpu", 0 )  // 18*22 pins PCB??
	ROM_LOAD("p1.32.bin",   0x0000, 0x1000, CRC(5eb58841) SHA1(160ba8a19b0926aab6f47497e625449d35efea2a) )
	ROM_LOAD("p2.32.bin",   0x1000, 0x1000, CRC(52567aeb) SHA1(b4723aff00a18f3cb9ee5c3071ed671f920458cf) )
	ROM_LOAD("p3.32.bin",   0x2000, 0x1000, CRC(1ae24244) SHA1(a10fba097b18607b1bc10d8720a020c1e01e75c3) )
	ROM_LOAD("p4.32.bin",   0x3000, 0x1000, CRC(15631824) SHA1(24d5607b186dc880609dd076012d0bc29d7581ba) )

	ROM_REGION( 0x10000, "soundcpu", 0 )  // IM1 instead of NMI. Identical halves
	ROM_LOAD("son_2732_8ccc.bin",   0x0000, 0x1000, CRC(c944a4ae) SHA1(989ec9f39a7761aa73d08ca39b081cb2c4c75a7c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "corona_82s123.bin",  0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END


/*
  Le Grandchamps.
  Isermatic France S.A.

  Four players 6-horses racing game.
  It has four independent coin slots of 10-francs each.

*/
ROM_START(legrandc)
	ROM_REGION( 0x10000, "maincpu", 0 )  // 18*22 pins PCB??
	ROM_LOAD("t10.bin",  0x0000, 0x0800, CRC(3b8f9293) SHA1(8af15b15f91568c8d8ba4910bac5fa63a05eab6a) )
	ROM_LOAD("t1.bin",   0x0800, 0x0800, CRC(99a8876b) SHA1(eaea6a6daf97f7baa021f6f4f8df4b9c220410b0) )
	ROM_LOAD("t2.bin",   0x1000, 0x0800, CRC(a4658a30) SHA1(a655b12a1669e73963c2861f91d0a8bfa7df8b1f) )
	ROM_LOAD("t3.bin",   0x1800, 0x0800, CRC(8ca8c20e) SHA1(0d4ab3f30189653871eee12385f8515734020b34) )
	ROM_LOAD("t4.bin",   0x2000, 0x0800, CRC(8a558bef) SHA1(9f8560864a60fa4c34ffb4c4b16f05bb4170cb42) )
	ROM_LOAD("t5.bin",   0x2800, 0x0800, CRC(8172f711) SHA1(9504ba3f931719489541e876a109da52175250a4) )

	ROM_REGION( 0x10000, "soundcpu", 0 )  // IM1 instead of NMI. Identical halves
	ROM_LOAD("t7.bin",   0x0000, 0x0800, CRC(aaaaa37a) SHA1(60daf9bf8f1e25da0e55e2d652a3a232f0717e9b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "legrandc_prom.bin",  0x0000, 0x0020, CRC(388e9052) SHA1(0472fb7ba8f24d98afa5d2a14c2304c501f0eef6) )
ROM_END


/***************************************************

  Ruleta RE-800
  Entretenimientos GEMINIS & GENATRON (C) 1991-92

  2x Z80.
  1x AY-3-8912.

  Xtal: 1x 16 MHz. (CPU)
        1x 20 MHz. (Video)

  1 or 2x 8 DIP switches banks.

***************************************************/

ROM_START(re800ea)
	ROM_REGION( 0x10000, "maincpu", 0 )  // seems Genatron RE800, early revision
	ROM_LOAD("ruleta1.128", 0x0000, 0x4000, CRC(e5931763) SHA1(35d47276275e691ae5f4f85bc54992381672df1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("re800snd.16", 0x0000, 0x0800, CRC(54d312fa) SHA1(6ed31f091362f7baa59afef91410fe946894e2ee) )
	ROM_RELOAD(                     0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promcoro.123",   0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END


ROM_START(re800v1)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("re800v1.128", 0x0000, 0x4000, CRC(503647fb) SHA1(ccecb18058a672d955c5f94b0c049e6dd64d12e3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("re800snd.16", 0x0000, 0x0800, CRC(54d312fa) SHA1(6ed31f091362f7baa59afef91410fe946894e2ee) )
	ROM_RELOAD(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promcoro.123",   0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END

ROM_START(re800v1a)  // same version, but with some registers changed.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("76b1.bin", 0x0000, 0x4000, CRC(622b57dd) SHA1(90ac6b845e97ace34d26c250c4e4533b601156d3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("re800snd.16", 0x0000, 0x0800, CRC(54d312fa) SHA1(6ed31f091362f7baa59afef91410fe946894e2ee) )
	ROM_RELOAD(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promcoro.123",   0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END


ROM_START(re800v3)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("re800v3.128", 0x0000, 0x4000, CRC(d08a2a1a) SHA1(d2382e7545da808fb7e5a639eda90b759275983b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("re800snd.16", 0x0000, 0x0800, CRC(54d312fa) SHA1(6ed31f091362f7baa59afef91410fe946894e2ee) )
	ROM_RELOAD(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promcoro.123",   0x0000, 0x0020, BAD_DUMP CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END


/******************************

  Ruleta RCI.
  6-players spanish roulette.

  Entretenimientos GEMINIS.

  2x Z80 @ 2MHz.
  1x AY-3-8910 @ 2MHz.

  18x22 pins Corona hardware.
  Multiplexed inputs.

******************************/

ROM_START(rcirulet)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rc1.256", 0x4000, 0x4000, CRC(be25f548) SHA1(f59ef5d6d047299ff77f28b24517ba6d0a3afc90) )
	ROM_CONTINUE(       0x0000, 0x4000)

	ROM_REGION( 0x10000, "soundcpu", 0 )  // IM1 instead of NMI
	ROM_LOAD("rcson.16",    0x0000, 0x0800, CRC(9ba72a6d) SHA1(0d06ee4952255a93a7f097dd84c5937b01367836) )
	ROM_RELOAD(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )  // color PROM
	ROM_LOAD( "rci_82s123_ic19_1b92.bin",   0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )

	ROM_REGION( 0x0020, "proms2", 0 )  // unknown from video
	ROM_LOAD( "rci_82s123_ic04_1f95.bin",   0x0000, 0x0020, CRC(3a6684b3) SHA1(c9461565a78f1024c6bd4088e4555f1a8020013b) )
ROM_END


/******************************

  Lucky Roulette
  6-players spanish roulette.

  Unknown manufacturer.

******************************/

ROM_START(luckyrlt)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rulaxx.256",  0x0000, 0x8000, CRC(2dd903b7) SHA1(112c335d089f922b9c2bccc3b39e439a12e01725) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("sonrulxx.16", 0x0000, 0x0800, CRC(37943210) SHA1(50cbc91fa52c02553552701393a11c2749d7ad2e) )
	ROM_RELOAD(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "promrulxx.123",  0x0000, 0x0020, CRC(051e5edc) SHA1(2305c056fa1fc21432189af12afb7d54c6569484) )
ROM_END

} // Anonymous namespace


/******************************************
*              Game Drivers               *
******************************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE         INIT        ROT     COMPANY                     FULLNAME                                   FLAGS                     LAYOUT
GAME(  1982, winner82,  0,        winner82, winner82, corona_state, empty_init, ROT0,   "Corona Co, LTD.",          "Winners Circle (82)",                      0 )
GAME(  1981, winner81,  winner82, winner81, winner81, corona_state, empty_init, ROT0,   "Corona Co, LTD.",          "Winners Circle (81, 28*28 PCB)",           MACHINE_IMPERFECT_SOUND )
GAME(  1981, winner81b, winner82, winner82, winner82, corona_state, empty_init, ROT0,   "Corona Co, LTD.",          "Winners Circle (81, 18*22 PCB)",           0 )
GAME(  198?, legrandc,  0,        winner82, winner82, corona_state, empty_init, ROT0,   "Isermatic France S.A.",    "Le Grandchamps",                           0 )
GAMEL( 1991, re800ea,   re800v1,  re800,    re800,    corona_state, empty_init, ROT90,  "Entretenimientos GEMINIS", "Ruleta RE-800 (earlier, no attract)",      0,                        layout_re800 )
GAMEL( 1991, re800v1,   0,        re800,    re800,    corona_state, empty_init, ROT90,  "Entretenimientos GEMINIS", "Ruleta RE-800 (v1.0, set 1)",              0,                        layout_re800 )
GAMEL( 1991, re800v1a,  re800v1,  re800,    re800,    corona_state, empty_init, ROT90,  "Entretenimientos GEMINIS", "Ruleta RE-800 (v1.0, set 2)",              0,                        layout_re800 )
GAMEL( 1991, re800v3,   0,        re800,    re800v3,  corona_state, empty_init, ROT90,  "Entretenimientos GEMINIS", "Ruleta RE-800 (v3.0)",                     MACHINE_IMPERFECT_COLORS, layout_re800 )
GAMEL( 199?, rcirulet,  0,        rcirulet, re800,    corona_state, empty_init, ROT90,  "Entretenimientos GEMINIS", "Ruleta RCI (6-players, Spanish)",          0,                        layout_re800 )
GAMEL( 1990, luckyrlt,  0,        luckyrlt, luckyrlt, corona_state, empty_init, ROT90,  "<unknown>",                "Lucky Roulette Plus (6-players, Spanish)", 0,                        layout_luckyrlt )
