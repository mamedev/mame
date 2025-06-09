// license:BSD-3-Clause
// copyright-holders: Bryan McPhail

/***************************************************************************

  Thunder Zone / Desert Assault (c) 1991 Data East Corporation

  Thunder Zone       (World 2 or 4 players, 2 sets)
  Thunder Zone       (World 4 players only)
  Thunder Zone       (Japan 2 or 4 players)
  Desert Assault     (USA 4 players only)
  Desert Assault     (USA Selectable 2-4 players)

  Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************

Thunder Zone / Desert Assault
Data East 1991

PCB Layout
----------

Main PCB:

DE-0344-3
|--------------------------------------------------------|
|TA8205      YM3012   YM2151    GT04         MAJ-01  GZ03|
| VOL VOL    YM3014   YM2203    6264   |---| MAJ-00  GT02|
|  3403 3403  M6295(1) MAJ-03          |45 |         GZ01|
|  3403 3403  M6295(2) GT07     32MHz  |---|         GT00|
|                      MAJ-02                            |
|  SW3 SW2 SW1         GT06                     6264     |
|J                     GT05            |-----|  6264     |
|A       MB7128                        | 55  |      6264 |
|M                                     |     |      6264 |
|M                                     |-----|           |
|A                                                       |
| RCDM-I111                                          77  |
| RCDM-I111                            |-----| 6264      |
| RCDM-I111                            | 55  | 6264      |
| RCDM-I111       6116                 |     |    |---|  |
|                 6116                 |-----|    |59 |  |
| CN4             6116                            |---|  |
| CN3                                                    |
|--------------------------------------------------------|
Notes:
            (some PCBs may use a 32.22MHz oscillator)
       59 - 68000-based CPU (in custom QFP64 package) disguised as Data East chip 59. Clock input 14.161MHz [28.322/2] (QFP64)
       45 - Hudson/NEC HuC6280 disguised as Data East chip 45. Clock input 8.000MHz on pin 10 [32/4] (QFP80)
       55 - Data East custom chip 55 graphic generator IC (QFP160)
       77 - Data East custom chip 77 (SOP28)
 M6295(1) - OKI M6295 4-Channel Mixing ADPCM Voice Synthesis LSI. Clock input 2.000MHz [32/16]. Pin 7 HIGH (QFP44)
 M6295(2) - OKI M6295 4-Channel Mixing ADPCM Voice Synthesis LSI. Clock input 1.000MHz [32/32]. Pin 7 HIGH (QFP44)
   YM2203 - Yamaha YM2203C FM Operator Type-N(OPN). Clock input 4.000MHz [32/8] (DIP40)
   YM2151 - Yamaha YM2151 FM Operator Type-M(OPM). Clock input 3.55556MHz comes from a 74F163 on pin 12. F163 input pin 2 is 32MHz (DIP24)
   YM3012 - Yamaha YM3012 2-Channel Serial Input Floating D/A Converter (DIP16)
   YM3014 - Yamaha YM3014 Serial Input Floating D/A Converter (DIP8)
     6116 - 2kx8 SRAM (DIP24)
     6264 - 8kx8 SRAM (DIP28)
     MAJ* - Mask ROMs
GT* & GZ* - EPROMs
   MB7128 - Fujitsu MB7128 Bi-Polar PROM marked 'GR-0' (DIP18)
    CN3/4 - Connector for extra joystick & buttons for player 3 and player 4
   TA8205 - Toshiba TA8205AH 18W BTL 2 Channel Audio Power Amplifier
     3403 - NEC uPC3403C Quad Operational Amplifier (DIP14)
RCDM-I111 - Custom Data East resistor array
  SW1/2/3 - 8-position DIP switch (2 populated, SW3 unpopulated)
    HSYNC - 15.80464kHz
    VSYNC - 58.1052Hz

Sub PCB:

DE-0345-1
|--------------------------------------------------------|
|                       28.322MHz         GT12           |
|            6116                         GT13    MAJ-04 |
|6116        6116            |-----|      GT14    MAJ-05 |
|6116                        | 52  |      GT15    MAJ-06 |
|                            |     |              MAJ-07 |
|      |---|                 |-----|                     |
|      |59 |                                      MAJ-08 |
|      |---|                 |-----|              MAJ-09 |
|                            | 52  |              MAJ-10 |
|              6116          |     |              MAJ-11 |
|GZ08          6116          |-----|                     |
|GT09                                                    |
|GZ10  6264                         6116                 |
|GT11  6264    6116                 6116                 |
|              6116                                      |
|MB8421 MB8431                                           |
|                                               MB7138   |
|                                       MB7138  MB7138   |
|--------------------------------------------------------|
Notes:
            (some PCBs may use a 28.0MHz oscillator)
       59 - 68000-based CPU (in custom QFP64 package) disguised as Data East chip 59. Clock input 14.161MHz [28.322/2] (QFP64)
       52 - Data East custom chip 52 graphic generator IC (QFP128)
   MB7138 - Fujitsu MB7138 Bi-Polar PROM marked 'GR-1' (DIP24)
     6116 - 2kx8 SRAM (DIP24)
     6264 - 8kx8 SRAM (DIP28)
     MAJ* - Mask ROMs
GT* & GZ* - EPROMs
   MB8421 - Fujitsu MB8421 CMOS 16k-bit Dual-port SRAM (SDIP52)
   MB8431 - Fujitsu MB8431 CMOS 16k-bit Dual-port SRAM (SDIP52)

***************************************************************************

Stephh's notes (based on the games M68000 code and some tests) :


1) 'thndzone'

  - "Max Players" Dip Switch set to "2" :

      * COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
      * COIN3    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * SERVICE1 : adds 4 coins/credits

      * START1   : starts a game for player 1
      * START2   : starts a game for player 2

      * BUTTON1n : "fire"
      * BUTTON2n : "nuke"

  - "Max Players" Dip Switch set to "4" :

      * COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
      * COIN3    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * SERVICE1 : adds 4 coins/credits

      * START1   : starts a game for player 1
      * START2   : starts a game for player 2

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"


2) 'dassault'

  - "Max Players" Dip Switch set to "2" :

      * COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
      * COIN3    : NO EFFECT !
      * COIN4    : NO EFFECT !
      * SERVICE1 : adds 1 coin/credit

      * START1   : starts a game for player 1
      * START2   : starts a game for player 2

      * BUTTON1n : "fire"
      * BUTTON2n : "nuke"

  - "Max Players" Dip Switch set to "3" :

      * COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
      * COIN3    : adds coin(s)/credit(s) for player 3 depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) for FAKE player 4 depending on "Coin A" Dip Switch
      * SERVICE1 : adds 1 coin/credit for all players (including FAKE player 4 !)

      * START1   : NO EFFECT !
      * START2   : NO EFFECT !

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"

  - "Max Players" Dip Switch set to "4" :

      * COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
      * COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
      * COIN3    : adds coin(s)/credit(s) for player 3 depending on "Coin A" Dip Switch
      * COIN4    : adds coin(s)/credit(s) for player 4 depending on "Coin A" Dip Switch
      * SERVICE1 : adds 1 coin/credit for all players

      * START1   : NO EFFECT !
      * START2   : NO EFFECT !

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"


3) 'dassault4'

  - always 4 players :

      * COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coinage" Dip Switch
      * COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coinage" Dip Switch
      * COIN3    : adds coin(s)/credit(s) for player 3 depending on "Coinage" Dip Switch
      * COIN4    : adds coin(s)/credit(s) for player 4 depending on "Coinage" Dip Switch
      * SERVICE1 : adds 1 coin/credit

      * NO START1 !
      * NO START2 !

      * BUTTON1n : "fire" + starts a game for player n
      * BUTTON2n : "nuke"


2008-08
Dip locations verified with US conversion kit manual.

TODO:
    Blend function aren't fully emulated.

***************************************************************************/

