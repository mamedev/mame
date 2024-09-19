// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino
/***************************************************************************

Solar-Warrior / Xain'd Sleena (Technos, 1986)
Hardware info by Guru

PCB Layout
----------

Top Board

TA-0019-P1-03
|------------------------------------------------------------------------|
|M51516  558   YM3014 YM2203 68A09 P2-0.IC49                             |
|  VOL   558   YM3014 YM2203                                             |
|        558                       6116                                 |--|
|                                                                       |  |
|        2018(1)                   *                                    |  |
|        6148                                                           |  |
|                           PT-0.IC59                                   |  |
|                                  P3-0.IC46                            |  |
|                                                                       |  |
|J                                                                      |  |
|A     SW2                         P4-0.IC45                            |--|
|M                                                     2018              |
|M                                                                       |
|A                                 P5-0.IC44                             |
|      SW1                                                               |
|                                                                       |--|
|                                  P6-0.IC43                            |  |
|            68B09(1)                                                   |  |
|                                              68B09(2)                 |  |
|                   P9-02.IC66     P7-0.IC42                            |  |
|                                                                       |  |
|                                                                       |  |
| 68705             PA-03.IC65     P8-0.IC41                            |  |
| (PZ-0.IC113)                             P1-02.IC29  P0-02.IC15  6264 |--|
|                                                                        |
|                         T518A    *                                     |
|------------------------------------------------------------------------|
Notes:
       68A09 - Hitachi HD68A09P CPU. Clock 1.500MHz [12/8] (sound CPU)          \
    68B09(1) - Hitachi HD68B09EP CPU. Clock 1.500MHz [12/8] (main program CPU)   | Note: Despite the CPUs being A and B types
    68B09(2) - Hitachi HD68B09EP CPU. Clock 1.500MHz [12/8] (sub program CPU)   /        they ALL run at 1.5MHz
      YM2203 - Yamaha YM2203 FM Operator Type-N(OPN) sound chip. Clock 3.000MHz [12/4]
      YM3014 - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 1.0000MHz [12/4/3]
       68705 - Motorola MC68705P5S Micro-Controller. Clock 3.000MHz [12/4] (labelled 'PZ-0')
           * - Empty sockets
        6264 - Hitachi HM6264LP-15 8kBx8-bit SRAM (main+sub program RAM)
        2018 - Toshiba TMM2018D-45 2kBx8-bit SRAM (tile RAM)
     2018(1) - Toshiba TMM2018D-45 2kBx8-bit SRAM  \ (color RAM)
        6148 - Hitachi HM6148HP-45 1kBx4-bit SRAM  /
        6116 - Hitachi HM6116LP-3 2kBx8-bit SRAM (DIP24) (sound program RAM)
      M51516 - Mitsubishi M51516L 12W BTL Audio Power Amplifier
       T518A - Mitsumi Electric PST518A System Reset IC with low voltage detection reset 4.2V
         558 - Hitachi HA17558 Dual Operational Amplifier
       SW1/2 - 8-position DIP switch
       HSync - 15.6165kHz \
       VSync - 57.4138Hz  / measured on bottom board on logic near 12MHz xtal

        ROMs - Name           Device       Use
               P9-02.IC66     TMM24256   \ CPU1 program
               PA-03.IC65     MBM27256   /

               P0-02.IC15     TMM24256   \ CPU2 program
               P1-02.IC29     TMM24256   /

               P2-0.IC49      TMM24256     Sound CPU program

               P3-0.IC46      TMM24256   / Tiles
               P4-0.IC45      TMM24256   |
               P5-0.IC44      TMM24256   |
               P6-0.IC43      TMM24256   |
               P7-0.IC42      TMM24256   |
               P8-0.IC41      TMM24256   /

               PT-0.IC59      Fujitsu MB7114E 256x4-bit PROM used for priority

Bottom Board

TA-0019-P2-03
|------------------------------------------------------------------------|
|    PK-0.IC136  PC-0.IC114                                              |
|    PL-0.IC135  PD-0.IC113                                              |
|                                                                       |--|
|    PM-0.IC134  PE-0.IC112                                             |  |
|    PN-02.IC133 PF-02.IC111                                            |  |
|                                                                       |  |
|           2018(1)                                                     |  |
|                                                                       |  |
|    PO-0.IC131  PG-0.IC109                                             |  |
|                                                                       |  |
|    PP-0.IC130  PH-0.IC108                                             |--|
|                                                                  12MHz |
|    PQ-0.IC129  PI-0.IC107                            PB-0.IC24         |
|                                                                        |
|    PR-0.IC128  PJ-0.IC106                            6116              |
|                                                                       |--|
|         2018                                                          |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|         2018                                                   2018   |  |
|                                                                       |--|
|             2018                                                       |
|             2018                                                       |
|------------------------------------------------------------------------|
Notes:
        2018 - Toshiba TMM2018D-45 2kBx8-bit SRAM (sprite RAM)
     2018(1) - Toshiba TMM2018D-45 2kBx8-bit SRAM (tile RAM)
        6116 - Hitachi HM6116 2kBx8-bit SRAM (character / text layer RAM)

        ROMs - Name           Device       Use
               PB-0.IC24      TMM24256     Characters / text layer

               PC-0.IC114     TMM24256   \
               PD-0.IC113     TMM24256   |
               PE-0.IC112     TMM24256   |
               PF-02.IC111    MBM27256   | Tiles (PN-02 and PF-02 are unique to Solar-Warrior)
               PK-0.IC136     TMM24256   |
               PL-0.IC135     TMM24256   |
               PM-0.IC134     TMM24256   |
               PN-02.IC133    MBM27256   /

               PG-0.IC109     TMM24256   \
               PH-0.IC108     TMM24256   |
               PI-0.IC107     TMM24256   |
               PJ-0.IC106     TMM24256   |
               PO-0.IC131     TMM24256   | Sprites
               PP-0.IC130     TMM24256   |
               PQ-0.IC129     TMM24256   |
               PR-0.IC128     TMM24256   /

***************************************************************************


Driver notes:

Driver by Carlos A. Lozano & Rob Rosenbrock & Phil Stroffolino
Updates by Bryan McPhail, 12/12/2004:
    Fixed NMI & FIRQ handling according to schematics.
    Fixed clock speeds.
    Implemented GFX priority register/priority PROM

    Xain has a semi-bug that shows up in MAME - at 0xa26b there is a tight
    loop that checks for the VBLANK input bit going high.  However at the
    start of a game the VBLANK interrupt routine doesn't return before
    the VBLANK input bit goes low (VBLANK is held high for 8 scanlines only).
    This would cause the emulation to hang, but it would work on the real
    board because the instruction currently being decoded would finish
    before the NMI was taken, so the VBLANK bit and NMI are not actually
    exactly synchronised in practice.  This is currently hacked in MAME
    by raising the VBLANK bit a scanline early.

***************************************************************************/

