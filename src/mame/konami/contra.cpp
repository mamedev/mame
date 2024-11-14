// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Phil Stroffolino
/***************************************************************************

Contra/Gryzor (c) 1987 Konami

Notes:
    Press 1P and 2P together to advance through tests.

Credits:
    Carlos A. Lozano: CPU emulation
    Phil Stroffolino: video driver
    Jose Tejada Gomez (of Grytra fame) for precious information on sprites
    Eric Hustvedt: palette optimizations and cocktail support

2008-07
Dip locations and factory settings verified with manual

***************************************************************************

Contra / Gryzor, Konami 1987
Hardware info by Guru

GX633 PWB302160
|-------------------------------------------------------|
|CN1  LA4445 CN3 CN4  YM2151    68B09     633M03.18A    |
|             VOL                      633I02.17A 63C09 |
|CN2          VOL          633E01.12A       007324      |
|    005924     LM324 YM3012                            |
|                                                       |
|                                   8416 8464           |
|     DIPSW1           24MHz                     007452 |
|005273(x5)            3.579545MHz           007766     |
|     DIPSW2                                            |
|J          633E04.7D             633E06.16D            |
|A    DIPSW3       |-----------|     |-----------|      |
|M                 |           |5164 |           |5164  |
|M                 |  KONAMI   |     |  KONAMI   |      |
|A|--------|       |  007121   |     |  007121   |      |
| |        |       |           |     |           |      |
| | KONAMI |       |           |     |           |      |
| | 007593 |       |-----------|     |-----------|      |
| |        |                                            |
| |        |633E05.7F             633E07.16F            |
| |--------| 633E08.10G  4464 4464  633E10.18G 4464 4464|
|                 633E09.12G          633E11.20G        |
|-------------------------------------------------------|
Notes:
      LA4445 - Sanyo LA4445 2-Channel (Stereo) 5.5W Audio Power Amplifier
       68B09 - 1.7897725MHz [3.579545/2] (audio CPU)
       63C09 - 3.000MHz [24/8] (main CPU)
      YM2151 - Yamaha YM2151 FM Operator Type-M (OPM) sound chip. Clock input 3.579545MHz
      YM3012 - Yamaha YM3012 2-Channel Serial Input Floating D/A Converter (DAC-MS). Clock input 1.7897725MHz [3.579545/2]
       LM324 - Texas Instruments LM324 Quad Operational Amplifier
         CN1 - 4-pin connector for additional composite sync output
         CN2 - 4-pin connector for 2nd speaker for stereo audio output
         CN3 - 4-pin connector for mono output selector   \ with loopback plug connected to either CN3 or CN4
         CN4 - 4-pin connector for stereo output selector /
        8416 - Fujitsu MB8416 2kBx8-bit SRAM (sound program RAM)
        8464 - Fujitsu MB8464 8kBx8-bit SRAM (main program RAM)
        5164 - Sharp LH5164 or Fujitsu MB8464 8kBx8-bit SRAM (background RAM)
        4464 - NEC D4464 64kBx8-bit DRAM (sprite RAM)
      005273 - Konami custom resistor pack for controls/inputs
      005924 - Konami custom ceramic coin counter driver
      007766 - PAL16L8 marked '007766' at location 20D
      007121 - Konami custom sprite / background generator
      007324 - Resistor array package containing 8x 150 ohm resistors. The IC looks like a DIP16 logic chip
               but with an epoxy top. The schematics show it connected to the 6309 data lines (D0-D7) and the
               program ROM data lines (D0-D7). It is a simple resistor array.
      007452 - Konami custom chip
      007593 - Konami custom ceramic color mixer and RGB video DAC
               Contains some logic and 2x 2kBx8-bit SRAMs for color RAM.
               This IC also has 16x I/O lines but they are not connected on this PCB.
  633I02.17A - 27512 64kBx8-bit EPROM \
  633M03.18A - 27512 64kBx8-bit EPROM / (main program)
  633E01.12A - 27256 32kBx8-bit EPROM (sound program)
   633E04.7D - \
   633E05.7F - |
  633E06.16D - | 256kBx8-bit (2Mbit) DIP40 mask ROM (graphics)
  633E07.16F - /
  633E08.10G - MMI 63S141 256x4-bit Bipolar PROM, compatible with 82S129 (sprite lookup table)
  633E09.12G - MMI 63S141 256x4-bit Bipolar PROM, compatible with 82S129 (character lookup table)
  633E10.18G - MMI 63S141 256x4-bit Bipolar PROM, compatible with 82S129 (sprite lookup table)
  633E11.20G - MMI 63S141 256x4-bit Bipolar PROM, compatible with 82S129 (character lookup table)
      DIPSW3 - 4-position DIP switch
    DIPSW1/2 - 8-position DIP switch


Contra / Gryzor bootleg
Hardware info by Guru

(no PCB number)
|-------------------------------------------------------------------------------------|
| C1182         51C64 51C64 51C64 51C64 51C64                                  G-4.A17| A
| VOL  PAL16R6    51C64 51C64 51C64 51C64 51C64                4016 82S129.B15 G-5.B17| B
|                      |------------------|                         82S129.C15        | C
|    18MHz      PAL16R8|                  |                                    G-6.D17| D
|                      |                  |                                           | E
|                      |----SUB-BOARD-----|                                    G-7.F17| F
|    G-3.G2     PAL16R4           PAL16L8                                      G-8.G17| G
|                                                                   PAL16R4           | H
|J   G-2.J2                                                                    G-9.J17| J
|A   68B09(1) 6264       PAL16L8                                              G-10.K17| K
|M   LM324   YM3012                                                 4016      G-11.L17| L
|M           DIPSW3   YM2151                                                  G-12.M17| M
|A                    68B09(2)                    PAL20L8  PAL20L8                    | N
|                                                                             G-13.P17| P
|     DIPSW1 DIPSW2 G-1.P5  2063    2063                        PAL16R8       G-14.Q17| Q
|14.31818MHz        2015                                                              | R
|                                                                             G-15.S17| S
|                                                                  82S129.T16 G-16.T17| T
|                                                                        82S129.U16   | U
|                                                                             G-17.V17| V
|       6116  6116                                                            G-18.W17| W
|-------------------------------------------------------------------------------------|
  1   2   3     4     5     6    7    8    9  10 11  12    13    14    15    16    17

Notes:
   68B09(1) - Motorola MC68B09EP CPU. Clock input 3.000MHz [18/6]
   68B09(2) - Motorola MC68B09EP CPU. Clock input 1.7897725MHz [14.31818/8]
     YM3012 - Yamaha YM3012 2-Channel Serial Input Floating D/A Converter (DAC-MS). Clock input 1.7897725MHz [3.579545/2]
     YM2151 - Yamaha YM2151 FM Operator Type-M (OPM) sound chip. Clock input 3.579545MHz [14.31818/4]
      51C64 - Hynix HY51C64 64kBx1-bit DRAM (sprite RAM). Total RAM is 80kBx8-bit whereas the original board has 128kBx8-bit DRAM (4x 4464 chips).
       6116 - HM6116 2kBx8-bit SRAM (color RAM). At power-on, this is tested as one RAM equivalent to the
              internal 4kBx8-bit SRAM in the custom color mixer/RGB DAC at 1F on the original board.
       6264 - Hitachi HM6264 8kBx8-bit SRAM (main program RAM)
       2063 - Toshiba TMM2063 8kBx8-bit SRAM (background RAM front layer)
       4016 - NEC D4016 2kBx8-bit SRAM (background RAM back layer)
       2015 - Toshiba TMM2015 2kBx8-bit SRAM (sound program RAM)
      C1182 - NEC C1182 Audio Power Amplifier
      LM324 - Texas Instruments LM324 Quad Operational Amplifier
  SUB-BOARD - Daughter board containing logic plugged into a DIP40 socket. Might be a re-implementation
              of Konami 007452 custom chip modified to fit a DIP40 socket.
     DIPSW3 - 4-position DIP switch
   DIPSW1/2 - 8-position DIP switch
      HSync - 15.35929kHz
      VSync - 61.0208Hz

***************************************************************************/