#include "emu.h"

#include "deco16ic.h"
#include "decocomn.h"
#include "decospr.h"

#include "cpu/h6280/h6280.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "machine/mb8421.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class dassault_state : public driver_device
{
public:
	dassault_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "sub")
		, m_deco_tilegen(*this, "tilegen%u", 1U)
		, m_oki2(*this, "oki2")
		, m_spriteram(*this, "spriteram%u", 1U)
		, m_sprgen(*this, "spritegen%u", 1U)
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_pf2_rowscroll(*this, "pf2_rowscroll")
		, m_pf4_rowscroll(*this, "pf4_rowscroll")
		, m_input(*this, { "P1_P2", "P3_P4", "DSW1", "DSW2", "SYSTEM" })
	{ }

	void dassault(machine_config &config) ATTR_COLD;

	void init_dassault() ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<okim6295_device> m_oki2;
	required_device_array<buffered_spriteram16_device, 2> m_spriteram;
	required_device_array<decospr_device, 2> m_sprgen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_pf2_rowscroll;
	required_shared_ptr<uint16_t> m_pf4_rowscroll;

	required_ioport_array<5> m_input;

	uint16_t m_priority = 0U;

	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void main_irq_ack_w(uint16_t data);
	void sub_irq_ack_w(uint16_t data);
	uint16_t control_r(offs_t offset);
	void control_w(uint16_t data);
	void sound_bankswitch_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mix_layer(bitmap_rgb32 &bitmap, bitmap_ind16 *sprite_bitmap, const rectangle &cliprect, uint16_t pri, uint16_t primask, uint16_t penbase, uint8_t alpha);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Desert Assault Video emulation - Bryan McPhail, mish@tendril.co.uk

  I'm not sure if one of the alpha blending effects is correct (mode 0x8000,
  the usual mode 0x4000 should be correct).  It may be some kind of orthogonal
  priority effect where it should cut a hole in other higher priority sprites
  to reveal a non-alpha'd hole, or alpha against a further back tilemap.
  (is this the helicopter shadow at the end of lv.1 ?)

  Also, some priorities are still a little questionable.

****************************************************************************/

void dassault_state::video_start()
{
	m_priority = 0;
	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();
	save_item(NAME(m_priority));
}

void dassault_state::mix_layer(bitmap_rgb32 &bitmap, bitmap_ind16 *sprite_bitmap, const rectangle &cliprect, uint16_t pri, uint16_t primask, uint16_t penbase, uint8_t alpha)
{
	pen_t const *const paldata = &m_palette->pen(0);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t const *const srcline = &sprite_bitmap->pix(y, 0);
		uint32_t *const dstline = &bitmap.pix(y, 0);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint16_t const pix = srcline[x];

			if ((pix & primask) != pri)
				continue;

			if (pix & 0xf)
			{
				uint16_t pen = pix & 0x1ff;
				if (pix & 0x800) pen += 0x200;

				if (alpha != 0xff)
				{
					if (pix & 0x400) // TODO, Additive/Subtractive Blending?
					{
						uint32_t const base = dstline[x];
						dstline[x] = alpha_blend_r32(base, paldata[pen + penbase], alpha);
					}
					else if (pix & 0x200)
					{
						uint32_t const base = dstline[x];
						dstline[x] = alpha_blend_r32(base, paldata[pen + penbase], alpha);
					}
					else
					{
						dstline[x] = paldata[pen + penbase];
					}
				}
				else
				{
					dstline[x] = paldata[pen + penbase];
				}
			}
		}
	}
}

