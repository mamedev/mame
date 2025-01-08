// license:BSD-3-Clause
// copyright-holders:R. Belmont, David Haywood
/************************************************************************************************************

    Sega System M1 medal games board (834-7795 standalone, 837-7571 networked)

    68000 - main CPU
    Z80 - sound CPU
    YM3438 - sound generator
    315-5242 - color encoder
    315-5292 - System 24/Model 1/Model 2 tilemaps
    315-5294 - priority?
    315-5296 - I/O (x2)
    315-5388A - priority?

    20 MHz and 32 MHz oscillators are present near the 68k, Z80, and YM3438.
    A 4.9152 MHz oscillator is near the video chips.

    To get past the boot error on Tinker Bell, F1 is mapped to the cabinet reset switch.

    TODO:
    - Hopper
    - tinkerbl, blicks: throws with "RAM data is BAD" at each soft reset, EEPROM?
    - Bingo Party and Bingo Planet put up a message about ROM version mismatch with the RAM and say to press the reset switch.
      However, when this is done, the code simply locks up (BRA to itself) and doesn't initialize the RAM.
    - Verify sound latch locations on Tinker Bell vs. the comms games

    Network version notes:
    Based on Caribbean Boule the following hardware setup is used:
    - One X-Board (segaxbd.cpp) drives a large rear-projection monitor which all players view to see the main game progress.
    - Multiple M1 boards ("satellite" board) for each player for them to view information privately.
    - One 'link' board which connects everything together.  The link board has audio hardware, a 68K, and a Z80 as
    well as a huge bank of UARTS and toslink connectors, but no video. It's possible the main game logic runs
    on the 'link' board.

    Unfortunately we don't have any dumps of anything other than an M1 board right now.

    Is this related to (or a component of?) bingoc.cpp, the EPR numbers are much lower there tho
    so it's probably an earlier version of the same thing or one of the 'link' boards?

************************************************************************************************************/

#include "emu.h"

#include "315_5296.h"
#include "segaic24.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8251.h"
#include "machine/mb8421.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class systemm1_state : public driver_device
{
public:
	systemm1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_commcpu(*this, "m1comm")
		, m_screen(*this, "screen")
		, m_tiles(*this, "tilemap")
		, m_mixer(*this, "mixer")
		, m_palette(*this, "palette")
		, m_ym(*this, "ym3438")
		, m_io1(*this, "io1")
		, m_io2(*this, "io2")
		, m_soundlatch(*this, "soundlatch%u", 1U)
		, m_soundbank(*this, "soundbank")
	{
	}

	void m1base(machine_config &config);
	void m1comm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
	void mem_comm_map(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;
	void z80_io_map(address_map &map) ATTR_COLD;
	void comm_map(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_commcpu;
	required_device<screen_device> m_screen;
	required_device<segas24_tile_device> m_tiles;
	required_device<segas24_mixer_device> m_mixer;
	required_device<palette_device> m_palette;
	required_device<ym3438_device> m_ym;
	required_device<sega_315_5296_device> m_io1, m_io2;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_memory_bank m_soundbank;

	u16 m_paletteram[0x4000 / sizeof(u16)];

	u16 paletteram_r(offs_t offset);
	void paletteram_w(offs_t offset, u16 data, u16 mem_mask);

	void sound_bank_w(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(scan_irq);

	struct layer_sort
	{
		layer_sort(segas24_mixer_device *_mixer) { mixer = _mixer; }

		bool operator()(int l1, int l2)
		{
			static const int default_pri[12] = {0, 1, 2, 3, 4, 5, 6, 7, -4, -3, -2, -1};
			int p1 = mixer->get_reg(l1) & 7;
			int p2 = mixer->get_reg(l2) & 7;
			if (p1 != p2)
			{
				return p1 - p2 < 0;
			}

			return default_pri[l2] - default_pri[l1] < 0;
		}

		segas24_mixer_device *mixer;
	};
};

u32 systemm1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_mixer->get_reg(13) & 1)
	{
		bitmap.fill(m_palette->black_pen());
		return 0;
	}

	screen.priority().fill(0);
	bitmap.fill(0, cliprect);

	int order[12];
	for (int i = 0; i < 12; i++)
	{
		order[i] = i;
	}

	std::sort(std::begin(order), std::end(order), layer_sort(m_mixer.target()));

	int level = 0;
	for (int i = 0; i < 12; i++)
	{
		if (order[i] < 8)
		{
			m_tiles->draw(screen, bitmap, cliprect, order[i], level, 0);
		}
		else
		{
			level++;
		}
	}

	return 0;
}

// NOTE: both irqs calls tas to work RAM buffers prior to SR flag disable
TIMER_DEVICE_CALLBACK_MEMBER(systemm1_state::scan_irq)
{
	const int scanline = param;

	if (scanline == 384)
	{
		m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE);
	}
	else if (scanline == 0)
	{
		// TODO: unchecked source
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
	}
}

