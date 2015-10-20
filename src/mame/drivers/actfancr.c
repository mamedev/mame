// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Act Fancer (Japan)              FD (c) 1989 Data East Corporation
    Act Fancer (World)              FE (c) 1989 Data East Corporation
    Trio The Punch (World)          FC (c) 1989 Data East Corporation
    Trio The Punch (Japan)          FF (c) 1989 Data East Corporation

    The 'World' set has rom code FE, the 'Japan' set has rom code FD.

    Most Data East games give the Japanese version the earlier code, though
    there is no real difference between the sets.

    I believe the USA version of Act Fancer is called 'Out Fencer'

    Emulation by Bryan McPhail, mish@tendril.co.uk

    Update of the 03/04/2005
    Added Trio The Punch (World), this set will be the new parent.
    Fixed filenames in both Trio The Punch sets to match correct labels on the pcb.

    Update by Roberto Gandola, sophitia@email.it

*******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/h6280/h6280.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/actfancr.h"

/******************************************************************************/

WRITE8_MEMBER(actfancr_state::triothep_control_select_w)
{
	m_trio_control_select = data;
}

READ8_MEMBER(actfancr_state::triothep_control_r)
{
	switch (m_trio_control_select)
	{
		case 0: return ioport("P1")->read();
		case 1: return ioport("P2")->read();
		case 2: return ioport("DSW1")->read();
		case 3: return ioport("DSW2")->read();
		case 4: return ioport("SYSTEM")->read();    /* VBL */
	}

	return 0xff;
}