// are the priorities 100% correct? they're the same as they were before conversion to DECO52 sprite device,
// but if (for example) you walk to the side of the crates in the first part of the game you appear over them...
uint32_t dassault_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t const priority = m_priority;

	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(BIT(flip, 7));
	m_sprgen[1]->set_flip_screen(BIT(flip, 7));

	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram[1]->buffer(), 0x400);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);
	bitmap_ind16 *sprite_bitmap1 = &m_sprgen[0]->get_sprite_temp_bitmap();
	bitmap_ind16 *sprite_bitmap2 = &m_sprgen[1]->get_sprite_temp_bitmap();

	// Update tilemaps
	m_deco_tilegen[0]->pf_update(nullptr, m_pf2_rowscroll);
	m_deco_tilegen[1]->pf_update(nullptr, m_pf4_rowscroll);

	// Draw playfields/update priority bitmap
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(3072), cliprect);
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	// The middle playfields can be swapped priority-wise
	if ((priority & 3) == 0)
	{
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0600, 0x0600, 0x400, 0xff); // 1
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2); // 2
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0400, 0x0600, 0x400, 0xff); // 8
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 16); // 16
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0200, 0x0600, 0x400, 0xff); // 32
		mix_layer(bitmap, sprite_bitmap2, cliprect, 0x0000, 0x0000, 0x800, 0x80); // 64?
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0000, 0x0600, 0x400, 0xff); // 128

	}
	else if ((priority & 3) == 1)
	{
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0600, 0x0600, 0x400, 0xff); // 1
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2); // 2
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0400, 0x0600, 0x400, 0xff); // 8
		mix_layer(bitmap, sprite_bitmap2, cliprect, 0x0000, 0x0000, 0x800, 0x80); // 16?
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0200, 0x0600, 0x400, 0xff); // 32
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 64); // 64
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0000, 0x0600, 0x400, 0xff); // 128
	}
	else if ((priority & 3) == 3)
	{
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0600, 0x0600, 0x400, 0xff); // 1
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2); // 2
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0400, 0x0600, 0x400, 0xff); // 8
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 16); // 16
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0200, 0x0600, 0x400, 0xff); // 32
		mix_layer(bitmap, sprite_bitmap2, cliprect, 0x0000, 0x0000, 0x800, 0x80); // 64?
		mix_layer(bitmap, sprite_bitmap1, cliprect, 0x0000, 0x0600, 0x400, 0xff); // 128
	}
	else
	{
		// Unused
	}

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/**********************************************************************************/

void dassault_state::priority_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_priority);
}

void dassault_state::main_irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}

void dassault_state::sub_irq_ack_w(uint16_t data)
{
	m_subcpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
}

uint16_t dassault_state::control_r(offs_t offset)
{
	switch (offset << 1)
	{
		case 0: // Player 1 & Player 2 joysticks & fire buttons
			return m_input[0]->read();

		case 2: // Player 3 & Player 4 joysticks & fire buttons
			return m_input[1]->read();

		case 4: // Dip 1 (stored at 0x3f8035)
			return m_input[2]->read();

		case 6: // Dip 2 (stored at 0x3f8034)
			return m_input[3]->read();

		case 8: // VBL, Credits
			return m_input[4]->read();
	}

	return 0xffff;
}

void dassault_state::control_w(uint16_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	if (data & 0xfffe)
		logerror("Coin cointrol %04x\n", data);
}

/**********************************************************************************/

void dassault_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x103fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x140004, 0x140005).w(FUNC(dassault_state::main_irq_ack_w));
	map(0x140006, 0x140007).nopw(); // ?

	map(0x180001, 0x180001).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0x1c0000, 0x1c000f).r(FUNC(dassault_state::control_r));
	map(0x1c000a, 0x1c000b).w(FUNC(dassault_state::priority_w));
	map(0x1c000c, 0x1c000d).w(m_spriteram[1], FUNC(buffered_spriteram16_device::write));
	map(0x1c000e, 0x1c000f).w(FUNC(dassault_state::control_w));

	map(0x200000, 0x201fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x202000, 0x203fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x212000, 0x212fff).writeonly().share(m_pf2_rowscroll);
	map(0x220000, 0x22000f).w(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_w));

	map(0x240000, 0x240fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x242000, 0x242fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x252000, 0x252fff).writeonly().share(m_pf4_rowscroll);
	map(0x260000, 0x26000f).w(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_w));

	map(0x3f8000, 0x3fbfff).ram(); // Main RAM
	map(0x3fc000, 0x3fcfff).ram().share("spriteram2");
	map(0x3fe000, 0x3fefff).rw("sharedram", FUNC(mb8421_mb8431_16_device::left_r), FUNC(mb8421_mb8431_16_device::left_w));
}

void dassault_state::sub_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x100001).w(m_spriteram[0], FUNC(buffered_spriteram16_device::write));
	map(0x100002, 0x100003).w(FUNC(dassault_state::sub_irq_ack_w));
	map(0x100004, 0x100007).nopw(); // ?
	map(0x100004, 0x100005).portr("VBLANK1");

	map(0x3f8000, 0x3fbfff).ram();
	map(0x3fc000, 0x3fcfff).ram().share("spriteram1");
	map(0x3fe000, 0x3fefff).rw("sharedram", FUNC(mb8421_mb8431_16_device::right_r), FUNC(mb8421_mb8431_16_device::right_w));
}

