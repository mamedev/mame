// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi

/***************************************************************************

MAGMAX
(c)1985 NihonBussan Co.,Ltd.

Driver by Takahiro Nogi 1999/11/05 -
Additional tweaking by Jarek Burczynski


Stephh's notes (based on the game M68000 code and some tests) :

  - Player 1 Button 2 shall not exist per se, but pressing it speeds the game.
    That's why I mapped it to a different key (F1) to avoid confusion.
    It appears (as well as Player 2 Button 2) in the schematics though.
    However I don't see it in the wiring connector page.
  - DSW2 bit 8 is not referenced in the US manual and only has an effect
    if you have EXACTLY 10 credits after you pressed any START button
    (which means that you need to have 11 credits if you choose a 1 player game
    or 12 credits if you choose a 2 players game).
    When activated, you are given infinite lives (in fact 0x60 = 60 lives)
    for both players, you can still lose parts of the ship but not the main ship.
    See code at 0x0001e6 (ships given at start) and 0x0044e6 (other stuff).

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/rescap.h"
#include "sound/ay8910.h"
#include "sound/flt_biquad.h"
#include "sound/mixer.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_GAINCTRL       (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_GAINCTRL)

#include "logmacro.h"

#define LOGGAINCTRL(...)       LOGMASKED(LOG_GAINCTRL,       __VA_ARGS__)


namespace {

class magmax_state : public driver_device
{
public:
	magmax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_vreg(*this, "vreg")
		, m_scroll_x(*this, "scroll_x")
		, m_scroll_y(*this, "scroll_y")
		, m_rom18b(*this, "user1")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ay(*this, "ay%u", 0U)
		, m_aymixer(*this, "aymixer%u", 0U)
		, m_ayfilter(*this, "ayfilter%u", 0U)
		, m_soundlatch(*this, "soundlatch")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void magmax(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_vreg;
	required_shared_ptr<uint16_t> m_scroll_x;
	required_shared_ptr<uint16_t> m_scroll_y;
	required_region_ptr<uint8_t> m_rom18b;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<ay8910_device, 3> m_ay;
	required_device_array<mixer_device, 3> m_aymixer;
	required_device_array<filter_biquad_device, 4> m_ayfilter;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint8_t m_ls74_clr = 0;
	uint8_t m_ls74_q = 0;
	uint8_t m_gain_control = 0;
	emu_timer *m_interrupt_timer = nullptr;
	uint8_t m_flipscreen = 0;
	std::unique_ptr<uint32_t[]> m_prom_tab;
	bitmap_ind16 m_bitmap;

	void cpu_irq_ack_w(uint16_t data);
	uint8_t sound_r();
	void vreg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ay8910_portb_0_w(uint8_t data);
	void ay8910_porta_0_w(uint8_t data);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(scanline_callback);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mag Max has three 256x4 palette PROMs (one per gun), connected to the
  RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
void magmax_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0-0x0f
	for (int i = 0; i < 0x10; i++)
		palette.set_pen_indirect(i, i);

	// sprites use colors 0x10-0x1f, color 0x1f being transparent
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i + 0x10, ctabentry);
	}

	// background uses all colors (no lookup table)
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i + 0x110, i);

}

void magmax_state::video_start()
{
	uint8_t * prom14d = memregion("user2")->base();

	// Set up save state
	save_item(NAME(m_flipscreen));

	m_prom_tab = std::make_unique<uint32_t[]>(256);

	m_screen->register_screen_bitmap(m_bitmap);

	// Allocate temporary bitmap
	for (int i = 0; i < 256; i++)
	{
		int const v = (prom14d[i] << 4) + prom14d[i + 0x100];
		m_prom_tab[i] = ((v & 0x1f) << 8) | ((v & 0x10) << 10) | ((v & 0xe0) >> 1); // convert data into more useful format
	}
}



uint32_t magmax_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// bit 2 flip screen
	m_flipscreen = *m_vreg & 0x04;

	// copy the background graphics
	if (*m_vreg & 0x40)     // background disable
		bitmap.fill(0, cliprect);
	else
	{
		uint32_t const scroll_h = (*m_scroll_x) & 0x3fff;
		uint32_t const scroll_v = (*m_scroll_y) & 0xff;

		// clear background-over-sprites bitmap
		m_bitmap.fill(0);

		for (int v = 2 * 8; v < 30 * 8; v++) // only for visible area
		{
			uint16_t line_data[256];

			uint32_t const map_v_scr_100 = (scroll_v + v) & 0x100;
			uint32_t rom18d_addr = ((scroll_v + v) & 0xf8) + (map_v_scr_100 << 5);
			uint32_t rom15f_addr = (((scroll_v + v) & 0x07) << 2) + (map_v_scr_100 << 5);
			uint32_t const map_v_scr_1fe_6 =((scroll_v + v) & 0x1fe) << 6;

			pen_t pen_base = 0x110 + 0x20 + (map_v_scr_100 >> 1);

			for (int h = 0; h < 0x100; h++)
			{
				uint32_t ls283 = scroll_h + h;

				if (!map_v_scr_100)
				{
					if (h & 0x80)
						ls283 = ls283 + (m_rom18b[map_v_scr_1fe_6 + (h ^ 0xff)] ^ 0xff);
					else
						ls283 = ls283 + m_rom18b[map_v_scr_1fe_6 + h] + 0xff01;
				}

				uint32_t const prom_data = m_prom_tab[(ls283 >> 6) & 0xff];

				rom18d_addr &= 0x20f8;
				rom18d_addr += (prom_data & 0x1f00) + ((ls283 & 0x38) >>3);

				rom15f_addr &= 0x201c;
				rom15f_addr += (m_rom18b[0x4000 + rom18d_addr]<<5) + ((ls283 & 0x6)>>1);
				rom15f_addr += (prom_data & 0x4000);

				uint32_t const graph_color = (prom_data & 0x0070);

				uint32_t graph_data = m_rom18b[0x8000 + rom15f_addr];
				if ((ls283 & 1))
					graph_data >>= 4;
				graph_data &= 0x0f;

				line_data[h] = pen_base + graph_color + graph_data;

				// priority: background over sprites
				if (map_v_scr_100 && ((graph_data & 0x0c)==0x0c))
					m_bitmap.pix(v, h) = line_data[h];
			}

			if (m_flipscreen)
			{
				uint16_t line_data_flip_x[256];
				for (int i = 0; i < 256; i++)
					line_data_flip_x[i] = line_data[255 - i];
				draw_scanline16(bitmap, 0, 255 - v, 256, line_data_flip_x, nullptr);
			}
			else
				draw_scanline16(bitmap, 0, v, 256, line_data, nullptr);
		}
	}

	// draw the sprites
	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int sy = m_spriteram[offs] & 0xff;

		if (sy)
		{
			int code = m_spriteram[offs + 1] & 0xff;
			int const attr = m_spriteram[offs + 2] & 0xff;
			int const color = (attr & 0xf0) >> 4;
			int flipx = attr & 0x04;
			int flipy = attr & 0x08;

			int sx = (m_spriteram[offs + 3] & 0xff) - 0x80 + 0x100 * (attr & 0x01);
			sy = 239 - sy;

			if (m_flipscreen)
			{
				sx = 255 - 16 - sx;
				sy = 239 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (code & 0x80)    // sprite bankswitch
				code += (*m_vreg & 0x30) * 0x8;

			m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					sx, sy,
					m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x1f));
		}
	}

	if (!(*m_vreg & 0x40))      // background disable
		copybitmap_trans(bitmap, m_bitmap, m_flipscreen, m_flipscreen, 0, 0, cliprect, 0);

	// draw the foreground characters
	for (int offs = 32 * 32 - 1; offs >= 0; offs -= 1)
	{
		//int const page = (*m_vreg >> 3) & 0x1;
		int const code = m_videoram[offs /*+ page*/] & 0xff;

		if (code)
		{
			int sx = (offs % 32);
			int sy = (offs / 32);

			if (m_flipscreen)
			{
				sx = 31 - sx;
				sy = 31 - sy;
			}

			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					code,
					0,
					m_flipscreen, m_flipscreen,
					8 * sx, 8 * sy, 0x0f);
		}
	}
	return 0;
}


