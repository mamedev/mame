// license: BSD-3-Clause
// copyright-holders: Pierpaolo Prazzoli, David Haywood, Dirk Best

/***************************************************************************

    Little Casino
    Little Casino II
    Mini Vegas 4in1

    Non-Payout 'Gambling' style games

    TODO:
    - Clocks need to be verified
    - Timing is probably wrong, IRQ sources need to be verified
    - Figure out the rest of the dipswitches
    - Keyboard

    Notes:
    - To enter service mode, press buttons 2 and 4 then reset. Let go
      of the buttons once the video test finishes. Some games also
      enter service mode if you disable all games via the dipswitches.
      Once in service mode, press both buttons 2 and 4 to advance to
      next screen.
    - The name "Little Casino II" is just reflected by a sticker on the
      cabinet - the title screen is still "Little Casino"
    - Color version of "Little Casino" is undumped (flyer exists)?


Mini Vegas
CC-089  (c) Entertainment Enterprises Ltd. 1983
 +---------------------------------------------------------------------+
 |        +-+                  M2114          A.IC19         18.432MHz |
 |        |H|                                                      D1  |
 |        |D|                  M2114  M2114                            |
++  2003C |6|                                 B.IC18               +-+ |
|         |8|                  M2114  M2114                        | | |
|         |2|                                                      |9| |
|         |1|                  M2114  M2114   C.IC17               |9| |
|         +-+                                                      |3| |
|         +-+                         M2114                        |7| |
|   DSW3  |H|                         TC5514  D.IC16               | | |
|         |D|       +-+                                            +-+ |
|         |6|       | |                                            +-+ |
|         |8|       |8|               TC5514  E.IC15               |R| |
|         |2|       |9|                                            |6| |
|         |1|       |1|                                            |5| |
++        +-+       |0|               TC5514  F.IC14               |0| |
 |                  | |                                            |2| |
 |    VR1     DSW2  +-+                                            +-+ |
 |  MB3712    DSW1  BAT               TC5514  G.IC13                   |
 +---------------------------------------------------------------------+

  CPU: Rockwell R6502AP 2MHz
Video: TMS9937 NL CRT5037 NMOS Single chip Video Timer/Controller (VTC)
Sound: AY-3-8910
       MB3712 5.7W Amp
  OSC: 18.432MHz
  RAM: M2114-3 1KBx4 SRAM x 8
       TC5514P-1 1KBx4 SRAM x 4
  DSW: 3 8-switch dipswitches
  VR1: Volume pot
Other: Hitachi HD46821P 1MHz NMOS Peripheral Interface Adapter (PIA) x 2
       NEC uPA2003C Darlington Array
       3.6v Battery
       D1 - Power On Diode
       44 pin edge connector

***************************************************************************

Little Casino II
Digital Controls Inc. 1984
 +---------------------------------------------------------------------+
 |    +----------+ DSW3 +---------+ M2114 M2114*     +---+         LED |
 |    | HD46821P |      | TMS9937 | M2114 M2114*     | S |   V18_10_RA |
 |    +----------+      +---------+ M2114 M2114*     | Y |             |
++                                  M2114 M2114*     | 6 |             |
|                                                    | 5 |   V18_10_RB |
|     +----------+                                   | 0 |             |
|     | HD46821P |                                   | 2 |             |
|     +----------+                                   +---+   V18_10_RC |
|                       V18_10_RV                                      |
|     +---------+                                                      |
|     |AY-3-9810|                                            V18_10_RD |
|     +---------+                                                      |
|                 M2114                                                |
|     DSW1 DSW2   M2114                                      V18_10_RE |
++                M2114                                                |
 |  MB3712        M2114  18.000MHz                                     |
 |                                                           V18_10_RF |
 |                                                                     |
 +---------------------------------------------------------------------+

  CPU: SY6502 2MHz
Video: TMS9937 NL CRT5037 NMOS Single chip Video Timer/Controller (VTC)
Sound: AY-3-8910
       MB3712 5.7W Amp
  OSC: 18.000MHz
  RAM: MM2114N-3 1KBx4 SRAM x 12 (*some PCBs only had 8 RAM chips populated)
  DSW: 3 8-switch dipswitches
Other: Hitachi HD46821P 1MHz NMOS Peripheral Interface Adapter (PIA) x 2
       LED - Power On LED
       44 pin edge connector


***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/tms9927.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "ltcasino.lh"
#include "ltcasinn.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ltcasino_state : public driver_device
{
public:
	ltcasino_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia%u", 0U),
		m_vtc(*this, "vtc"),
		m_screen(*this, "screen"),
		m_ay(*this, "ay"),
		m_video_ram(*this, "video_ram"),
		m_attribute_ram(*this, "attribute_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_input_q(*this, "Q"),
		m_input_s(*this, "S"),
		m_lamps(*this, "button_%u", 0U),
		m_tilemap(nullptr)
	{ }

	void ltcasino(machine_config &config);

protected:
	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<crt5037_device> m_vtc;
	required_device<screen_device> m_screen;
	required_device<ay8910_device> m_ay;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_attribute_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport m_input_q;
	required_ioport m_input_s;
	output_finder<5> m_lamps;

	tilemap_t *m_tilemap;

private:
	void main_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(tile_info);

	uint8_t input_q_r();
	uint8_t input_s_r();

	void output_r_w(uint8_t data);
	void output_t_w(uint8_t data);
};

class ltcasin2_state : public ltcasino_state
{
public:
	using ltcasino_state::ltcasino_state;

	void init_mv4in1();

	void ltcasin2(machine_config &config);
	void mv4in1(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(tile_info);
};

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ltcasino_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x8000, 0xcfff).rom();
	map(0xd000, 0xd7ff).ram().share(m_video_ram);
	map(0xe000, 0xe7ff).ram().share(m_attribute_ram);
	map(0xec00, 0xec03).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xec10, 0xec13).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xec20, 0xec21).r(m_ay, FUNC(ay8910_device::data_r));
	map(0xec20, 0xec21).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0xec30, 0xec3f).rw(m_vtc, FUNC(crt5037_device::read), FUNC(crt5037_device::write));
	map(0xf000, 0xffff).rom();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( ltcasino )
	PORT_START("COIN")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1) PORT_WRITE_LINE_DEVICE_MEMBER("pia0", pia6821_device, ca1_w)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2) PORT_WRITE_LINE_DEVICE_MEMBER("pia0", pia6821_device, cb1_w)

	PORT_START("Q")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2) PORT_NAME("Button 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3) PORT_NAME("Button 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4) PORT_NAME("Button 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5) PORT_NAME("Button 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_CUSTOM)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM)

	PORT_START("S")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "C:1")
	PORT_DIPNAME(0x02, 0x00, "Enable Black Jack")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x02, DEF_STR( Off ))
	PORT_DIPLOCATION("C:2")
	PORT_DIPNAME(0x04, 0x00, "Enable Craps")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x04, DEF_STR( Off ))
	PORT_DIPLOCATION("C:3")
	PORT_DIPNAME(0x08, 0x00, "Enable Poker")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x08, DEF_STR( Off ))
	PORT_DIPLOCATION("C:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "C:5")
	PORT_DIPUNUSED_DIPLOC(0x20, IP_ACTIVE_LOW, "C:6") // would enable game 5
	PORT_DIPNAME(0x40, 0x00, "Enable Hi-Lo")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPLOCATION("C:7")
	PORT_DIPNAME(0x80, 0x00, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPLOCATION("C:8")

	PORT_START("A")
	PORT_DIPNAME(0x03, 0x03, "Hands per Game")
	PORT_DIPSETTING(   0x00, "6")
	PORT_DIPSETTING(   0x01, "5")
	PORT_DIPSETTING(   0x02, "4")
	PORT_DIPSETTING(   0x03, "3")
	PORT_DIPLOCATION("A:1,2")
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR( Coinage ))
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_1C ))
	PORT_DIPLOCATION("A:3,4")
	PORT_DIPNAME(0x10, 0x10, "Coin Limit")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x10, DEF_STR( On )) // limits to 15 coins
	PORT_DIPLOCATION("A:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "A:6") // coin window on ltcasin2
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "A:7")
	PORT_DIPNAME(0x80, 0x80, "Screen Mode")
	PORT_DIPSETTING(   0x00, "50 Cycle")
	PORT_DIPSETTING(   0x80, "60 Cycle")
	PORT_DIPLOCATION("A:8")

	PORT_START("B")
	PORT_DIPNAME(0x01, 0x01, "Topten") // toggles availability of the high score table
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPLOCATION("B:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "B:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "B:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "B:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "B:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "B:6")
	PORT_DIPNAME(0x40, 0x40, "Free Hand")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPLOCATION("B:7")
	PORT_DIPNAME(0x80, 0x80, "Autoplay")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPLOCATION("B:8")
INPUT_PORTS_END

static INPUT_PORTS_START( ltcasin2 )
	PORT_INCLUDE(ltcasino)

	PORT_MODIFY("COIN")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("S")
	PORT_DIPNAME(0x01, 0x01, "Keyboard")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPLOCATION("C:1")
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "C:5")
	PORT_DIPNAME(0x20, 0x00, "Enable Horse")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPLOCATION("C:6")
	PORT_DIPNAME(0x40, 0x00, "Enable Slots")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPLOCATION("C:7")

	PORT_MODIFY("A")
	PORT_DIPUNUSED_DIPLOC(0x10, IP_ACTIVE_LOW, "A:5") // Coin Limit for other sets, v18.1 always locked to 15 coins
	PORT_DIPNAME(0x20, 0x00, "Coin Window") // needs to be disabled or it can reset on coin-up
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x20, DEF_STR( On ))
	PORT_DIPLOCATION("A:6")
	PORT_DIPNAME(0x40, 0x40, "Topten") // toggles availability of the high score table
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPLOCATION("A:7")

	PORT_MODIFY("B")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "B:1")
INPUT_PORTS_END

static INPUT_PORTS_START( ltcasin2a )
	PORT_INCLUDE(ltcasino)

	PORT_MODIFY("COIN")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("S")
	PORT_DIPNAME(0x01, 0x01, "Keyboard")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPLOCATION("C:1")
	PORT_DIPNAME(0x20, 0x00, "Enable Horse")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPLOCATION("C:6")

	PORT_MODIFY("A")
	PORT_DIPNAME(0x20, 0x00, "Coin Window") // needs to be disabled or it can reset on coin-up
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x20, DEF_STR( On ))
	PORT_DIPLOCATION("A:6")

	PORT_MODIFY("B")
	PORT_DIPNAME(0x02, 0x02, "Memory Test") // tests d000 to d7ff
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x02, DEF_STR( On ))
	PORT_DIPLOCATION("B:2")
INPUT_PORTS_END

static INPUT_PORTS_START( mv4in1 )
	PORT_INCLUDE(ltcasino)

	PORT_MODIFY("S")
	PORT_DIPNAME(0x01, 0x01, "Keyboard")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPLOCATION("C:1")
	PORT_DIPNAME(0x02, 0x00, "Enable 21")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x02, DEF_STR( Off ))
	PORT_DIPLOCATION("C:2")
	PORT_DIPNAME(0x04, 0x00, "Enable Dice")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x04, DEF_STR( Off ))
	PORT_DIPLOCATION("C:3")
	PORT_DIPNAME(0x40, 0x00, "Enable Red-Dog")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPLOCATION("C:7")
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void ltcasin2_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
	{
		// basic 3 bit palette
		palette.set_indirect_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1)));

		// setup pen colors for the drawgfx system
		for (int j = 0; j < 8; j++)
		{
			palette.set_pen_indirect((j << 4) | (i << 1) | 0, i);
			palette.set_pen_indirect((j << 4) | (i << 1) | 1, j);
		}
	}
}

/*
 x--- ---- tile bank
 -xxx ---- foreground color
 ---- x--- unknown (used by ltcasino)
 ---- -xxx background color
 */