/******************************************************************************/

void dassault_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x110000, 0x110001).rw("ym2", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki2, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x1f0000, 0x1f1fff).ram();
}

/**********************************************************************************/

static INPUT_PORTS_START( thndzone )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Adds 4 credits/coins !
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "2 Coins to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" ) // OFF & Not to be changed, according to manual
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" ) // OFF & Not to be changed, according to manual
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" ) // OFF & Not to be changed, according to manual
	PORT_DIPNAME( 0x20, 0x20, "Max Players" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" ) // OFF & Not to be changed, according to manual
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8") // Check code at 0x001490
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P3_P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("VBLANK1") // Cpu 1 vblank
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
INPUT_PORTS_END

static INPUT_PORTS_START( thndzone4 ) // Coin-B selectable values work for this set
	PORT_INCLUDE( thndzone )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) // Start Buttons not used, hit Player Button1 to start for that player
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) // Start Buttons not used, hit Player Button1 to start for that player

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" ) // No selectable number of players DSW
INPUT_PORTS_END

static INPUT_PORTS_START( dassault )
	PORT_INCLUDE( thndzone )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x30, 0x30, "Max Players" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "4 (buggy)" )
INPUT_PORTS_END

static INPUT_PORTS_START( dassault4 )
	PORT_INCLUDE( thndzone )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) // Start Buttons not used, hit Player Button1 to start for that player
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) // Start Buttons not used, hit Player Button1 to start for that player

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" ) // No selectable Coin-B values
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" ) // No selectable number of players DSW
INPUT_PORTS_END

/**********************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_dassault )
	// "chars" is copied to "tiles1" at runtime
	GFXDECODE_ENTRY( "tiles1",   0, charlayout,     0,  32 )      // 8x8
	GFXDECODE_ENTRY( "tiles1",   0, tilelayout,     0,  32 )      // 16x16
	GFXDECODE_ENTRY( "tiles2",   0, tilelayout,   512,  32 )      // 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_dassault_spr1 )
	GFXDECODE_ENTRY( "sprites1", 0, tilelayout,  0/*1024*/,  64 ) // 16x16
GFXDECODE_END

static GFXDECODE_START( gfx_dassault_spr2 )
	GFXDECODE_ENTRY( "sprites2", 0, tilelayout,  0/*2048*/,  64 ) // 16x16
GFXDECODE_END

/**********************************************************************************/

void dassault_state::sound_bankswitch_w(uint8_t data)
{
	// the second OKIM6295 ROM is bank switched
	m_oki2->set_rom_bank(data & 1);
}

DECO16IC_BANK_CB_MEMBER(dassault_state::bank_callback)
{
	return ((bank >> 4) & 0xf) << 12;
}

void dassault_state::machine_reset()
{
	m_priority = 0;
}

void dassault_state::dassault(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(28'000'000) / 2); // 14MHz - Accurate
	m_maincpu->set_addrmap(AS_PROGRAM, &dassault_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(dassault_state::irq4_line_assert));

	M68000(config, m_subcpu, XTAL(28'000'000) / 2); // 14MHz - Accurate
	m_subcpu->set_addrmap(AS_PROGRAM, &dassault_state::sub_map);
	m_subcpu->set_vblank_int("screen", FUNC(dassault_state::irq5_line_assert));

	H6280(config, m_audiocpu, XTAL(32'220'000) / 8); // Accurate
	m_audiocpu->set_addrmap(AS_PROGRAM, &dassault_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	m_audiocpu->add_route(ALL_OUTPUTS, "speaker", 0, 1);

	config.set_maximum_quantum(attotime::from_hz(m_maincpu->clock() / 4)); // I was seeing random lockups.. let's see if this helps

	mb8421_mb8431_16_device &sharedram(MB8421_MB8431_16BIT(config, "sharedram"));
	sharedram.intl_callback().set_inputline("maincpu", M68K_IRQ_5);
	sharedram.intr_callback().set_inputline("sub", M68K_IRQ_6);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(28'000'000) / 4, 442, 0, 320, 274, 8, 248); // same as robocop2(cninja.cpp)? verify this from real PCB.
	screen.set_screen_update(FUNC(dassault_state::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_dassault);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 4096);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	DECO16IC(config, m_deco_tilegen[0], 0);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0);
	m_deco_tilegen[0]->set_pf2_col_bank(16);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(dassault_state::bank_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(dassault_state::bank_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag("gfxdecode");

	DECO16IC(config, m_deco_tilegen[1], 0);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0);
	m_deco_tilegen[1]->set_pf2_col_bank(16);
	m_deco_tilegen[1]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[1]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(dassault_state::bank_callback));
	m_deco_tilegen[1]->set_bank2_callback(FUNC(dassault_state::bank_callback));
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen[0], 0, m_palette, gfx_dassault_spr1);
	DECO_SPRITE(config, m_sprgen[1], 0, m_palette, gfx_dassault_spr2);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0); // IRQ1

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(32'220'000) / 8));
	ym1.add_route(ALL_OUTPUTS, "speaker", 0.40, 0);
	ym1.add_route(ALL_OUTPUTS, "speaker", 0.40, 1);

	ym2151_device &ym2(YM2151(config, "ym2", XTAL(32'220'000) / 9));
	ym2.irq_handler().set_inputline(m_audiocpu, 1);
	ym2.port_write_handler().set(FUNC(dassault_state::sound_bankswitch_w));
	ym2.add_route(0, "speaker", 0.45, 0);
	ym2.add_route(1, "speaker", 0.45, 1);

	okim6295_device &oki1(OKIM6295(config, "oki1", XTAL(32'220'000) / 32, okim6295_device::PIN7_HIGH)); // verified
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.50, 0);
	oki1.add_route(ALL_OUTPUTS, "speaker", 0.50, 1);

	OKIM6295(config, m_oki2, XTAL(32'220'000) / 16, okim6295_device::PIN7_HIGH); // verified
	m_oki2->add_route(ALL_OUTPUTS, "speaker", 0.25, 0);
	m_oki2->add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
}