#include "emu.h"
#include "xain.h"

#include "cpu/m6809/m6809.h"
#include "sound/ymopn.h"
#include "speaker.h"


static constexpr XTAL MASTER_CLOCK(12_MHz_XTAL);
static constexpr XTAL CPU_CLOCK(MASTER_CLOCK / 8);
static constexpr XTAL MCU_CLOCK(MASTER_CLOCK / 4);
static constexpr XTAL PIXEL_CLOCK(MASTER_CLOCK / 2);


/*
    Based on the Solar Warrior schematics, vertical timing counts as follows:

        08,09,0A,0B,...,FC,FD,FE,FF,E8,E9,EA,EB,...,FC,FD,FE,FF,
        08,09,....

    Thus, it counts from 08 to FF, then resets to E8 and counts to FF again.
    This gives (256 - 8) + (256 - 232) = 248 + 24 = 272 total scanlines.

    VBLK is signalled starting when the counter hits F8, and continues through
    the reset to E8 and through until the next reset to 08 again.

    Since MAME's video timing is 0-based, we need to convert this.
*/

inline int xain_state::scanline_to_vcount(int scanline)
{
	int const vcount = scanline + 8;
	if (vcount < 0x100)
		return vcount;
	else
		return (vcount - 0x18) | 0x100;
}

TIMER_DEVICE_CALLBACK_MEMBER(xain_state::scanline)
{
	int const scanline = param;
	int const screen_height = m_screen->height();
	int const vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int const vcount = scanline_to_vcount(scanline);

	// update to the current point
	if (scanline > 0)
	{
		m_screen->update_partial(scanline - 1);
	}

	// FIRQ (IMS) fires every on every 8th scanline (except 0)
	if (!(vcount_old & 8) && (vcount & 8))
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);

	// NMI fires on scanline 248 (VBL) and is latched
	if (vcount == 0xf8)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// VBLANK input bit is held high from scanlines 248-255
	if (vcount >= 248-1)    // -1 is a hack - see notes above
		m_vblank = 1;
	else
		m_vblank = 0;
}