TILE_GET_INFO_MEMBER(ltcasino_state::tile_info)
{
	uint16_t code = m_video_ram[tile_index];
	// +1 on attribute offset otherwise glitches occurs on left side of objects?
	uint8_t const attr = m_attribute_ram[(tile_index + 1) & 0x7ff];

	code |= BIT(attr, 7) << 8;

	tileinfo.set(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(ltcasin2_state::tile_info)
{
	uint16_t code = m_video_ram[tile_index];
	// +1 on attribute offset otherwise glitches occurs on left side of objects?
	uint8_t const attr = m_attribute_ram[(tile_index + 1) & 0x7ff];

	code |= BIT(attr, 7) << 8;

	tileinfo.set(0, code, ((attr & 0x70) >> 1) | (attr & 7), 0);
}

uint32_t ltcasino_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->mark_all_dirty();
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_ltcasino )
	GFXDECODE_ENTRY("tiles", 0, tiles8x8_layout, 0, 1)
GFXDECODE_END

static GFXDECODE_START( gfx_ltcasin2 )
	GFXDECODE_ENTRY("tiles", 0, tiles8x8_layout, 0, 64)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ltcasin2_state::init_mv4in1()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 1, 2, 0);
}

void ltcasino_state::machine_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ltcasino_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_lamps.resolve();
}