/**********************************************************************************/

ROM_START( thndzone ) // World rev 1 set, DSW selectable 2 or 4 players
	ROM_REGION(0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gz01-1.a15", 0x00000, 0x20000, CRC(20250da6) SHA1(2d01d59b67a2ecc2ddc88eded43f451931a0a33b) )
	ROM_LOAD16_BYTE("gz03-1.a17", 0x00001, 0x20000, CRC(3595fad0) SHA1(5d61776cdf2274cb26ea06ce97c35f5ce7f27e66) )
	ROM_LOAD16_BYTE("gt00.a14",   0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) // Same data as GS00.A14
	ROM_LOAD16_BYTE("gt02.a16",   0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) // Same data as GS02.A16

	ROM_REGION(0x80000, "sub", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gz10-1.a12", 0x00000, 0x20000, CRC(811d86d7) SHA1(94971acad2c648b7a65ca54d9315414ed3b94f24) )
	ROM_LOAD16_BYTE("gz08-1.a9",  0x00001, 0x20000, CRC(8f61ab1e) SHA1(df4e7db889915eca39ed4e1a4b5fcae9cd1a9882) )
	ROM_LOAD16_BYTE("gt11-1.a14", 0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) // Same data as GS11.A14
	ROM_LOAD16_BYTE("gt09-1.a11", 0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) // Same data as GS09.A11

	ROM_REGION(0x10000, "audiocpu", 0 )
	ROM_LOAD( "gt04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) // Same data for all regions, different label

	ROM_REGION(0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "gt05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) // Same data for all regions, different label
	ROM_LOAD16_BYTE( "gt06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) // Same data for all regions, different label

	ROM_REGION(0x120000, "tiles1", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	// Other 0x20000 filled in later

	ROM_REGION(0x200000, "tiles2", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "gt12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gt13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gt14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gt15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 )
	ROM_LOAD( "gt07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) ) /* Same data as GS07.H15 */

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )   // banked

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )   // Priority?  Unused
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   /* Timing??  Unused */
	/* Above prom also at 16s and 17s */

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( thndzonea ) /* World set, DSW selectable 2 or 4 players */
	ROM_REGION(0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gz01.a15", 0x00000, 0x20000, CRC(15e8c328) SHA1(8876b5fde77604c2fe4654271ceb341a8fa460c1) )
	ROM_LOAD16_BYTE("gz03.a17", 0x00001, 0x20000, CRC(aab5c86e) SHA1(c3560b15360ddf14e8444d9f70724e698b2bd42f) )
	ROM_LOAD16_BYTE("gt00.a14", 0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) /* Same data as GS00.A14 */
	ROM_LOAD16_BYTE("gt02.a16", 0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) /* Same data as GS02.A16 */

	ROM_REGION(0x80000, "sub", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gz10.a12",   0x00000, 0x20000, CRC(79f919e9) SHA1(b6793173e310b1df07cf3e9209da1fbec3a8a05b) )
	ROM_LOAD16_BYTE("gz08.a9",    0x00001, 0x20000, CRC(d47d7836) SHA1(8a5d3e8b89f5dfd6bac83f7b093ddb03d5ecef73) )
	ROM_LOAD16_BYTE("gt11-1.a14", 0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) /* Same data as GS11.A14 */
	ROM_LOAD16_BYTE("gt09-1.a11", 0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) /* Same data as GS09.A11 */

	ROM_REGION(0x10000, "audiocpu", 0 )
	ROM_LOAD( "gt04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) // Same data for all regions, different label

	ROM_REGION(0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "gt05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) // Same data for all regions, different label
	ROM_LOAD16_BYTE( "gt06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) // Same data for all regions, different label

	ROM_REGION(0x120000, "tiles1", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	// Other 0x20000 filled in later

	ROM_REGION(0x200000, "tiles2", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "gt12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gt13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gt14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gt15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 )
	ROM_LOAD( "gt07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) ) /* Same data as GS07.H15 */

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )   // banked

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )   // Priority?  Unused
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused - Data identical for 3 proms!

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( thndzone4 ) /* World set, 4 Player (shared credits) only English set from a Korean PCB without labels */
	ROM_REGION(0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE("27c010.a15", 0x00000, 0x20000, CRC(30f21608) SHA1(087defd8869faf3f7f4569b98debe691a75fcec4) )
	ROM_LOAD16_BYTE("27c010.a17", 0x00001, 0x20000, CRC(60886a33) SHA1(5b215e460845705af4b5e0cd00f6b0ad488520bb) )
	ROM_LOAD16_BYTE("gt00.a14", 0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) /* Same data as GS00.A14 */
	ROM_LOAD16_BYTE("gt02.a16", 0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) /* Same data as GS02.A16 */

	ROM_REGION(0x80000, "sub", 0 ) // 68000 code
	ROM_LOAD16_BYTE("d27c010.a12", 0x00000, 0x20000, CRC(99356cba) SHA1(2bc2b031bd44101e12213bb04a94d2d438f96ee0) )
	ROM_LOAD16_BYTE("d27c010.a9",  0x00001, 0x20000, CRC(8bf114e7) SHA1(84b1b8d8aea8788902367cae3b766bb4e6e44d5a) )
	ROM_LOAD16_BYTE("d27c010.a14", 0x40000, 0x20000, CRC(3d96d47e) SHA1(e2c01a17237cb6dc914da847642629415eda14a8) )
	ROM_LOAD16_BYTE("d27c010.a11", 0x40001, 0x20000, CRC(2ab9b63f) SHA1(2ab06abbdee6e0d9c83004cdcb871c7389624086) )

	ROM_REGION(0x10000, "audiocpu", 0 )
	ROM_LOAD( "gu04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) // Same data for all regions, different label

	ROM_REGION(0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "27512.j10", 0x000000, 0x10000, CRC(ab22a078) SHA1(246c9ebae5c2f296652395267fa3eeb81b8b52bd) ) /* Only set with different data here! */
	ROM_LOAD16_BYTE( "27512.j12", 0x000001, 0x10000, CRC(34fc4428) SHA1(c912441ab8433391193b4199c2553d7909221d93) ) /* Only set with different data here! */

	ROM_REGION(0x120000, "tiles1", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	// Other 0x20000 filled in later

	ROM_REGION(0x200000, "tiles2", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "gt12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gt13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gt14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gt15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 )
	ROM_LOAD( "gs07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )   // banked

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )   // Priority?  Unused
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused - Data identical for 3 proms!

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( thndzonej ) /* Japan set, DSW selectable 2 or 4 players - Japanese language */
	ROM_REGION(0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gu01.a15", 0x00000, 0x20000, CRC(eb28f8e8) SHA1(834f89db3ef48a71d20c0ec3a0c2231e115d7f48) )
	ROM_LOAD16_BYTE("gu03.a17", 0x00001, 0x20000, CRC(9ad2b431) SHA1(c2fb88b4d2df93e3f787fe49c240573e1bc2844e) )
	ROM_LOAD16_BYTE("gu00.a14", 0x40000, 0x20000, CRC(fca9e84f) SHA1(a0ecf99eace7357b05da8f8fe06b9bbf7d16d95a) )
	ROM_LOAD16_BYTE("gu02.a16", 0x40001, 0x20000, CRC(b6026bae) SHA1(673b7f7a432580ec1780d1efa2b48184af428698) )

	ROM_REGION(0x80000, "sub", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gu10.a12", 0x00000, 0x20000, CRC(8042e87d) SHA1(dc69b13fc06d94a2bc5569e96931e6d9496bd44f) )
	ROM_LOAD16_BYTE("gu08.a9",  0x00001, 0x20000, CRC(c8895bfa) SHA1(6a5421bd926e0aa86c81e345f2dfe5265bd3add2) )
	ROM_LOAD16_BYTE("gu11.a14", 0x40000, 0x20000, CRC(c0d6eb82) SHA1(44070e6d37f5327cf7f647e44ea49a1fe6844e5e) )
	ROM_LOAD16_BYTE("gu09.a11", 0x40001, 0x20000, CRC(42de13a7) SHA1(f948d31e368499fd8c35da0c7dd7519cfbd4b5f7) )

	ROM_REGION(0x10000, "audiocpu", 0 )
	ROM_LOAD( "gu04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) // Same data for all regions, different label

	ROM_REGION(0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "gu05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) // Same data for all regions, different label
	ROM_LOAD16_BYTE( "gu06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) // Same data for all regions, different label

	ROM_REGION(0x120000, "tiles1", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	// Other 0x20000 filled in later

	ROM_REGION(0x200000, "tiles2", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	// Although the other Mask ROMs on the PCB are MAJ-xx, for the Japan version, these 4 are actually MAL-xx
	ROM_LOAD16_BYTE( "mal-12.n1", 0x000000, 0x20000, NO_DUMP ) // Mask ROM - Need to verify if these are the same or different as the other sets
	ROM_LOAD16_BYTE( "mal-13.n2", 0x000001, 0x20000, NO_DUMP ) // Mask ROM - Need to verify if these are the same or different as the other sets
	ROM_LOAD16_BYTE( "mal-14.n3", 0x040000, 0x20000, NO_DUMP ) // Mask ROM - Need to verify if these are the same or different as the other sets
	ROM_LOAD16_BYTE( "mal-15.n5", 0x040001, 0x20000, NO_DUMP ) // Mask ROM - Need to verify if these are the same or different as the other sets
	ROM_LOAD16_BYTE( "gt12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) ) // REMOVE when MAL-12.N1 is dumped & added
	ROM_LOAD16_BYTE( "gt13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) ) // REMOVE when MAL-13.N2 is dumped & added
	ROM_LOAD16_BYTE( "gt14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) ) // REMOVE when MAL-14.N3 is dumped & added
	ROM_LOAD16_BYTE( "gt15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) ) // REMOVE when MAL-15.N5 is dumped & added

	ROM_REGION(0x40000, "oki1", 0 )
	// This rom is also a Mask ROM label MAL-07 and _NOT_ MAJ-07
	ROM_LOAD( "mal-07.h15",  0x00000,  0x20000, NO_DUMP ) // Mask ROM - Need to verify if these are the same or different as the other sets
	ROM_LOAD( "gs07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) ) // REMOVE when MAL-07.H15 is dumped & added

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )   // banked

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )   // Priority?  Unused
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused - Data identical for 3 proms!

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( dassault ) // USA set, DSW selectable 2, 3 or 4 players
	ROM_REGION(0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE("01.a15",   0x00000, 0x20000, CRC(14f17ea7) SHA1(0bb8b7dba05f1ea42e68838861f0d4c263eac6b3) )
	ROM_LOAD16_BYTE("03.a17",   0x00001, 0x20000, CRC(bed1b90c) SHA1(c100f89b69025e2ff885b35a733abc627da98a07) )
	ROM_LOAD16_BYTE("gs00.a14", 0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) // Same data as GT00.A14
	ROM_LOAD16_BYTE("gs02.a16", 0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) // Same data as GT02.A16

	ROM_REGION(0x80000, "sub", 0 ) // 68000 code
	ROM_LOAD16_BYTE("hc10-1.a12", 0x00000, 0x20000, CRC(ac5ac770) SHA1(bf6640900c2f9c8091168bf106edf85350c34652) )
	ROM_LOAD16_BYTE("hc08-1.a9",  0x00001, 0x20000, CRC(864dca56) SHA1(0967f613684b539d10b67e4f6033c890e2134ea2) )
	ROM_LOAD16_BYTE("gs11.a14",   0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) // Same data as GT11-1.A14
	ROM_LOAD16_BYTE("gs09.a11",   0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) // Same data as GT09-1.A11

	ROM_REGION(0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) // Same data for all regions, different label

	ROM_REGION(0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "gs05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) // Same data for all regions, different label
	ROM_LOAD16_BYTE( "gs06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) // Same data for all regions, different label

	ROM_REGION(0x120000, "tiles1", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	// Other 0x20000 filled in later

	ROM_REGION(0x200000, "tiles2", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "gs12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gs13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gs14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gs15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 )
	ROM_LOAD( "gs07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )   // banked

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )   // Priority?  Unused
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused - Data identical for 3 proms!

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( dassault4 ) // USA set, 4 player only
	ROM_REGION(0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gs01.a15", 0x00000, 0x20000, CRC(8613634d) SHA1(69b64e54fde3b5f1ee3435d7327b84e7a7d43f6d) )
	ROM_LOAD16_BYTE("gs03.a17", 0x00001, 0x20000, CRC(ea860bd4) SHA1(6e4e2d004433ad5842b4bc895eaa8f55bd1ee168) )
	ROM_LOAD16_BYTE("gs00.a14", 0x40000, 0x20000, CRC(b7277175) SHA1(ffb19c4dd12e0391f01de57c46a7998885fe22bf) ) // Same data as GT00.A14
	ROM_LOAD16_BYTE("gs02.a16", 0x40001, 0x20000, CRC(cde31e35) SHA1(0219845308c9f46e73b0504bd2aefa2fa74f388e) ) // Same data as GT02.A16

	ROM_REGION(0x80000, "sub", 0 ) // 68000 code
	ROM_LOAD16_BYTE("gs10.a12", 0x00000, 0x20000, CRC(285f72a3) SHA1(d01972aec500805ca1abed14983064cd14e942d4) )
	ROM_LOAD16_BYTE("gs08.a9",  0x00001, 0x20000, CRC(16691ede) SHA1(dc481dfc6104833a6fd18be6275e77ecc0510165) )
	ROM_LOAD16_BYTE("gs11.a14", 0x40000, 0x20000, CRC(80cb23de) SHA1(d52426460eea2285c57cfc3fe37aa6dc79990e25) ) // Same data as GT11-1.A14
	ROM_LOAD16_BYTE("gs09.a11", 0x40001, 0x20000, CRC(0a8fa7e1) SHA1(330ae9602b5f56b5dc4961a41991b64412a59880) ) // Same data as GT09-1.A11

	ROM_REGION(0x10000, "audiocpu", 0 )
	ROM_LOAD( "gs04.f18",    0x00000, 0x10000, CRC(81c29ebf) SHA1(1b241277a8e35cdeaeb120970d14a09d33032459) ) // Same data for all regions, different label

	ROM_REGION(0x020000, "chars", 0 )
	ROM_LOAD16_BYTE( "gs05.h11", 0x000000, 0x10000, CRC(0aae996a) SHA1(d37a12b057e9934212362d7eafa575c961819a27) ) // Same data for all regions, different label
	ROM_LOAD16_BYTE( "gs06.h12", 0x000001, 0x10000, CRC(4efdf03d) SHA1(835d22829c6d0f4efc76801b449f9a779f460f1c) ) // Same data for all regions, different label

	ROM_REGION(0x120000, "tiles1", 0 )
	ROM_LOAD( "maj-02.h14", 0x000000, 0x100000, CRC(383bbc37) SHA1(c537ab147a2770ce28ee185b08dd62d35249bfa9) )
	// Other 0x20000 filled in later

	ROM_REGION(0x200000, "tiles2", 0 )
	ROM_LOAD( "maj-01.c18", 0x000000, 0x100000, CRC(9840a204) SHA1(096c351769da5184c3d9a05495370134acc9507a) )
	ROM_LOAD( "maj-00.c17", 0x100000, 0x100000, CRC(87ea8d16) SHA1(db47123aa2ebbb800cfc5cfcf50309bc39cadbcd) )

	ROM_REGION( 0x400000, "sprites1", 0 )
	ROM_LOAD( "maj-04.r1",  0x000000, 0x80000, CRC(36e49b19) SHA1(bfbc45b635bf3d46ff8b8a514a3f352bf3a95535) )
	ROM_LOAD( "maj-05.r2",  0x080000, 0x80000, CRC(80fc71cc) SHA1(65b15afbe5d628051b012777d486b6ce92a3795c) )
	ROM_LOAD( "maj-06.r3",  0x100000, 0x80000, CRC(2e7a684b) SHA1(cffeda1a816dad30d6b1cb12458661188d625d40) )
	ROM_LOAD( "maj-07.r5",  0x180000, 0x80000, CRC(3acc1f78) SHA1(87ec65b4f54a66370754534d03f4c9217531b42f) )
	ROM_LOAD( "maj-08.s6",  0x200000, 0x80000, CRC(1958a36d) SHA1(466a30dcd2ea13028272ed2187f890ee20d6636b) )
	ROM_LOAD( "maj-09.s8",  0x280000, 0x80000, CRC(c21087a1) SHA1(b769c5f2f9b9c525d121902fe9557a6bfc077b99) )
	ROM_LOAD( "maj-10.s9",  0x300000, 0x80000, CRC(a02fa641) SHA1(14b999a441964e612700bf21945a948eaebb253e) )
	ROM_LOAD( "maj-11.s11", 0x380000, 0x80000, CRC(dabe9305) SHA1(44d69fe55e674de7f4c610d295d4528d4b2eb150) )

	ROM_REGION( 0x80000, "sprites2", 0 )
	ROM_LOAD16_BYTE( "gs12.n1", 0x000000, 0x20000, CRC(9a86a015) SHA1(968576b8422393ab9a93d98c15428b1c11417b3d) )
	ROM_LOAD16_BYTE( "gs13.n2", 0x000001, 0x20000, CRC(f4709905) SHA1(697842a3d7bc2588c77833c3af8938e6f0b1238d) )
	ROM_LOAD16_BYTE( "gs14.n3", 0x040000, 0x20000, CRC(750fc523) SHA1(ef8794359ff3a44a97ab402821fbe205a0be8f6a) )
	ROM_LOAD16_BYTE( "gs15.n5", 0x040001, 0x20000, CRC(f14edd3d) SHA1(802d576df6dac2c9bf99f963f1955fc3a7ffdac0) )

	ROM_REGION(0x40000, "oki1", 0 )
	ROM_LOAD( "gs07.h15",  0x00000,  0x20000,  CRC(750b7e5d) SHA1(d33b17a1d8c9b05d5c1daf0c80fed6381e04b167) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "maj-03.h16", 0x00000, 0x80000,  CRC(31dcfac3) SHA1(88c7fc139f871991defbc8dc2c9c66b150dd6f6f) )   // banked

	ROM_REGION( 0x2000, "proms", 0 )
	ROM_LOAD( "mb7128y.10m", 0x00000,  0x800,  CRC(bde780a2) SHA1(94ea9fe6c3a421e976d077e67f564ca5c37a5e88) )   // Priority?  Unused
	ROM_LOAD( "mb7128y.16p", 0x00800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.16s", 0x01000,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused
	ROM_LOAD( "mb7128y.17s", 0x01800,  0x800,  CRC(c44d2751) SHA1(7c195650689d5cbbdccba696e0e7d3dc5bb7c506) )   // Unknown,  unused - Data identical for 3 proms!

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "pal16r8a 1h",  0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7c",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7d",  0x0400, 0x0104, CRC(199e83fd) SHA1(ebb5d66f29935b0a58e79b0db30611b5dce328a6) ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7e",  0x0600, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.7l",  0x0800, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.8e",  0x0a00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.9d",  0x0c00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16l8b.10c", 0x0e00, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

/**********************************************************************************/


void dassault_state::init_dassault()
{
	const uint8_t *src = memregion("chars")->base();
	uint8_t *dst = memregion("tiles1")->base();
	std::vector<uint8_t> tmp(0x80000);

	/* Playfield 4 also has access to the char graphics, make things easier
	by just copying the chars to both banks (if I just used a different gfx
	bank then the colours would be wrong). */
	memcpy(&tmp[0x000000], dst + 0x80000, 0x80000);
	memcpy(dst + 0x090000, &tmp[0x00000], 0x80000);
	memcpy(dst + 0x080000, src + 0x00000, 0x10000);
	memcpy(dst + 0x110000, src + 0x10000, 0x10000);
}

} // anonymous namespace


/**********************************************************************************/

GAME( 1991, thndzone,  0,        dassault, thndzone,  dassault_state, init_dassault, ROT0, "Data East Corporation", "Thunder Zone (World, Rev 1)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, thndzonea, thndzone, dassault, thndzone,  dassault_state, init_dassault, ROT0, "Data East Corporation", "Thunder Zone (World)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, thndzone4, thndzone, dassault, thndzone4, dassault_state, init_dassault, ROT0, "Data East Corporation", "Thunder Zone (World 4 Players)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, thndzonej, thndzone, dassault, thndzone,  dassault_state, init_dassault, ROT0, "Data East Corporation", "Thunder Zone (Japan)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, dassault,  thndzone, dassault, dassault,  dassault_state, init_dassault, ROT0, "Data East Corporation", "Desert Assault (US)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, dassault4, thndzone, dassault, dassault4, dassault_state, init_dassault, ROT0, "Data East Corporation", "Desert Assault (US 4 Players)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