u16 systemm1_state::paletteram_r(offs_t offset)
{
	return m_paletteram[offset];
}

void systemm1_state::paletteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	int r, g, b;
	COMBINE_DATA(m_paletteram + offset);
	data = m_paletteram[offset];

	r = (data & 0x00f) << 4;
	if (data & 0x1000)
		r |= 8;

	g = data & 0x0f0;
	if (data & 0x2000)
		g |= 8;

	b = (data & 0xf00) >> 4;
	if (data & 0x4000)
		b |= 8;

	r |= r >> 5;
	g |= g >> 5;
	b |= b >> 5;

	m_palette->set_pen_color(offset, rgb_t(r, g, b));

	if (data & 0x8000)
	{
		r = 255 - 0.6 * (255 - r);
		g = 255 - 0.6 * (255 - g);
		b = 255 - 0.6 * (255 - b);
	}
	else
	{
		r = 0.6 * r;
		g = 0.6 * g;
		b = 0.6 * b;
	}
	m_palette->set_pen_color(offset + m_palette->entries() / 2, rgb_t(r, g, b));
}

void systemm1_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x100000, 0x17ffff).rom().region("maincpu", 0x80000);

	map(0xb00000, 0xb0ffff).rw(m_tiles, FUNC(segas24_tile_device::tile_r), FUNC(segas24_tile_device::tile_w));
	map(0xb20000, 0xb20001).nopw(); // Horizontal split position (ABSEL)
	map(0xb40000, 0xb40001).nopw(); // Scanline trigger position (XHOUT)
	map(0xb60000, 0xb60001).nopw(); // Frame trigger position (XVOUT)
	map(0xb70000, 0xb70001).nopw(); // Synchronization mode
	map(0xb80000, 0xbfffff).rw(m_tiles, FUNC(segas24_tile_device::char_r), FUNC(segas24_tile_device::char_w));
	map(0xc00000, 0xc01fff).rw(FUNC(systemm1_state::paletteram_r), FUNC(systemm1_state::paletteram_w)).share("paletteram");

	map(0xc04000, 0xc0401f).rw(m_mixer, FUNC(segas24_mixer_device::read), FUNC(segas24_mixer_device::write));

	map(0xe00000, 0xe0003f).rw(m_io1, FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);
	map(0xe40000, 0xe40001).portr("DIP1");
	map(0xe40005, 0xe40005).r(m_soundlatch[1], FUNC(generic_latch_8_device::read)).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0xe80000, 0xe8003f).rw(m_io2, FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0x00ff);

	map(0xf00000, 0xf03fff).mirror(0x0fc000).ram().share("nvram");
}

void systemm1_state::mem_comm_map(address_map &map)
{
	mem_map(map);

	map(0x340000, 0x340fff).rw("dpram", FUNC(mb8421_device::right_r), FUNC(mb8421_device::right_w)).umask16(0x00ff);

	map(0xe40000, 0xe40001).portr("INX");
	map(0xe40002, 0xe40003).portr("INY");
	map(0xe40008, 0xe40009).portr("INZ");
}

// Sound system is based on System 32, minus the 5C68, the second YM, and YM IRQs
void systemm1_state::z80_map(address_map &map)
{
	map(0x0000, 0x9fff).rom().region("soundcpu", 0);
	map(0xa000, 0xbfff).bankr(m_soundbank);
	map(0xe000, 0xffff).ram();
}

void systemm1_state::z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x83).mirror(0x0c).rw(m_ym, FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0xa0, 0xa0).w(FUNC(systemm1_state::sound_bank_w));
	map(0xc0, 0xc0).r(m_soundlatch[0], FUNC(generic_latch_8_device::read)).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
}

void systemm1_state::machine_start()
{
	m_soundbank->configure_entries(0x00, 0x10, memregion("soundcpu")->base(), 0x2000);

	save_item(NAME(m_paletteram));
}

void systemm1_state::sound_bank_w(u8 data)
{
	m_soundbank->set_entry(data & 0x0f);
}

void systemm1_state::comm_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa7ff).rw("dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0xc000, 0xc001).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xe003, 0xe003).nopw(); // ???
}

