// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
/***************************************************************************

    Apache 3                                            ATF-011
    Round Up 5                                          ATC-011
    Cycle Warriors                                      ABA-011
    Big Fight                                           ABA-011

    Incredibly complex hardware!  These are all different boards, but share
    a similar sprite chip (TZB215 on Apache 3, TZB315 on others).  Other
    graphics (road, sky, bg/fg layers) all differ between games.

    TODO:
    - Sprite rotation
    - Round Up 5: Finish road layer.
      Tunnel sections are borderline unplayable, plus slopes are ugly to watch.
    - Apache 3: road layer, has twelve rotation registers!
    - (fixed) Cycle Warriors: transparent road layer on sidelines, wrong mask_data?
    - (fixed) Missing BG layer (Round Up 5) - banked VRAM data from somewhere!?
    - Round Up 5: always boots with a coin inserted
      $5152 is the coin counter, gets an explicit 1 at boot.
      There are other two buffers read from 68k before that, written to $5156 and $515a
      If these are 0xffff by then game boots normally ...
    - (fixed) Round Up 5 doesn't survive a reset
    - (fixed?) Cycle Warriors: test mode text does not appear as it needs a -256 Y
      scroll offset from somewhere.
    - (fixed) Cycle Warriors: sometimes it draws garbage on character select or even hangs
      depending on where player coins up, most likely caused by miscommunication with sub CPU?
    - (fixed) Cycle Warriors: ranking screen is completely wrong;
    - (fixed) Cycle Warriors: ugly orange color on character select and briefing screens, layer disable?
    - Combine Big Fight & CycleWarriors video routines - currently each
      game uses different sized tilemaps - these are probably software
      controlled rather than hardwired, but I don't think either game
      changes the size at runtime.
    - Split these games into individual drivers, write new devices for video routines.
    - Dip switches
    - Deviceify HD6445 (superset of 6845)
    - Various other things..


    reference of bigfight : https://youtu.be/aUUoUCr6yhk

    Emulation by Bryan McPhail, mish@tendril.co.uk


    Cycle Warriors Board Layout

    ABA-011


            6296             CW24A                   5864
                                CW25A                   5864
            YM2151                  50MHz

                                TZ8315                 CW26A
                                                    5864
        TC51821  TC51832                               D780C-1
        TC51821  TC51832
        TC51821  TC51832                     16MHz
        TC51821  TC51832

        CW00A   CW08A
        CW01A   CW09A
        CW02A   CW10A
        CW03A   CW11A            68000-12              81C78
        CW04A   CW12A                                  81C78
        CW05A   CW13A                CW16B  CW18B      65256
        CW06A   CW14A                CW17A  CW19A      65256
        CW07A   CW15A                       CW20A
                                            CW21       65256
                                68000-12   CW22A      65256
                                            CW23

    ABA-012

                            HD6445


                            51832
                            51832
                            51832
                            51832

                            CW28
                            CW29
                            CW30

    CW27

    Big Fight
    Tatsumi, 1992

    PCB Layout
    ----------

    ABA-011
    A-8
    |-----------------------------------------------------------------|
    |     LM324   M6295    ROM15                TC5563                |
    |LM324  VOL   KA51             50MHz        TC5563   PAL       |-||
    |    TC51832 TC51832          |--------|                       | ||
    |    TC51832 TC51832          |TATSUMI |                       | ||
    |    TC51832 TC51832          |TZB315  |           ROM20       | ||
    |    TC51832 TC51832          |        |           TMM2063     | ||
    |     ROM0    ROM8            |--------|           Z80B        | ||
    |                                                              | ||
    |J                     PAL                  16MHz              |-||
    |A    ROM2    ROM10     |--------------|                 PAL      |
    |M                      |     68000    |            TMM2088       |
    |M                      |--------------|                          |
    |A    ROM4    ROM12                                 TMM2088       |
    |                                ROM16    ROM17                |-||
    |                            PAL      PAL           TC51832    | ||
    |     ROM6    ROM14                       ROM18                | ||
    |                            EPL204   PAL           TC51832    | ||
    |                       |--------------|                       | ||
    |   CXD1095Q  CXD1095Q  |    68000     |  ROM19     TC51832    | ||
    |                       |--------------|                       | ||
    |                                                   TC51832    |-||
    |      DSW3(4) DSW2(8) DSW1(8)                                    |
    |-----------------------------------------------------------------|
    Z80 clock - 4.000MHz [16/4]
    68k clocks - 12.500MHz [50/4]
    M6295 clock - 2.000MHz [16/8]. Sample rate = 2000000/132
    YM2151 clock - 4.000MHz [16/4]

    |-------------------------|
    |       D65005(x16)       |
    |ROM21                 |-||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |PAL                   |-||
    |     ROM24       PAL  PAL|
    |        ROM23     HD6445 |
    |           ROM22         |
    |          TC51832(x4)    |
    |      PAL             |-||
    |      PAL             | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      | ||
    |                      |-||
    |PAL                      |
    |-------------------------|

***************************************************************************/

#include "emu.h"
#include "includes/tatsumi.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

#include "roundup5.lh"


#define CLOCK_1     XTAL(16'000'000)
#define CLOCK_2     XTAL(50'000'000)


/***************************************************************************/


WRITE8_MEMBER(cyclwarr_state::cyclwarr_sound_w)
{
	m_soundlatch->write(space, 0, data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


READ16_MEMBER(cyclwarr_state::cyclwarr_sprite_r)
{
	return m_spriteram[offset];
}

WRITE16_MEMBER(cyclwarr_state::cyclwarr_sprite_w)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

WRITE16_MEMBER(cyclwarr_state::bigfight_a40000_w)
{
	COMBINE_DATA(&m_bigfight_a40000[offset]);
}

template<int Bank>
READ16_MEMBER(cyclwarr_state::cyclwarr_videoram_r)
{
	return m_cyclwarr_videoram[Bank][offset];
}

template<int Bank>
WRITE16_MEMBER(cyclwarr_state::cyclwarr_videoram_w)
{
	COMBINE_DATA(&m_cyclwarr_videoram[Bank][offset]);
	m_layer[(Bank<<1)|0]->mark_tile_dirty(offset);
	m_layer[(Bank<<1)|1]->mark_tile_dirty(offset);
}

WRITE16_MEMBER(cyclwarr_state::output_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
	if(data & 0xfffc)
		logerror("output_w = %04x & %04x\n",data,mem_mask);
}

WRITE8_MEMBER(roundup5_state::output_w)
{
	/*
	    ---- x--- depending on Output Mode dipswitch:
	              A Mode: enables when police siren is on
	              B Mode: enables when player collides with objects or go offroad
	    ---- -x-- start button light
	    ---- --xx coin counters
	*/
	// avoid spurious write to coin counters
	if(data == 0xff)
		return;

	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

	if(data & 0xf0)
		logerror("output_w = %02x\n",data);
}

/***************************************************************************/

void apache3_state::apache3_v30_map(address_map &map)
{
	map(0x00000, 0x03fff).ram();
	map(0x04000, 0x07fff).ram().share("nvram");
	map(0x08000, 0x08fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0c000, 0x0dfff).ram().w(FUNC(apache3_state::text_w)).share("videoram");
	map(0x0e800, 0x0e803).w(FUNC(tatsumi_state::hd6445_crt_w)).umask16(0x00ff);
	map(0x0f000, 0x0f001).portr("DSW");
	map(0x0f000, 0x0f001).nopw(); // todo
	map(0x0f800, 0x0f801).rw(FUNC(apache3_state::apache3_bank_r), FUNC(apache3_state::apache3_bank_w));
	map(0x10000, 0x1ffff).rw(FUNC(apache3_state::apache3_v30_v20_r), FUNC(apache3_state::apache3_v30_v20_w));
	map(0x20000, 0x2ffff).rw(FUNC(apache3_state::tatsumi_v30_68000_r), FUNC(apache3_state::tatsumi_v30_68000_w));
	map(0x80000, 0xfffff).rom().region("master_rom", 0);
}

void apache3_state::apache3_68000_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("slave_rom", 0);
	map(0x80000, 0x83fff).ram().share("sharedram");
	map(0x90000, 0x93fff).ram().share("spriteram");
	map(0x9a000, 0x9a1ff).rw(FUNC(apache3_state::tatsumi_sprite_control_r), FUNC(apache3_state::tatsumi_sprite_control_w)).share("obj_ctrl_ram");
	map(0xa0000, 0xa0001).w(FUNC(apache3_state::apache3_rotate_w)); // /BNKCS
	map(0xb0000, 0xb0001).w(FUNC(apache3_state::apache3_z80_ctrl_w));
	map(0xc0000, 0xc0001).w(FUNC(apache3_state::apache3_road_z_w)); // /LINCS
	map(0xd0000, 0xdffff).ram().share("apache3_g_ram"); // /GRDCS
	map(0xe0000, 0xe7fff).rw(FUNC(apache3_state::apache3_z80_r), FUNC(apache3_state::apache3_z80_w));
}

void apache3_state::apache3_v20_map(address_map &map)
{
	map(0x00000, 0x01fff).ram();
	map(0x04000, 0x04003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x06000, 0x06001).portr("IN0"); // esw
	map(0x08000, 0x08001).r(FUNC(apache3_state::tatsumi_hack_ym2151_r)).w(m_ym2151, FUNC(ym2151_device::write));
	map(0x0a000, 0x0a000).r(m_oki, FUNC(okim6295_device::read)).w(m_oki, FUNC(okim6295_device::write));
	map(0x0e000, 0x0e007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w));
	map(0xf0000, 0xfffff).rom().region("sound_rom", 0);
}

