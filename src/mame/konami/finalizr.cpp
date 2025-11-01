// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

Finalizer (GX523) (c) 1985 Konami

TODO:
- does Konami SND01 MCU have anything custom or is it the same as 8049?
- bootleg uses ENT0 CLK connected to T1 instead of internal timer, but it doesn't
  look like MAME can handle it with the speed it wants (eg. with set_t0_clk_cb and
  clock_device), so right now it's done with machine().time() when it reads T1.

***************************************************************************/

#include "emu.h"

#include "k005885.h"
#include "konami1.h"
#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class finalizr_state : public driver_device
{
public:
	finalizr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k005885(*this, "k005885"),
		m_palette(*this, "palette")
	{ }

	void finalizr(machine_config &config);
	void finalizrb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mcs48_cpu_device> m_audiocpu;
	required_device<k005885_device> m_k005885;
	required_device<palette_device> m_palette;

	void coin_w(uint8_t data);
	void sound_irq_w(uint8_t data);
	void sound_irqen_w(uint8_t data);
	int bootleg_t1_r();
	void tile_callback(int layer, int attr, int &gfx, int &code, int &color, int &flags, int codebank);
	void palette(palette_device &palette) const;
	void sprite_callback(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int size, bool flip, gfx_element *sgfx, int &code, int &color, int &sx, int &sy, bool flipx, bool flipy);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void nmi_callback(int state);
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};



/***************************************************************************

  The palette PROMs are connected to the RGB output this way:

  bit 7 -- 220  ohm resistor  -- \
        -- 1   kohm resistor  -- | -- 470 ohm pulldown resistor -- GREEN
        -- 470  ohm resistor  -- |
        -- 2.2 kohm resistor  -- /
        -- 220  ohm resistor  -- \
        -- 470  ohm resistor  -- | -- 470 ohm pulldown resistor -- RED
        -- 1   kohm resistor  -- |
  bit 0 -- 2.2 kohm resistor  -- /


  bit 3 -- 220  ohm resistor  -- \
        -- 470  ohm resistor  -- | -- 470 ohm pulldown resistor -- BLUE
        -- 1   kohm resistor  -- |
  bit 0 -- 2.2 kohm resistor  -- /

***************************************************************************/