void magmax_state::cpu_irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

uint8_t magmax_state::sound_r()
{
	return (m_soundlatch->read() << 1) | m_ls74_q;
}

void magmax_state::ay8910_portb_0_w(uint8_t data)
{
	// bit 0 is input to CLR line of the LS74
	m_ls74_clr = data & 1;
	if (m_ls74_clr == 0)
		m_ls74_q = 0;
}

TIMER_CALLBACK_MEMBER(magmax_state::scanline_callback)
{
	int scanline = param;

	/* bit 0 goes hi whenever line V6 from video part goes lo->hi
	   that is when scanline is 64 and 192 accordingly */
	if (m_ls74_clr != 0)
		m_ls74_q = 1;

	scanline += 128;
	scanline &= 255;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

void magmax_state::machine_start()
{
	// Create interrupt timer
	m_interrupt_timer = timer_alloc(FUNC(magmax_state::scanline_callback), this);

	// Set up save state
	save_item(NAME(m_ls74_clr));
	save_item(NAME(m_ls74_q));
	save_item(NAME(m_gain_control));
}

void magmax_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(64), 64);
}



void magmax_state::ay8910_porta_0_w(uint8_t data)
{
/*There are three AY8910 chips and four(!) separate amplifiers on the board
* Each of AY channels is hardware mapped in following way:
* amplifier 0 gain x 1.00 <- AY0 CHA
* amplifier 1 gain x 1.00 <- AY0 CHB + AY0 CHC + AY1 CHA + AY1 CHB
* amplifier 2 gain x 4.54 (150K/33K) <- AY1 CHC + AY2 CHA
* amplifier 3 gain x 4.54 (150K/33K) <- AY2 CHB + AY2 CHC
*
* Each of the amps has its own analog circuit:
* amp0, amp1 and amp2 are different from each other; amp3 is the same as amp2
*
* Outputs of those amps are inputs to post amps, each having own circuit
* that is partially controlled by AY #0 port A.
* PORT A BIT 0 - control postamp 0 (gain x10.0 | gain x 5.00)
* PORT A BIT 1 - control postamp 1 (gain x4.54 | gain x 2.27)
* PORT A BIT 2 - control postamp 2 (gain x1.00 | gain x 0.50)
* PORT A BIT 3 - control postamp 3 (gain x1.00 | gain x 0.50)
*
* The "control" means assert/clear input pins on chip called 4066 (it is analog switch)
* which results in volume gain (exactly 2 times).
* I use set_output_gain() to emulate the effect.

gain summary:
port A control ON         OFF
amp0 = *1*10.0=10.0  *1*5.0   = 5.0
amp1 = *1*4.54=4.54  *1*2.27  = 2.27
amp2 = *4.54*1=4.54  *4.54*0.5= 2.27
amp3 = *4.54*1=4.54  *4.54*0.5= 2.27
*/

/*
bit0 - SOUND Chan#0 name=AY-3-8910 #0 Ch A

bit1 - SOUND Chan#1 name=AY-3-8910 #0 Ch B
bit1 - SOUND Chan#2 name=AY-3-8910 #0 Ch C
bit1 - SOUND Chan#3 name=AY-3-8910 #1 Ch A
bit1 - SOUND Chan#4 name=AY-3-8910 #1 Ch B

bit2 - SOUND Chan#5 name=AY-3-8910 #1 Ch C
bit2 - SOUND Chan#6 name=AY-3-8910 #2 Ch A

bit3 - SOUND Chan#7 name=AY-3-8910 #2 Ch B
bit3 - SOUND Chan#8 name=AY-3-8910 #2 Ch C
*/

	if (m_gain_control == (data & 0x0f))
		return;

	m_gain_control = data & 0x0f;

	LOGGAINCTRL("gain_ctrl = %2x", data & 0x0f);

	const double mix_resistors[4] = { RES_K(1.0), RES_K(2.2), RES_K(10.0), RES_K(10.0) }; // R35, R33, R32, R34
	for (int i = 0; i < 4; i++)
	{
		// RES_K(5) == (1.0 / ((1.0 / RES_K(10)) + (1.0 / RES_K(10)))) because of the optional extra 10k in parallel in each inverting amplifier circuit
		// the total max number of output gain 'units' of all 4 inputs is 10 + 10/2.2 + 1 + 1 = 16.545454, so we divide the gain amount by this number so we don't clip
		m_ayfilter[i]->set_output_gain(0, (-1.0 * (((m_gain_control & (1 << i)) ? RES_K(10) : RES_K(5)) / mix_resistors[i] )) / 16.545454);
		//m_ayfilter[i]->set_output_gain(0, (m_gain_control & (1 << i)) ? 1.0 : 0.5);
	}
}