void ltcasin2_state::machine_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ltcasin2_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_lamps.resolve();
}

uint8_t ltcasino_state::input_s_r()
{
	uint8_t data = m_input_s->read() & 0xf1;

	// bit 1, 2 and 3 from input port Q bit 7, 5 and 6
	data |= BIT(m_input_q->read(), 7) << 1;
	data |= BIT(m_input_q->read(), 5) << 2;
	data |= BIT(m_input_q->read(), 6) << 3;

	return data;
}

uint8_t ltcasino_state::input_q_r()
{
	uint8_t data = m_input_q->read() & 0x1f;

	// bit 5, 6 and 7 from input port Q bit 2, 3 and 1
	data |= BIT(m_input_s->read(), 2) << 5;
	data |= BIT(m_input_s->read(), 3) << 6;
	data |= BIT(m_input_s->read(), 1) << 7;

	return data;
}

void ltcasino_state::output_r_w(uint8_t data)
{
	// 7------- unknown (toggles rapidly)
	// -6------ unknown (toggles rapidly)
	// --5----- coin counter
	// ---43210 button lamps 5 to 1

	m_lamps[0] = BIT(data, 0); // button 1
	m_lamps[1] = BIT(data, 1); // button 2
	m_lamps[2] = BIT(data, 2); // button 3
	m_lamps[3] = BIT(data, 3); // button 4
	m_lamps[4] = BIT(data, 4); // button 5

	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
}