void finalizr_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances[4] = { 2200, 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, &resistances[0], rweights, 470, 0,
			4, &resistances[0], gweights, 470, 0,
			4, &resistances[0], bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = combine_weights(rweights, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 5);
		bit3 = BIT(color_prom[i], 7);
		int const g = combine_weights(gweights, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(color_prom[i + 0x20], 0);
		bit1 = BIT(color_prom[i + 0x20], 1);
		bit2 = BIT(color_prom[i + 0x20], 2);
		bit3 = BIT(color_prom[i + 0x20], 3);
		int const b = combine_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x40;

	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void finalizr_state::tile_callback(int layer, int attr, int &gfx, int &code, int &color, int &flags, int codebank)
{
	code += (attr & 0xc0) << 2;
	if (layer == 0)
		code += codebank << 10;
	color = attr & 0x0f;
	flags = TILE_FLIPYX((attr & 0x30) >> 4);
	gfx = 0;
}


/**************************************************************************/

void finalizr_state::sprite_callback(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int size, bool flip, gfx_element *sgfx, int &code, int &color, int &sx, int &sy, bool flipx, bool flipy)
{
	sx = 32 + 1 + (sx & 0xff) - (sx & 0x100);
	code = ((code & 0x3ff) << 2) | ((code & 0xc00) >> 10);
	color += 16;

	int width = 1, height = 1;

	if (size >= 0x10)
	{
		// 32x32
		if (flip)
		{
			sx = 256 - sx;
			sy = 224 - sy;
		}

		code &= ~3;
		width = 4;
		height = 4;
	}
	else
	{
		if (flip)
		{
			sx = (BIT(size, 3) ? 280: 272) - sx;
			sy = (BIT(size, 2) ? 248: 240) - sy;
		}

		if (size == 0x00)
		{
			// 16x16
			code &= ~3;
			width = 2;
			height = 2;
		}
		else
		{
			if (size == 0x04)
			{
				// 16x8
				code &= ~1;
				width = 2;
				height = 1;
			}
			else if (size == 0x08)
			{
				// 8x16
				code &= ~2;
				width = 1;
				height = 2;
			}
			else if (size == 0x0c)
			{
				// 8x8
				width = 1;
				height = 1;
			}
		}
	}
	static const int x_offset[4] = {  0,  1,  4,  5 };
	static const int y_offset[4] = {  0,  2,  8, 10 };
	for (int dy = 0; dy < height; dy++)
	{
		const int ey = flipy ? (height - dy - 1) : dy;
		for (int dx = 0; dx < width; dx++)
		{
			const int ex = flipx ? (width - dx - 1) : dx;
			sgfx->transpen(bitmap, cliprect,
					code + x_offset[ex] + y_offset[ey],
					color,
					flipx, flipy,
					sx + dx * 8, sy + dy * 8, 0);
		}
	}
}


uint32_t finalizr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k005885->tilemap_set_scrollx(0, 0, m_k005885->get_xscroll() - 32);
	m_k005885->tilemap_draw(0, screen, bitmap, cliprect, 0, 0);

	m_k005885->sprite_draw(screen, bitmap, cliprect, flip_screen(), m_k005885->get_spriterambank(), 0x200, 0);

	// draw top status region
	const rectangle &visarea = screen.visible_area();
	rectangle clip = cliprect;

	if (flip_screen())
	{
		clip.min_x = visarea.max_x - 31;
		clip.max_x = visarea.max_x;
	}
	else
	{
		clip.min_x = visarea.min_x;
		clip.max_x = visarea.min_x + 31;
	}

	m_k005885->tilemap_draw(1, screen, bitmap, clip, 0, 0);

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(finalizr_state::scanline)
{
	int const scanline = param;

	if (scanline == 240) // vblank irq
		m_k005885->irq_set();
	else if ((scanline % 32) == 0) // timer irq
		m_k005885->nmi_set();
}


void finalizr_state::nmi_callback(int state)
{
	if (state)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void finalizr_state::coin_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}

void finalizr_state::sound_irq_w(uint8_t data)
{
	m_audiocpu->set_input_line(0, ASSERT_LINE);
}

void finalizr_state::sound_irqen_w(uint8_t data)
{
	/*  bit 0x80 goes active low, indicating that the
	    external IRQ being serviced is complete
	    bit 0x40 goes active high to enable the DAC ?
	*/

	if (BIT(~data, 7))
		m_audiocpu->set_input_line(0, CLEAR_LINE);
}

int finalizr_state::bootleg_t1_r()
{
	/*  The clock-out from the MCS48 T0 line should be connected here.
	    Accounting for the MCS48 input clock, and internal/external divisors
	    the frequency here should be 192KHz (MCS48 CLKIN / 48)
	*/

	return (machine().time().as_ticks(m_audiocpu->clock() / 48 * 2)) & 1;
}

void finalizr_state::main_map(address_map &map)
{
	// Konami 005885
	map(0x0000, 0x0004).m(m_k005885, FUNC(k005885_device::regs_map));
//  map(0x0020, 0x003f).w(m_k005885, FUNC(k005885_device::scroll_w));

	map(0x0800, 0x0800).portr("DSW3");
	map(0x0808, 0x0808).portr("DSW2");
	map(0x0810, 0x0810).portr("SYSTEM");
	map(0x0811, 0x0811).portr("P1");
	map(0x0812, 0x0812).portr("P2");
	map(0x0813, 0x0813).portr("DSW1");
	map(0x0818, 0x0818).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x0819, 0x0819).w(FUNC(finalizr_state::coin_w));
	map(0x081a, 0x081a).w("snsnd", FUNC(sn76489a_device::write));   // This address triggers the SN chip to read the data port.
	map(0x081b, 0x081b).nopw();        // Loads the snd command into the snd latch
	map(0x081c, 0x081c).w(FUNC(finalizr_state::sound_irq_w)); // custom sound chip
	map(0x081d, 0x081d).w("soundlatch", FUNC(generic_latch_8_device::write)); // custom sound chip
	map(0x2000, 0x2fff).rw(m_k005885, FUNC(k005885_device::vram_r), FUNC(k005885_device::vram_w));
	map(0x3000, 0x3fff).rw(m_k005885, FUNC(k005885_device::spriteram_r), FUNC(k005885_device::spriteram_w));
	map(0x4000, 0xffff).rom();
}

void finalizr_state::sound_io_map(address_map &map)
{
	map(0x00, 0xff).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( finalizr )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_10
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("P1")
	KONAMI8_MONO_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 150000" )
	PORT_DIPSETTING(    0x10, "50000 300000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Controls ) )         PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( finalizra )
	PORT_INCLUDE( finalizr )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20000 100000" )
	PORT_DIPSETTING(    0x10, "30000 150000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )

	PORT_MODIFY("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
INPUT_PORTS_END



static GFXDECODE_START( gfx_finalizr )
	GFXDECODE_ENTRY( "k005885", 0, gfx_8x8x4_packed_msb, 0, 16*2 )
GFXDECODE_END


void finalizr_state::machine_start()
{
}

void finalizr_state::finalizr(machine_config &config)
{
	// basic machine hardware
	KONAMI1(config, m_maincpu, 18.432_MHz_XTAL / 12); // 1.536MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &finalizr_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(finalizr_state::scanline), "screen", 0, 1);

	I8049(config, m_audiocpu, 18.432_MHz_XTAL / 3); // 6.144MHz
	m_audiocpu->set_addrmap(AS_IO, &finalizr_state::sound_io_map);
	m_audiocpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_audiocpu->p2_out_cb().set(FUNC(finalizr_state::sound_irqen_w));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(36*8, 32*8);
	screen.set_visarea(1*8, 35*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(finalizr_state::screen_update));
	screen.set_palette(m_palette);

	K005885(config, m_k005885, 18.432_MHz_XTAL, gfx_finalizr, m_palette, "screen");
	m_k005885->set_split_tilemap(true);
	m_k005885->set_irq_cb().set_inputline(m_maincpu, M6809_IRQ_LINE, HOLD_LINE);
	m_k005885->set_nmi_cb().set(FUNC(finalizr_state::nmi_callback));
	m_k005885->set_flipscreen_cb().set(FUNC(finalizr_state::flip_screen_set)).invert();
	m_k005885->set_sprite_callback(FUNC(finalizr_state::sprite_callback));
	m_k005885->set_tile_callback(FUNC(finalizr_state::tile_callback));

	PALETTE(config, m_palette, FUNC(finalizr_state::palette), 2*16*16, 32);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	SN76489A(config, "snsnd", 18.432_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "speaker", 0.75);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.325); // unknown DAC
}

void finalizr_state::finalizrb(machine_config &config)
{
	finalizr(config);

	I8749(config.replace(), m_audiocpu, 18.432_MHz_XTAL / 2); // ???
	m_audiocpu->set_addrmap(AS_IO, &finalizr_state::sound_io_map);
	m_audiocpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_audiocpu->p2_out_cb().set(FUNC(finalizr_state::sound_irqen_w));
	m_audiocpu->t1_in_cb().set(FUNC(finalizr_state::bootleg_t1_r));
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( finalizr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "523k01.9c",    0x4000, 0x4000, CRC(716633cb) SHA1(9c21e608b6a73967688fa6aeb5995c20c1b48c74) )
	ROM_LOAD( "523k02.12c",   0x8000, 0x4000, CRC(1bccc696) SHA1(3c29f4a030e76660b5a25347e042e344b0653343) )
	ROM_LOAD( "523k03.13c",   0xc000, 0x4000, CRC(c48927c6) SHA1(9cf6b285034670370ba0246c33e1fe0a057457e7) )

	ROM_REGION( 0x0800, "audiocpu", 0 ) // Konami custom
	ROM_LOAD( "snd01_715-057p.8a", 0x0000, 0x0800, CRC(5459ab95) SHA1(3537b1b3ff0196493a6a03a1578cb2878b1c52bd) )

	ROM_REGION( 0x20000, "k005885", 0 )
	ROM_LOAD16_BYTE( "523h04.5e",    0x00000, 0x4000, CRC(c056d710) SHA1(3fe0ab7ef3bce7298c2a073d0985c33f9dc40062) )
	ROM_LOAD16_BYTE( "523h07.5f",    0x00001, 0x4000, CRC(50e512ba) SHA1(f916afb9df1872f9de571d20b9045b20d9172eaa) )
	ROM_LOAD16_BYTE( "523h05.6e",    0x08000, 0x4000, CRC(ae0d0f76) SHA1(6dd0119e4ba7ebb32ba1ca6395f80d18f1617ce8) )
	ROM_LOAD16_BYTE( "523h08.6f",    0x08001, 0x4000, CRC(79f44e17) SHA1(cb32edc4df9f2209f13fc258fec4e67ee91badef) )
	ROM_LOAD16_BYTE( "523h06.7e",    0x10000, 0x4000, CRC(d2db9689) SHA1(ceb5913716b4da2ddff2e837ddaa04d91e52f9e1) )
	ROM_LOAD16_BYTE( "523h09.7f",    0x10001, 0x4000, CRC(8896dc85) SHA1(91493c6b69655de482f0c2a0cb3662fc0d1b6e45) )
	// 18000-1ffff empty

	ROM_REGION( 0x0240, "proms", 0 ) // PROMs at 2F & 3F are MMI 63S081N (or compatibles), PROMs at 10F & 11F are MMI 6301-1N (or compatibles)
	ROM_LOAD( "523h10.2f",    0x0000, 0x0020, CRC(ec15dd15) SHA1(710384b154a9363fdc88edffda252f1d60e000dc) ) // palette
	ROM_LOAD( "523h11.3f",    0x0020, 0x0020, CRC(54be2e83) SHA1(3200abc7f2238d62d7204ef57a6daa2df150538d) ) // palette
	ROM_LOAD( "523h13.11f",   0x0040, 0x0100, CRC(4e0647a0) SHA1(fb87f878456b8b76bb2c028cb890d2a5c1c3e388) ) // characters
	ROM_LOAD( "523h12.10f",   0x0140, 0x0100, CRC(53166a2a) SHA1(6cdde206036df7176679711f7888d72acee27c8f) ) // sprites
ROM_END

ROM_START( finalizra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.9c",    0x4000, 0x4000, CRC(7d464e5c) SHA1(45b1591a6be713dfc58ab657e61531ea2b9263c1) )
	ROM_LOAD( "2.12c",   0x8000, 0x4000, CRC(383dc94e) SHA1(f192e16e83ae34cc97af07072a4dc68e7c4c362c) )
	ROM_LOAD( "3.13c",   0xc000, 0x4000, CRC(ce177f6e) SHA1(034cbe0c1e2baf9577741b3c222a8b4a8ac8c919) )

	ROM_REGION( 0x0800, "audiocpu", 0 ) // Konami custom
	ROM_LOAD( "snd01_715-057p.8a", 0x0000, 0x0800, CRC(5459ab95) SHA1(3537b1b3ff0196493a6a03a1578cb2878b1c52bd) )

	ROM_REGION( 0x20000, "k005885", 0 )
	ROM_LOAD16_BYTE( "523h04.5e",    0x00000, 0x4000, CRC(c056d710) SHA1(3fe0ab7ef3bce7298c2a073d0985c33f9dc40062) )
	ROM_LOAD16_BYTE( "523h07.5f",    0x00001, 0x4000, CRC(50e512ba) SHA1(f916afb9df1872f9de571d20b9045b20d9172eaa) )
	ROM_LOAD16_BYTE( "523h05.6e",    0x08000, 0x4000, CRC(ae0d0f76) SHA1(6dd0119e4ba7ebb32ba1ca6395f80d18f1617ce8) )
	ROM_LOAD16_BYTE( "523h08.6f",    0x08001, 0x4000, CRC(79f44e17) SHA1(cb32edc4df9f2209f13fc258fec4e67ee91badef) )
	ROM_LOAD16_BYTE( "523h06.7e",    0x10000, 0x4000, CRC(d2db9689) SHA1(ceb5913716b4da2ddff2e837ddaa04d91e52f9e1) )
	ROM_LOAD16_BYTE( "523h09.7f",    0x10001, 0x4000, CRC(8896dc85) SHA1(91493c6b69655de482f0c2a0cb3662fc0d1b6e45) )
	// 18000-1ffff empty

	ROM_REGION( 0x0240, "proms", 0 ) // PROMs at 2F & 3F are MMI 63S081N (or compatibles), PROMs at 10F & 11F are MMI 6301-1N (or compatibles)
	ROM_LOAD( "523h10.2f",    0x0000, 0x0020, CRC(ec15dd15) SHA1(710384b154a9363fdc88edffda252f1d60e000dc) ) // palette
	ROM_LOAD( "523h11.3f",    0x0020, 0x0020, CRC(54be2e83) SHA1(3200abc7f2238d62d7204ef57a6daa2df150538d) ) // palette
	ROM_LOAD( "523h13.11f",   0x0040, 0x0100, CRC(4e0647a0) SHA1(fb87f878456b8b76bb2c028cb890d2a5c1c3e388) ) // characters
	ROM_LOAD( "523h12.10f",   0x0140, 0x0100, CRC(53166a2a) SHA1(6cdde206036df7176679711f7888d72acee27c8f) ) // sprites
ROM_END

ROM_START( finalizrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "finalizr.5",   0x4000, 0x8000, CRC(a55e3f14) SHA1(47f6da214b36cc56be547fa4313afcc5572508a2) )
	ROM_LOAD( "finalizr.6",   0xc000, 0x4000, CRC(ce177f6e) SHA1(034cbe0c1e2baf9577741b3c222a8b4a8ac8c919) )

	ROM_REGION( 0x0800, "audiocpu", 0 ) // 8749
	ROM_LOAD( "d8749hd.bin",  0x0000, 0x0800, CRC(978dfc33) SHA1(13d24ce577b88bf6ec2e970d36dc67a7ec691c55) )

	ROM_REGION( 0x20000, "k005885", 0 )
	ROM_LOAD16_BYTE( "523h04.5e",    0x00000, 0x4000, CRC(c056d710) SHA1(3fe0ab7ef3bce7298c2a073d0985c33f9dc40062) )
	ROM_LOAD16_BYTE( "523h07.5f",    0x00001, 0x4000, CRC(50e512ba) SHA1(f916afb9df1872f9de571d20b9045b20d9172eaa) )
	ROM_LOAD16_BYTE( "523h05.6e",    0x08000, 0x4000, CRC(ae0d0f76) SHA1(6dd0119e4ba7ebb32ba1ca6395f80d18f1617ce8) )
	ROM_LOAD16_BYTE( "523h08.6f",    0x08001, 0x4000, CRC(79f44e17) SHA1(cb32edc4df9f2209f13fc258fec4e67ee91badef) )
	ROM_LOAD16_BYTE( "523h06.7e",    0x10000, 0x4000, CRC(d2db9689) SHA1(ceb5913716b4da2ddff2e837ddaa04d91e52f9e1) )
	ROM_LOAD16_BYTE( "523h09.7f",    0x10001, 0x4000, CRC(8896dc85) SHA1(91493c6b69655de482f0c2a0cb3662fc0d1b6e45) )
	// 18000-1ffff empty

	ROM_REGION( 0x0240, "proms", 0 ) // PROMs at 2F & 3F are MMI 63S081N (or compatibles), PROMs at 10F & 11F are MMI 6301-1N (or compatibles)
	ROM_LOAD( "523h10.2f",    0x0000, 0x0020, CRC(ec15dd15) SHA1(710384b154a9363fdc88edffda252f1d60e000dc) ) // palette
	ROM_LOAD( "523h11.3f",    0x0020, 0x0020, CRC(54be2e83) SHA1(3200abc7f2238d62d7204ef57a6daa2df150538d) ) // palette
	ROM_LOAD( "523h13.11f",   0x0040, 0x0100, CRC(4e0647a0) SHA1(fb87f878456b8b76bb2c028cb890d2a5c1c3e388) ) // characters
	ROM_LOAD( "523h12.10f",   0x0140, 0x0100, CRC(53166a2a) SHA1(6cdde206036df7176679711f7888d72acee27c8f) ) // sprites
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE    INPUT      CLASS           INIT        ROT    COMPANY    FULLNAME                                      FLAGS
GAME( 1985, finalizr,  0,        finalizr,  finalizr,  finalizr_state, empty_init, ROT90, "Konami",  "Finalizer - Super Transformation (set 1)",   MACHINE_SUPPORTS_SAVE )
GAME( 1985, finalizra, finalizr, finalizr,  finalizra, finalizr_state, empty_init, ROT90, "Konami",  "Finalizer - Super Transformation (set 2)",   MACHINE_SUPPORTS_SAVE )
GAME( 1985, finalizrb, finalizr, finalizrb, finalizra, finalizr_state, empty_init, ROT90, "bootleg", "Finalizer - Super Transformation (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