void magmax_state::vreg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// VRAM CONTROL REGISTER
	// bit0 - coin counter 1
	// bit1 - coin counter 2
	// bit2 - flip screen (INV)
	// bit3 - page bank to be displayed (PG)
	// bit4 - sprite bank LSB (DP0)
	// bit5 - sprite bank MSB (DP1)
	// bit6 - BG display enable (BE)
	COMBINE_DATA(m_vreg);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}



void magmax_state::main_map(address_map &map)
{
	map(0x000000, 0x013fff).rom();
	map(0x018000, 0x018fff).ram();
	map(0x020000, 0x0207ff).ram().share(m_videoram);
	map(0x028000, 0x0281ff).ram().share(m_spriteram);
	map(0x030000, 0x030001).portr("P1");
	map(0x030002, 0x030003).portr("P2");
	map(0x030004, 0x030005).portr("SYSTEM");
	map(0x030006, 0x030007).portr("DSW");
	map(0x030010, 0x030011).w(FUNC(magmax_state::vreg_w)).share(m_vreg);
	map(0x030012, 0x030013).writeonly().share(m_scroll_x);
	map(0x030014, 0x030015).writeonly().share(m_scroll_y);
	map(0x03001d, 0x03001d).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x03001e, 0x03001f).w(FUNC(magmax_state::cpu_irq_ack_w));
}