#include "emu.h"

#include "konamipt.h"
#include "k007452.h"
#include "k007121.h"

#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class contra_state : public driver_device
{
public:
	contra_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cram(*this, "cram%u", 0U),
		m_vram(*this, "vram%u", 0U),
		m_mainbank(*this, "mainbank"),
		m_buffered_spriteram(*this, "spriteram%u", 1U),
		m_audiocpu(*this, "audiocpu"),
		m_k007121(*this, "k007121_%u", 1U),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void contra(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<uint8_t, 3> m_cram;
	required_shared_ptr_array<uint8_t, 3> m_vram;
	required_memory_bank m_mainbank;

	// video-related
	tilemap_t *m_tilemap[3]{};
	rectangle m_clip[3]{};

	// devices
	required_device_array<buffered_spriteram8_device, 2> m_buffered_spriteram;
	required_device<cpu_device> m_audiocpu;
	required_device_array<k007121_device, 2> m_k007121;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void bankswitch_w(uint8_t data);
	void sh_irqtrigger_w(uint8_t data);
	void sirq_clear_w(uint8_t data);
	void coin_counter_w(uint8_t data);
	template <uint8_t Which> void vram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void cram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void K007121_ctrl_w(offs_t offset, uint8_t data);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	template <uint8_t Which> void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
**
**  Contra has palette RAM, but it also has four lookup table PROMs
**
**  0   sprites #0
**  1   tiles   #0
**  2   sprites #1
**  3   tiles   #1
**
***************************************************************************/

void contra_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int chip = 0; chip < 2; chip++)
	{
		for (int pal = 0; pal < 8; pal++)
		{
			int const clut = (chip << 1) | (pal & 1);

			for (int i = 0; i < 0x100; i++)
			{
				uint8_t ctabentry;

				if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

				palette.set_pen_indirect((chip << 11) | (pal << 8) | i, ctabentry);
			}
		}
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

template <uint8_t Which>
TILE_GET_INFO_MEMBER(contra_state::get_tile_info)
{
	uint8_t ctrl_3 = m_k007121[Which]->ctrlram_r(3);
	uint8_t ctrl_4 = m_k007121[Which]->ctrlram_r(4);
	uint8_t ctrl_5 = m_k007121[Which]->ctrlram_r(5);
	uint8_t ctrl_6 = m_k007121[Which]->ctrlram_r(6);
	int attr = m_cram[Which][tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	tileinfo.set(0,
			m_vram[Which][tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}


TILE_GET_INFO_MEMBER(contra_state::get_tx_tile_info)
{
	uint8_t ctrl_5 = m_k007121[0]->ctrlram_r(5);
	uint8_t ctrl_6 = m_k007121[0]->ctrlram_r(6);
	int attr = m_cram[2][tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10);

	tileinfo.set(0,
			m_vram[2][tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void contra_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_k007121[0], tilemap_get_info_delegate(*this, FUNC(contra_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_k007121[1], tilemap_get_info_delegate(*this, FUNC(contra_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_k007121[0], tilemap_get_info_delegate(*this, FUNC(contra_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_clip[1] = m_screen->visible_area();
	m_clip[1].min_x += 40;

	m_clip[0] = m_clip[1];

	m_clip[2] = m_screen->visible_area();
	m_clip[2].max_x = 39;
	m_clip[2].min_x = 0;

	m_tilemap[0]->set_transparent_pen(0);
}


/***************************************************************************

    Memory handlers

***************************************************************************/

template <uint8_t Which>
void contra_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void contra_state::cram_w(offs_t offset, uint8_t data)
{
	m_cram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void contra_state::K007121_ctrl_w(offs_t offset, uint8_t data)
{
	uint8_t prev = m_k007121[Which]->ctrlram_r(offset);

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			m_buffered_spriteram[Which]->copy(0x800, 0x800);
		else
			m_buffered_spriteram[Which]->copy(0x000, 0x800);
	}

	if (offset == 6)
	{
		if (prev != data)
			m_tilemap[Which]->mark_all_dirty();
	}

	if (offset == 7)
		m_tilemap[Which]->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_k007121[Which]->ctrl_w(offset, data);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

template <uint8_t Which>
void contra_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap)
{
	int base_color = (m_k007121[Which]->ctrlram_r(6) & 0x30) * 2;

	m_k007121[Which]->sprites_draw(bitmap, cliprect, m_buffered_spriteram[Which]->buffer(), base_color, 40, 0, priority_bitmap, (uint32_t)-1);
}

uint32_t contra_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t ctrl_1_0 = m_k007121[0]->ctrlram_r(0);
	uint8_t ctrl_1_2 = m_k007121[0]->ctrlram_r(2);
	uint8_t ctrl_2_0 = m_k007121[1]->ctrlram_r(0);
	uint8_t ctrl_2_2 = m_k007121[1]->ctrlram_r(2);
	rectangle bg_finalclip = m_clip[1];
	rectangle fg_finalclip = m_clip[0];
	rectangle tx_finalclip = m_clip[2];

	bg_finalclip &= cliprect;
	fg_finalclip &= cliprect;
	tx_finalclip &= cliprect;

	m_tilemap[0]->set_scrollx(0, ctrl_1_0 - 40);
	m_tilemap[0]->set_scrolly(0, ctrl_1_2);
	m_tilemap[1]->set_scrollx(0, ctrl_2_0 - 40);
	m_tilemap[1]->set_scrolly(0, ctrl_2_2);

	m_tilemap[1]->draw(screen, bitmap, bg_finalclip, 0 ,0);
	m_tilemap[0]->draw(screen, bitmap, fg_finalclip, 0 ,0);
	draw_sprites<0>(bitmap, cliprect, screen.priority());
	draw_sprites<1>(bitmap, cliprect, screen.priority());
	m_tilemap[2]->draw(screen, bitmap, tx_finalclip, 0 ,0);
	return 0;
}


INTERRUPT_GEN_MEMBER(contra_state::interrupt)
{
	if (m_k007121[0]->ctrlram_r(7) & 0x02)
		device.execute().set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

void contra_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x0f);
}

void contra_state::sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void contra_state::sirq_clear_w(uint8_t data)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void contra_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
}

void contra_state::main_map(address_map &map)
{
	map(0x0000, 0x0007).w(FUNC(contra_state::K007121_ctrl_w<0>));
	map(0x0008, 0x000f).rw("k007452", FUNC(k007452_device::read), FUNC(k007452_device::write));
	map(0x0010, 0x0010).portr("SYSTEM");
	map(0x0011, 0x0011).portr("P1");
	map(0x0012, 0x0012).portr("P2");

	map(0x0014, 0x0014).portr("DSW1");
	map(0x0015, 0x0015).portr("DSW2");
	map(0x0016, 0x0016).portr("DSW3");

	map(0x0018, 0x0018).w(FUNC(contra_state::coin_counter_w));
	map(0x001a, 0x001a).w(FUNC(contra_state::sh_irqtrigger_w));
	map(0x001c, 0x001c).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x001e, 0x001e).nopw();    // ?
	map(0x0060, 0x0067).w(FUNC(contra_state::K007121_ctrl_w<1>));

	map(0x0c00, 0x0cff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");

	map(0x1000, 0x1fff).ram();

	map(0x2000, 0x23ff).ram().w(FUNC(contra_state::cram_w<0>)).share(m_cram[0]);
	map(0x2400, 0x27ff).ram().w(FUNC(contra_state::vram_w<0>)).share(m_vram[0]);
	map(0x2800, 0x2bff).ram().w(FUNC(contra_state::cram_w<2>)).share(m_cram[2]);
	map(0x2c00, 0x2fff).ram().w(FUNC(contra_state::vram_w<2>)).share(m_vram[2]);
	map(0x3000, 0x3fff).ram().share("spriteram1");
	map(0x4000, 0x43ff).ram().w(FUNC(contra_state::cram_w<1>)).share(m_cram[1]);
	map(0x4400, 0x47ff).ram().w(FUNC(contra_state::vram_w<1>)).share(m_vram[1]);
	map(0x4800, 0x4fff).ram();
	map(0x5000, 0x5fff).ram().share("spriteram2");

	map(0x6000, 0x7fff).bankr(m_mainbank);
	map(0x7000, 0x7000).w(FUNC(contra_state::bankswitch_w));

	map(0x8000, 0xffff).rom().region("maincpu", 0x18000);
}

void contra_state::sound_map(address_map &map)
{
	map(0x0000, 0x0000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x2000, 0x2001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x4000).w(FUNC(contra_state::sirq_clear_w)); // read triggers irq reset and latch read (in the hardware only).
	map(0x6000, 0x67ff).ram();
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}



static INPUT_PORTS_START( contra )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 70000" )
	PORT_DIPSETTING(    0x10, "40000 80000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Sound" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Stereo ) )
INPUT_PORTS_END



static INPUT_PORTS_START( gryzor )
	PORT_INCLUDE( contra )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:3")    // Not Used according to manual, used in gryzor sets
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" ) PORT_DIPLOCATION("SW3:2")    // Not Used according to manual, used in gryzor sets
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_contra_1 )
	GFXDECODE_ENTRY( "k007121_1", 0, gfx_8x8x4_packed_msb,       0, 8*16 )
GFXDECODE_END

static GFXDECODE_START( gfx_contra_2 )
	GFXDECODE_ENTRY( "k007121_2", 0, gfx_8x8x4_packed_msb, 8*16*16, 8*16 )
GFXDECODE_END



void contra_state::machine_start()
{
	m_mainbank->configure_entries(0, 16, memregion("maincpu")->base(), 0x2000);
}

void contra_state::machine_reset()
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void contra_state::contra(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, XTAL(24'000'000) / 8); // (HD63C09EP)
	m_maincpu->set_addrmap(AS_PROGRAM, &contra_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(contra_state::interrupt));

	MC6809E(config, m_audiocpu, XTAL(3'579'545) / 2); // (HD68B09EP)
	m_audiocpu->set_addrmap(AS_PROGRAM, &contra_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(6000));  // enough for the sound CPU to read all commands

	KONAMI_007452_MATH(config, "k007452");

	// video hardware
	BUFFERED_SPRITERAM8(config, m_buffered_spriteram[0]);

	BUFFERED_SPRITERAM8(config, m_buffered_spriteram[1]);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(37*8, 32*8);
	m_screen->set_visarea(0*8, 35*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(contra_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(contra_state::palette));
	m_palette->set_format(palette_device::xBGR_555, 2 * 8 * 16 * 16);
	m_palette->set_indirect_entries(128);
	m_palette->set_endianness(ENDIANNESS_LITTLE);

	K007121(config, m_k007121[0], 0, m_palette, gfx_contra_1);
	K007121(config, m_k007121[1], 0, m_palette, gfx_contra_2);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "lspeaker", 0.60).add_route(1, "rspeaker", 0.60);
}


ROM_START( contra )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "633i02.17a",   0x00000, 0x10000, CRC(b2f7bd9a) SHA1(6c29568419bc49f0be3995b0c34edd9038f6f8d9) )
	ROM_LOAD( "633m03.18a",   0x10000, 0x10000, CRC(d045e1da) SHA1(ec781e98a6efb14861223250c6239b06ec98ed0b) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "633e01.12a",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "k007121_1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table

	ROM_REGION( 0x0001, "pals", 0 )
	ROM_LOAD( "007766.20d.bin", 0x0000, 0x0001, NO_DUMP ) // PAL16L8A-2CN
ROM_END

ROM_START( contra1 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "633i02.17a",   0x00000, 0x10000, CRC(b2f7bd9a) SHA1(6c29568419bc49f0be3995b0c34edd9038f6f8d9) )
	ROM_LOAD( "633i03.18a",   0x10000, 0x10000, CRC(7fc0d8cf) SHA1(cf1cf15646a4e5dc72671e957bc51ca44d30995c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "633e01.12a",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "k007121_1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table

	ROM_REGION( 0x0001, "pals", 0 )
	ROM_LOAD( "007766.20d.bin", 0x0000, 0x0001, NO_DUMP ) // PAL16L8A-2CN
ROM_END

ROM_START( contrae )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "633_e02.17a",   0x00000, 0x10000, CRC(9d5ebe66) SHA1(5218426e1494b4f6dec667f1ade7ada13aa04f2b) )
	ROM_LOAD( "633_e03.18a",   0x10000, 0x10000, CRC(7ebdb314) SHA1(b42c032cce7ae0c9b3eea6a41b7ffa5cb7fced5d) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "633e01.12a",   0x000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	// this PCB used official Konami riser-boards in place of the mask ROMs
	ROM_REGION( 0x80000, "k007121_1", 0 )
	ROM_LOAD16_BYTE( "633_e04_a.7d",    0x00000, 0x10000, CRC(e027f330) SHA1(e3480c0ed9f5ed5df829e66eb72e01ea39d5fca3) )
	ROM_LOAD16_BYTE( "633_e04_b.7d",    0x20000, 0x10000, CRC(a71230f5) SHA1(c2c92b42a04adbb4c7ba3d4632b9a9db0555840e) )
	ROM_LOAD16_BYTE( "633_e04_c.7d",    0x40000, 0x10000, CRC(0b103d01) SHA1(95e7feb7103d71b43ba921b7a376a2faf642621b) )
	ROM_LOAD16_BYTE( "633_e04_d.7d",    0x60000, 0x10000, CRC(ab3faa60) SHA1(905c1eaa5fb46904058977582c9bb3132ba165f7) )
	ROM_LOAD16_BYTE( "633_e05_a.7f",    0x00001, 0x10000, CRC(81a70a77) SHA1(c7493474af0144c05d3d98b2f77a3dae2d8d7ca5) )
	ROM_LOAD16_BYTE( "633_e05_b.7f",    0x20001, 0x10000, CRC(55556f29) SHA1(ebd52abd4adb9c4302050579fa126fde49fb468d) )
	ROM_LOAD16_BYTE( "633_e05_c.7f",    0x40001, 0x10000, CRC(acba86bf) SHA1(c248c837b1f8093bcaf465b0c75b67f1f67a3f61) )
	ROM_LOAD16_BYTE( "633_e05_d.7f",    0x60001, 0x10000, CRC(59cf234d) SHA1(4a6bb30789e581c0600c55d8e8fba778e30ba299) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	ROM_LOAD16_BYTE( "633_e06_a.16d",   0x00000, 0x10000, CRC(030079c5) SHA1(3f93e05e9df0a9dde570b771e04b719cf0ace967) )
	ROM_LOAD16_BYTE( "633_e06_b.16d",   0x20000, 0x10000, CRC(e17d5807) SHA1(15ebbf62d026cc8ac75c9877304458cbc0c5d5e0) )
	ROM_LOAD16_BYTE( "633_e06_c.16d",   0x40000, 0x10000, CRC(7d6a28cd) SHA1(9fbbe0460406bb8b3e2e572c4d5a2f8be4ba9c2e) )
	ROM_LOAD16_BYTE( "633_e06_d.16d",   0x60000, 0x10000, CRC(83db378f) SHA1(b9a2235382836fe06868d3bbd45978653bf5c91d) )
	ROM_LOAD16_BYTE( "633_e07_a.16f",   0x00001, 0x10000, CRC(8fcd40e5) SHA1(980273dc9108cc546b2146ec1e9db5dd440c4bd2) )
	ROM_LOAD16_BYTE( "633_e07_b.16f",   0x20001, 0x10000, CRC(694e306e) SHA1(f077aefb2b1ed29b27ed02b094007fe6b50a06a1) )
	ROM_LOAD16_BYTE( "633_e07_c.16f",   0x40001, 0x10000, CRC(fb33f3ff) SHA1(c6f6035a1478e8d2cdab01ddf2ec07a834b79593) )
	ROM_LOAD16_BYTE( "633_e07_d.16f",   0x60001, 0x10000, CRC(cfab0988) SHA1(3961bcbc3093b787211ab2815914f90a89df78b1) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633e10.18g",   0x0200, 0x0100, CRC(e782c494) SHA1(9459e721a4361fc4fbace3a017211f0199dee24d) )    // 007121 #1 sprite lookup table // earlier rev
	ROM_LOAD( "633e11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table

	ROM_REGION( 0x0001, "pals", 0 )
	ROM_LOAD( "007766.20d.bin", 0x0000, 0x0001, NO_DUMP ) // PAL16L8A-2CN
ROM_END

ROM_START( contraj )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "633k02.17a",   0x00000, 0x10000, CRC(5d5f7438) SHA1(489fe56ca57ef4f6a7792fba07a9656009f3f285) )
	ROM_LOAD( "633n03.18a",   0x10000, 0x10000, CRC(fedab568) SHA1(7fd4546335bdeef7f8326d4cbde7fa36d74e5cfc) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "633e01.12a",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "k007121_1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table

	ROM_REGION( 0x0001, "pals", 0 )
	ROM_LOAD( "007766.20d.bin", 0x0000, 0x0001, NO_DUMP ) // PAL16L8A-2CN
ROM_END

ROM_START( contraj1 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "633k02.17a",   0x00000, 0x10000, CRC(5d5f7438) SHA1(489fe56ca57ef4f6a7792fba07a9656009f3f285) )
	ROM_LOAD( "633k03.18a",   0x10000, 0x10000, CRC(bdb9196d) SHA1(fad170e8fda94c9c9d7b82433daa30b80af12efc) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "633e01.12a",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "k007121_1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table

	ROM_REGION( 0x0001, "pals", 0 )
	ROM_LOAD( "007766.20d.bin", 0x0000, 0x0001, NO_DUMP ) // PAL16L8A-2CN
ROM_END

ROM_START( gryzor )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "633j02.17a",   0x00000, 0x10000, CRC(b5922f9a) SHA1(441a23dc99a908ec2c09c855e73070dbab8c5ae2) )
	ROM_LOAD( "633j03.18a",   0x10000, 0x10000, CRC(20919162) SHA1(2f375166428ee03f6e8ac0372a373bb8ab35e64c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "633e01.12a",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "k007121_1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table

	ROM_REGION( 0x0001, "pals", 0 )
	ROM_LOAD( "007766.20d.bin", 0x0000, 0x0001, NO_DUMP ) // PAL16L8A-2CN
ROM_END

ROM_START( gryzor1 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "633g3.17a",    0x00000, 0x10000, CRC(bbd9e95e) SHA1(fd5de1bcc485de7b8fc2e321351c2e3ddd25d053) )
	ROM_LOAD( "633g2.18a",    0x10000, 0x10000, CRC(92ca77bd) SHA1(3a56f51a617edff9f2a60df0141dff040881b82a) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "633e01.12a",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) )

	ROM_REGION( 0x80000, "k007121_1", 0 )
	ROM_LOAD16_BYTE( "633e04.7d",    0x00000, 0x40000, CRC(14ddc542) SHA1(c7d8592672a6e50c2fe6b0670001c340022f16f9) )
	ROM_LOAD16_BYTE( "633e05.7f",    0x00001, 0x40000, CRC(42185044) SHA1(a6e2598d766e6995c1a912e4a04987e6f4d547ff) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	ROM_LOAD16_BYTE( "633e06.16d",   0x00000, 0x40000, CRC(9cf6faae) SHA1(9ab79c06cb541ce6fdac322886b8a14a2f3f5cf7) )
	ROM_LOAD16_BYTE( "633e07.16f",   0x00001, 0x40000, CRC(f2d06638) SHA1(0fa0fbfc53ab5c31b9de22f90153d9af37ff22ce) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table

	ROM_REGION( 0x0001, "pals", 0 )
	ROM_LOAD( "007766.20d.bin", 0x0000, 0x0001, NO_DUMP ) // PAL16L8A-2CN
ROM_END

//  Bootlegs

ROM_START( contrab )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1.ic19",   0x00000, 0x10000, CRC(b2f7bd9a) SHA1(6c29568419bc49f0be3995b0c34edd9038f6f8d9) ) // == 633i03.18a
	ROM_LOAD( "3.ic20",   0x10000, 0x10000, CRC(d045e1da) SHA1(ec781e98a6efb14861223250c6239b06ec98ed0b) ) // == 633m03.18a

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "3.ic63",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) ) // == 633k01.12a

	ROM_REGION( 0x80000, "k007121_1", 0 )
	// bootleg versions use smaller gfx ROMs, but the data is the same
	ROM_LOAD( "7.rom",      0x00000, 0x10000, CRC(57f467d2) SHA1(e30be315980f421143d1357174af678362836285) )
	ROM_LOAD( "10.rom",     0x10000, 0x10000, CRC(e6db9685) SHA1(4d5ccfe95b082fe9830e7a316f88fd6f02464900) )
	ROM_LOAD( "9.rom",      0x20000, 0x10000, CRC(875c61de) SHA1(e8dc42fef810a9f5471d96cb5297eb29296ba472) )
	ROM_LOAD( "8.rom",      0x30000, 0x10000, CRC(642765d6) SHA1(d1563a392b8d8409f0f2159c2e82cd34b9ca2900) )
	ROM_LOAD( "15.rom",     0x40000, 0x10000, CRC(daa2324b) SHA1(8a5fb8b79957291dc952e19e6973c64bb7230816) )
	ROM_LOAD( "16.rom",     0x50000, 0x10000, CRC(e27cc835) SHA1(cb980b1fed110c7e4ef21fa11f44e5aea100881b) )
	ROM_LOAD( "17.rom",     0x60000, 0x10000, CRC(ce4330b9) SHA1(0a2bd31baa0bc5e3745ee5ddac995557a551d58c) )
	ROM_LOAD( "18.rom",     0x70000, 0x10000, CRC(1571ce42) SHA1(04082ed78b5e7f20b99d6edfb6c363574abd6158) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	// bootleg versions use smaller gfx ROMs, but the data is the same
	ROM_LOAD( "4.rom",      0x00000, 0x10000, CRC(2cc7e52c) SHA1(7598a63346bf06dd34fd643fdff53fc3de6768a6) )
	ROM_LOAD( "5.rom",      0x10000, 0x10000, CRC(e01a5b9c) SHA1(58c99cf99f209c584da757320a2f107244056d4c) )
	ROM_LOAD( "6.rom",      0x20000, 0x10000, CRC(aeea6744) SHA1(220b42f707db99967bdcbd9ac66fcc83675a72aa) )
	ROM_LOAD( "14.rom",     0x30000, 0x10000, CRC(765afdc7) SHA1(b7f6871cb154ee7e42e683bce08b73b00e61b0bc) )
	ROM_LOAD( "11.rom",     0x40000, 0x10000, CRC(bd9ba92c) SHA1(e7f65ed20cd7754cc476e8fab7e56105cedcdb98) )
	ROM_LOAD( "12.rom",     0x50000, 0x10000, CRC(d0be7ec2) SHA1(5aa829b8ffbe3f5f92ba672b1c24bfb7836ba1a3) )
	ROM_LOAD( "13.rom",     0x60000, 0x10000, CRC(2b513d12) SHA1(152ebd849751cc2e95513134ce773a6b2eeb320e) )
	// This last section, 0x70000-0x7ffff is empty

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table
	ROM_LOAD( "conprom.53",   0x0400, 0x0100, CRC(05a1da7e) SHA1(ec0bdfc9da05c99e6a283014769db6d641f1a0aa) )    // unknown (only present in this bootleg)
ROM_END

ROM_START( contrabj )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1.2h",    0x00000, 0x10000, CRC(5d5f7438) SHA1(489fe56ca57ef4f6a7792fba07a9656009f3f285) ) // == 633k02.17a
	ROM_LOAD( "2.2k",    0x10000, 0x10000, CRC(fedab568) SHA1(7fd4546335bdeef7f8326d4cbde7fa36d74e5cfc) ) // == 633n03.18a

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "a3.4p",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) ) // == 633k01.12a

	ROM_REGION( 0x80000, "k007121_1", 0 )
	// bootleg versions use smaller gfx ROMs, but the data is the same
	ROM_LOAD( "a7.14f",      0x00000, 0x10000, CRC(57f467d2) SHA1(e30be315980f421143d1357174af678362836285) )
	ROM_LOAD( "a10.14l",     0x10000, 0x10000, CRC(e6db9685) SHA1(4d5ccfe95b082fe9830e7a316f88fd6f02464900) )
	ROM_LOAD( "a9.14k",      0x20000, 0x10000, CRC(875c61de) SHA1(e8dc42fef810a9f5471d96cb5297eb29296ba472) )
	ROM_LOAD( "a8.14h",      0x30000, 0x10000, CRC(642765d6) SHA1(d1563a392b8d8409f0f2159c2e82cd34b9ca2900) )
	ROM_LOAD( "a15.14r",     0x40000, 0x10000, CRC(daa2324b) SHA1(8a5fb8b79957291dc952e19e6973c64bb7230816) )
	ROM_LOAD( "a16.14t",     0x50000, 0x10000, CRC(e27cc835) SHA1(cb980b1fed110c7e4ef21fa11f44e5aea100881b) )
	ROM_LOAD( "a17.14v",     0x60000, 0x10000, CRC(ce4330b9) SHA1(0a2bd31baa0bc5e3745ee5ddac995557a551d58c) )
	ROM_LOAD( "a18.14w",     0x70000, 0x10000, CRC(1571ce42) SHA1(04082ed78b5e7f20b99d6edfb6c363574abd6158) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	// bootleg versions use smaller gfx ROMs, but the data is the same
	ROM_LOAD( "a4.14a",      0x00000, 0x10000, CRC(2cc7e52c) SHA1(7598a63346bf06dd34fd643fdff53fc3de6768a6) )
	ROM_LOAD( "a5.14c",      0x10000, 0x10000, CRC(e01a5b9c) SHA1(58c99cf99f209c584da757320a2f107244056d4c) )
	ROM_LOAD( "e6.14d",      0x20000, 0x10000, CRC(aeea6744) SHA1(220b42f707db99967bdcbd9ac66fcc83675a72aa) ) // Yes, this one was labeled "E" and not "A"
	ROM_LOAD( "a14.14q",     0x30000, 0x10000, CRC(765afdc7) SHA1(b7f6871cb154ee7e42e683bce08b73b00e61b0bc) )
	ROM_LOAD( "a11.14m",     0x40000, 0x10000, CRC(bd9ba92c) SHA1(e7f65ed20cd7754cc476e8fab7e56105cedcdb98) )
	ROM_LOAD( "a12.14n",     0x50000, 0x10000, CRC(d0be7ec2) SHA1(5aa829b8ffbe3f5f92ba672b1c24bfb7836ba1a3) )
	ROM_LOAD( "a13.14p",     0x60000, 0x10000, CRC(2b513d12) SHA1(152ebd849751cc2e95513134ce773a6b2eeb320e) )
	// This last section, 0x70000-0x7ffff is empty


	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table
ROM_END

ROM_START( contrabj1 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1.2h",              0x00000, 0x10000, CRC(5d5f7438) SHA1(489fe56ca57ef4f6a7792fba07a9656009f3f285) ) // == 633k02.17a
	ROM_LOAD( "2__,contrabtj2.2k", 0x10000, 0x10000, CRC(bdb9196d) SHA1(fad170e8fda94c9c9d7b82433daa30b80af12efc) ) // == 633k03.18a

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "a3.4p",   0x0000, 0x8000, CRC(d1549255) SHA1(d700c7de36746ba247e3a5d0410b7aa036aa4073) ) // == 633k01.12a

	ROM_REGION( 0x80000, "k007121_1", 0 )
	// bootleg versions use smaller gfx ROMs, but the data is the same
	ROM_LOAD( "a7.14f",      0x00000, 0x10000, CRC(57f467d2) SHA1(e30be315980f421143d1357174af678362836285) )
	ROM_LOAD( "a10.14l",     0x10000, 0x10000, CRC(e6db9685) SHA1(4d5ccfe95b082fe9830e7a316f88fd6f02464900) )
	ROM_LOAD( "a9.14k",      0x20000, 0x10000, CRC(875c61de) SHA1(e8dc42fef810a9f5471d96cb5297eb29296ba472) )
	ROM_LOAD( "a8.14h",      0x30000, 0x10000, CRC(642765d6) SHA1(d1563a392b8d8409f0f2159c2e82cd34b9ca2900) )
	ROM_LOAD( "a15.14r",     0x40000, 0x10000, CRC(daa2324b) SHA1(8a5fb8b79957291dc952e19e6973c64bb7230816) )
	ROM_LOAD( "a16.14t",     0x50000, 0x10000, CRC(e27cc835) SHA1(cb980b1fed110c7e4ef21fa11f44e5aea100881b) )
	ROM_LOAD( "a17.14v",     0x60000, 0x10000, CRC(ce4330b9) SHA1(0a2bd31baa0bc5e3745ee5ddac995557a551d58c) )
	ROM_LOAD( "a18.14w",     0x70000, 0x10000, CRC(1571ce42) SHA1(04082ed78b5e7f20b99d6edfb6c363574abd6158) )

	ROM_REGION( 0x80000, "k007121_2", 0 )
	// bootleg versions use smaller gfx ROMs, but the data is the same
	ROM_LOAD( "a4.14a",      0x00000, 0x10000, CRC(2cc7e52c) SHA1(7598a63346bf06dd34fd643fdff53fc3de6768a6) )
	ROM_LOAD( "a5.14c",      0x10000, 0x10000, CRC(e01a5b9c) SHA1(58c99cf99f209c584da757320a2f107244056d4c) )
	ROM_LOAD( "e6.14d",      0x20000, 0x10000, CRC(aeea6744) SHA1(220b42f707db99967bdcbd9ac66fcc83675a72aa) ) // Yes, this one was labeled "E" and not "A"
	ROM_LOAD( "a14.14q",     0x30000, 0x10000, CRC(765afdc7) SHA1(b7f6871cb154ee7e42e683bce08b73b00e61b0bc) )
	ROM_LOAD( "a11.14m",     0x40000, 0x10000, CRC(bd9ba92c) SHA1(e7f65ed20cd7754cc476e8fab7e56105cedcdb98) )
	ROM_LOAD( "a12.14n",     0x50000, 0x10000, CRC(d0be7ec2) SHA1(5aa829b8ffbe3f5f92ba672b1c24bfb7836ba1a3) )
	ROM_LOAD( "a13.14p",     0x60000, 0x10000, CRC(2b513d12) SHA1(152ebd849751cc2e95513134ce773a6b2eeb320e) )
	// This last section, 0x70000-0x7ffff is empty


	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "633e08.10g",   0x0000, 0x0100, CRC(9f0949fa) SHA1(7c8fefdcae4523d008a7d39062194c7a80aa3500) )    // 007121 #0 sprite lookup table
	ROM_LOAD( "633e09.12g",   0x0100, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #0 char lookup table
	ROM_LOAD( "633f10.18g",   0x0200, 0x0100, CRC(2b244d84) SHA1(c3bde7afb501bae58d07721c637dc06938c22150) )    // 007121 #1 sprite lookup table
	ROM_LOAD( "633f11.20g",   0x0300, 0x0100, CRC(14ca5e19) SHA1(eeee2f8b3d1e4acf47de1e74c4e507ff924591e7) )    // 007121 #1 char lookup table
ROM_END

} // anonymous namespace


GAME( 1987, contra,    0,      contra, contra, contra_state, empty_init, ROT90, "Konami",  "Contra (US / Asia, set 1)",     MACHINE_SUPPORTS_SAVE )
GAME( 1987, contra1,   contra, contra, contra, contra_state, empty_init, ROT90, "Konami",  "Contra (US / Asia, set 2)",     MACHINE_SUPPORTS_SAVE )
GAME( 1987, contrae,   contra, contra, contra, contra_state, empty_init, ROT90, "Konami",  "Contra (US / Asia, set 3)",     MACHINE_SUPPORTS_SAVE )
GAME( 1987, contraj,   contra, contra, contra, contra_state, empty_init, ROT90, "Konami",  "Contra (Japan, set 1)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, contraj1,  contra, contra, contra, contra_state, empty_init, ROT90, "Konami",  "Contra (Japan, set 2)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, gryzor,    contra, contra, gryzor, contra_state, empty_init, ROT90, "Konami",  "Gryzor (set 1)",                MACHINE_SUPPORTS_SAVE )
GAME( 1987, gryzor1,   contra, contra, gryzor, contra_state, empty_init, ROT90, "Konami",  "Gryzor (set 2)",                MACHINE_SUPPORTS_SAVE )
GAME( 1987, contrab,   contra, contra, contra, contra_state, empty_init, ROT90, "bootleg", "Contra (bootleg)",              MACHINE_SUPPORTS_SAVE )
GAME( 1987, contrabj,  contra, contra, contra, contra_state, empty_init, ROT90, "bootleg", "Contra (Japan bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, contrabj1, contra, contra, contra, contra_state, empty_init, ROT90, "bootleg", "Contra (Japan bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