void apache3_state::apache3_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("apache3_z80_ram");
	map(0x8000, 0xffff).w(FUNC(apache3_state::apache3_road_x_w));
}

/*****************************************************************/

void roundup5_state::roundup5_v30_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x08000, 0x0bfff).ram().w(FUNC(roundup5_state::text_w)).share("videoram");
	map(0x0c000, 0x0c003).w(FUNC(tatsumi_state::hd6445_crt_w)).umask16(0x00ff);
	map(0x0d000, 0x0d001).portr("DSW");
	map(0x0d400, 0x0d40f).ram().share("vregs");
	map(0x0d800, 0x0d801).writeonly().share("bg_scrollx");
	map(0x0dc00, 0x0dc01).writeonly().share("bg_scrolly");
	map(0x0e000, 0x0e001).w(FUNC(roundup5_state::roundup5_control_w));
	map(0x0f000, 0x0ffff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x10000, 0x1ffff).rw(FUNC(roundup5_state::roundup_v30_z80_r), FUNC(roundup5_state::roundup_v30_z80_w));
	map(0x20000, 0x2ffff).rw(FUNC(roundup5_state::tatsumi_v30_68000_r), FUNC(roundup5_state::tatsumi_v30_68000_w));
	map(0x30000, 0x3ffff).rw(FUNC(roundup5_state::gfxdata_r), FUNC(roundup5_state::gfxdata_w)).umask16(0x00ff);
	map(0x80000, 0xfffff).rom().region("master_rom", 0);
}

void roundup5_state::roundup5_68000_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("slave_rom", 0);
	map(0x80000, 0x83fff).ram().share("sharedram");
	map(0x90000, 0x93fff).ram().share("spriteram");
	map(0x9a000, 0x9a1ff).rw(FUNC(roundup5_state::tatsumi_sprite_control_r), FUNC(roundup5_state::tatsumi_sprite_control_w)).share("obj_ctrl_ram");
	map(0xa0000, 0xa0fff).ram().share("road_ctrl_ram"); // Road control data
	map(0xb0000, 0xb0fff).ram().share("road_pixel_ram"); // Road pixel data
	map(0xc0000, 0xc0fff).ram().share("road_color_ram"); // Road colour data
	map(0xd0000, 0xd0001).ram().share("road_yclip");
	map(0xe0000, 0xe0001).ram().w(FUNC(roundup5_state::road_vregs_w)).share("road_vregs");
}

void roundup5_state::roundup5_z80_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xffef).ram();
	map(0xfff0, 0xfff1).r(FUNC(roundup5_state::tatsumi_hack_ym2151_r)).w(m_ym2151, FUNC(ym2151_device::write));
	map(0xfff4, 0xfff4).r(m_oki, FUNC(okim6295_device::read)).w(m_oki, FUNC(okim6295_device::write));
	map(0xfff8, 0xfffb).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfffc, 0xfffc).portr("STICKX");
}

/******************************************************************************/

void cyclwarr_state::common_map(address_map &map)
{
	map(0x080000, 0x08ffff).rw(FUNC(cyclwarr_state::cyclwarr_videoram_r<1>), FUNC(cyclwarr_state::cyclwarr_videoram_w<1>)).share("cw_videoram1");
	map(0x090000, 0x09ffff).rw(FUNC(cyclwarr_state::cyclwarr_videoram_r<0>), FUNC(cyclwarr_state::cyclwarr_videoram_w<0>)).share("cw_videoram0");

	map(0x0a2000, 0x0a2007).w(FUNC(cyclwarr_state::video_config_w));
	map(0x0a4000, 0x0a4001).w(FUNC(cyclwarr_state::bigfight_a40000_w));
	map(0x0a6000, 0x0a6001).w(FUNC(cyclwarr_state::mixing_control_w));
	map(0x0ac000, 0x0ac003).w(FUNC(tatsumi_state::hd6445_crt_w)).umask16(0x00ff);

	map(0x0b8000, 0x0b8001).w(FUNC(cyclwarr_state::cyclwarr_sound_w)).umask16(0xff00);
	map(0x0b9000, 0x0b900f).rw("io1", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write)).umask16(0x00ff).cswidth(16);
	map(0x0ba000, 0x0ba00f).rw("io2", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write)).umask16(0x00ff).cswidth(16);
	map(0x0bc000, 0x0bc001).w(FUNC(cyclwarr_state::output_w));

	map(0x0c0000, 0x0c3fff).rw(FUNC(cyclwarr_state::cyclwarr_sprite_r), FUNC(cyclwarr_state::cyclwarr_sprite_w)).share("spriteram");
	map(0x0ca000, 0x0ca1ff).rw(FUNC(cyclwarr_state::tatsumi_sprite_control_r), FUNC(cyclwarr_state::tatsumi_sprite_control_w)).share("obj_ctrl_ram");
	map(0x0d0000, 0x0d3fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	// games accesses these ranges differently, we do mirroring in rom loading to make them match.
	// address bit A19 controls if access routes to upper or lower roms
	// TODO: it's unknown what Big Fight is supposed to return for the lower roms, let's assume mirror for the time being.
	// slave ROMs
	// 0x140000 - 0x1bffff tested in Cycle Warriors
	// 0x100000 - 0x17ffff tested in Big Fight
	map(0x100000, 0x1fffff).rom().region("slave_rom",0);
	// same as above but A20 instead of A19
	// master ROMs
	// 0x2c0000 - 0x33ffff tested in Cycle Warriors
	// 0x200000 - 0x27ffff tested in Big Fight
	map(0x200000, 0x3fffff).rom().region("master_rom",0);
}

void cyclwarr_state::master_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("master_ram");
	map(0x03e000, 0x03efff).ram(); // RAM_A
	map(0x040000, 0x04ffff).ram().share("slave_ram");
	common_map(map);
}

void cyclwarr_state::slave_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram().share("slave_ram");
	common_map(map);
}

void cyclwarr_state::sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xffef).ram();
	map(0xfff0, 0xfff1).r(FUNC(cyclwarr_state::tatsumi_hack_ym2151_r)).w(m_ym2151, FUNC(ym2151_device::write));
	map(0xfff4, 0xfff4).r(FUNC(cyclwarr_state::oki_status_xor_r)).w(m_oki, FUNC(okim6295_device::write));
	map(0xfffc, 0xfffc).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xfffe, 0xfffe).nopw();
}


/******************************************************************************/