void xain_state::cpuA_bankswitch_w(uint8_t data)
{
	m_pri = data & 0x7;
	m_rom_banks[0]->set_entry((data >> 3) & 1);
}

void xain_state::cpuB_bankswitch_w(uint8_t data)
{
	m_rom_banks[1]->set_entry(data & 1);
}

void xain_state::main_irq_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: /* 0x3a09 - NMI clear */
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		break;
	case 1: /* 0x3a0a - FIRQ clear */
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
		break;
	case 2: /* 0x3a0b - IRQ clear */
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
		break;
	case 3: /* 0x3a0c - IRQB assert */
		m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		break;
	}
}

void xain_state::irqA_assert_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void xain_state::irqB_clear_w(uint8_t data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

int xain_state::vblank_r()
{
	return m_vblank;
}


/***************************************************************************

    MC68705P5 I/O

***************************************************************************/

ioport_value xain_state::mcu_status_r()
{
	// bit 0 is host MCU flag, bit 1 is host semaphore flag (both active low)
	return
			((m_mcu && (CLEAR_LINE != m_mcu->mcu_semaphore_r())) ? 0x00 : 0x01) |
			((m_mcu && (CLEAR_LINE != m_mcu->host_semaphore_r())) ? 0x00 : 0x02);
}

uint8_t xain_state::mcu_comm_reset_r()
{
	if (m_mcu.found() && !machine().side_effects_disabled())
	{
		m_mcu->reset_w(ASSERT_LINE);
		m_mcu->reset_w(CLEAR_LINE);
	}
	return 0xff;
}


/***************************************************************************

  Memory handlers

***************************************************************************/

template <unsigned N> void xain_state::bgram_w(offs_t offset, uint8_t data)
{
	m_bgram[N][offset] = data;
	m_bg_tilemaps[N]->mark_tile_dirty(offset & 0x3ff);
}

template <unsigned N> void xain_state::scrollx_w(offs_t offset, uint8_t data)
{
	m_scrollx[N][offset] = data;
	m_bg_tilemaps[N]->set_scrollx(0, m_scrollx[N][0] | (m_scrollx[N][1] << 8));
}

template <unsigned N> void xain_state::scrolly_w(offs_t offset, uint8_t data)
{
	m_scrolly[N][offset] = data;
	m_bg_tilemaps[N]->set_scrolly(0, m_scrolly[N][0] | (m_scrolly[N][1] << 8));
}


void xain_state::bootleg_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("share1");
	map(0x2000, 0x27ff).ram().w(FUNC(xain_state::charram_w)).share(m_charram);
	map(0x2800, 0x2fff).ram().w(FUNC(xain_state::bgram_w<1>)).share(m_bgram[1]);
	map(0x3000, 0x37ff).ram().w(FUNC(xain_state::bgram_w<0>)).share(m_bgram[0]);
	map(0x3800, 0x397f).ram().share(m_spriteram);
	map(0x3a00, 0x3a00).portr("P1");
	map(0x3a00, 0x3a01).w(FUNC(xain_state::scrollx_w<1>));
	map(0x3a01, 0x3a01).portr("P2");
	map(0x3a02, 0x3a02).portr("DSW0");
	map(0x3a02, 0x3a03).w(FUNC(xain_state::scrolly_w<1>));
	map(0x3a03, 0x3a03).portr("DSW1");
	map(0x3a04, 0x3a05).w(FUNC(xain_state::scrollx_w<0>));
	map(0x3a05, 0x3a05).portr("VBLANK");
	map(0x3a06, 0x3a07).w(FUNC(xain_state::scrolly_w<0>));
	map(0x3a08, 0x3a08).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x3a09, 0x3a0c).w(FUNC(xain_state::main_irq_w));
	map(0x3a0d, 0x3a0d).w(FUNC(xain_state::flipscreen_w));
	map(0x3a0f, 0x3a0f).w(FUNC(xain_state::cpuA_bankswitch_w));
	map(0x3c00, 0x3dff).w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x3e00, 0x3fff).w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x4000, 0x7fff).bankr(m_rom_banks[0]);
	map(0x8000, 0xffff).rom();
}