void magmax_state::sound_map(address_map &map)
{
	map.global_mask(0x7fff); // A15 not connected
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4000).mirror(0x1fff).rw(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_r), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x6000, 0x67ff).mirror(0x1800).ram();
}

void magmax_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w(m_ay[0], FUNC(ay8910_device::address_data_w));
	map(0x02, 0x03).w(m_ay[1], FUNC(ay8910_device::address_data_w));
	map(0x04, 0x05).w(m_ay[2], FUNC(ay8910_device::address_data_w));
	map(0x06, 0x06).r(FUNC(magmax_state::sound_r));
}


static INPUT_PORTS_START( magmax )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Speed") PORT_CODE(KEYCODE_F1) PORT_TOGGLE   // see notes
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  )   PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )            // see notes
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x000c, "30k 80k 50k+" )
	PORT_DIPSETTING(      0x0008, "50k 120k 70k+" )
	PORT_DIPSETTING(      0x0004, "70k 160k 90k+" )
	PORT_DIPSETTING(      0x0000, "90k 200k 110k+" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:6")     // undocumented in the US manual
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )            PORT_DIPLOCATION("SW2:8")     // see notes
	PORT_DIPSETTING(      0x8000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
INPUT_PORTS_END


static const gfx_layout spritelayout =
{
	16, 16, // 16*16 characters
	512,    // 512 characters
	4,  // 4 bits per pixel
	{ 0, 1, 2, 3 },
	{ 4, 0, 4+512*64*8, 0+512*64*8, 12, 8, 12+512*64*8, 8+512*64*8,
		20, 16, 20+512*64*8, 16+512*64*8, 28, 24, 28+512*64*8, 24+512*64*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8
};


static GFXDECODE_START( gfx_magmax )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x4_packed_lsb,    0,  1 ) // no color codes
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,         1*16, 16 ) // 16 color codes
GFXDECODE_END