static INPUT_PORTS_START( apache3 )
	PORT_START("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Trigger" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Power" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Missile" )

	PORT_START("STICK_X")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("STICK_Y")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("THROTTLE")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Z ) PORT_SENSITIVITY(25) PORT_KEYDELTA(79)

	PORT_START("VR1")
	PORT_ADJUSTER(100, "VR1")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x0000, "3" )
	PORT_DIPSETTING(    0x0001, "4" )
	PORT_DIPSETTING(    0x0002, "5" )
	PORT_DIPSETTING(    0x0003, "6" )
	PORT_DIPNAME( 0x000c, 0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x000c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6") /* Listed as "Always On" */
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7") /* Listed as "Not Used" */
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8") /* Listed as "Always On" */
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0700, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4") /* Manual only shows a 3-Way dip box, so 4-8 are unknown */
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Test ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( roundup5 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Accelerator")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Brake")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Shift") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Turbo")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	// Tested in service mode, probably unused
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-3:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("DSW") /* Verified by Manual & in Game service menu */
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW-1:1,2")
	PORT_DIPSETTING(      0x0003, "Shortest" )
	PORT_DIPSETTING(      0x0002, "Short" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0001, "Long" )
	PORT_DIPNAME( 0x000c, 0x0000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW-1:3,4")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW-1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW-1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Stage 5 Continue" )  PORT_DIPLOCATION("SW-1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Output Mode" )       PORT_DIPLOCATION("SW-1:8")
	PORT_DIPSETTING(      0x0000, "A (Light)" )
	PORT_DIPSETTING(      0x0080, "B (Vibration)" )
	// TODO: Coinage was all wrong, maybe manual refers to an undumped version?
	PORT_DIPNAME( 0x0700, 0x0000, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW-2:1,2,3")
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x0000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW-2:4,5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW-2:7") /* Manual only shows nothing for this one */
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// putting this and sw-2:6 ON position after POST to enable debugging info
	PORT_DIPNAME( 0x8000, 0x8000, "Hardware Test Mode" ) PORT_DIPLOCATION("SW-2:8") /* Manual only shows nothing for this one */
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cyclwarr )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW3")
	PORT_DIPNAME( 0x1, 0x1, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(   0x1, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x2, 0x0, "Player Select" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(   0x2, "Coin Slot" )
	PORT_DIPSETTING(   0x0, "Select SW" )
	PORT_DIPNAME( 0x4, 0x4, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(   0x4, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x8, 0x8, "Hardware Test Mode" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(   0x8, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPNAME( 0x04, 0x04, "Ticket Dispenser" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPNAME( 0x18, 0x08, "Machine Type" ) PORT_DIPLOCATION("SW2:4,5")
//  PORT_DIPSETTING(    0x00, "2 Players" ) // same as 4 players but text layout is 2p (invalid setting)
	PORT_DIPSETTING(    0x08, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPSETTING(    0x18, "4 Players" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(4)
INPUT_PORTS_END

static INPUT_PORTS_START( cyclwarb )
	PORT_INCLUDE(cyclwarr)

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x8, 0x8, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(   0x8, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bigfight )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW3")
	PORT_DIPNAME( 0x1, 0x1, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(   0x1, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x2, 0x0, "Player Select" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(   0x2, "Coin Slot" )
	PORT_DIPSETTING(   0x0, "Select SW" )
	PORT_DIPNAME( 0x4, 0x4, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(   0x4, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )
	PORT_DIPNAME( 0x8, 0x8, "Hardware Test Mode" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(   0x8, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Ticket Dispenser" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "100000" )
	PORT_DIPSETTING(    0x00, "150000" )
	PORT_DIPNAME( 0x08, 0x08, "Continue Coin" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Extend" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(4)
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout spritelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 8,12,0,4, 24,28,16,20 },
	{ STEP8(0,4*8) },
	32*8
};

static const gfx_layout roundup5_vramlayout =
{
	8,8,
	4096,
	3,
	{ 0x10000 * 8, 0x8000 * 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_apache3 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,    1024, 256)
	GFXDECODE_ENTRY( "text",    0, gfx_8x8x3_planar, 768,  16)
GFXDECODE_END

static GFXDECODE_START( gfx_roundup5 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     1024, 256)
	GFXDECODE_ENTRY(  nullptr,  0, roundup5_vramlayout, 0,  16)
GFXDECODE_END

static GFXDECODE_START( gfx_cyclwarr )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,  8192, 512)
	GFXDECODE_ENTRY( "tilerom", 0, gfx_8x8x3_planar, 0,  16)
GFXDECODE_END

/******************************************************************************/

INTERRUPT_GEN_MEMBER(tatsumi_state::v30_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xc8/4);   /* VBL */
}

WRITE_LINE_MEMBER(apache3_state::apache3_68000_reset)
{
	m_subcpu2->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

MACHINE_RESET_MEMBER(apache3_state,apache3)
{
	m_subcpu2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // TODO

	/* Hook the RESET line, which resets the Z80 */
	m_subcpu->set_reset_callback(write_line_delegate(FUNC(apache3_state::apache3_68000_reset),this));
}


void apache3_state::apache3(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, CLOCK_1 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &apache3_state::apache3_v30_map);
	m_maincpu->set_vblank_int("screen", FUNC(tatsumi_state::v30_interrupt));

	M68000(config, m_subcpu, CLOCK_2 / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &apache3_state::apache3_68000_map);
	m_subcpu->set_vblank_int("screen", FUNC(apache3_state::irq4_line_hold));

	V20(config, m_audiocpu, CLOCK_1 / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &apache3_state::apache3_v20_map);

	Z80(config, m_subcpu2, CLOCK_2 / 8);
	m_subcpu2->set_addrmap(AS_PROGRAM, &apache3_state::apache3_z80_map);
	m_subcpu2->set_vblank_int("screen", FUNC(apache3_state::irq0_line_hold));

	config.m_minimum_quantum = attotime::from_hz(6000);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	MCFG_MACHINE_RESET_OVERRIDE(apache3_state, apache3)

	m58990_device &adc(M58990(config, "adc", 1000000)); // unknown clock
	adc.in_callback<0>().set_ioport("STICK_X");
	adc.in_callback<1>().set_ioport("STICK_Y");
	adc.in_callback<2>().set_constant(0); // VSP1
	adc.in_callback<4>().set(FUNC(apache3_state::apache3_vr1_r));
	adc.in_callback<5>().set_ioport("THROTTLE");
	adc.in_callback<6>().set_constant(0); // RPSNC
	adc.in_callback<7>().set_constant(0); // LPSNC

	I8255(config, "ppi");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(CLOCK_2 / 8, 400, 0, 320, 272, 0, 240); // TODO: Hook up CRTC
	screen.set_screen_update(FUNC(apache3_state::screen_update_apache3));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_apache3);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024 + 4096); // 1024 real colours, and 4096 arranged as series of CLUTs

	/* apache 3 schematics state
	bit 4:  250
	bit 3:  500
	bit 2:  1k
	bit 1:  2k
	bit 0:  3.9kOhm resistor
	*/

	MCFG_VIDEO_START_OVERRIDE(apache3_state, apache3)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2151(config, m_ym2151, CLOCK_1 / 4);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	m_ym2151->add_route(0, "lspeaker", 0.45);
	m_ym2151->add_route(1, "rspeaker", 0.45);

	OKIM6295(config, m_oki, CLOCK_1 / 4 / 2, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.75);
}

void roundup5_state::roundup5(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, CLOCK_1 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &roundup5_state::roundup5_v30_map);
	m_maincpu->set_vblank_int("screen", FUNC(tatsumi_state::v30_interrupt));

	M68000(config, m_subcpu, CLOCK_2 / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &roundup5_state::roundup5_68000_map);

	Z80(config, m_audiocpu, CLOCK_1 / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &roundup5_state::roundup5_z80_map);

	config.m_minimum_quantum = attotime::from_hz(6000);

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("IN1");
	ppi.out_pc_callback().set(FUNC(roundup5_state::output_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(CLOCK_2 / 8, 400, 0, 320, 272, 0, 240); // TODO: Hook up CRTC
	screen.set_screen_update(FUNC(roundup5_state::screen_update_roundup5));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_roundup5);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 1024 + 4096); // 1024 real colours, and 4096 arranged as series of CLUTs
	m_palette->set_membits(8).set_endianness(ENDIANNESS_BIG);

	MCFG_VIDEO_START_OVERRIDE(roundup5_state,roundup5)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2151(config, m_ym2151, CLOCK_1 / 4);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	m_ym2151->add_route(0, "lspeaker", 0.45);
	m_ym2151->add_route(1, "rspeaker", 0.45);

	OKIM6295(config, m_oki, CLOCK_1 / 4 / 2, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.75);
}

void cyclwarr_state::machine_reset()
{
	uint16_t *src;

	// transfer data from rom to initial vector table
	src = (uint16_t *)memregion("master_rom")->base();

	for(int i=0;i<0x100/2;i++)
		m_master_ram[i] = src[i];

	src = (uint16_t *)memregion("slave_rom")->base();

	for(int i=0;i<0x100/2;i++)
		m_slave_ram[i] = src[i];

	// reset CPUs again so that above will be notified.
	// TODO: better way?
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	m_subcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	m_last_control = 0;
	m_control_word = 0;

	m_road_color_bank = m_prev_road_bank = 0;
}

void cyclwarr_state::cyclwarr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, CLOCK_2 / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &cyclwarr_state::master_map);
	m_maincpu->set_vblank_int("screen", FUNC(cyclwarr_state::irq5_line_hold));

	M68000(config, m_subcpu, CLOCK_2 / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &cyclwarr_state::slave_map);
	m_subcpu->set_vblank_int("screen", FUNC(cyclwarr_state::irq5_line_hold));

	Z80(config, m_audiocpu, CLOCK_1 / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cyclwarr_state::sound_map);

	// saner sync value (avoids crashing after crediting)
	config.m_minimum_quantum = attotime::from_hz(CLOCK_2 / 1024);

	cxd1095_device &io1(CXD1095(config, "io1", 0));
	io1.in_portb_cb().set_ioport("SERVICE");
	io1.in_portc_cb().set_ioport("P1");
	io1.in_portd_cb().set_ioport("P2");
	io1.in_porte_cb().set_ioport("DSW3");

	cxd1095_device &io2(CXD1095(config, "io2", 0));
	io2.in_porta_cb().set_ioport("DSW1");
	io2.in_portb_cb().set_ioport("DSW2");
	io2.in_portc_cb().set_ioport("P3");
	io2.in_portd_cb().set_ioport("P4");
	io2.out_porte_cb().set(FUNC(cyclwarr_state::cyclwarr_control_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(CLOCK_2 / 8, 400, 0, 320, 272, 0, 240); // TODO: Hook up CRTC
	screen.set_screen_update(FUNC(cyclwarr_state::screen_update_cyclwarr));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cyclwarr);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 8192 + 8192);

	MCFG_VIDEO_START_OVERRIDE(cyclwarr_state, cyclwarr)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
//  m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2151(config, m_ym2151, CLOCK_1 / 4);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	m_ym2151->add_route(0, "lspeaker", 0.45);
	m_ym2151->add_route(1, "rspeaker", 0.45);

	OKIM6295(config, m_oki, CLOCK_1 / 8, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.75);
}

void cyclwarr_state::bigfight(machine_config &config)
{
	cyclwarr(config);

	// TODO: it's same video HW, we don't know how/where video registers are mapped
//  subdevice<screen_device>("screen")->set_screen_update(FUNC(cyclwarr_state::screen_update_bigfight));

	MCFG_VIDEO_START_OVERRIDE(cyclwarr_state, bigfight)

	/* sound hardware */
	// TODO: 2MHz was too fast. Can the clock be software controlled?
	m_oki->set_clock(CLOCK_1 / 8 / 2);
}

/***************************************************************************/

ROM_START( apache3 )
	ROM_REGION( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ap-25f.125",   0x020001, 0x10000, CRC(3c7530f4) SHA1(9f7b58a3abddbdc3081ba9dfc1732406eb8c1752) )
	ROM_LOAD16_BYTE( "ap-26f.133",   0x020000, 0x10000, CRC(2955997f) SHA1(86e37def923d9cf4eb33e7979118ec6f1ef62678) )
	ROM_LOAD16_BYTE( "ap-23f.110",   0x060001, 0x10000, CRC(d7077149) SHA1(b08f5a9ee03641c20bdd5e5c9671a22c740150c6) )
	ROM_LOAD16_BYTE( "ap-24f.118",   0x060000, 0x10000, CRC(0bdef11b) SHA1(ed687600962ed2ca3a8e67cbd84fa5486778eade) )

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ap-19c.80",   0x000001, 0x10000, CRC(0908e468) SHA1(a2d725993bd4cd5425468736154fd3dd9dd7b060) )
	ROM_LOAD16_BYTE( "ap-21c.97",   0x000000, 0x10000, CRC(38a056fb) SHA1(67c8ae58670cebde0771854e1fb5fc2eb2543ecc) )
	ROM_LOAD16_BYTE( "ap-20a.89",   0x040001, 0x20000, CRC(92d24b5e) SHA1(1ea270d46a607e47b7e0961b532316aa05dc8f4e) )
	ROM_LOAD16_BYTE( "ap-22a.105",  0x040000, 0x20000, CRC(a8458a92) SHA1(43674731c2e9962c2bfbb73a85484cf03d6be223) )

	ROM_REGION( 0x10000, "sound_rom", 0 ) /* 64k code for sound V20 */
	ROM_LOAD( "ap-27d.151",   0x00000, 0x10000, CRC(294b4d79) SHA1(2b03418a12a2aaf3919b98161d8d0ce6ae29a2bb) )

	ROM_REGION( 0x200000, "sprites", ROMREGION_ERASE00 )
	/* Filled in by both regions below */

	ROM_REGION( 0x100000, "sprites_l", 0)
	ROM_LOAD32_BYTE( "ap-00c.15",   0x000000, 0x20000, CRC(ad1ddc2b) SHA1(81f64663c4892ab5fb0e2dc99513dbfee73f15b8) )
	ROM_LOAD32_BYTE( "ap-01c.22",   0x000001, 0x20000, CRC(6286ff00) SHA1(920da4a3a441dbf54ad86c0f4fb6f47a867e9cda) )
	ROM_LOAD32_BYTE( "ap-04c.58",   0x000002, 0x20000, CRC(dc6d55e4) SHA1(9f48f8d6aa1a329a71913139a8d5a50d95a9b9e5) )
	ROM_LOAD32_BYTE( "ap-05c.65",   0x000003, 0x20000, CRC(2e6e495f) SHA1(af610f265da53735b20ddc6df1bda47fc54ee0c3) )
	ROM_LOAD32_BYTE( "ap-02c.34",   0x080000, 0x20000, CRC(af4ee7cb) SHA1(4fe2361b7431971b07671f145abf1ea5861d01db) )
	ROM_LOAD32_BYTE( "ap-03c.46",   0x080001, 0x20000, CRC(60ab495c) SHA1(18340d4fba550495b1e52f8023a0a2ec6349dfeb) )
	ROM_LOAD32_BYTE( "ap-06c.71",   0x080002, 0x20000, CRC(0ea90e55) SHA1(b16d6b8be4853797507d3e5c933a9dd1d451308e) )
	ROM_LOAD32_BYTE( "ap-07c.75",   0x080003, 0x20000, CRC(ba685543) SHA1(140a2b708d4e4de4d207fc2c4a96a5cab8639988) )

	ROM_REGION( 0x100000, "sprites_h", 0)
	ROM_LOAD32_BYTE( "ap-08c.14",   0x000000, 0x20000, CRC(6437b580) SHA1(2b2ba42add18bbec04fbcf53645a8d44b972e26a) )
	ROM_LOAD32_BYTE( "ap-09c.21",   0x000001, 0x20000, CRC(54d18ef9) SHA1(40ebc6ea49b2a501fe843d60bec8c32d07f2d25d) )
	ROM_LOAD32_BYTE( "ap-12c.57",   0x000002, 0x20000, CRC(f95cf5cf) SHA1(ce373c648cbf3e4863bbc3a1175efe065c75eb13) )
	ROM_LOAD32_BYTE( "ap-13c.64",   0x000003, 0x20000, CRC(67a248c3) SHA1(cc945f7cfecaaab5075c1a3d202369b070d4c656) )
	ROM_LOAD32_BYTE( "ap-10c.33",   0x080000, 0x20000, CRC(74418df4) SHA1(cc1206b10afc2de919b2fb9899486122d27290a4) )
	ROM_LOAD32_BYTE( "ap-11c.45",   0x080001, 0x20000, CRC(195bf78e) SHA1(c3c472f3c4244545b89491b6ebec4f838a6bbb73) )
	ROM_LOAD32_BYTE( "ap-14c.70",   0x080002, 0x20000, CRC(58f7fe16) SHA1(a5b87b42b85808c226df0d2a7b7cdde12d474a41) )
	ROM_LOAD32_BYTE( "ap-15c.74",   0x080003, 0x20000, CRC(1ffd5496) SHA1(25efb568957fc9441a40a7d64cc6afe1a14b392b) )

	ROM_REGION( 0x18000, "text", 0 )
	ROM_LOAD( "ap-18d.73",   0x000000, 0x8000, CRC(55e664bf) SHA1(505bec8b5ff3f9fa2c5fb1213d54683347905be1) )
	ROM_LOAD( "ap-17d.68",   0x008000, 0x8000, CRC(6199afe4) SHA1(ad8c0ed6c33d984bb29c89f2e7fc7e5a923cefe3) )
	ROM_LOAD( "ap-16d.63",   0x010000, 0x8000, CRC(f115656d) SHA1(61798858dc0172192d89e666696b2c7642756899) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ap-28c.171",   0x000000, 0x20000, CRC(b349f0c2) SHA1(cb1ff1c0e784f669c87ab1eccd3b358950761b74) )
	ROM_LOAD( "ap-29c.176",   0x020000, 0x10000, CRC(b38fced3) SHA1(72f61a719f393957bcccf14687bfbb2e7a5f7aee) )

	ROM_REGION( 0x200, "proms", 0 ) /* Road stripe PROM */
	ROM_LOAD( "am27s29.ic41",   0x000, 0x200, CRC(c981f1e0) SHA1(7d8492d9f4033ab3734c09ee23016a0b210648b5) )
ROM_END

ROM_START( apache3a )
	ROM_REGION( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ap-25c.125",   0x020001, 0x10000, CRC(7bc496a6) SHA1(5491d06181d729407e975b85a8715fdc3b489c67) )
	ROM_LOAD16_BYTE( "ap-26c.133",   0x020000, 0x10000, CRC(9393a470) SHA1(00376f7a545629a83eb5a90b9d1685a68430e4ce) )
	ROM_LOAD16_BYTE( "ap-23e.110",   0x060001, 0x10000, CRC(3e465b8e) SHA1(dfd009221974eb43263dd3f4f80f39bb32c30ced) ) /* hand written label:  23E   DD65 */
	ROM_LOAD16_BYTE( "ap-24e.118",   0x060000, 0x10000, CRC(1ef746f2) SHA1(31d77bd954ea6cc00cac40a3a514281da371030b) ) /* hand written label:  24E   E2A2 */

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ap-19c.80",   0x000001, 0x10000, CRC(0908e468) SHA1(a2d725993bd4cd5425468736154fd3dd9dd7b060) )
	ROM_LOAD16_BYTE( "ap-21c.97",   0x000000, 0x10000, CRC(38a056fb) SHA1(67c8ae58670cebde0771854e1fb5fc2eb2543ecc) )
	ROM_LOAD16_BYTE( "ap-20a.89",   0x040001, 0x20000, CRC(92d24b5e) SHA1(1ea270d46a607e47b7e0961b532316aa05dc8f4e) )
	ROM_LOAD16_BYTE( "ap-22a.105",  0x040000, 0x20000, CRC(a8458a92) SHA1(43674731c2e9962c2bfbb73a85484cf03d6be223) )

	ROM_REGION( 0x10000, "sound_rom", 0 ) /* 64k code for sound V20 */
	ROM_LOAD( "ap-27d.151",   0x00000, 0x10000, CRC(294b4d79) SHA1(2b03418a12a2aaf3919b98161d8d0ce6ae29a2bb) )

	ROM_REGION( 0x200000, "sprites", ROMREGION_ERASE00 )
	/* Filled in by both regions below */

	ROM_REGION( 0x100000, "sprites_l", 0)
	ROM_LOAD32_BYTE( "ap-00c.15",   0x000000, 0x20000, CRC(ad1ddc2b) SHA1(81f64663c4892ab5fb0e2dc99513dbfee73f15b8) )
	ROM_LOAD32_BYTE( "ap-01c.22",   0x000001, 0x20000, CRC(6286ff00) SHA1(920da4a3a441dbf54ad86c0f4fb6f47a867e9cda) )
	ROM_LOAD32_BYTE( "ap-04c.58",   0x000002, 0x20000, CRC(dc6d55e4) SHA1(9f48f8d6aa1a329a71913139a8d5a50d95a9b9e5) )
	ROM_LOAD32_BYTE( "ap-05c.65",   0x000003, 0x20000, CRC(2e6e495f) SHA1(af610f265da53735b20ddc6df1bda47fc54ee0c3) )
	ROM_LOAD32_BYTE( "ap-02c.34",   0x080000, 0x20000, CRC(af4ee7cb) SHA1(4fe2361b7431971b07671f145abf1ea5861d01db) )
	ROM_LOAD32_BYTE( "ap-03c.46",   0x080001, 0x20000, CRC(60ab495c) SHA1(18340d4fba550495b1e52f8023a0a2ec6349dfeb) )
	ROM_LOAD32_BYTE( "ap-06c.71",   0x080002, 0x20000, CRC(0ea90e55) SHA1(b16d6b8be4853797507d3e5c933a9dd1d451308e) )
	ROM_LOAD32_BYTE( "ap-07c.75",   0x080003, 0x20000, CRC(ba685543) SHA1(140a2b708d4e4de4d207fc2c4a96a5cab8639988) )

	ROM_REGION( 0x100000, "sprites_h", 0)
	ROM_LOAD32_BYTE( "ap-08c.14",   0x000000, 0x20000, CRC(6437b580) SHA1(2b2ba42add18bbec04fbcf53645a8d44b972e26a) )
	ROM_LOAD32_BYTE( "ap-09c.21",   0x000001, 0x20000, CRC(54d18ef9) SHA1(40ebc6ea49b2a501fe843d60bec8c32d07f2d25d) )
	ROM_LOAD32_BYTE( "ap-12c.57",   0x000002, 0x20000, CRC(f95cf5cf) SHA1(ce373c648cbf3e4863bbc3a1175efe065c75eb13) )
	ROM_LOAD32_BYTE( "ap-13c.64",   0x000003, 0x20000, CRC(67a248c3) SHA1(cc945f7cfecaaab5075c1a3d202369b070d4c656) )
	ROM_LOAD32_BYTE( "ap-10c.33",   0x080000, 0x20000, CRC(74418df4) SHA1(cc1206b10afc2de919b2fb9899486122d27290a4) )
	ROM_LOAD32_BYTE( "ap-11c.45",   0x080001, 0x20000, CRC(195bf78e) SHA1(c3c472f3c4244545b89491b6ebec4f838a6bbb73) )
	ROM_LOAD32_BYTE( "ap-14c.70",   0x080002, 0x20000, CRC(58f7fe16) SHA1(a5b87b42b85808c226df0d2a7b7cdde12d474a41) )
	ROM_LOAD32_BYTE( "ap-15c.74",   0x080003, 0x20000, CRC(1ffd5496) SHA1(25efb568957fc9441a40a7d64cc6afe1a14b392b) )

	ROM_REGION( 0x18000, "text", 0 )
	ROM_LOAD( "ap-18e.73",   0x000000, 0x10000, CRC(d7861a26) SHA1(b1a1e089a293a5536d342c9edafbea303f4f128c) )
	ROM_LOAD( "ap-16e.63",   0x008000, 0x10000, CRC(d3251965) SHA1(aef4f58a6f773060434abda9d7f5f003693577bf) )
	ROM_LOAD( "ap-17e.68",   0x008000, 0x08000, CRC(4509c2ed) SHA1(97a6a6710e83aca212ce43d06c3f26c35f9782b8) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ap-28c.171",   0x000000, 0x20000, CRC(b349f0c2) SHA1(cb1ff1c0e784f669c87ab1eccd3b358950761b74) )
	ROM_LOAD( "ap-29c.176",   0x020000, 0x10000, CRC(b38fced3) SHA1(72f61a719f393957bcccf14687bfbb2e7a5f7aee) )

	ROM_REGION( 0x200, "proms", 0 ) /* Road stripe PROM */
	ROM_LOAD( "am27s29.ic41",   0x000, 0x200, CRC(c981f1e0) SHA1(7d8492d9f4033ab3734c09ee23016a0b210648b5) )
ROM_END

ROM_START( apache3b )
	ROM_REGION( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ap-25c.125",   0x020001, 0x10000, CRC(7bc496a6) SHA1(5491d06181d729407e975b85a8715fdc3b489c67) )
	ROM_LOAD16_BYTE( "ap-26c.133",   0x020000, 0x10000, CRC(9393a470) SHA1(00376f7a545629a83eb5a90b9d1685a68430e4ce) )
	ROM_LOAD16_BYTE( "ap-23g.110",   0x060001, 0x10000, CRC(0ab485e4) SHA1(d8d0695312732c31cedcb1c298810a6793835e80) ) /* Kana Corporation license program ROMs */
	ROM_LOAD16_BYTE( "ap-24g.118",   0x060000, 0x10000, CRC(6348e196) SHA1(6be537491a56a28b62981cae6db8dfc4eb2fece2) ) /* Kana Corporation license program ROMs */

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ap-19c.80",   0x000001, 0x10000, CRC(0908e468) SHA1(a2d725993bd4cd5425468736154fd3dd9dd7b060) )
	ROM_LOAD16_BYTE( "ap-21c.97",   0x000000, 0x10000, CRC(38a056fb) SHA1(67c8ae58670cebde0771854e1fb5fc2eb2543ecc) )
	ROM_LOAD16_BYTE( "ap-20a.89",   0x040001, 0x20000, CRC(92d24b5e) SHA1(1ea270d46a607e47b7e0961b532316aa05dc8f4e) )
	ROM_LOAD16_BYTE( "ap-22a.105",  0x040000, 0x20000, CRC(a8458a92) SHA1(43674731c2e9962c2bfbb73a85484cf03d6be223) )

	ROM_REGION( 0x10000, "sound_rom", 0 ) /* 64k code for sound V20 */
	ROM_LOAD( "ap-27d.151",   0x00000, 0x10000, CRC(294b4d79) SHA1(2b03418a12a2aaf3919b98161d8d0ce6ae29a2bb) )

	ROM_REGION( 0x200000, "sprites", ROMREGION_ERASE00 )
	/* Filled in by both regions below */

	ROM_REGION( 0x100000, "sprites_l", 0)
	ROM_LOAD32_BYTE( "ap-00c.15",   0x000000, 0x20000, CRC(ad1ddc2b) SHA1(81f64663c4892ab5fb0e2dc99513dbfee73f15b8) )
	ROM_LOAD32_BYTE( "ap-01c.22",   0x000001, 0x20000, CRC(6286ff00) SHA1(920da4a3a441dbf54ad86c0f4fb6f47a867e9cda) )
	ROM_LOAD32_BYTE( "ap-04c.58",   0x000002, 0x20000, CRC(dc6d55e4) SHA1(9f48f8d6aa1a329a71913139a8d5a50d95a9b9e5) )
	ROM_LOAD32_BYTE( "ap-05c.65",   0x000003, 0x20000, CRC(2e6e495f) SHA1(af610f265da53735b20ddc6df1bda47fc54ee0c3) )
	ROM_LOAD32_BYTE( "ap-02c.34",   0x080000, 0x20000, CRC(af4ee7cb) SHA1(4fe2361b7431971b07671f145abf1ea5861d01db) )
	ROM_LOAD32_BYTE( "ap-03c.46",   0x080001, 0x20000, CRC(60ab495c) SHA1(18340d4fba550495b1e52f8023a0a2ec6349dfeb) )
	ROM_LOAD32_BYTE( "ap-06c.71",   0x080002, 0x20000, CRC(0ea90e55) SHA1(b16d6b8be4853797507d3e5c933a9dd1d451308e) )
	ROM_LOAD32_BYTE( "ap-07c.75",   0x080003, 0x20000, CRC(ba685543) SHA1(140a2b708d4e4de4d207fc2c4a96a5cab8639988) )

	ROM_REGION( 0x100000, "sprites_h", 0)
	ROM_LOAD32_BYTE( "ap-08c.14",   0x000000, 0x20000, CRC(6437b580) SHA1(2b2ba42add18bbec04fbcf53645a8d44b972e26a) )
	ROM_LOAD32_BYTE( "ap-09c.21",   0x000001, 0x20000, CRC(54d18ef9) SHA1(40ebc6ea49b2a501fe843d60bec8c32d07f2d25d) )
	ROM_LOAD32_BYTE( "ap-12c.57",   0x000002, 0x20000, CRC(f95cf5cf) SHA1(ce373c648cbf3e4863bbc3a1175efe065c75eb13) )
	ROM_LOAD32_BYTE( "ap-13c.64",   0x000003, 0x20000, CRC(67a248c3) SHA1(cc945f7cfecaaab5075c1a3d202369b070d4c656) )
	ROM_LOAD32_BYTE( "ap-10c.33",   0x080000, 0x20000, CRC(74418df4) SHA1(cc1206b10afc2de919b2fb9899486122d27290a4) )
	ROM_LOAD32_BYTE( "ap-11c.45",   0x080001, 0x20000, CRC(195bf78e) SHA1(c3c472f3c4244545b89491b6ebec4f838a6bbb73) )
	ROM_LOAD32_BYTE( "ap-14c.70",   0x080002, 0x20000, CRC(58f7fe16) SHA1(a5b87b42b85808c226df0d2a7b7cdde12d474a41) )
	ROM_LOAD32_BYTE( "ap-15c.74",   0x080003, 0x20000, CRC(1ffd5496) SHA1(25efb568957fc9441a40a7d64cc6afe1a14b392b) )

	ROM_REGION( 0x18000, "text", 0 )
	ROM_LOAD( "ap-18e.73",   0x000000, 0x10000, CRC(d7861a26) SHA1(b1a1e089a293a5536d342c9edafbea303f4f128c) )
	ROM_LOAD( "ap-16e.63",   0x008000, 0x10000, CRC(d3251965) SHA1(aef4f58a6f773060434abda9d7f5f003693577bf) )
	ROM_LOAD( "ap-17e.68",   0x008000, 0x08000, CRC(4509c2ed) SHA1(97a6a6710e83aca212ce43d06c3f26c35f9782b8) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ap-28c.171",   0x000000, 0x20000, CRC(b349f0c2) SHA1(cb1ff1c0e784f669c87ab1eccd3b358950761b74) )
	ROM_LOAD( "ap-29c.176",   0x020000, 0x10000, CRC(b38fced3) SHA1(72f61a719f393957bcccf14687bfbb2e7a5f7aee) )

	ROM_REGION( 0x200, "proms", 0 ) /* Road stripe PROM */
	ROM_LOAD( "am27s29.ic41",   0x000, 0x200, CRC(c981f1e0) SHA1(7d8492d9f4033ab3734c09ee23016a0b210648b5) )
ROM_END

ROM_START( roundup5 )
	ROM_REGION( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ru-23s",   0x000000, 0x20000, CRC(2dc8c521) SHA1(b78de101db3ef00fc4375ae32a7871e0da2dac6c) )
	ROM_LOAD16_BYTE( "ru-26s",   0x000001, 0x20000, CRC(1e16b531) SHA1(d7badef29cf1c4a9bd262933ecd1ca3343ea94bd) )
	ROM_LOAD16_BYTE( "ru-22t",   0x040000, 0x20000, CRC(9611382e) SHA1(c99258782dbad6d69ba7f54115ee3aa218f9b6ee) )
	ROM_LOAD16_BYTE( "ru-25t",   0x040001, 0x20000, CRC(b6cd0f2d) SHA1(61925c2346d79baaf9bce3d19a7dfc45b8232f92) )

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ru-20s",   0x000000, 0x20000, CRC(c5524558) SHA1(a94e7e4548148c83a332524ab4e06607732e13d5) )
	ROM_LOAD16_BYTE( "ru-18s",   0x000001, 0x20000, CRC(163ef03d) SHA1(099ac2d74164bdc6402b08efb521f49275780858) )
	ROM_LOAD16_BYTE( "ru-21s",   0x040000, 0x20000, CRC(b9f91b70) SHA1(43c5d9dafb60ed3e5c3eb0e612c2dbc5497f8a6c) )
	ROM_LOAD16_BYTE( "ru-19s",   0x040001, 0x20000, CRC(e3953800) SHA1(28fbc6bf154b512fcefeb04fe12db598b1b20cfe) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "ru-28d",   0x000000, 0x10000, CRC(df36c6c5) SHA1(c046482043f6b54c55696ba3d339ffb11d78f674) )

	ROM_REGION( 0x180000, "sprites", ROMREGION_ERASE00 )
	/* Filled in by both regions below */

	ROM_REGION( 0x0c0000, "sprites_l", 0)
	ROM_LOAD32_BYTE( "ru-00b",   0x000000, 0x20000, CRC(388a0647) SHA1(e4ab43832872f44c0fe1aaede4372cc00ca7d32b) )
	ROM_LOAD32_BYTE( "ru-02b",   0x000001, 0x20000, CRC(eff33945) SHA1(3f4c3aaa11ccf945c2f898dfdf815705d8539e21) )
	ROM_LOAD32_BYTE( "ru-04b",   0x000002, 0x20000, CRC(40fda247) SHA1(f5fbc07fda024baedf35ac209210e94df9f15065) )
	ROM_LOAD32_BYTE( "ru-06b",   0x000003, 0x20000, CRC(cd2484f3) SHA1(a23a4d36a8b913104bcc75228317b2979afec888) )
	ROM_LOAD32_BYTE( "ru-01b",   0x080000, 0x10000, CRC(5e91f401) SHA1(df976c5ba0f14b14f5642b5ca35b996bca64e369) )
	ROM_LOAD32_BYTE( "ru-03b",   0x080001, 0x10000, CRC(2fb109de) SHA1(098c103e6bae0f52ec66f0cdda2da60bd7108736) )
	ROM_LOAD32_BYTE( "ru-05b",   0x080002, 0x10000, CRC(23dd10e1) SHA1(f30ff1a8c7ed9bc567b901cbdd202028fffb9f80) )
	ROM_LOAD32_BYTE( "ru-07b",   0x080003, 0x10000, CRC(bb40f46e) SHA1(da694e16d19f60a0dee47551f00f3e50b2d5dcaf) )

	ROM_REGION( 0x0c0000, "sprites_h", 0)
	ROM_LOAD32_BYTE( "ru-08b",   0x000000, 0x20000, CRC(01729e3c) SHA1(1445287fde0b993d053aab73efafc902a6b7e2cc) )
	ROM_LOAD32_BYTE( "ru-10b",   0x000001, 0x20000, CRC(cd2357a7) SHA1(313460a74244325ce2c659816f2b738f3dc5358a) )
	ROM_LOAD32_BYTE( "ru-12b",   0x000002, 0x20000, CRC(ca63b1f8) SHA1(a50ef8259745dc166eb0a1b2c812ff620818a755) )
	ROM_LOAD32_BYTE( "ru-14b",   0x000003, 0x20000, CRC(dde79bfc) SHA1(2d5888189a6f954801f248a3365e328370fed837) )
	ROM_LOAD32_BYTE( "ru-09b",   0x080000, 0x10000, CRC(629ac0a6) SHA1(c3eeccd6c07be7455cf180c9c7d5efcd6d08c0b5) )
	ROM_LOAD32_BYTE( "ru-11b",   0x080001, 0x10000, CRC(fe3fbf53) SHA1(7400c088025ac22e5d9db816792533fc02f2dcf5) )
	ROM_LOAD32_BYTE( "ru-13b",   0x080002, 0x10000, CRC(d0f6e747) SHA1(ef15ed41124b2d37bc6e92254138690dd644e50f) )
	ROM_LOAD32_BYTE( "ru-15b",   0x080003, 0x10000, CRC(6ee6b22e) SHA1(a28edaf23ca6c7231264de962d5ea37bad39f996) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ru-17b",   0x000000, 0x20000, CRC(82391b47) SHA1(6b1977522c6e906503abc50bdd24c4c38cdc9bdb) )
	ROM_LOAD( "ru-16b",   0x020000, 0x10000, CRC(374fe170) SHA1(5d190a2735698b0384948bfdb1a900f56f0d7ebc) )
ROM_END

ROM_START( cyclwarr )
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "cw16c",   0x000000, 0x20000, CRC(4d88892b) SHA1(dc85231a3c4f83118922c13615381f185bcee832) )
	ROM_LOAD16_BYTE( "cw18c",   0x000001, 0x20000, CRC(4ff56209) SHA1(d628dc3fdc3e9de568ba8dbabf8e13a62e20a215) )
	ROM_COPY("master_rom",      0x000000, 0x040000, 0x040000 )
	ROM_COPY("master_rom",      0x000000, 0x080000, 0x040000 )
	ROM_COPY("master_rom",      0x000000, 0x0c0000, 0x040000 )
	ROM_LOAD16_BYTE( "cw17b",   0x100000, 0x20000, CRC(da998afc) SHA1(dd9377ce079df5c66bdb29dfd333428cce817656) )
	ROM_LOAD16_BYTE( "cw19b",   0x100001, 0x20000, CRC(c15a8413) SHA1(647b2a994a4912b5d7dc71b875f5d08c14412c6a) )
	ROM_COPY("master_rom",      0x100000, 0x140000, 0x040000 )
	ROM_COPY("master_rom",      0x100000, 0x180000, 0x040000 )
	ROM_COPY("master_rom",      0x100000, 0x1c0000, 0x040000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "cw20b",   0x000000, 0x20000, CRC(4d75292a) SHA1(71d59c1d03b323d4021209a7f0506b4a855a73af) )
	ROM_LOAD16_BYTE( "cw22b",   0x000001, 0x20000, CRC(0aec0ba4) SHA1(d559e54d303afac4a981c4a933a05278044ac068) )
	ROM_COPY("slave_rom",       0x000000, 0x40000, 0x040000 )
	ROM_LOAD16_BYTE( "cw21",    0x080000, 0x20000, CRC(ed90d956) SHA1(f533f93da31ac6eb631fb506357717e7cac8e186) )
	ROM_LOAD16_BYTE( "cw23",    0x080001, 0x20000, CRC(009cdc78) SHA1(a77933a7736546397e8c69226703d6f9be7b55e5) )
	ROM_COPY("slave_rom",       0x080000, 0xc0000, 0x040000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "cw26a",   0x000000, 0x10000, CRC(f7a70e3a) SHA1(5581633bf1f15d7f5c1e03de897d65d60f9f1e33) )

	ROM_REGION( 0x200000, "sprites", ROMREGION_ERASE00 )
	/* Filled in by both regions below */

	ROM_REGION( 0x100000, "sprites_l", 0)
	ROM_LOAD32_BYTE( "cw00a",   0x000000, 0x20000, CRC(058a77f1) SHA1(93f99fcf6ce6714d76af6f6e930115516f0379d3) )
	ROM_LOAD32_BYTE( "cw08a",   0x000001, 0x20000, CRC(f53993e7) SHA1(ef2d502ab180d2bc0bdb698c2878fdee9a2c33a8) )
	ROM_LOAD32_BYTE( "cw02a",   0x000002, 0x20000, CRC(4dadf3cb) SHA1(e42c56e295a443cb605d48eba23a16fab3c86525) )
	ROM_LOAD32_BYTE( "cw10a",   0x000003, 0x20000, CRC(3b7cd251) SHA1(52b9637404fa193421294dfb52c1a7bba0d94c9b) )
	ROM_LOAD32_BYTE( "cw01a",   0x080000, 0x20000, CRC(7c639948) SHA1(d58ff5735cd3179ffafead385a625baa7962e1d0) )
	ROM_LOAD32_BYTE( "cw09a",   0x080001, 0x20000, CRC(4ba24af5) SHA1(9203c2639e04aaa09996339f11259750ff8129b9) )
	ROM_LOAD32_BYTE( "cw03a",   0x080002, 0x20000, CRC(3ca6f98e) SHA1(8526fe38d3b4c66e09049ba18651a9e7255d85d6) )
	ROM_LOAD32_BYTE( "cw11a",   0x080003, 0x20000, CRC(5d760392) SHA1(7bbda2880af4659c267193ce10ed887a1b54a981) )

	ROM_REGION( 0x100000, "sprites_h", 0)
	ROM_LOAD32_BYTE( "cw04a",   0x000000, 0x20000, CRC(f05f594d) SHA1(80effaa517b2154c013419e0bc05fd0797b74c8d) )
	ROM_LOAD32_BYTE( "cw12a",   0x000001, 0x20000, CRC(4ac07e8b) SHA1(f9de96fba39d5752d61b8f6be87fb605694624ed) )
	ROM_LOAD32_BYTE( "cw06a",   0x000002, 0x20000, CRC(f628edc9) SHA1(473f7ec28000e6bf72782c1c3f4afb5e021bd430) )
	ROM_LOAD32_BYTE( "cw14a",   0x000003, 0x20000, CRC(a9131f5f) SHA1(3a2059946984733e6939f3298f0db676e6a3301b) )
	ROM_LOAD32_BYTE( "cw05a",   0x080000, 0x20000, CRC(c8f5faa9) SHA1(f374531ffd645597eeb1440fd2cadb426fcd3d79) )
	ROM_LOAD32_BYTE( "cw13a",   0x080001, 0x20000, CRC(8091d381) SHA1(7faf068ce20b2877559f0335df55d61be13146b4) )
	ROM_LOAD32_BYTE( "cw07a",   0x080002, 0x20000, CRC(314579b5) SHA1(3c10ec490f7821a5b5412295232bbb104d0e4b83) )
	ROM_LOAD32_BYTE( "cw15a",   0x080003, 0x20000, CRC(7ed4b721) SHA1(b87865effeff77a9ea74354ef2b5911a5102a647) )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "cw27",   0x000000, 0x20000, CRC(2db48a9e) SHA1(16c307340d17cd3b5455ebcee681fbe0335dec58) )

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "cw30",   0x000000, 0x20000, CRC(331d0711) SHA1(82251fe1f1d36f079080943ab1fd04a60077c353) )
	ROM_LOAD( "cw29",   0x020000, 0x20000, CRC(64dd519c) SHA1(e23611fc2be896861997063546c3eb03527eaf8e) )
	ROM_LOAD( "cw28",   0x040000, 0x20000, CRC(3fc568ed) SHA1(91125c9deddc659449ca6791a847fe908c2818b2) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "cw24a",   0x000000, 0x20000, CRC(22600cba) SHA1(a1514fbe037942f1493a17eb0b7986949470cb22) )
	ROM_LOAD( "cw25a",   0x020000, 0x20000, CRC(372c6bc8) SHA1(d4875bf3bffecf338bebba3b8d6a791585556a06) )
ROM_END

ROM_START( cyclwarra )
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "cw16b",   0x000000, 0x20000, CRC(cb1a737a) SHA1(a603ee1256be5641d00a72f64efaaacb65ed9d7d) )
	ROM_LOAD16_BYTE( "cw18b",   0x000001, 0x20000, CRC(0633ddcb) SHA1(1196ab17065352ec5b37f2f6b383a43a2d0fa3a6) )
	ROM_COPY("master_rom",      0x000000, 0x040000, 0x040000 )
	ROM_COPY("master_rom",      0x000000, 0x080000, 0x040000 )
	ROM_COPY("master_rom",      0x000000, 0x0c0000, 0x040000 )
	ROM_LOAD16_BYTE( "cw17a",   0x100000, 0x20000, CRC(2ad6f836) SHA1(5fa4275b433013943ba1d1b64a3c725097f946f9) )
	ROM_LOAD16_BYTE( "cw19a",   0x100001, 0x20000, CRC(d3853658) SHA1(c9338083a04f55bd22285176831f4b0bdb78564f) )
	ROM_COPY("master_rom",      0x100000, 0x140000, 0x040000 )
	ROM_COPY("master_rom",      0x100000, 0x180000, 0x040000 )
	ROM_COPY("master_rom",      0x100000, 0x1c0000, 0x040000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "cw20a",   0x000000, 0x20000, CRC(c3578ac1) SHA1(21d369da874f01922d0f0b757a42b4321df891d4) )
	ROM_LOAD16_BYTE( "cw22a",   0x000001, 0x20000, CRC(5339ed24) SHA1(5b0a54c2442dcf7373ff8b55b91af9772473ff77) )
	ROM_COPY("slave_rom",       0x000000, 0x040000, 0x040000 )
	ROM_LOAD16_BYTE( "cw21",    0x080000, 0x20000, CRC(ed90d956) SHA1(f533f93da31ac6eb631fb506357717e7cac8e186) )
	ROM_LOAD16_BYTE( "cw23",    0x080001, 0x20000, CRC(009cdc78) SHA1(a77933a7736546397e8c69226703d6f9be7b55e5) )
	ROM_COPY("slave_rom",       0x080000, 0x0c0000, 0x040000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "cw26a",   0x000000, 0x10000, CRC(f7a70e3a) SHA1(5581633bf1f15d7f5c1e03de897d65d60f9f1e33) )

	ROM_REGION( 0x200000, "sprites", ROMREGION_ERASE00 )
	/* Filled in by both regions below */

	ROM_REGION( 0x100000, "sprites_l", 0)
	ROM_LOAD32_BYTE( "cw00a",   0x000000, 0x20000, CRC(058a77f1) SHA1(93f99fcf6ce6714d76af6f6e930115516f0379d3) )
	ROM_LOAD32_BYTE( "cw08a",   0x000001, 0x20000, CRC(f53993e7) SHA1(ef2d502ab180d2bc0bdb698c2878fdee9a2c33a8) )
	ROM_LOAD32_BYTE( "cw02a",   0x000002, 0x20000, CRC(4dadf3cb) SHA1(e42c56e295a443cb605d48eba23a16fab3c86525) )
	ROM_LOAD32_BYTE( "cw10a",   0x000003, 0x20000, CRC(3b7cd251) SHA1(52b9637404fa193421294dfb52c1a7bba0d94c9b) )
	ROM_LOAD32_BYTE( "cw01a",   0x080000, 0x20000, CRC(7c639948) SHA1(d58ff5735cd3179ffafead385a625baa7962e1d0) )
	ROM_LOAD32_BYTE( "cw09a",   0x080001, 0x20000, CRC(4ba24af5) SHA1(9203c2639e04aaa09996339f11259750ff8129b9) )
	ROM_LOAD32_BYTE( "cw03a",   0x080002, 0x20000, CRC(3ca6f98e) SHA1(8526fe38d3b4c66e09049ba18651a9e7255d85d6) )
	ROM_LOAD32_BYTE( "cw11a",   0x080003, 0x20000, CRC(5d760392) SHA1(7bbda2880af4659c267193ce10ed887a1b54a981) )

	ROM_REGION( 0x100000, "sprites_h", 0)
	ROM_LOAD32_BYTE( "cw04a",   0x000000, 0x20000, CRC(f05f594d) SHA1(80effaa517b2154c013419e0bc05fd0797b74c8d) )
	ROM_LOAD32_BYTE( "cw12a",   0x000001, 0x20000, CRC(4ac07e8b) SHA1(f9de96fba39d5752d61b8f6be87fb605694624ed) )
	ROM_LOAD32_BYTE( "cw06a",   0x000002, 0x20000, CRC(f628edc9) SHA1(473f7ec28000e6bf72782c1c3f4afb5e021bd430) )
	ROM_LOAD32_BYTE( "cw14a",   0x000003, 0x20000, CRC(a9131f5f) SHA1(3a2059946984733e6939f3298f0db676e6a3301b) )
	ROM_LOAD32_BYTE( "cw05a",   0x080000, 0x20000, CRC(c8f5faa9) SHA1(f374531ffd645597eeb1440fd2cadb426fcd3d79) )
	ROM_LOAD32_BYTE( "cw13a",   0x080001, 0x20000, CRC(8091d381) SHA1(7faf068ce20b2877559f0335df55d61be13146b4) )
	ROM_LOAD32_BYTE( "cw07a",   0x080002, 0x20000, CRC(314579b5) SHA1(3c10ec490f7821a5b5412295232bbb104d0e4b83) )
	ROM_LOAD32_BYTE( "cw15a",   0x080003, 0x20000, CRC(7ed4b721) SHA1(b87865effeff77a9ea74354ef2b5911a5102a647) )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "cw27",   0x000000, 0x20000, CRC(2db48a9e) SHA1(16c307340d17cd3b5455ebcee681fbe0335dec58) )

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "cw30",   0x000000, 0x20000, CRC(331d0711) SHA1(82251fe1f1d36f079080943ab1fd04a60077c353) )
	ROM_LOAD( "cw29",   0x020000, 0x20000, CRC(64dd519c) SHA1(e23611fc2be896861997063546c3eb03527eaf8e) )
	ROM_LOAD( "cw28",   0x040000, 0x20000, CRC(3fc568ed) SHA1(91125c9deddc659449ca6791a847fe908c2818b2) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "cw24a",   0x000000, 0x20000, CRC(22600cba) SHA1(a1514fbe037942f1493a17eb0b7986949470cb22) )
	ROM_LOAD( "cw25a",   0x020000, 0x20000, CRC(372c6bc8) SHA1(d4875bf3bffecf338bebba3b8d6a791585556a06) )
ROM_END

ROM_START( bigfight )
	ROM_REGION16_BE( 0x200000, "master_rom", 0 ) /* 68000 main cpu */
	ROM_LOAD16_BYTE( "rom16.ic77",   0x000000, 0x40000, CRC(e7304ec8) SHA1(31a37e96bf963b349d36534bc5ebbf45e19ad00e) )
	ROM_LOAD16_BYTE( "rom17.ic98",   0x000001, 0x40000, CRC(4cf090f6) SHA1(9ae0274c890e829a90108ce316aff9665128c982) )
	ROM_COPY("master_rom",       0x000000, 0x080000, 0x080000 )
	ROM_COPY("master_rom",       0x100000, 0x080000, 0x080000 )
	ROM_COPY("master_rom",       0x180000, 0x080000, 0x080000 )

	ROM_REGION16_BE( 0x100000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "rom18.ic100",   0x000000, 0x40000, CRC(49df6207) SHA1(c4126f4542add11a3a3d236311c8787c24c98440) )
	ROM_LOAD16_BYTE( "rom19.ic102",   0x000001, 0x40000, CRC(c12aa9e9) SHA1(19cc7feaa97c6f5148ae8c0077174f96be684f05) )
	ROM_COPY("slave_rom",       0x000000, 0x080000, 0x080000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rom20.ic91",   0x000000, 0x10000, CRC(b3add091) SHA1(8a67bfff75c13fe4d9b89d30449199200d11cea7) )

	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASE00 )
	/* Filled in by both regions below */

	ROM_REGION( 0x200000, "sprites_l", 0 )
	ROM_LOAD32_BYTE( "rom0.ic26",   0x000000, 0x80000, CRC(a4a3c8d6) SHA1(b5365d9bc6068260c23ba9d5971c7c7d7cc07a97) )
	ROM_LOAD32_BYTE( "rom8.ic45",   0x000001, 0x80000, CRC(220956ed) SHA1(68e0ba1e850101b4cc2778819dfa76f04d88d2d6) )
	ROM_LOAD32_BYTE( "rom2.ic28",   0x000002, 0x80000, CRC(c4f6d243) SHA1(e23b241b5a40b332165a34e2f1bc4366973b2070) )
	ROM_LOAD32_BYTE( "rom10.ic47",  0x000003, 0x80000, CRC(0212d472) SHA1(5549461195fd7b6b43c0174462d7fe1a1bac24e9) )

	ROM_REGION( 0x200000, "sprites_h", 0 )
	ROM_LOAD32_BYTE( "rom4.ic30",   0x000000, 0x80000, CRC(999ff7e9) SHA1(a53b06ad084722d7a52fcf01c52967f68620e609) )
	ROM_LOAD32_BYTE( "rom12.ic49",  0x000001, 0x80000, CRC(cb4c1f0b) SHA1(32d64b78ed3d5971eb5d25be2c38e6f2c9048f74) )
	ROM_LOAD32_BYTE( "rom6.ic32",   0x000002, 0x80000, CRC(f70e2d47) SHA1(00517b5f3b2deb6f3f3bd12df421e63884c22b2e) )
	ROM_LOAD32_BYTE( "rom14.ic51",  0x000003, 0x80000, CRC(77430bc9) SHA1(0b1fd54ace84a9fb5b44d5600de8089a20bcbd47) )

	ROM_REGION( 0x20000, "cw_tileclut", 0 )
	ROM_LOAD( "rom21.ic128",   0x000000, 0x20000, CRC(da027dcf) SHA1(47d18a8a273fea72cb3ad3d58166fe38ca28a860) )

	ROM_REGION( 0x60000, "tilerom", 0 )
	ROM_LOAD( "rom24.ic73",   0x000000, 0x20000, CRC(c564185d) SHA1(e9b5fc10a5a5014735852c22db2a054d5787d8cb) )
	ROM_LOAD( "rom23.ic72",   0x020000, 0x20000, CRC(f8bb340b) SHA1(905a1ec778d6ed5c6f53d9d08cd105eed7e307ca) )
	ROM_LOAD( "rom22.ic71",   0x040000, 0x20000, CRC(fb505074) SHA1(b6d9b20be7c3e971e5a4392736f087e807b9c850) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "rom15.ic39",   0x000000, 0x40000, CRC(58d136e8) SHA1(4aa063c4b9b057cba4655ecbe44a87c8c411e3aa) )
ROM_END

/***************************************************************************/

void apache3_state::init_apache3()
{
	uint8_t *dst = memregion("sprites")->base();
	uint8_t *src1 = memregion("sprites_l")->base();
	uint8_t *src2 = memregion("sprites_h")->base();

	for (int i = 0; i < 0x100000; i += 32)
	{
		memcpy(dst,src1,32);
		src1+=32;
		dst+=32;
		memcpy(dst,src2,32);
		dst+=32;
		src2+=32;
	}

	// Copy sprite & palette data out of GFX rom area
	m_rom_sprite_lookup[0] = memregion("sprites_l")->base();
	m_rom_sprite_lookup[1] = memregion("sprites_h")->base();
	m_rom_clut[0] = memregion("sprites_l")->base()+ 0x100000 - 0x800;
	m_rom_clut[1] = memregion("sprites_h")->base()+ 0x100000 - 0x800;

	tatsumi_reset();

	m_apache3_rot_idx = 0;

	save_item(NAME(m_apache3_rot_idx));
	save_item(NAME(m_apache3_rotate_ctrl));

	// TODO: ym2151_set_port_write_handler for CT1/CT2 outputs
}

void roundup5_state::init_roundup5()
{
	uint8_t *dst = memregion("sprites")->base();
	uint8_t *src1 = memregion("sprites_l")->base();
	uint8_t *src2 = memregion("sprites_h")->base();

	for (int i = 0; i < 0xc0000; i += 32)
	{
		memcpy(dst,src1,32);
		src1+=32;
		dst+=32;
		memcpy(dst,src2,32);
		dst+=32;
		src2+=32;
	}

	// Copy sprite & palette data out of GFX rom area
	m_rom_sprite_lookup[0] = memregion("sprites_l")->base();
	m_rom_sprite_lookup[1] = memregion("sprites_h")->base();
	m_rom_clut[0] = memregion("sprites_l")->base()+ 0xc0000 - 0x800;
	m_rom_clut[1] = memregion("sprites_h")->base()+ 0xc0000 - 0x800;

	tatsumi_reset();
}

void cyclwarr_state::init_cyclwarr()
{
	uint8_t *dst = memregion("sprites")->base();
	uint8_t *src1 = memregion("sprites_l")->base();
	int len1 = memregion("sprites_l")->bytes();
	uint8_t *src2 = memregion("sprites_h")->base();
	int len2 = memregion("sprites_h")->bytes();

	for (int i = 0; i < len1; i += 32)
	{
		memcpy(dst,src1,32);
		src1+=32;
		dst+=32;
		memcpy(dst,src2,32);
		dst+=32;
		src2+=32;
	}

	// Copy sprite & palette data out of GFX rom area
	m_rom_sprite_lookup[0] = memregion("sprites_l")->base();
	m_rom_sprite_lookup[1] = memregion("sprites_h")->base();
	m_rom_clut[0] = memregion("sprites_l")->base() + len1 - 0x1000;
	m_rom_clut[1] = memregion("sprites_h")->base() + len2 - 0x1000;

	tatsumi_reset();
}

/***************************************************************************/

/* http://www.tatsu-mi.co.jp/game/trace/index.html */

/* ** 1987  grayout    - Gray Out (not dumped yet) */
GAME( 1988, apache3,   0,        apache3,   apache3,  apache3_state,  init_apache3,  ROT0, "Tatsumi", "Apache 3 (rev F)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rev F CPU code
GAME( 1988, apache3a,  apache3,  apache3,   apache3,  apache3_state,  init_apache3,  ROT0, "Tatsumi", "Apache 3 (rev E)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rev C & E CPU code
GAME( 1988, apache3b,  apache3,  apache3,   apache3,  apache3_state,  init_apache3,  ROT0, "Tatsumi (Kana Corporation license)", "Apache 3 (Kana Corporation license, rev G)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rev C & G CPU code
GAMEL(1989, roundup5,  0,        roundup5,  roundup5, roundup5_state, init_roundup5, ROT0, "Tatsumi", "Round Up 5 - Super Delta Force", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING, layout_roundup5 )
GAME( 1991, cyclwarr,  0,        cyclwarr,  cyclwarr, cyclwarr_state, init_cyclwarr, ROT0, "Tatsumi", "Cycle Warriors (rev C)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rev C & B CPU code
GAME( 1991, cyclwarra, cyclwarr, cyclwarr,  cyclwarb, cyclwarr_state, init_cyclwarr, ROT0, "Tatsumi", "Cycle Warriors (rev B)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // Rev B & A CPU code
GAME( 1992, bigfight,  0,        bigfight,  bigfight, cyclwarr_state, init_cyclwarr, ROT0, "Tatsumi", "Big Fight - Big Trouble In The Atlantic Ocean", MACHINE_IMPERFECT_GRAPHICS )