void ltcasino_state::output_t_w(uint8_t data)
{
	// 76543210 unknown

	logerror("output_t_w: %02x\n", data);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void ltcasino_state::ltcasino(machine_config &config)
{
	M6502(config, m_maincpu, 18_MHz_XTAL / 16); // clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ltcasino_state::main_map);

	input_merger_device &mainirq(INPUT_MERGER_ANY_HIGH(config, "mainirq"));
	mainirq.output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	NVRAM(config, "nvram");

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(ltcasino_state::input_q_r));
	m_pia[0]->writepb_handler().set(FUNC(ltcasino_state::output_r_w));
	m_pia[0]->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set(FUNC(ltcasino_state::input_s_r));
	m_pia[1]->writepb_handler().set(FUNC(ltcasino_state::output_t_w));

	// video hardware
	CRT5037(config, m_vtc, 18_MHz_XTAL / 16); // this clock gives about 60/50 hz
	m_vtc->set_char_width(8);
	m_vtc->set_screen("screen");
	m_vtc->set_visarea(48, 463, 0, 255);
	m_vtc->vsyn_callback().set(m_pia[0], FUNC(pia6821_device::cb2_w)).invert(); // ? (CA1, CA2 also enabled)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(18_MHz_XTAL / 2, 560, 48, 464, 268, 0, 256);
	m_screen->set_screen_update(FUNC(ltcasino_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_ltcasino);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay, 18_MHz_XTAL/16); // clock unknown
	m_ay->port_a_read_callback().set_ioport("A");
	m_ay->port_b_read_callback().set_ioport("B");
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.4);
}