void magmax_state::magmax(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000) / 2);   // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &magmax_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(magmax_state::irq1_line_assert));

	Z80(config, m_audiocpu, XTAL(20'000'000) / 8); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &magmax_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &magmax_state::sound_io_map);

	config.set_maximum_quantum(attotime::from_hz(600));


	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(magmax_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_magmax);
	PALETTE(config, m_palette, FUNC(magmax_state::palette), 1*16 + 16*16 + 256, 256);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	FILTER_BIQUAD(config, m_ayfilter[0]).opamp_sk_lowpass_setup(RES_K(10), RES_K(10), RES_M(999.99), RES_R(0.001), CAP_N(22), CAP_N(10)); // R22, R23, nothing(infinite resistance), wire(short), C16, C19
	m_ayfilter[0]->add_route(ALL_OUTPUTS, "speaker", 1.0); // <- gain here is controlled by m_ay[0] IOA0 and resistor R35
	FILTER_BIQUAD(config, m_ayfilter[1]).opamp_sk_lowpass_setup(RES_K(4.7), RES_K(4.7), RES_M(999.99), RES_R(0.001), CAP_N(10), CAP_N(4.7)); // R26, R27, nothing(infinite resistance), wire(short), C18, C14
	m_ayfilter[1]->add_route(ALL_OUTPUTS, "speaker", 1.0); // <- gain here is controlled by m_ay[0] IOA1 and resistor R33
	FILTER_BIQUAD(config, m_ayfilter[2]).opamp_mfb_lowpass_setup(RES_K(33), 0.0, RES_K(150), 0.0, CAP_P(330)); // R24, wire(short), R28, wire(short), C22
	m_ayfilter[2]->add_route(ALL_OUTPUTS, "speaker", 1.0); // <- gain here is controlled by m_ay[0] IOA2 and resistor R32
	FILTER_BIQUAD(config, m_ayfilter[3]).opamp_mfb_lowpass_setup(RES_K(33), 0.0, RES_K(150), 0.0, CAP_P(330)); // R25, wire(short), R31, wire(short), C23
	m_ayfilter[3]->add_route(ALL_OUTPUTS, "speaker", 1.0); // <- gain here is controlled by m_ay[0] IOA3 and resistor R34

	MIXER(config, m_aymixer[0]).add_route(0, m_ayfilter[1], 1.0);
	MIXER(config, m_aymixer[1]).add_route(0, m_ayfilter[2], 1.0);
	MIXER(config, m_aymixer[2]).add_route(0, m_ayfilter[3], 1.0);

	AY8910(config, m_ay[0], XTAL(20'000'000) / 16); // @20G verified on PCB and schematics
	m_ay[0]->port_a_write_callback().set(FUNC(magmax_state::ay8910_porta_0_w));
	m_ay[0]->port_b_write_callback().set(FUNC(magmax_state::ay8910_portb_0_w));
	m_ay[0]->add_route(0, m_ayfilter[0], 1.0);
	m_ay[0]->add_route(1, m_aymixer[0], 1.0);
	m_ay[0]->add_route(2, m_aymixer[0], 1.0);

	AY8910(config, m_ay[1], XTAL(20'000'000) / 16); // @18G verified on PCB and schematics
	m_ay[1]->add_route(0, m_aymixer[0], 1.0);
	m_ay[1]->add_route(1, m_aymixer[0], 1.0);
	m_ay[1]->add_route(2, m_aymixer[1], 1.0);

	AY8910(config, m_ay[2], XTAL(20'000'000) / 16); // @16G verified on PCB and schematics
	m_ay[2]->add_route(0, m_aymixer[1], 1.0);
	m_ay[2]->add_route(1, m_aymixer[2], 1.0);
	m_ay[2]->add_route(2, m_aymixer[2], 1.0);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);
	m_soundlatch->set_separate_acknowledge(true);
}