WRITE8_MEMBER(actfancr_state::actfancr_sound_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

/******************************************************************************/

WRITE8_MEMBER(actfancr_state::actfancr_buffer_spriteram_w)
{
	UINT8 *src = reinterpret_cast<UINT8 *>(memshare("spriteram")->ptr());
	// copy to a 16-bit region for our sprite draw code too
	for (int i=0;i<0x800/2;i++)
	{
		m_spriteram16[i] = src[i*2] | (src[(i*2)+1] <<8);
	}
}

static ADDRESS_MAP_START( actfan_map, AS_PROGRAM, 8, actfancr_state )
	AM_RANGE(0x000000, 0x02ffff) AM_ROM
	AM_RANGE(0x060000, 0x060007) AM_DEVWRITE("tilegen1", deco_bac06_device, pf_control0_8bit_w)
	AM_RANGE(0x060010, 0x06001f) AM_DEVWRITE("tilegen1", deco_bac06_device, pf_control1_8bit_swap_w)
	AM_RANGE(0x062000, 0x063fff) AM_DEVREADWRITE("tilegen1", deco_bac06_device, pf_data_8bit_swap_r, pf_data_8bit_swap_w)
	AM_RANGE(0x070000, 0x070007) AM_DEVWRITE("tilegen2", deco_bac06_device, pf_control0_8bit_w)
	AM_RANGE(0x070010, 0x07001f) AM_DEVWRITE("tilegen2", deco_bac06_device, pf_control1_8bit_swap_w)
	AM_RANGE(0x072000, 0x0727ff) AM_DEVREADWRITE("tilegen2", deco_bac06_device, pf_data_8bit_swap_r, pf_data_8bit_swap_w)
	AM_RANGE(0x100000, 0x1007ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x110000, 0x110001) AM_WRITE(actfancr_buffer_spriteram_w)
	AM_RANGE(0x120000, 0x1205ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x130000, 0x130000) AM_READ_PORT("P1")
	AM_RANGE(0x130001, 0x130001) AM_READ_PORT("P2")
	AM_RANGE(0x130002, 0x130002) AM_READ_PORT("DSW1")
	AM_RANGE(0x130003, 0x130003) AM_READ_PORT("DSW2")
	AM_RANGE(0x140000, 0x140001) AM_READ_PORT("SYSTEM") /* VBL */
	AM_RANGE(0x150000, 0x150001) AM_WRITE(actfancr_sound_w)
	AM_RANGE(0x1f0000, 0x1f3fff) AM_RAM AM_SHARE("main_ram") /* Main ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( triothep_map, AS_PROGRAM, 8, actfancr_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x040007) AM_DEVWRITE("tilegen2", deco_bac06_device, pf_control0_8bit_w)
	AM_RANGE(0x040010, 0x04001f) AM_DEVWRITE("tilegen2", deco_bac06_device, pf_control1_8bit_swap_w)
	AM_RANGE(0x044000, 0x045fff) AM_DEVREADWRITE("tilegen2", deco_bac06_device, pf_data_8bit_swap_r, pf_data_8bit_swap_w)
	AM_RANGE(0x046400, 0x0467ff) AM_DEVREADWRITE("tilegen2", deco_bac06_device, pf_rowscroll_8bit_swap_r, pf_rowscroll_8bit_swap_w)
	AM_RANGE(0x060000, 0x060007) AM_DEVWRITE("tilegen1", deco_bac06_device, pf_control0_8bit_w)
	AM_RANGE(0x060010, 0x06001f) AM_DEVWRITE("tilegen1", deco_bac06_device, pf_control1_8bit_swap_w)
	AM_RANGE(0x064000, 0x0647ff) AM_DEVREADWRITE("tilegen1", deco_bac06_device, pf_data_8bit_swap_r, pf_data_8bit_swap_w)
	AM_RANGE(0x066400, 0x0667ff) AM_DEVREADWRITE("tilegen1", deco_bac06_device, pf_rowscroll_8bit_swap_r, pf_rowscroll_8bit_swap_w)
	AM_RANGE(0x100000, 0x100001) AM_WRITE(actfancr_sound_w)
	AM_RANGE(0x110000, 0x110001) AM_WRITE(actfancr_buffer_spriteram_w)
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x130000, 0x1305ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x140000, 0x140001) AM_READNOP /* Value doesn't matter */
	AM_RANGE(0x1f0000, 0x1f3fff) AM_RAM AM_SHARE("main_ram") /* Main ram */
	AM_RANGE(0x1ff000, 0x1ff001) AM_READWRITE(triothep_control_r, triothep_control_select_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE("maincpu", h6280_device, irq_status_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( dec0_s_map, AS_PROGRAM, 8, actfancr_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0801) AM_DEVWRITE("ym1", ym2203_device, write)
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym2", ym3812_device, write)
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x3800, 0x3800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( actfancr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "100 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "800000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */
INPUT_PORTS_END

static INPUT_PORTS_START( triothep )
	PORT_INCLUDE( actfancr )

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x03, "12" )
	PORT_DIPSETTING(    0x02, "14" )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty (Time)" )     PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "Easy (130)" )
	PORT_DIPSETTING(    0x0c, "Normal (100)" )
	PORT_DIPSETTING(    0x04, "Hard (70)" )
	PORT_DIPSETTING(    0x00, "Hardest (60)" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Lives" )           PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout chars =
{
	8,8,    /* 8*8 chars */
	4096,
	4,      /* 4 bits per pixel  */
	{ 0x08000*8, 0x18000*8, 0x00000*8, 0x10000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tiles =
{
	16,16,  /* 16*16 sprites */
	2048,
	4,
	{ 0, 0x10000*8, 0x20000*8,0x30000*8 },  /* plane offset */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout sprites =
{
	16,16,  /* 16*16 sprites */
	2048+1024,
	4,
	{ 0, 0x18000*8, 0x30000*8, 0x48000*8 }, /* plane offset */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( actfan )
	GFXDECODE_ENTRY( "gfx1", 0, chars,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprites,   512, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles,     256, 16 )
GFXDECODE_END

static GFXDECODE_START( triothep )
	GFXDECODE_ENTRY( "gfx1", 0, chars,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprites,   256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles,     512, 16 )
GFXDECODE_END

/******************************************************************************/

MACHINE_START_MEMBER(actfancr_state,triothep)
{
	save_item(NAME(m_trio_control_select));
}

MACHINE_RESET_MEMBER(actfancr_state,triothep)
{
	m_trio_control_select = 0;
}

/******************************************************************************/

static MACHINE_CONFIG_START( actfancr, actfancr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, 21477200/3) /* Should be accurate */
	MCFG_CPU_PROGRAM_MAP(actfan_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", actfancr_state,  irq0_line_hold) /* VBL */

	MCFG_CPU_ADD("audiocpu",M6502, 1500000) /* Should be accurate */
	MCFG_CPU_PROGRAM_MAP(dec0_s_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(actfancr_state, screen_update_actfancr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", actfan)

	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("tilegen1", DECO_BAC06, 0)
	deco_bac06_device::set_gfx_region_wide(*device,2,2,2);
	MCFG_DECO_BAC06_GFXDECODE("gfxdecode")
	MCFG_DEVICE_ADD("tilegen2", DECO_BAC06, 0)
	deco_bac06_device::set_gfx_region_wide(*device,0,0,0);
	MCFG_DECO_BAC06_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_MXC06, 0)
	deco_mxc06_device::set_gfx_region(*device, 1);
	MCFG_DECO_MXC06_GFXDECODE("gfxdecode")
	MCFG_DECO_MXC06_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1500000)
	MCFG_SOUND_ROUTE(0, "mono", 0.90)
	MCFG_SOUND_ROUTE(1, "mono", 0.90)
	MCFG_SOUND_ROUTE(2, "mono", 0.90)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("ym2", YM3812, 3000000)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", M6502_IRQ_LINE))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	MCFG_OKIM6295_ADD("oki", 1024188, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.85)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( triothep, actfancr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",H6280,XTAL_21_4772MHz/3) /* XIN=21.4772Mhz, verified on pcb */
	MCFG_CPU_PROGRAM_MAP(triothep_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", actfancr_state,  irq0_line_hold) /* VBL */

	MCFG_CPU_ADD("audiocpu",M6502, XTAL_12MHz/8) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(dec0_s_map)

	MCFG_MACHINE_START_OVERRIDE(actfancr_state,triothep)
	MCFG_MACHINE_RESET_OVERRIDE(actfancr_state,triothep)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(actfancr_state, screen_update_actfancr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", triothep)

	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("tilegen1", DECO_BAC06, 0)
	deco_bac06_device::set_gfx_region_wide(*device,2,2,0);
	MCFG_DECO_BAC06_GFXDECODE("gfxdecode")
	MCFG_DEVICE_ADD("tilegen2", DECO_BAC06, 0)
	deco_bac06_device::set_gfx_region_wide(*device,0,0,0);
	MCFG_DECO_BAC06_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_MXC06, 0)
	deco_mxc06_device::set_gfx_region(*device, 1);
	MCFG_DECO_MXC06_GFXDECODE("gfxdecode")
	MCFG_DECO_MXC06_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_12MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(0, "mono", 0.90)
	MCFG_SOUND_ROUTE(1, "mono", 0.90)
	MCFG_SOUND_ROUTE(2, "mono", 0.90)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("ym2", YM3812, XTAL_12MHz/4) /* verified on pcb */
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", M6502_IRQ_LINE))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	MCFG_OKIM6295_ADD("oki", XTAL_1_056MHz, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.85)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( actfancr )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Need to allow full RAM allocation for now */
	ROM_LOAD( "fe08-2.bin", 0x00000, 0x10000, CRC(0d36fbfa) SHA1(cef5cfd053beac5ca2ac52421024c316bdbfba42) )
	ROM_LOAD( "fe09-2.bin", 0x10000, 0x10000, CRC(27ce2bb1) SHA1(52a423dfc2bba7b3330d1a10f4149ae6eeb9198c) )
	ROM_LOAD( "10",   0x20000, 0x10000, CRC(cabad137) SHA1(41ca833649671a29e9395968cde2be8137a9ff0a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "17-1", 0x08000, 0x8000, CRC(289ad106) SHA1(cf1b32ac41d3d92860fab04d82a08efe57b6ecf3) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "15", 0x00000, 0x10000, CRC(a1baf21e) SHA1(b85cf9180efae6c95cc0310064b52a78e591826a) ) /* Chars */
	ROM_LOAD( "16", 0x10000, 0x10000, CRC(22e64730) SHA1(f1376c6e2c9d021eca7ccee3daab00593ba724b6) )

	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_LOAD( "02", 0x00000, 0x10000, CRC(b1db0efc) SHA1(a7bd7748ea37f473499ba5bf8ab4995b9240ff48) ) /* Sprites */
	ROM_LOAD( "03", 0x10000, 0x08000, CRC(f313e04f) SHA1(fe69758910d38f742971c1027fc8f498c88262b1) )
	ROM_LOAD( "06", 0x18000, 0x10000, CRC(8cb6dd87) SHA1(fab4fe76d2426c906a9070cbf7ce81200ba27ff6) )
	ROM_LOAD( "07", 0x28000, 0x08000, CRC(dd345def) SHA1(44fbf9da636a4e18c421fdc0a1eadc3c7ba66068) )
	ROM_LOAD( "00", 0x30000, 0x10000, CRC(d50a9550) SHA1(b366826e0df11ab6b97e2cb0e813432e95f9513d) )
	ROM_LOAD( "01", 0x40000, 0x08000, CRC(34935e93) SHA1(8cd02a72659f6cb0536b54c1c8b34dae818fbfdc) )
	ROM_LOAD( "04", 0x48000, 0x10000, CRC(bcf41795) SHA1(1d18afc974ac43fe6194e2840bbb2e93cd2b6cff) )
	ROM_LOAD( "05", 0x58000, 0x08000, CRC(d38b94aa) SHA1(773d01427744fda9104f673d2b4183a0f7471a39) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "14", 0x00000, 0x10000, CRC(d6457420) SHA1(d03d2e944e768b297ec0c3389320c42bc0259d00) ) /* Tiles */
	ROM_LOAD( "12", 0x10000, 0x10000, CRC(08787b7a) SHA1(23b10b75c4cbff8effadf4c6ed15d90b87648ce9) )
	ROM_LOAD( "13", 0x20000, 0x10000, CRC(c30c37dc) SHA1(0f7a325738eafa85239497e2b97aa51a6f2ffc4d) )
	ROM_LOAD( "11", 0x30000, 0x10000, CRC(1f006d9f) SHA1(74bc2d4d022ad7c65be781f974919262cacb4b64) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM sounds */
	ROM_LOAD( "18",   0x00000, 0x10000, CRC(5c55b242) SHA1(62ba60b2f02483875da12aefe849f7e2fd137ef1) )
ROM_END

ROM_START( actfancr1 )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Need to allow full RAM allocation for now */
	ROM_LOAD( "08-1", 0x00000, 0x10000, CRC(3bf214a4) SHA1(f7513672b2292d3acb4332b392695888bf6560a5) )
	ROM_LOAD( "09-1", 0x10000, 0x10000, CRC(13ae78d5) SHA1(eba77d3dbfe273e18c7fa9c0ca305ac2468f9381) )
	ROM_LOAD( "10",   0x20000, 0x10000, CRC(cabad137) SHA1(41ca833649671a29e9395968cde2be8137a9ff0a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "17-1", 0x08000, 0x8000, CRC(289ad106) SHA1(cf1b32ac41d3d92860fab04d82a08efe57b6ecf3) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "15", 0x00000, 0x10000, CRC(a1baf21e) SHA1(b85cf9180efae6c95cc0310064b52a78e591826a) ) /* Chars */
	ROM_LOAD( "16", 0x10000, 0x10000, CRC(22e64730) SHA1(f1376c6e2c9d021eca7ccee3daab00593ba724b6) )

	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_LOAD( "02", 0x00000, 0x10000, CRC(b1db0efc) SHA1(a7bd7748ea37f473499ba5bf8ab4995b9240ff48) ) /* Sprites */
	ROM_LOAD( "03", 0x10000, 0x08000, CRC(f313e04f) SHA1(fe69758910d38f742971c1027fc8f498c88262b1) )
	ROM_LOAD( "06", 0x18000, 0x10000, CRC(8cb6dd87) SHA1(fab4fe76d2426c906a9070cbf7ce81200ba27ff6) )
	ROM_LOAD( "07", 0x28000, 0x08000, CRC(dd345def) SHA1(44fbf9da636a4e18c421fdc0a1eadc3c7ba66068) )
	ROM_LOAD( "00", 0x30000, 0x10000, CRC(d50a9550) SHA1(b366826e0df11ab6b97e2cb0e813432e95f9513d) )
	ROM_LOAD( "01", 0x40000, 0x08000, CRC(34935e93) SHA1(8cd02a72659f6cb0536b54c1c8b34dae818fbfdc) )
	ROM_LOAD( "04", 0x48000, 0x10000, CRC(bcf41795) SHA1(1d18afc974ac43fe6194e2840bbb2e93cd2b6cff) )
	ROM_LOAD( "05", 0x58000, 0x08000, CRC(d38b94aa) SHA1(773d01427744fda9104f673d2b4183a0f7471a39) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "14", 0x00000, 0x10000, CRC(d6457420) SHA1(d03d2e944e768b297ec0c3389320c42bc0259d00) ) /* Tiles */
	ROM_LOAD( "12", 0x10000, 0x10000, CRC(08787b7a) SHA1(23b10b75c4cbff8effadf4c6ed15d90b87648ce9) )
	ROM_LOAD( "13", 0x20000, 0x10000, CRC(c30c37dc) SHA1(0f7a325738eafa85239497e2b97aa51a6f2ffc4d) )
	ROM_LOAD( "11", 0x30000, 0x10000, CRC(1f006d9f) SHA1(74bc2d4d022ad7c65be781f974919262cacb4b64) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM sounds */
	ROM_LOAD( "18",   0x00000, 0x10000, CRC(5c55b242) SHA1(62ba60b2f02483875da12aefe849f7e2fd137ef1) )
ROM_END

ROM_START( actfancrj )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Need to allow full RAM allocation for now */
	ROM_LOAD( "fd08-1.bin", 0x00000, 0x10000, CRC(69004b60) SHA1(7c6b876ca04377d2aa2d3c3f19d8e6cc7345363d) )
	ROM_LOAD( "fd09-1.bin", 0x10000, 0x10000, CRC(a455ae3e) SHA1(960798271c8370c1c4ffce2a453f59d7a301c9f9) )
	ROM_LOAD( "10",   0x20000, 0x10000, CRC(cabad137) SHA1(41ca833649671a29e9395968cde2be8137a9ff0a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "17-1", 0x08000, 0x8000, CRC(289ad106) SHA1(cf1b32ac41d3d92860fab04d82a08efe57b6ecf3) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "15", 0x00000, 0x10000, CRC(a1baf21e) SHA1(b85cf9180efae6c95cc0310064b52a78e591826a) ) /* Chars */
	ROM_LOAD( "16", 0x10000, 0x10000, CRC(22e64730) SHA1(f1376c6e2c9d021eca7ccee3daab00593ba724b6) )

	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_LOAD( "02", 0x00000, 0x10000, CRC(b1db0efc) SHA1(a7bd7748ea37f473499ba5bf8ab4995b9240ff48) ) /* Sprites */
	ROM_LOAD( "03", 0x10000, 0x08000, CRC(f313e04f) SHA1(fe69758910d38f742971c1027fc8f498c88262b1) )
	ROM_LOAD( "06", 0x18000, 0x10000, CRC(8cb6dd87) SHA1(fab4fe76d2426c906a9070cbf7ce81200ba27ff6) )
	ROM_LOAD( "07", 0x28000, 0x08000, CRC(dd345def) SHA1(44fbf9da636a4e18c421fdc0a1eadc3c7ba66068) )
	ROM_LOAD( "00", 0x30000, 0x10000, CRC(d50a9550) SHA1(b366826e0df11ab6b97e2cb0e813432e95f9513d) )
	ROM_LOAD( "01", 0x40000, 0x08000, CRC(34935e93) SHA1(8cd02a72659f6cb0536b54c1c8b34dae818fbfdc) )
	ROM_LOAD( "04", 0x48000, 0x10000, CRC(bcf41795) SHA1(1d18afc974ac43fe6194e2840bbb2e93cd2b6cff) )
	ROM_LOAD( "05", 0x58000, 0x08000, CRC(d38b94aa) SHA1(773d01427744fda9104f673d2b4183a0f7471a39) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "14", 0x00000, 0x10000, CRC(d6457420) SHA1(d03d2e944e768b297ec0c3389320c42bc0259d00) ) /* Tiles */
	ROM_LOAD( "12", 0x10000, 0x10000, CRC(08787b7a) SHA1(23b10b75c4cbff8effadf4c6ed15d90b87648ce9) )
	ROM_LOAD( "13", 0x20000, 0x10000, CRC(c30c37dc) SHA1(0f7a325738eafa85239497e2b97aa51a6f2ffc4d) )
	ROM_LOAD( "11", 0x30000, 0x10000, CRC(1f006d9f) SHA1(74bc2d4d022ad7c65be781f974919262cacb4b64) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM sounds */
	ROM_LOAD( "18",   0x00000, 0x10000, CRC(5c55b242) SHA1(62ba60b2f02483875da12aefe849f7e2fd137ef1) )
ROM_END

ROM_START( triothep )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Need to allow full RAM allocation for now */
	ROM_LOAD( "fg-16.bin", 0x00000, 0x20000, CRC(7238355a) SHA1(4ac6c3fd808e7c94025972fdb45956bd707ec89f) )
	ROM_LOAD( "fg-15.bin", 0x20000, 0x10000, CRC(1c0551ab) SHA1(1f90f80db44d92af4b233bc16cb1023db2797e8a) )
	ROM_LOAD( "fg-14.bin", 0x30000, 0x10000, CRC(4ba7de4a) SHA1(bf552fa33746f3d27f9b193424a38fef58fe0765) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "fg-18.bin", 0x00000, 0x10000, CRC(9de9ee63) SHA1(c91b824b9a791cb90365d45c8e1b69e67f7d065f) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "fg-12.bin", 0x00000, 0x10000, CRC(15fb49f2) SHA1(a81ff1dbc813ab9b37edb832e01aab9a9a3ed5a1) ) /* Chars */
	ROM_LOAD( "fg-13.bin", 0x10000, 0x10000, CRC(e20c9623) SHA1(b5a58599a016378f34217396212f81ede9272598) )

	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_LOAD( "fg-11.bin", 0x00000, 0x10000, CRC(1143ebd7) SHA1(0ef2cf40f852bf0842beeb9727508e28437ab54b) ) /* Sprites */
	ROM_LOAD( "fg-10.bin", 0x10000, 0x08000, CRC(4b6b477a) SHA1(77486e0ff957cbfdae16d2b5977e95b7a7ced948) )
	ROM_LOAD( "fg-09.bin", 0x18000, 0x10000, CRC(6bf6c803) SHA1(c16fd4b7e1e86db48c6e78a4b5dcd42e8269b465) )
	ROM_LOAD( "fg-08.bin", 0x28000, 0x08000, CRC(1391e445) SHA1(bd53a969567bb5a46a35bd02e84bbb58c446a0a2) )
	ROM_LOAD( "fg-03.bin", 0x30000, 0x10000, CRC(3d3ca9ad) SHA1(de6532063500a4ddccdecfca1024f03a1fbb78f7) )
	ROM_LOAD( "fg-02.bin", 0x40000, 0x08000, CRC(6b9d24ce) SHA1(9d6d52e742fc37d83682291f918f3348395f0cd8) )
	ROM_LOAD( "fg-01.bin", 0x48000, 0x10000, CRC(4987f7ac) SHA1(e8e81b15f6b6c8597d34eef3cabb89b90d3ae7f5) )
	ROM_LOAD( "fg-00.bin", 0x58000, 0x08000, CRC(41232442) SHA1(1c10a4f5607e41d6239cb478ed7355963ad6b2d0) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "fg-04.bin", 0x00000, 0x10000, CRC(7cea3c87) SHA1(b58156140a75f88ee6ec97ca7cdc02619ec51726) ) /* Tiles */
	ROM_LOAD( "fg-06.bin", 0x10000, 0x10000, CRC(5e7f3e8f) SHA1(c92ec281b3985b442957f7d9237eb38a6d621cd4) )
	ROM_LOAD( "fg-05.bin", 0x20000, 0x10000, CRC(8bb13f05) SHA1(f524cb0a38d0025c93124fc329d913e000155e9b) )
	ROM_LOAD( "fg-07.bin", 0x30000, 0x10000, CRC(0d7affc3) SHA1(59f9fbf13216aaf67c7d1ad3a11a1738c4afd9e5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM sounds */
	ROM_LOAD( "fg-17.bin", 0x00000, 0x10000, CRC(f0ab0d05) SHA1(29d3ab513a8d46a1cb70f5333fa56bb787a58288) )
ROM_END

/* All roms are FF even the ones matching the parent FG roms */
ROM_START( triothepj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Need to allow full RAM allocation for now */
	ROM_LOAD( "ff-16.bin", 0x00000, 0x20000, CRC(84d7e1b6) SHA1(28381d2e1f6d22a959383eb2e8d73f2e03f4d39f) )
	ROM_LOAD( "ff-15.bin", 0x20000, 0x10000, CRC(6eada47c) SHA1(98fc4e93c47bc42ea7c20e8ac994b117cd7cb5a5) )
	ROM_LOAD( "ff-14.bin", 0x30000, 0x10000, CRC(4ba7de4a) SHA1(bf552fa33746f3d27f9b193424a38fef58fe0765) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "ff-18.bin", 0x00000, 0x10000, CRC(9de9ee63) SHA1(c91b824b9a791cb90365d45c8e1b69e67f7d065f) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "ff-12.bin", 0x00000, 0x10000, CRC(15fb49f2) SHA1(a81ff1dbc813ab9b37edb832e01aab9a9a3ed5a1) ) /* Chars */
	ROM_LOAD( "ff-13.bin", 0x10000, 0x10000, CRC(e20c9623) SHA1(b5a58599a016378f34217396212f81ede9272598) )

	ROM_REGION( 0x60000, "gfx2", 0 )
	ROM_LOAD( "ff-11.bin", 0x00000, 0x10000, CRC(19e885c7) SHA1(694f0aa4c1c976320d985ee50bb59c1894b853ed) ) /* Sprites */
	ROM_LOAD( "ff-10.bin", 0x10000, 0x08000, CRC(4b6b477a) SHA1(77486e0ff957cbfdae16d2b5977e95b7a7ced948) )
	ROM_LOAD( "ff-09.bin", 0x18000, 0x10000, CRC(79c6bc0e) SHA1(d4bf195f6114103d2eb68f3aaf65d4044947f600) )
	ROM_LOAD( "ff-08.bin", 0x28000, 0x08000, CRC(1391e445) SHA1(bd53a969567bb5a46a35bd02e84bbb58c446a0a2) )
	ROM_LOAD( "ff-03.bin", 0x30000, 0x10000, CRC(b36ad42d) SHA1(9d72cbb0904e82271e4835d668b133f17dec8255) )
	ROM_LOAD( "ff-02.bin", 0x40000, 0x08000, CRC(6b9d24ce) SHA1(9d6d52e742fc37d83682291f918f3348395f0cd8) )
	ROM_LOAD( "ff-01.bin", 0x48000, 0x10000, CRC(68d80a66) SHA1(526ed8c920915877f5ee0519c9c8eee7e5580c54) )
	ROM_LOAD( "ff-00.bin", 0x58000, 0x08000, CRC(41232442) SHA1(1c10a4f5607e41d6239cb478ed7355963ad6b2d0) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ff-04.bin", 0x00000, 0x10000, CRC(7cea3c87) SHA1(b58156140a75f88ee6ec97ca7cdc02619ec51726) ) /* Tiles */
	ROM_LOAD( "ff-06.bin", 0x10000, 0x10000, CRC(5e7f3e8f) SHA1(c92ec281b3985b442957f7d9237eb38a6d621cd4) )
	ROM_LOAD( "ff-05.bin", 0x20000, 0x10000, CRC(8bb13f05) SHA1(f524cb0a38d0025c93124fc329d913e000155e9b) )
	ROM_LOAD( "ff-07.bin", 0x30000, 0x10000, CRC(0d7affc3) SHA1(59f9fbf13216aaf67c7d1ad3a11a1738c4afd9e5) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM sounds */
	ROM_LOAD( "ff-17.bin", 0x00000, 0x10000, CRC(f0ab0d05) SHA1(29d3ab513a8d46a1cb70f5333fa56bb787a58288) )
ROM_END

/******************************************************************************/

GAME( 1989, actfancr, 0,        actfancr, actfancr, driver_device, 0, ROT0, "Data East Corporation", "Act-Fancer Cybernetick Hyper Weapon (World revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, actfancr1,actfancr, actfancr, actfancr, driver_device, 0, ROT0, "Data East Corporation", "Act-Fancer Cybernetick Hyper Weapon (World revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, actfancrj,actfancr, actfancr, actfancr, driver_device, 0, ROT0, "Data East Corporation", "Act-Fancer Cybernetick Hyper Weapon (Japan revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, triothep, 0,        triothep, triothep, driver_device, 0, ROT0, "Data East Corporation", "Trio The Punch - Never Forget Me... (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, triothepj,triothep, triothep, triothep, driver_device, 0, ROT0, "Data East Corporation", "Trio The Punch - Never Forget Me... (Japan)", MACHINE_SUPPORTS_SAVE )