void xain_state::main_map(address_map &map)
{
	bootleg_map(map);
	map(0x3a04, 0x3a04).r(m_mcu, FUNC(taito68705_mcu_device::data_r));
	map(0x3a06, 0x3a06).r(FUNC(xain_state::mcu_comm_reset_r));
	map(0x3a0e, 0x3a0e).w(m_mcu, FUNC(taito68705_mcu_device::data_w));
}

void xain_state::cpu_map_B(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("share1");
	map(0x2000, 0x2000).w(FUNC(xain_state::irqA_assert_w));
	map(0x2800, 0x2800).w(FUNC(xain_state::irqB_clear_w));
	map(0x3000, 0x3000).w(FUNC(xain_state::cpuB_bankswitch_w));
	map(0x4000, 0x7fff).bankr(m_rom_banks[1]);
	map(0x8000, 0xffff).rom();
}

void xain_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x1000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x2800, 0x2801).w("ym1", FUNC(ym2203_device::write));
	map(0x3000, 0x3001).w("ym2", FUNC(ym2203_device::write));
	map(0x4000, 0xffff).rom();
}


static INPUT_PORTS_START( xsleena )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "Slow" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "20k 70k and every 70k" )
	PORT_DIPSETTING(    0x20, "30k 80k and every 80k" )
	PORT_DIPSETTING(    0x10, "20k and 80k" )
	PORT_DIPSETTING(    0x00, "30k and 80k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "6")
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")

	PORT_START("VBLANK")
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(xain_state, mcu_status_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xain_state, vblank_r)   // VBLANK
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ STEP2(0*8+1,-1), STEP2(8*8+1,-1), STEP2(16*8+1,-1), STEP2(24*8+1,-1) },
	{ STEP8(0,8) },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(0*8+3,-1), STEP4(16*8+3,-1), STEP4(32*8+3,-1), STEP4(48*8+3,-1) },
	{ STEP16(0,8) },
	64*8
};

static GFXDECODE_START( gfx_xain )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 8 )    /* 8x8 text */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 256, 8 )    /* 16x16 Background */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 384, 8 )    /* 16x16 Background */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 128, 8 )    /* Sprites */
GFXDECODE_END


void xain_state::machine_start()
{
	m_rom_banks[0]->configure_entries(0, 2, memregion("maincpu")->base() + 0x4000, 0xc000);
	m_rom_banks[1]->configure_entries(0, 2, memregion("sub")->base()  + 0x4000, 0xc000);
	m_rom_banks[0]->set_entry(0);
	m_rom_banks[1]->set_entry(0);

	save_item(NAME(m_vblank));
}

void xain_state::xsleena(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, CPU_CLOCK); // 68B09E
	m_maincpu->set_addrmap(AS_PROGRAM, &xain_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(xain_state::scanline), "screen", 0, 1);

	MC6809E(config, m_subcpu, CPU_CLOCK); // 68B09E
	m_subcpu->set_addrmap(AS_PROGRAM, &xain_state::cpu_map_B);

	MC6809(config, m_audiocpu, PIXEL_CLOCK); // 68A09
	m_audiocpu->set_addrmap(AS_PROGRAM, &xain_state::sound_map);

	TAITO68705_MCU(config, m_mcu, MCU_CLOCK);

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, 384, 0, 256, 272, 8, 248);   // based on ddragon driver
	m_screen->set_screen_update(FUNC(xain_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_xain);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 512);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set_inputline(m_audiocpu, M6809_IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", MCU_CLOCK));
	ym1.irq_handler().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	ym1.add_route(0, "mono", 0.50);
	ym1.add_route(1, "mono", 0.50);
	ym1.add_route(2, "mono", 0.50);
	ym1.add_route(3, "mono", 0.40);

	ym2203_device &ym2(YM2203(config, "ym2", MCU_CLOCK));
	ym2.add_route(0, "mono", 0.50);
	ym2.add_route(1, "mono", 0.50);
	ym2.add_route(2, "mono", 0.50);
	ym2.add_route(3, "mono", 0.40);
}