ROM_START( magmax )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.3b", 0x00001, 0x4000, CRC(33793cbb) SHA1(a0bc0e4be434d9fc8115de8d63c92e942334bc85) )
	ROM_LOAD16_BYTE( "6.3d", 0x00000, 0x4000, CRC(677ef450) SHA1(9003ff1c1c455970c1bd036b0b5e44dae2e379a5) )
	ROM_LOAD16_BYTE( "2.5b", 0x08001, 0x4000, CRC(1a0c84df) SHA1(77ff21de33392a148d7ca69a77acc654260af0db) )
	ROM_LOAD16_BYTE( "7.5d", 0x08000, 0x4000, CRC(01c35e95) SHA1(4f1a0d0463a956d8f9ed425cbeaed6186eb130a5) )
	ROM_LOAD16_BYTE( "3.6b", 0x10001, 0x2000, CRC(d06e6cae) SHA1(94047b2bcf030d34295ff8107f95097ce57efe6b) )
	ROM_LOAD16_BYTE( "8.6d", 0x10000, 0x2000, CRC(790a82be) SHA1(9a25d5a7c87aeef5e736b0f2fb8dde1c9be70039) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "15.17b", 0x00000, 0x2000, CRC(19e7b983) SHA1(b1cd0b728e7cce87d9b1039be179d0915d939a4f) )
	ROM_LOAD( "16.18b", 0x02000, 0x2000, CRC(055e3126) SHA1(8c9b03eb7588512ef17f8c1b731a2fd7cf372bf8) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "23.15g", 0x00000, 0x2000, CRC(a7471da2) SHA1(ec2815a5801bc55955e612173a845399fd493eb7) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "17.3e",  0x00000, 0x2000, CRC(8e305b2e) SHA1(74c318089f6bebafbee31c22302e93a09d3ffa32) )
	ROM_LOAD( "18.5e",  0x02000, 0x2000, CRC(14c55a60) SHA1(fd2a1b434bb65502f0f791995caf1cd869ccd254) )
	ROM_LOAD( "19.6e",  0x04000, 0x2000, CRC(fa4141d8) SHA1(a5279d1ada5a13df14a8bbc18ceeea79f82a4c23) )
	ROM_LOAD( "20.3g",  0x08000, 0x2000, CRC(6fa3918b) SHA1(658bdbdc581732922c986b07746a9601d86ec5a2) )
	ROM_LOAD( "21.5g",  0x0a000, 0x2000, CRC(dd52eda4) SHA1(773e92c918f5b076ce3cae55a33a27c38d958edf) )
	ROM_LOAD( "22.6g",  0x0c000, 0x2000, CRC(4afc98ff) SHA1(a34d63befdb3c749460d1cfb62e15ced52859b9b) )

	ROM_REGION( 0x10000, "user1", 0 ) // surface scroll control
	ROM_LOAD16_BYTE( "4.18b",  0x00000, 0x2000, CRC(1550942e) SHA1(436424d63ca576d13b0f4a3713f009a38e33f2f3) )
	ROM_LOAD16_BYTE( "5.20b",  0x00001, 0x2000, CRC(3b93017f) SHA1(b1b67c2050c8033c29bb74ab909075c39e4f7c6a) )
	// BG control data
	ROM_LOAD( "9.18d",  0x04000, 0x2000, CRC(9ecc9ab8) SHA1(ea5fbd9e9ce09e25f532dc74623e0f7e8464b7f3) ) // surface
	ROM_LOAD( "10.20d", 0x06000, 0x2000, CRC(e2ff7293) SHA1(d93c30f7edac53747efcf840325a8ce5f5e47b32) ) // underground
	// background tiles
	ROM_LOAD( "11.15f", 0x08000, 0x2000, CRC(91f3edb6) SHA1(64e8008cad0e9c42c2ee972c2ee867c7c51cae27) ) // surface
	ROM_LOAD( "12.17f", 0x0a000, 0x2000, CRC(99771eff) SHA1(5a1e2316b4055a1332d9d1f02edee5bc6aae90ac) ) // underground
	ROM_LOAD( "13.18f", 0x0c000, 0x2000, CRC(75f30159) SHA1(d188ccf926e7a842e90ebc1aad3dc20c37d84b98) ) // surface of mechanical level
	ROM_LOAD( "14.20f", 0x0e000, 0x2000, CRC(96babcba) SHA1(fec58ccc1e5cc2cec56658a412b94fe7b989541d) ) // underground of mechanical level

	ROM_REGION( 0x0200, "user2", 0 ) // BG control data
	ROM_LOAD( "mag_b.14d",  0x0000, 0x0100, CRC(a0fb7297) SHA1(e6461050e7e586475343156aae1066b944ceab66) ) // background control PROM
	ROM_LOAD( "mag_c.15d",  0x0100, 0x0100, CRC(d84a6f78) SHA1(f2ce329b1adf39bde6df2eb79be6d144adea65d0) ) // background control PROM

	ROM_REGION( 0x0500, "proms", 0 ) // color PROMs
	ROM_LOAD( "mag_e.10f",  0x0000, 0x0100, CRC(75e4f06a) SHA1(cdaccc3e56df4ac9ace04b93b3bab9a62f1ea6f5) ) // red
	ROM_LOAD( "mag_d.10e",  0x0100, 0x0100, CRC(34b6a6e3) SHA1(af254ccf0d38e1f4644375cd357d468ad4efe450) ) // green
	ROM_LOAD( "mag_a.10d",  0x0200, 0x0100, CRC(a7ea7718) SHA1(4789586d6795644517a18f179b4ae5f23737b21d) ) // blue
	ROM_LOAD( "mag_g.2e",   0x0300, 0x0100, CRC(830be358) SHA1(f412587718040a783c4e6453619930c90daf385e) ) // sprites color lookup table
	ROM_LOAD( "mag_f.13b",  0x0400, 0x0100, CRC(4a6f9a6d) SHA1(65f1e0bfacd1f354ece1b18598a551044c27c4d1) ) // state machine data used for video signals generation (not used in emulation)