void ltcasin2_state::ltcasin2(machine_config &config)
{
	ltcasino(config);

	PALETTE(config.replace(), "palette", FUNC(ltcasin2_state::palette), 128, 8);

	m_gfxdecode->set_info(gfx_ltcasin2);
}

void ltcasin2_state::mv4in1(machine_config &config)
{
	ltcasin2(config);

	// different XTAL
	m_maincpu->set_clock(18.432_MHz_XTAL / 16);
	m_vtc->set_clock(18.432_MHz_XTAL / 16);
	m_screen->set_raw(18.432_MHz_XTAL / 2, 560, 48, 464, 268, 0, 256);
	m_ay->set_clock(18.432_MHz_XTAL / 16);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// Selection text: "SELECT GAME DESIRED!"
// Games: Black Jack, Draw Poker, Craps, Hi-Lo
ROM_START( ltcasino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a", 0x8000, 0x1000, CRC(14909fee) SHA1(bf53fa65da7f013ea1ac6b4942cdfdb34ef16252) )
	ROM_LOAD( "b", 0x9800, 0x0800, CRC(1473f854) SHA1(eadaec1f6d653e61458bc262945c20140f4530eb) )
	ROM_LOAD( "c", 0xa800, 0x0800, CRC(7a07004b) SHA1(62bd0f3d12b7eada6fc271abea60569aca7262b0) )
	ROM_LOAD( "d", 0xb800, 0x0800, CRC(5148cafc) SHA1(124039f48784bf032f612714db73fb67a216a1e7) )
	ROM_LOAD( "e", 0xc800, 0x0800, CRC(5f9e103a) SHA1(b0e9ace4c3962c06e5250fac16a245dca711350f) )
	ROM_LOAD( "f", 0xf000, 0x1000, CRC(7345aada) SHA1(6640f5eb1130c8f1cb197eb12b8e6403c7f8d34d) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "v", 0x0000, 0x0800, CRC(f1f75675) SHA1(8f3777e6b2a3f824f94b28669cac501ec02bbf36) )
ROM_END

// Selection text: "SELECT GAME DESIRED!"
// Games: Black Jack, Draw Poker, Craps, Hi-Lo
ROM_START( ltcasinoa ) // all labels handwritten, tile ROM without label
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1", 0x8000, 0x1000, CRC(14909fee) SHA1(bf53fa65da7f013ea1ac6b4942cdfdb34ef16252) )
	ROM_LOAD( "b1", 0x9800, 0x0800, CRC(1473f854) SHA1(eadaec1f6d653e61458bc262945c20140f4530eb) )
	ROM_LOAD( "c1", 0xa800, 0x0800, CRC(7a07004b) SHA1(62bd0f3d12b7eada6fc271abea60569aca7262b0) )
	ROM_LOAD( "d1", 0xb800, 0x0800, CRC(5148cafc) SHA1(124039f48784bf032f612714db73fb67a216a1e7) )
	ROM_LOAD( "e1", 0xc800, 0x0800, CRC(5f9e103a) SHA1(b0e9ace4c3962c06e5250fac16a245dca711350f) )
	ROM_LOAD( "f1", 0xf000, 0x1000, CRC(37265ddf) SHA1(f6ec12f8836a280c7a06cc19f740b39c5149eabc) ) // only different ROM

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "g", 0x0000, 0x0800, CRC(f1f75675) SHA1(8f3777e6b2a3f824f94b28669cac501ec02bbf36) )
ROM_END

// Selection text: "PLAY YOUR FAVORITES"
// Games: 21, Draw Poker, Dice, Red-Dog
ROM_START( mv4in1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g.ic13", 0x8000, 0x1000, CRC(ac33bd85) SHA1(fd555f70d0a7040473d35ec38e19185671a471ea) )
	ROM_LOAD( "f.ic14", 0x9000, 0x1000, CRC(f95c87d1) SHA1(df5ed53722ec55a97eabe10b0ed3f1ba32cbe55f) )
	ROM_LOAD( "e.ic15", 0xa000, 0x1000, CRC(e525fcf2) SHA1(f1ec0c514e25ec4a1caf737ff8a962c81fb2706a) )
	ROM_LOAD( "d.ic16", 0xb000, 0x1000, CRC(ab34673f) SHA1(520a173a342a27b5f9d995e6f53c3a2f0f359f9e) )
	ROM_LOAD( "c.ic17", 0xc000, 0x1000, CRC(e384edf4) SHA1(99042528ce2b35191248d90162ca06a1a585667c) )
	ROM_LOAD( "b.ic18", 0xf000, 0x1000, CRC(3450b862) SHA1(816d13fd8d03c299c1dbecf971ee5fae2f1d64bc) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "a.ic19", 0x0000, 0x1000, CRC(a25c125e) SHA1(e0ba83ccddbd82a2bf52585ae0accb9192cbb00e) )
ROM_END

// Selection text: "PLEASE MAKE SELECTION!"
// Games: Black Jack, Draw Poker, Craps, Slots, Horse
ROM_START( ltcasin2 ) // all labels peeled off - need to verify labels
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v30_00_ra.bin", 0x8000, 0x1000, CRC(8d446c26) SHA1(d5b78fd17798bf69fbac5e060f020799bb10cf64) ) // 30.00.00 in test mode
	ROM_LOAD( "v30_00_rb.bin", 0x9000, 0x1000, CRC(38ca5193) SHA1(715add68a633b78eceabe149a7564aa2fb513837) ) // 30.00.00 in test mode
	ROM_LOAD( "v30_00_rc.bin", 0xa000, 0x1000, CRC(f05095db) SHA1(61c5c9bef20c057348ce1321c71195d340dc0cd6) ) // 30.00.00 in test mode
	ROM_LOAD( "v30_00_rd.bin", 0xb000, 0x1000, CRC(53e534dc) SHA1(3d964f51b254f9bd0bd3fb4926f57aa7d5224968) ) // 30.00.00 in test mode
	ROM_LOAD( "v30_00_re.bin", 0xc000, 0x1000, CRC(972fd4ab) SHA1(f91556588315e0836a860f138730314688f99ec7) ) // 30.00.00 in test mode
	ROM_LOAD( "v30_00_rf.bin", 0xf000, 0x1000, CRC(b711c779) SHA1(2bab84cab174a35fccfd23003a8a41aa241d4595) ) // 18.01.02 in test mode (== v18_10_rf.bin below)

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "v30_00_rv.bin", 0x0000, 0x1000, CRC(135ec308) SHA1(699711ceaeb5a00f31ccd88b7be7e9f0055fa58b) )
ROM_END

// Selection text: "PLEASE MAKE SELECTION!"
// Games: Black Jack, Draw Poker, Craps, Slots, Horse
// Board was marked version 18.1 (C)1984
ROM_START( ltcasin2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v18_10_ra.bin", 0x8000, 0x1000, CRC(f0c5cc96) SHA1(ec50918ba2a2487df70694f9e1a52d4b8d1bc7e2) ) // 18.01.01 in test mode
	ROM_LOAD( "v18_10_rb.bin", 0x9000, 0x1000, CRC(2ece16e4) SHA1(ef6adc45be2ecc510cd8b2e9682635066013a5e4) ) // 18.01.01 in test mode
	ROM_LOAD( "v18_10_rc.bin", 0xa000, 0x1000, CRC(16bae5c9) SHA1(e5cb61d9dcae3c46c7139f3494d1bf981ec8821f) ) // 18.01.00 in test mode
	ROM_LOAD( "v18_10_rd.bin", 0xb000, 0x1000, CRC(d12f2d6b) SHA1(e3544bf6b778c21b704a01f1ed06d6517ca01604) ) // 18.01.00 in test mode
	ROM_LOAD( "v18_10_re.bin", 0xc000, 0x1000, CRC(2acdad10) SHA1(2732b791fea0a9d1c6e4c174739381466f2b0270) ) // 18.01.00 in test mode
	ROM_LOAD( "v18_10_rf.bin", 0xf000, 0x1000, CRC(b711c779) SHA1(2bab84cab174a35fccfd23003a8a41aa241d4595) ) // 18.01.02 in test mode

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "v18_10_rv.bin", 0x0000, 0x1000, CRC(7209898d) SHA1(94bd7e8c3a544429af721e9564c11cc56d7805be) )
ROM_END

// Selection text: "PLEASE PICK YOUR POISON!"
// Games: Black Jack, Draw Poker, Craps, Hi-Lo, Horse
ROM_START( ltcasin2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v17_00_ra.bin", 0x8000, 0x1000, CRC(1a595442) SHA1(b8fe3e5ed2024a57187c0ce547c1bbef2429ed63) ) // 17.00.00 in test mode
	ROM_LOAD( "v17_00_rb.bin", 0x9000, 0x1000, CRC(4f5502c1) SHA1(cd1b7c08d26fed71c45e44ebd208bd18dc262e8f) ) // 17.00.00 in test mode
	ROM_LOAD( "v17_00_rc.bin", 0xa000, 0x1000, CRC(990283b8) SHA1(8a3fe5be8381894b8e8dd14c7d42190e60a25600) ) // 17.00.00 in test mode
	ROM_LOAD( "v17_00_rd.bin", 0xb000, 0x1000, CRC(884f39dc) SHA1(fe149faf118279205e82760c5052cefb88a2f5be) ) // 17.00.00 in test mode
	ROM_LOAD( "v17_00_re.bin", 0xc000, 0x1000, CRC(fae38204) SHA1(e5908734cee0a89d873ab3761ded285f8ae138d3) ) // 17.00.00 in test mode
	ROM_LOAD( "v17_00_rf.bin", 0xf000, 0x1000, CRC(7e8ad9d3) SHA1(8cbe342af7d9f32b2214664db318edd3d2e75630) ) // 17.00.00 in test mode

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "v17_00_rv.bin", 0x0000, 0x1000, CRC(84cbee7b) SHA1(742831d5ae0db6c7c644a18a837831ee0474d472) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//     YEAR  NAME       PARENT    MACHINE   INPUT      CLASS           INIT         ROTATION  COMPANY                            FULLNAME                     FLAGS
GAMEL( 1982, ltcasino,  0,        ltcasino, ltcasino,  ltcasino_state, empty_init,  ROT0,     "Digital Controls Inc.",           "Little Casino (set 1)",     MACHINE_SUPPORTS_SAVE, layout_ltcasino )
GAMEL( 1982, ltcasinoa, ltcasino, ltcasino, ltcasino,  ltcasino_state, empty_init,  ROT0,     "Digital Controls Inc.",           "Little Casino (set 2)",     MACHINE_SUPPORTS_SAVE, layout_ltcasino )
GAMEL( 1983, mv4in1,    0,        mv4in1,   mv4in1,    ltcasin2_state, init_mv4in1, ROT0,     "Entertainment Enterprises, Ltd.", "Mini Vegas 4in1",           MACHINE_SUPPORTS_SAVE, layout_ltcasinn )
GAMEL( 1984, ltcasin2,  0,        ltcasin2, ltcasin2,  ltcasin2_state, empty_init,  ROT0,     "Digital Controls Inc.",           "Little Casino II (v30.0)",  MACHINE_SUPPORTS_SAVE, layout_ltcasinn )
GAMEL( 1984, ltcasin2a, ltcasin2, ltcasin2, ltcasin2,  ltcasin2_state, empty_init,  ROT0,     "Digital Controls Inc.",           "Little Casino II (v18.1)",  MACHINE_SUPPORTS_SAVE, layout_ltcasinn )
GAMEL( 1984, ltcasin2b, ltcasin2, ltcasin2, ltcasin2a, ltcasin2_state, empty_init,  ROT0,     "Digital Controls Inc.",           "Little Casino II (v17.0)",  MACHINE_SUPPORTS_SAVE, layout_ltcasinn )