void xain_state::xsleenab(machine_config &config)
{
	xsleena(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &xain_state::bootleg_map);

	config.device_remove("mcu");
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( xsleena )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-08.ic66",   0x08000, 0x8000, CRC(5179ae3f) SHA1(9e4e2825e56b090aa759b0da39ccb17ccd77ede2) )
	ROM_LOAD( "pa-09.ic65",   0x04000, 0x4000, CRC(10a7c800) SHA1(f19201fe1414faed649b8e49416025aae44bcb6c) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x800, "mcu:mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-0.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( xsleenaj )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-01.ic66",   0x08000, 0x8000, CRC(370164be) SHA1(65c9951cac7dc3943fa4d5f9919ebb4c4f29b3ae) ) /* the '1' on the label was ink stamped */
	ROM_LOAD( "pa-0.ic65",    0x04000, 0x4000, CRC(d22bf859) SHA1(9edb159bef2eba2c5d93c03c15fbcb87eea52236) ) /* Need to verify proper rom label */
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x800, "mcu:mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-0.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( solrwarr )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-02.ic66",   0x08000, 0x8000, CRC(8ff372a8) SHA1(0fc396e662419fb9cb5bea11748aa8e0e8d072e6) )
	ROM_LOAD( "pa-03.ic65",   0x04000, 0x4000, CRC(154f946f) SHA1(25b776eb9c494e5302795ae79e494cbfc7c104b1) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-02.ic29",   0x08000, 0x8000, CRC(f5f235a3) SHA1(9f57dd7c5e514afa750edc6da6d263bf1e913c14) )
	ROM_LOAD( "p0-02.ic133",  0x04000, 0x4000, CRC(51ae95ae) SHA1(e03f7ccb0b33b05547577c60a7f92dc75e24b4d6) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x800, "mcu:mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-0.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-02.ic133",  0x18000, 0x8000, CRC(d2ed6f94) SHA1(155a0d1d978f07517400d0c602fc40657f8569dc) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-02.ic111",  0x38000, 0x8000, CRC(6e627a77) SHA1(1d16031acd53c9e691ae7eac8a6f1ae3954fac8c) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( xsleenab )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.rom",        0x08000, 0x8000, CRC(79f515a7) SHA1(e61f18e3639dd9afe16c7bcb90fa7be31905e2c6) )
	ROM_LOAD( "pa-0.ic65",    0x04000, 0x4000, CRC(d22bf859) SHA1(9edb159bef2eba2c5d93c03c15fbcb87eea52236) ) /* Need to verify proper rom label */
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-0.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( xsleenaba )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "xs87b-10.7d",    0x08000, 0x8000, CRC(3d5f9fb4) SHA1(d315b5415a471e05ee61b84fcaf739c75f890061) )
	ROM_LOAD( "xs87b-11.7c",    0x04000, 0x4000, CRC(81c80d54) SHA1(2049c5843c6134560d8fbacf020cbff34833ded7) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-0.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

ROM_START( xsleenabb ) // This set runs on very accurate reproductions of the original PCBs. The code seems to have been modified the bare minimum to run without MCU
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "ic66",    0x08000, 0x8000, CRC(f91cae92) SHA1(0cc85eb136584f870b466753cdb65e759360bc15) ) // only ROM that differs: ic66  p9-01.ic66  99.954224%
	ROM_LOAD( "ic65",    0x04000, 0x4000, CRC(d22bf859) SHA1(9edb159bef2eba2c5d93c03c15fbcb87eea52236) )
	ROM_CONTINUE(        0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END


GAME( 1986, xsleena,   0,       xsleena,  xsleena, xain_state, empty_init, ROT0, "Technos Japan (Taito license)",            "Xain'd Sleena (World)",             MACHINE_SUPPORTS_SAVE )
GAME( 1986, xsleenaj,  xsleena, xsleena,  xsleena, xain_state, empty_init, ROT0, "Technos Japan",                            "Xain'd Sleena (Japan)",             MACHINE_SUPPORTS_SAVE )
GAME( 1986, solrwarr,  xsleena, xsleena,  xsleena, xain_state, empty_init, ROT0, "Technos Japan (Taito / Memetron license)", "Solar-Warrior (US)",                MACHINE_SUPPORTS_SAVE )
GAME( 1986, xsleenab,  xsleena, xsleenab, xsleena, xain_state, empty_init, ROT0, "bootleg",                                  "Xain'd Sleena (bootleg, set 1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1987, xsleenaba, xsleena, xsleenab, xsleena, xain_state, empty_init, ROT0, "bootleg",                                  "Xain'd Sleena (bootleg, bugfixed)", MACHINE_SUPPORTS_SAVE ) // newer bootleg, fixes some of the issues with the other one
GAME( 1987, xsleenabb, xsleena, xsleenab, xsleena, xain_state, empty_init, ROT0, "bootleg",                                  "Xain'd Sleena (bootleg, set 2)",    MACHINE_SUPPORTS_SAVE )