ROM_END

ROM_START( magmaxa )
	ROM_REGION( 0x14000, "maincpu", 0 ) // all differ from the parent
	ROM_LOAD16_BYTE( "1.3b", 0x00001, 0x4000, CRC(f112b450) SHA1(ab10d4015b8736c5fc5aaa2f266fb026afeb2658) ) // sldh
	ROM_LOAD16_BYTE( "6.3d", 0x00000, 0x4000, CRC(89a6d9e3) SHA1(99c00fae13201d2204034c079f49f6c6c1260280) ) // sldh
	ROM_LOAD16_BYTE( "2.5b", 0x08001, 0x4000, CRC(53560842) SHA1(b5e918d902e0149282558ca750fa874cac76e43e) ) // sldh
	ROM_LOAD16_BYTE( "7.5d", 0x08000, 0x4000, CRC(e20c2c05) SHA1(9660e29eb73a2b8fae5addbd300023cd8c881ca0) ) // sldh
	ROM_LOAD16_BYTE( "3.6b", 0x10001, 0x2000, CRC(a1276b61) SHA1(ae9a024c817c60b847a1552bbc5af4c9d6aa612c) ) // sldh
	ROM_LOAD16_BYTE( "8.6d", 0x10000, 0x2000, CRC(da172797) SHA1(a713c32218cc9f3ca2fa20e3647b4e4fa9a74609) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "15.17b", 0x00000, 0x2000, CRC(5f2016b7) SHA1(473be0a67e29f8fa614c026d621a2906143e1939) ) // sldh, only other different ROM
	ROM_LOAD( "16.18b", 0x02000, 0x2000, CRC(055e3126) SHA1(8c9b03eb7588512ef17f8c1b731a2fd7cf372bf8) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "23.15g", 0x00000, 0x2000, CRC(a7471da2) SHA1(ec2815a5801bc55955e612173a845399fd493eb7) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "17.3e",  0x00000, 0x2000, CRC(8e305b2e) SHA1(74c318089f6bebafbee31c22302e93a09d3ffa32) )
	ROM_LOAD( "18.5e",  0x02000, 0x2000, CRC(14c55a60) SHA1(fd2a1b434bb65502f0f791995caf1cd869ccd254) )
	ROM_LOAD( "19.6e",  0x04000, 0x2000, CRC(fa4141d8) SHA1(a5279d1ada5a13df14a8bbc18ceeea79f82a4c23) )
	ROM_LOAD( "20.3g",  0x08000, 0x2000, CRC(6fa3918b) SHA1(658bdbdc581732922c986b07746a9601d86ec5a2) )
	ROM_LOAD( "21.5g",  0x0a000, 0x2000, CRC(dd52eda4) SHA1(773e92c918f5b076ce3cae55a33a27c38d958edf) )
	ROM_LOAD( "22.6g",  0x0c000, 0x2000, CRC(4afc98ff) SHA1(a34d63befdb3c749460d1cfb62e15ced52859b9b) )

	ROM_REGION( 0x10000, "user1", 0 ) // surface scroll control
	ROM_LOAD16_BYTE( "4.18b",  0x00000, 0x2000, CRC(1550942e) SHA1(436424d63ca576d13b0f4a3713f009a38e33f2f3) )
	ROM_LOAD16_BYTE( "5.20b",  0x00001, 0x2000, CRC(3b93017f) SHA1(b1b67c2050c8033c29bb74ab909075c39e4f7c6a) )
	// BG control data
	ROM_LOAD( "9.18d",  0x04000, 0x2000, CRC(9ecc9ab8) SHA1(ea5fbd9e9ce09e25f532dc74623e0f7e8464b7f3) ) // surface
	ROM_LOAD( "10.20d", 0x06000, 0x2000, CRC(e2ff7293) SHA1(d93c30f7edac53747efcf840325a8ce5f5e47b32) ) // underground
	// background tiles
	ROM_LOAD( "11.15f", 0x08000, 0x2000, CRC(91f3edb6) SHA1(64e8008cad0e9c42c2ee972c2ee867c7c51cae27) ) // surface
	ROM_LOAD( "12.17f", 0x0a000, 0x2000, CRC(99771eff) SHA1(5a1e2316b4055a1332d9d1f02edee5bc6aae90ac) ) // underground
	ROM_LOAD( "13.18f", 0x0c000, 0x2000, CRC(75f30159) SHA1(d188ccf926e7a842e90ebc1aad3dc20c37d84b98) ) // surface of mechanical level
	ROM_LOAD( "14.20f", 0x0e000, 0x2000, CRC(96babcba) SHA1(fec58ccc1e5cc2cec56658a412b94fe7b989541d) ) // underground of mechanical level

	// the PROMs weren't dumped for this set
	ROM_REGION( 0x0200, "user2", 0 ) // BG control data
	ROM_LOAD( "mag_b.14d",  0x0000, 0x0100, CRC(a0fb7297) SHA1(e6461050e7e586475343156aae1066b944ceab66) ) // background control PROM
	ROM_LOAD( "mag_c.15d",  0x0100, 0x0100, CRC(d84a6f78) SHA1(f2ce329b1adf39bde6df2eb79be6d144adea65d0) ) // background control PROM

	ROM_REGION( 0x0500, "proms", 0 ) // color PROMs
	ROM_LOAD( "mag_e.10f",  0x0000, 0x0100, CRC(75e4f06a) SHA1(cdaccc3e56df4ac9ace04b93b3bab9a62f1ea6f5) ) // red
	ROM_LOAD( "mag_d.10e",  0x0100, 0x0100, CRC(34b6a6e3) SHA1(af254ccf0d38e1f4644375cd357d468ad4efe450) ) // green
	ROM_LOAD( "mag_a.10d",  0x0200, 0x0100, CRC(a7ea7718) SHA1(4789586d6795644517a18f179b4ae5f23737b21d) ) // blue
	ROM_LOAD( "mag_g.2e",   0x0300, 0x0100, CRC(830be358) SHA1(f412587718040a783c4e6453619930c90daf385e) ) // sprites color lookup table
	ROM_LOAD( "mag_f.13b",  0x0400, 0x0100, CRC(4a6f9a6d) SHA1(65f1e0bfacd1f354ece1b18598a551044c27c4d1) ) // state machine data used for video signals generation (not used in emulation)
ROM_END

} // anonymous namespace


GAME( 1985, magmax,  0,      magmax, magmax, magmax_state, empty_init, ROT0, "Nichibutsu", "Mag Max (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, magmaxa, magmax, magmax, magmax, magmax_state, empty_init, ROT0, "Nichibutsu", "Mag Max (set 2)", MACHINE_SUPPORTS_SAVE )