static INPUT_PORTS_START( tinkerbl )
	PORT_START("IN1_PA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1_PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )

	PORT_START("IN1_PC")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Switch") PORT_CODE(KEYCODE_F1)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Analyzer")

	PORT_START("IN1_PD")
	// Following can't be IPT_SERVICE1, it will collide with IPT_GAMBLE_SERVICE
	// TODO: verify what's for (doesn't increment credits)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Service Switch")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("All Reset")
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1_PE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1_PF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIP1")
	PORT_DIPNAME( 0x03, 0x03, "Expected Payout" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING( 0x02, "86%" )
	PORT_DIPSETTING( 0x01, "89%" )
	PORT_DIPSETTING( 0x03, "92%" )
	PORT_DIPSETTING( 0x00, "96%" )
	PORT_DIPNAME( 0x04, 0x04, "10 Bet Royal" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING( 0x04, "5000" )
	PORT_DIPSETTING( 0x00, "3000" )
	PORT_DIPNAME( 0x08, 0x08, "Double Up Limit" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08, "10000" )
	PORT_DIPSETTING( 0x00, "5000" )
	PORT_DIPNAME( 0x10, 0x10, "Hopper" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x20, 0x20, "Hopper Pay Max" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20, "400" )
	PORT_DIPSETTING( 0x00, "800" )
	PORT_DIPNAME( 0x40, 0x40, "Credit Max" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40, "5000" )
	PORT_DIPSETTING( 0x00, "100000" )
	PORT_DIPNAME( 0x80, 0x80, "Use Joker" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
INPUT_PORTS_END

static INPUT_PORTS_START( blicks )
	PORT_INCLUDE( tinkerbl )

	PORT_MODIFY("DIP1")
	PORT_DIPNAME( 0x03, 0x03, "Set Payout Ratio" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING( 0x02, "84%" )
	PORT_DIPSETTING( 0x01, "88%" )
	PORT_DIPSETTING( 0x03, "92%" )
	PORT_DIPSETTING( 0x00, "96%" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3")
	PORT_DIPNAME( 0x08, 0x08, "Double Up Limit" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08, "10000" )
	PORT_DIPSETTING( 0x00, "5000" )
	PORT_DIPNAME( 0x10, 0x10, "Hopper" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x20, 0x20, "Hopper Pay Max" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20, "400" )
	PORT_DIPSETTING( 0x00, "800" )
	PORT_DIPNAME( 0x40, 0x40, "Credit Max" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40, "2000" )
	PORT_DIPSETTING( 0x00, "10000" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8")
INPUT_PORTS_END

static INPUT_PORTS_START( bingpty )
	PORT_START("IN1_PA")
	PORT_DIPNAME( 0x01, 0x01, "DIPA1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPA2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPA3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPA4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPA5" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPA6" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPA7" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPA8" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("IN1_PB")
	PORT_DIPNAME( 0x01, 0x01, "DIPB1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPB2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPB3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPB4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPB5" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPB6" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPB7" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPB8" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("IN1_PC")
	PORT_DIPNAME( 0x01, 0x01, "DIPC1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPC2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPC3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPC4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPC5" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPC6" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPC7" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPC8" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("IN1_PD")
	PORT_DIPNAME( 0x01, 0x01, "DIPD1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPD2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPD3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPD4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPD5" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPD6" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_DIPNAME( 0x80, 0x80, "DIPD8" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("IN1_PE")
	PORT_DIPNAME( 0x01, 0x01, "DIPE1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPE2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPE3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPE4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPE5" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPE6" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPE7" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPE8" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("IN1_PF")
	PORT_DIPNAME( 0x01, 0x01, "DIPF1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPF2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Switch") PORT_CODE(KEYCODE_F1)
	PORT_DIPNAME( 0x08, 0x08, "DIPF4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPF5" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPF6" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPF7" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPF8" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("INX")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INY")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INZ")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIP1")
	PORT_DIPNAME( 0x01, 0x01, "DIP1-1" )
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP1-2" )
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP1-3" )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP1-4" )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP1-5" )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP1-6" )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP1-7" )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP1-8" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

void systemm1_state::m1base(machine_config &config)
{
	M68000(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &systemm1_state::mem_map);

	Z80(config, m_soundcpu, 4'000'000);
	m_soundcpu->set_addrmap(AS_PROGRAM, &systemm1_state::z80_map);
	m_soundcpu->set_addrmap(AS_IO, &systemm1_state::z80_io_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	// TODO: from System 24, might not be accurate for System M1
	m_screen->set_raw(XTAL(16'000'000), 656, 0 /*+69*/, 496 /*+69*/, 424, 0 /*+25*/, 384 /*+25*/);
	m_screen->set_screen_update(FUNC(systemm1_state::screen_update));

	TIMER(config, "scantimer").configure_scanline(FUNC(systemm1_state::scan_irq), m_screen, 0, 1);

	PALETTE(config, m_palette).set_entries(8192);

	S24TILE(config, m_tiles, 0, 0x3fff);
	m_tiles->set_palette(m_palette);

	S24MIXER(config, m_mixer, 0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM3438(config, m_ym, XTAL(8'000'000));
	m_ym->add_route(0, "lspeaker", 0.40);
	m_ym->add_route(1, "rspeaker", 0.40);

	SEGA_315_5296(config, m_io1, XTAL(16'000'000));
	m_io1->in_pa_callback().set_ioport("IN1_PA");
	m_io1->in_pb_callback().set_ioport("IN1_PB");
	m_io1->in_pc_callback().set_ioport("IN1_PC");
	m_io1->in_pd_callback().set_ioport("IN1_PD");
	m_io1->in_pe_callback().set_ioport("IN1_PE");
	m_io1->in_pf_callback().set_ioport("IN1_PF");

	SEGA_315_5296(config, m_io2, XTAL(16'000'000));

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);
	m_soundlatch[0]->set_separate_acknowledge(false);

	GENERIC_LATCH_8(config, m_soundlatch[1]);
}

void systemm1_state::m1comm(machine_config &config)
{
	m1base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &systemm1_state::mem_comm_map);

	Z80(config, m_commcpu, 4'000'000); // unknown clock
	m_commcpu->set_addrmap(AS_PROGRAM, &systemm1_state::comm_map);

	I8251(config, "uart", 4'000'000); // unknown clock

	mb8421_device &dpram(MB8421(config, "dpram"));
	dpram.intl_callback().set_inputline("m1comm", 0);
}

ROM_START( tinkerbl )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("epr-a13637.ic8", 0x000000, 0x040000, CRC(de270e16) SHA1(e77fedbd11a698b1f7ed03feec64204f712c3cad))
	ROM_LOAD16_BYTE("epr-a13639.ic7", 0x000001, 0x040000, CRC(56ade038) SHA1(c807b63e1ff7cc9577dc45689ebc67ede396f7b6))
	ROM_LOAD16_BYTE("epr-13638.ic10", 0x080000, 0x040000, CRC(5fb6b19e) SHA1(795bd5e31c3261f981b2fb0fce38d9b04d2031c2))
	ROM_LOAD16_BYTE("epr-13640.ic9", 0x080001, 0x040000, CRC(afce4c2a) SHA1(b37e2e11c240d9679216410d8f004d566e053f52))

	ROM_REGION(0x20000, "soundcpu", 0)
	ROM_LOAD("epr-13641.ic104", 0x000000, 0x020000, CRC(13142c1b) SHA1(b24535a4ac75780d9a3e0bcbecf673a5701c8be8))

	ROM_REGION(0x40000, "gals", 0)
	ROM_LOAD("gal18v8a_315-5391.ic103", 0x000000, 0x040000, CRC(29480530) SHA1(d3d629fb4c2a4ae851f14b1d9e5b72b37b567f0b))
ROM_END

ROM_START( blicks )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("epr-14163.ic8", 0x000000, 0x040000, CRC(19bc94fa) SHA1(b81a61394a853dda6ed157cfcb3cb40ecd103c3d) )
	ROM_LOAD16_BYTE("epr-14165.ic7", 0x000001, 0x040000, CRC(61b8d395) SHA1(452b718d7ce9cbec913eeb6f2758e8dbecced6b1) )
	ROM_LOAD16_BYTE("epr.14164.ic10", 0x080000, 0x040000, CRC(5d3ccf3b) SHA1(31db63c87bf8417d58ae829759d2014ef140e891))
	ROM_LOAD16_BYTE("epr-14166.ic9", 0x080001, 0x040000, CRC(5e63db91) SHA1(1d754675a2ca9d4e945c314ce2a42e7ed86a9ecf) )

	ROM_REGION(0x20000, "soundcpu", 0)
	ROM_LOAD( "epr-14167.ic104", 0x000000, 0x020000, CRC(305a9afe) SHA1(5b27d50797c6048e92b86f3748dfff3c873bbf13) )

	ROM_REGION(0x40000, "gals", 0)
	ROM_LOAD( "gal18v8a_315-5391.ic103", 0x000000, 0x040000, CRC(29480530) SHA1(d3d629fb4c2a4ae851f14b1d9e5b72b37b567f0b) )
	ROM_LOAD( "315-5391.jed", 0x000000, 0x00038e, CRC(9918d5c8) SHA1(2d599573c716ec840b98dff44c167a15117ba824) )
ROM_END

ROM_START( bingpty ) // 1994/05/01 string
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-16648b.bin", 0x00000, 0x20000, CRC(e4fceb4c) SHA1(0a248bb328d2f6d72d540baefbe62838f4b76585) )
	ROM_LOAD16_BYTE( "epr-16649b.bin", 0x00001, 0x20000, CRC(736d8bbd) SHA1(c359ad513d4a7693cbb1a27ce26f89849e894d05) )

	ROM_REGION( 0x20000, "soundcpu", 0 ) // Z80 code
	ROM_LOAD( "epr-14845.bin", 0x00000, 0x20000, CRC(90d47101) SHA1(7bc002c104e3dbde1986aaec54112d5658eab523) )

	ROM_REGION( 0x8000, "m1comm", 0 ) // Z80 code
	ROM_LOAD( "epr-14221a.bin", 0x00000, 0x8000, CRC(a13e67a4) SHA1(4cd269c7f04a64ae7806c8784f86bf6553a25d85) )

	// dumps of the X-Board part, and the LINK PCB are missing.
ROM_END

ROM_START( bingplnt ) // 1997/06/30 string
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-19373c.ic8", 0x00000, 0x40000, CRC(965e5dba) SHA1(82eab3261ad610a43aa06206d6b44b6fe4aeb9d8) )
	ROM_LOAD16_BYTE( "epr-19374c.ic7", 0x00001, 0x40000, CRC(ad305112) SHA1(8a33cf719074875c96a0e50db2ab7c7b6223924d) )

	ROM_REGION( 0x20000, "soundcpu", 0 ) // Z80 code
	ROM_LOAD( "epr-14845.bin", 0x00000, 0x20000, CRC(90d47101) SHA1(7bc002c104e3dbde1986aaec54112d5658eab523) )

	ROM_REGION( 0x8000, "m1comm", 0 ) // Z80 code
	ROM_LOAD( "epr-14221a.bin", 0x00000, 0x8000, CRC(a13e67a4) SHA1(4cd269c7f04a64ae7806c8784f86bf6553a25d85) )

	// dumps of the X-Board part, and the LINK PCB are missing.
ROM_END

ROM_START( carboule ) // 1992.01.31 string
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "epr-14427.ic8", 0x00000, 0x40000, CRC(2d904fc6) SHA1(7062f47d77d09906420118c85e1cb565bec345a7) )
	ROM_LOAD16_BYTE( "epr-14428.ic7", 0x00001, 0x40000, CRC(97a317f4) SHA1(19bc4cf6b6c580caa44f36c929b445ed94b2d9eb) )

	ROM_REGION( 0x20000, "soundcpu", 0 ) // Z80 code
	ROM_LOAD( "epr-14429.ic104", 0x00000, 0x20000, CRC(1ff8262d) SHA1(fb90bd877b2dc65eb3e5495d6e21dee1f871fb44) )

	ROM_REGION( 0x8000, "m1comm", 0 )
	ROM_LOAD( "epr-14426.ic2", 0x0000, 0x8000, NO_DUMP ) // on "SYSTEM M1 COM" board with Z80, MB8421 and TMP82C51AP

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "315-5472-01.ic22", 0x000, 0x0eb, CRC(828ee6e2) SHA1(f32dd0f6297cc8bd3049be4bca502c0f8ec738cf) )
	// dumps of the X-Board part, and the LINK PCB are missing.
ROM_END

} // anonymous namespace

// Standalone M1 games
GAME(1990, tinkerbl, 0, m1base, tinkerbl, systemm1_state, empty_init, ROT0, "Sega", "Tinker Bell", MACHINE_NOT_WORKING)
GAME(1990, blicks,   0, m1base, blicks,   systemm1_state, empty_init, ROT0, "Sega", "Blicks (Japan)", MACHINE_NOT_WORKING)

// M1 comm multi-board games
GAME(1994, bingpty,  0, m1comm, bingpty, systemm1_state, empty_init, ROT0, "Sega", "Bingo Party Multicart (Rev B) (M1 Satellite board)", MACHINE_NOT_WORKING)
GAME(1997, bingplnt, 0, m1comm, bingpty, systemm1_state, empty_init, ROT0, "Sega", "Bingo Planet (Rev C) (M1 Satellite board)", MACHINE_NOT_WORKING) // title inferred from date, may be wrong
GAME(1992, carboule, 0, m1comm, bingpty, systemm1_state, empty_init, ROT0, "Sega", "Caribbean Boule (M1 Satellite board)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
