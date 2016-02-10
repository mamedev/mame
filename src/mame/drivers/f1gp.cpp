// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    F-1 Grand Prix       (c) 1991 Video System Co.

    driver by Nicola Salmoria

    Notes:
    - The ROZ layer generator is a Konami 053936.
    - f1gp2's hardware is very similar to Lethal Crash Race, main difference
      being an extra 68000.

    TODO:
    - Hook up link for Multi Player game mode. Currently will show ID CHECK
      ERROR then hang.

    f1gp:
    - gfxctrl register not understood - handling of fg/sprite priority to fix
      "continue" screen is just a kludge.
    f1gp2:
    - sprite lag noticeable in the animation at the end of a race (the wheels
      of the car are sprites while the car is the fg tilemap)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"

#include "sound/2610intf.h"
#include "sound/okim6295.h"
#include "includes/f1gp.h"


WRITE8_MEMBER(f1gp_state::f1gp_sh_bankswitch_w)
{
	m_z80bank->set_entry(data & 0x01);
}


WRITE16_MEMBER(f1gp_state::sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_pending_command = 1;
		soundlatch_byte_w(space, offset, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

READ16_MEMBER(f1gp_state::command_pending_r)
{
	return (m_pending_command ? 0xff : 0);
}

WRITE8_MEMBER(f1gp_state::pending_command_clear_w)
{
	m_pending_command = 0;
}


static ADDRESS_MAP_START( f1gp_cpu1_map, AS_PROGRAM, 16, f1gp_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x2fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xa00000, 0xbfffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0xc00000, 0xc3ffff) AM_READWRITE(f1gp_zoomdata_r, f1gp_zoomdata_w)
	AM_RANGE(0xd00000, 0xd01fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w) AM_SHARE("rozvideoram")
	AM_RANGE(0xd02000, 0xd03fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w)                            /* mirror */
	AM_RANGE(0xd04000, 0xd05fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w)                            /* mirror */
	AM_RANGE(0xd06000, 0xd07fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w)                            /* mirror */
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_SHARE("spr1cgram")               // SPR-1 CG RAM
	AM_RANGE(0xe04000, 0xe07fff) AM_RAM AM_SHARE("spr2cgram")               // SPR-2 CG RAM
	AM_RANGE(0xf00000, 0xf003ff) AM_RAM AM_SHARE("spr1vram")                                // SPR-1 VRAM
	AM_RANGE(0xf10000, 0xf103ff) AM_RAM AM_SHARE("spr2vram")                                // SPR-2 VRAM
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM                                                         // WORK RAM-1
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM AM_SHARE("sharedram")       // DUAL RAM
	AM_RANGE(0xffd000, 0xffdfff) AM_RAM_WRITE(f1gp_fgvideoram_w) AM_SHARE("fgvideoram")         // CHARACTER
	AM_RANGE(0xffe000, 0xffefff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // PALETTE
	AM_RANGE(0xfff000, 0xfff001) AM_READ_PORT("INPUTS")
	AM_RANGE(0xfff000, 0xfff001) AM_WRITE(f1gp_gfxctrl_w)
//  AM_RANGE(0xfff002, 0xfff003)    analog wheel?
	AM_RANGE(0xfff004, 0xfff005) AM_READ_PORT("DSW1")
	AM_RANGE(0xfff002, 0xfff005) AM_WRITE(f1gp_fgscroll_w)
	AM_RANGE(0xfff006, 0xfff007) AM_READ_PORT("DSW2")
	AM_RANGE(0xfff008, 0xfff009) AM_READ(command_pending_r)
	AM_RANGE(0xfff008, 0xfff009) AM_WRITE(sound_command_w)
	AM_RANGE(0xfff040, 0xfff05f) AM_DEVWRITE("k053936", k053936_device, ctrl_w)
	AM_RANGE(0xfff050, 0xfff051) AM_READ_PORT("DSW3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( f1gp2_cpu1_map, AS_PROGRAM, 16, f1gp_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x2fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xa00000, 0xa07fff) AM_RAM AM_SHARE("sprcgram")                                    // SPR-1 CG RAM + SPR-2 CG RAM
	AM_RANGE(0xd00000, 0xd01fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w) AM_SHARE("rozvideoram")   // BACK VRAM
	AM_RANGE(0xe00000, 0xe00fff) AM_RAM AM_SHARE("spritelist")                          // not checked + SPR-1 VRAM + SPR-2 VRAM
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM                                                             // WORK RAM-1
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM AM_SHARE("sharedram")           // DUAL RAM
	AM_RANGE(0xffd000, 0xffdfff) AM_RAM_WRITE(f1gp_fgvideoram_w) AM_SHARE("fgvideoram")             // CHARACTER
	AM_RANGE(0xffe000, 0xffefff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")            // PALETTE
	AM_RANGE(0xfff000, 0xfff001) AM_READ_PORT("INPUTS") AM_WRITE(f1gp2_gfxctrl_w)
//  AM_RANGE(0xfff002, 0xfff003)    analog wheel?
	AM_RANGE(0xfff004, 0xfff005) AM_READ_PORT("DSW1")
	AM_RANGE(0xfff006, 0xfff007) AM_READ_PORT("DSW2")
	AM_RANGE(0xfff008, 0xfff009) AM_READWRITE(command_pending_r, sound_command_w)
	AM_RANGE(0xfff00a, 0xfff00b) AM_READ_PORT("DSW3")
	AM_RANGE(0xfff020, 0xfff03f) AM_DEVWRITE("k053936", k053936_device, ctrl_w)
	AM_RANGE(0xfff044, 0xfff047) AM_WRITE(f1gp_fgscroll_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( f1gp_cpu2_map, AS_PROGRAM, 16, f1gp_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM  AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, f1gp_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, f1gp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(f1gp_sh_bankswitch_w) // f1gp
	AM_RANGE(0x0c, 0x0c) AM_WRITE(f1gp_sh_bankswitch_w) // f1gp2
	AM_RANGE(0x14, 0x14) AM_READ(soundlatch_byte_r) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x18, 0x1b) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
ADDRESS_MAP_END

WRITE16_MEMBER(f1gp_state::f1gpb_misc_w)
{
	/*
	static int old=-1;
	static int old_bank = -1;
	int new_bank = (data & 0xf0) >> 4; //wrong!

	if(old_bank != new_bank && new_bank < 5)
	{
	    // oki banking
	    UINT8 *src = memregion("oki")->base() + 0x40000 + 0x10000 * new_bank;
	    UINT8 *dst = memregion("oki")->base() + 0x30000;
	    memcpy(dst, src, 0x10000);

	    old_bank = new_bank;
	}

	//data & 0x80 toggles

	if((data & 0x7f) != old)
	    printf("misc = %X\n",old=data & 0x7f);

	*/
}

static ADDRESS_MAP_START( f1gpb_cpu1_map, AS_PROGRAM, 16, f1gp_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x2fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xa00000, 0xbfffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0x800000, 0x801fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc00000, 0xc3ffff) AM_READWRITE(f1gp_zoomdata_r, f1gp_zoomdata_w)
	AM_RANGE(0xd00000, 0xd01fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w) AM_SHARE("rozvideoram")
	AM_RANGE(0xd02000, 0xd03fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w)   /* mirror */
	AM_RANGE(0xd04000, 0xd05fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w)   /* mirror */
	AM_RANGE(0xd06000, 0xd07fff) AM_READWRITE(f1gp_rozvideoram_r, f1gp_rozvideoram_w)   /* mirror */
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM //unused
	AM_RANGE(0xe04000, 0xe07fff) AM_RAM //unused
	AM_RANGE(0xf00000, 0xf003ff) AM_RAM //unused
	AM_RANGE(0xf10000, 0xf103ff) AM_RAM //unused
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0xffd000, 0xffdfff) AM_RAM_WRITE(f1gp_fgvideoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0xffe000, 0xffefff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xfff000, 0xfff001) AM_READ_PORT("INPUTS")
	AM_RANGE(0xfff004, 0xfff005) AM_READ_PORT("DSW1")
	AM_RANGE(0xfff006, 0xfff007) AM_READ_PORT("DSW2")
	AM_RANGE(0xfff008, 0xfff009) AM_READNOP //?
	AM_RANGE(0xfff006, 0xfff007) AM_WRITENOP
	AM_RANGE(0xfff00a, 0xfff00b) AM_RAM AM_SHARE("fgregs")
	AM_RANGE(0xfff00e, 0xfff00f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0xfff00c, 0xfff00d) AM_WRITE(f1gpb_misc_w)
	AM_RANGE(0xfff010, 0xfff011) AM_WRITENOP
	AM_RANGE(0xfff020, 0xfff023) AM_RAM //?
	AM_RANGE(0xfff050, 0xfff051) AM_READ_PORT("DSW3")
	AM_RANGE(0xfff800, 0xfff809) AM_RAM AM_SHARE("rozregs")
ADDRESS_MAP_END

static ADDRESS_MAP_START( f1gpb_cpu2_map, AS_PROGRAM, 16, f1gp_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0xfff030, 0xfff031) AM_NOP //?
ADDRESS_MAP_END

static INPUT_PORTS_START( f1gp )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW1:1" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coin_A ) )       PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coin_B ) )       PORT_CONDITION("DSW1",0x8000,EQUALS,0x8000) PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x7e00, 0x7e00, DEF_STR( Coinage ) )      PORT_CONDITION("DSW1",0x8000,NOTEQUALS,0x8000) PORT_DIPLOCATION("SW1:2,3,4,5,6,7")
	PORT_DIPSETTING(      0x7e00, "2 to Start, 1 to Continue" )
	PORT_DIPNAME( 0x8000, 0x8000, "Continue Coin" )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, "Normal Coinage" )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Game Mode" )         PORT_DIPLOCATION("SW2:3") /* Setting to Multiple results in "ID CHECK   ERROR" then hang */
	PORT_DIPSETTING(      0x0004, DEF_STR( Single ) )
	PORT_DIPSETTING(      0x0000, "Multiple" )
	PORT_DIPNAME( 0x0008, 0x0008, "Multi Player Mode" )     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, "Single or Multi Player" )        PORT_CONDITION("DSW1",0x0004,EQUALS,0x0000)
	PORT_DIPSETTING(      0x0000, "Multi Player Game Only" )        PORT_CONDITION("DSW1",0x0004,EQUALS,0x0000)
	PORT_DIPSETTING(      0x0008, "Multi Player Off" )          PORT_CONDITION("DSW1",0x0004,NOTEQUALS,0x0000)
	PORT_DIPSETTING(      0x0000, "Multi Player Off" )          PORT_CONDITION("DSW1",0x0004,NOTEQUALS,0x0000)
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )        /* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW3:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW3:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW3:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW3:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW3:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW3:8" )        /* Listed as "Unused" */

	PORT_START("DSW3")
	PORT_DIPNAME( 0x001f, 0x0010, DEF_STR( Region ) )           /* Jumpers?? */
	PORT_DIPSETTING(      0x0010, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0001, "USA & Canada" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Taiwan ) )
	/* all other values are invalid */
INPUT_PORTS_END


/* the same as f1gp, but with an extra button */
static INPUT_PORTS_START( f1gp2 )
	PORT_INCLUDE( f1gp )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPUNUSED( 0x001e, 0x001e )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16
};

#define XOR(a) WORD_XOR_BE(a)

static const gfx_layout tilelayout2 =
{
	16,16,
	0x800,
	4,
	{ 0, 1, 2, 3 },
	{ XOR(0)*4, XOR(1)*4, XOR(2)*4, XOR(3)*4, XOR(4)*4, XOR(5)*4, XOR(6)*4, XOR(7)*4,
			XOR(8)*4, XOR(9)*4, XOR(10)*4, XOR(11)*4, XOR(12)*4, XOR(13)*4, XOR(14)*4, XOR(15)*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( f1gp )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x000,  1 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x100, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x200, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout2,  0x300, 16 )
GFXDECODE_END

static GFXDECODE_START( f1gp2 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x000,  1 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x200, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   0x100, 16 )
GFXDECODE_END



WRITE_LINE_MEMBER(f1gp_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

MACHINE_START_MEMBER(f1gp_state,f1gpb)
{
	save_item(NAME(m_pending_command));
	save_item(NAME(m_roz_bank));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_gfxctrl));
	save_item(NAME(m_scroll));
}

MACHINE_START_MEMBER(f1gp_state,f1gp)
{
	membank("bank1")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0x8000);

	MACHINE_START_CALL_MEMBER(f1gpb);
}

MACHINE_RESET_MEMBER(f1gp_state,f1gp)
{
	m_pending_command = 0;
	m_roz_bank = 0;
	m_flipscreen = 0;
	m_gfxctrl = 0;
	m_scroll[0] = 0;
	m_scroll[1] = 0;
}

static MACHINE_CONFIG_START( f1gp, f1gp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,XTAL_20MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(f1gp_cpu1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", f1gp_state,  irq1_line_hold)

	MCFG_CPU_ADD("sub", M68000,XTAL_20MHz/2)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(f1gp_cpu2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", f1gp_state,  irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,XTAL_20MHz/4)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000)) /* 100 CPU slices per frame */

	MCFG_MACHINE_START_OVERRIDE(f1gp_state,f1gp)
	MCFG_MACHINE_RESET_OVERRIDE(f1gp_state,f1gp)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(f1gp_state, screen_update_f1gp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", f1gp)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("vsystem_spr_old", VSYSTEM_SPR2, 0)
	MCFG_VSYSTEM_SPR2_SET_TILE_INDIRECT( f1gp_state, f1gp_old_tile_callback )
	MCFG_VSYSTEM_SPR2_SET_GFXREGION(1)
	MCFG_VSYSTEM_SPR2_SET_PRITYPE(2)
	MCFG_VSYSTEM_SPR2_GFXDECODE("gfxdecode")
	MCFG_VSYSTEM_SPR2_PALETTE("palette")

	MCFG_DEVICE_ADD("vsystem_spr_ol2", VSYSTEM_SPR2, 0)
	MCFG_VSYSTEM_SPR2_SET_TILE_INDIRECT( f1gp_state, f1gp_ol2_tile_callback )
	MCFG_VSYSTEM_SPR2_SET_GFXREGION(2)
	MCFG_VSYSTEM_SPR2_SET_PRITYPE(2)
	MCFG_VSYSTEM_SPR2_GFXDECODE("gfxdecode")
	MCFG_VSYSTEM_SPR2_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(f1gp_state,f1gp)

	MCFG_DEVICE_ADD("k053936", K053936, 0)
	MCFG_K053936_WRAP(1)
	MCFG_K053936_OFFSETS(-58, -2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_8MHz)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(f1gp_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( f1gpb, f1gp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,10000000) /* 10 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(f1gpb_cpu1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", f1gp_state,  irq1_line_hold)

	MCFG_CPU_ADD("sub", M68000,10000000)    /* 10 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(f1gpb_cpu2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", f1gp_state,  irq1_line_hold)

	/* NO sound CPU */
	MCFG_QUANTUM_TIME(attotime::from_hz(6000)) /* 100 CPU slices per frame */

	MCFG_MACHINE_START_OVERRIDE(f1gp_state,f1gpb)
	MCFG_MACHINE_RESET_OVERRIDE(f1gp_state,f1gp)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(f1gp_state, screen_update_f1gpb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", f1gp)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_VIDEO_START_OVERRIDE(f1gp_state,f1gpb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( f1gp2, f1gp )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(f1gp2_cpu1_map)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", f1gp2)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(f1gp_state, screen_update_f1gp2)

	MCFG_DEVICE_REMOVE("vsystem_spr_old")
	MCFG_DEVICE_ADD("vsystem_spr", VSYSTEM_SPR, 0)
	MCFG_VSYSTEM_SPR_SET_TILE_INDIRECT( f1gp_state, f1gp2_tile_callback )
	MCFG_VSYSTEM_SPR_SET_GFXREGION(1)
	MCFG_VSYSTEM_SPR_GFXDECODE("gfxdecode")
	MCFG_VSYSTEM_SPR_PALETTE("palette")

	MCFG_DEVICE_MODIFY("k053936")
	MCFG_K053936_OFFSETS(-48, -21)

	MCFG_VIDEO_START_OVERRIDE(f1gp_state,f1gp2)
MACHINE_CONFIG_END



ROM_START( f1gp )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rom1-a.3",     0x000000, 0x20000, CRC(2d8f785b) SHA1(6eca42ad2d57a31e055496141c89cb537f284378) )

	ROM_REGION16_BE( 0x200000, "user1", 0 )  /* extra ROMs mapped at 100000 */
	ROM_LOAD16_BYTE( "rom10-a.1",    0x000000, 0x40000, CRC(46a289fb) SHA1(6a8c19e08b6d836fe83378fd77fead82a0b2db7c) )
	ROM_LOAD16_BYTE( "rom11-a.2",    0x000001, 0x40000, CRC(53df8ea1) SHA1(25d50bb787f3bd35c9a8ae2b0ab9a21e000debb0) )
	ROM_LOAD16_BYTE( "rom12-a.3",    0x080000, 0x40000, CRC(d8c1bcf4) SHA1(d6d77354eb1ab413ba8cfa5d973cf5b0c851c23b) )
	ROM_LOAD16_BYTE( "rom13-a.4",    0x080001, 0x40000, CRC(7d92e1fa) SHA1(c23f5beea85b0804c61ef9e7f131b186d076221f) )
	ROM_LOAD16_BYTE( "rom7-a.5",     0x100000, 0x40000, CRC(7a014ba6) SHA1(8f0abbb68100e396e5a41337254cb6bf1a2ed00b) )
	ROM_LOAD16_BYTE( "rom6-a.6",     0x100001, 0x40000, CRC(6d947a3f) SHA1(2cd01ee2a73ab105a45a5464a29fd75aa43ba2db) )
	ROM_LOAD16_BYTE( "rom8-a.7",     0x180000, 0x40000, CRC(0ed783c7) SHA1(c0c467ede51c08d84999897c6d5cc8b584b23b67) )
	ROM_LOAD16_BYTE( "rom9-a.8",     0x180001, 0x40000, CRC(49286572) SHA1(c5e16bd1ccd43452337a4cd76db70db079ca0706) )

	ROM_REGION16_BE( 0x200000, "user2", 0 )  /* extra ROMs mapped at a00000 */
											/* containing gfx data for the 053936 */
	ROM_LOAD16_WORD_SWAP( "rom2-a.06",    0x000000, 0x100000, CRC(747dd112) SHA1(b9264bec61467ab256cf6cb698b6e0ea8f8006e0) )
	ROM_LOAD16_WORD_SWAP( "rom3-a.05",    0x100000, 0x100000, CRC(264aed13) SHA1(6f0de860d4299befffc530b7a8f19656982a51c4) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rom4-a.4",     0x000000, 0x20000, CRC(8e811d36) SHA1(2b806b50a3a307a21894687f16485ace287a7c4c) )

	ROM_REGION( 0x30000, "audiocpu", 0 )    /* 64k for the audio CPU + banks */
	ROM_LOAD( "rom5-a.8",     0x00000, 0x08000, CRC(9ea36e35) SHA1(9254dea8362318d8cfbd5e36e476e0e235e6326a) )
	ROM_CONTINUE(             0x10000, 0x18000 )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "rom3-b.07",    0x000000, 0x100000, CRC(ffb1d489) SHA1(9330b67e0eaaf67d6c38f40a02c72419bd38fb81) )
	ROM_LOAD( "rom2-b.04",    0x100000, 0x100000, CRC(d1b3471f) SHA1(d1a95fbaad1c3d9ec2121bf65abbcdb5441bd0ac) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD32_WORD( "rom5-b.2",     0x000000, 0x80000, CRC(17572b36) SHA1(c58327c2f708783a3e8470e290cae0d71454f1da) )
	ROM_LOAD32_WORD( "rom4-b.3",     0x000002, 0x80000, CRC(72d12129) SHA1(11da6990a54ae1b6f6d0bed5d0431552f83a0dda) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD32_WORD( "rom7-b.17",    0x000000, 0x40000, CRC(2aed9003) SHA1(45ff9953ad98063573e7fd7b930ae8b0183cdd04) )
	ROM_LOAD32_WORD( "rom6-b.16",    0x000002, 0x40000, CRC(6789ef12) SHA1(9b0d1cc6e9c6398ccb7f635c4c148fddd224a21f) )

	ROM_REGION( 0x40000, "gfx4", ROMREGION_ERASE00 )    /* gfx data for the 053936 */
	/* RAM, not ROM - handled at run time */

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 ) /* sound samples */
	ROM_LOAD( "rom14-a.09",   0x000000, 0x100000, CRC(b4c1ac31) SHA1(acab2e1b5ce4ca3a5c4734562481b54db4b46995) )

	ROM_REGION( 0x100000, "ymsnd", 0 ) /* sound samples */
	ROM_LOAD( "rom17-a.08",   0x000000, 0x100000, CRC(ea70303d) SHA1(8de1a0e6d47cd80a622663c1745a1da54cd0ea05) )
ROM_END

/* This is a bootleg of f1gp, produced by Playmark in Italy
   the video hardware is different, it lacks the sound z80, and has less samples
 */

ROM_START( f1gpb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	/* these have extra data at 0x30000 which isn't preset in the f1gp set, is it related to the changed sound hardware? */
	ROM_LOAD16_BYTE( "1.ic38",     0x000001, 0x20000, CRC(046dd83a) SHA1(ea65fa88f9d9a79664de666e63594a7a7de86650) )
	ROM_LOAD16_BYTE( "7.ic39",     0x000000, 0x20000, CRC(960f5db4) SHA1(addc461538e2140afae400e8d7364d0bcc42a0cb) )

	ROM_REGION16_BE( 0x200000, "user1", 0 )  /* extra ROMs mapped at 100000 */
	ROM_LOAD16_BYTE( "8.ic41",    0x000000, 0x80000, CRC(39af8180) SHA1(aa1577195b1463069870db2d64db3b5e61d6bbe8) )
	ROM_LOAD16_BYTE( "2.ic48",    0x000001, 0x80000, CRC(b3b315c3) SHA1(568592e450401cd95206dbe439e565dd28499dd1) )
	ROM_LOAD16_BYTE( "9.ic166",   0x100000, 0x80000, CRC(bb596d5b) SHA1(f29ed135e8f09d4a15353360a811c13aba681382) )
	ROM_LOAD16_BYTE( "3.ic165",   0x100001, 0x80000, CRC(b7295a30) SHA1(4120dda38673d59343aea0f030d2f275a0ae3d95) )

	ROM_REGION16_BE( 0x200000, "user2", 0 )  /* extra ROMs mapped at a00000 */
	ROM_LOAD16_BYTE( "10.ic43",   0x000000, 0x80000, CRC(d60e7706) SHA1(23c383e47e6600a68d6fd8bcfc9552fe0d660630) )
	ROM_LOAD16_BYTE( "4.ic42",    0x000001, 0x80000, CRC(5dbde98a) SHA1(536553eaad0ebfe219e44a4f50a4707209024469) )
	ROM_LOAD16_BYTE( "11.ic168",  0x100000, 0x80000, CRC(92a28e52) SHA1(dc203486b96fdc1930f7e63021e84f203540a64e) )
	ROM_LOAD16_BYTE( "5.ic167",   0x100001, 0x80000, CRC(48c36293) SHA1(2a5d92537ba331a99697d13b4394b8d2737eeaf2) )

	ROM_REGION( 0x20000, "sub", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "16.u7",     0x000000, 0x10000, CRC(7609d818) SHA1(eb841b8e7b34f1c677f1a79bfeda5dafc1f6849f) )
	ROM_LOAD16_BYTE( "17.u6",     0x000001, 0x10000, CRC(951befde) SHA1(28754f00ca0fe38fe1d4e68c203a7b401baa9714) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "13.ic151",    0x000000, 0x080000, CRC(4238074b) SHA1(a6b169165c7f7da9e746db8f1fb02e15c02c2b60) )
	ROM_LOAD( "12.ic152",    0x080000, 0x080000, CRC(e97c2b6e) SHA1(3d964999b70af2f39a734eba3feec6d4583261c7) )
	ROM_LOAD( "15.ic153",    0x100000, 0x080000, CRC(c2867d7f) SHA1(86b1be9672cf9f610e1d7efff90d6a73dc1cdb90) )
	ROM_LOAD( "14.ic154",    0x180000, 0x080000, CRC(0cd20423) SHA1(cddad02247b898c0a5a2fe061c41f68ecdf04d5c) )

	/*
	Roms 20 and 21 were missing from the PCB, however the others match perfectly (just with a different data layout)
	I've reconstructed what should be the correct data for this bootleg.

	Note, the bootleg combines 2 GFX regions into a single set of 4-way interleaved roms, so we load them in a user
	region and use ROM_COPY.
	*/

	ROM_REGION( 0x200000, "user3", 0 )
	ROMX_LOAD( "rom21",        0x000003, 0x80000, CRC(7a08c3b7) SHA1(369123348a88513c066c239ed6aa4db5ae4ef0ac), ROM_SKIP(3) )
	ROMX_LOAD( "rom20",        0x000001, 0x80000, CRC(bd1273d0) SHA1(cc7caee231fe3bd87d8403d34059e1292c7f7a00), ROM_SKIP(3) )
	ROMX_LOAD( "19.ic141",     0x000002, 0x80000, CRC(aa4ebdfe) SHA1(ed117e6a84554c5ed2ad4379b834898a4c40d51e), ROM_SKIP(3) )
	ROMX_LOAD( "18.ic140",     0x000000, 0x80000, CRC(9b2a4325) SHA1(b2020e08251366686c4c0045f3fd523fa327badf), ROM_SKIP(3) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_COPY("user3", 0x000000, 0, 0x100000)

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_COPY("user3", 0x100000, 0, 0x80000)

	ROM_REGION( 0x40000, "gfx4", ROMREGION_ERASE00 )    /* gfx data for the 053936 */
	/* RAM, not ROM - handled at run time */

	ROM_REGION( 0x90000, "oki", 0 ) /* sound samples */
	ROM_LOAD( "6.ic13",   0x000000, 0x030000, CRC(6e83ffd8) SHA1(618fd6cd6c0844a4be96f77ff22cd41364718d16) )
	ROM_CONTINUE(         0x040000, 0x050000 )
ROM_END


ROM_START( f1gp2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "rom12.v1",     0x000000, 0x20000, CRC(c5c5f199) SHA1(56fcbf1d9b15a37204296c578e1585599f76a107) )
	ROM_LOAD16_BYTE( "rom14.v2",     0x000001, 0x20000, CRC(dd5388e2) SHA1(66e88f86edc2407e5794519f988203a52d65636d) )

	ROM_REGION16_BE( 0x200000, "user1", 0 )  /* extra ROMs mapped at 100000 */
	ROM_LOAD16_WORD_SWAP( "rom2",         0x100000, 0x100000, CRC(3b0cfa82) SHA1(ea6803dd8d30aa9f3bd578e113fc26f20c640751) )
	ROM_CONTINUE(             0x000000, 0x100000 )

	ROM_REGION( 0x20000, "sub", 0 ) /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rom13.v3",     0x000000, 0x20000, CRC(c37aa303) SHA1(0fe09b398191888620fb676ed0f1593be575512d) )

	ROM_REGION( 0x30000, "audiocpu", 0 )    /* 64k for the audio CPU + banks */
	ROM_LOAD( "rom5.v4",      0x00000, 0x08000, CRC(6a9398a1) SHA1(e907fe5f9c135c5b10ec650ec0c6d08cb856230c) )
	ROM_CONTINUE(             0x10000, 0x18000 )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "rom1",         0x000000, 0x200000, CRC(f2d55ad7) SHA1(2f2d9dc4fab63b06ed7cba0ef1ced286dbfaa7b4) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "rom15",        0x000000, 0x200000, CRC(1ac03e2e) SHA1(9073d0ae24364229a993046bd71e403988692993) )

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD( "rom11",        0x000000, 0x100000, CRC(b22a2c1f) SHA1(b5e67726be5a8561cd04c3c07895b8518b73b89c) )
	ROM_LOAD( "rom10",        0x100000, 0x100000, CRC(43fcbe23) SHA1(54ab58d904890a0b907e674f855092e974c45edc) )
	ROM_LOAD( "rom9",         0x200000, 0x100000, CRC(1bede8a1) SHA1(325ecc3afb30d281c2c8a56719e83e4dc20545bb) )
	ROM_LOAD( "rom8",         0x300000, 0x100000, CRC(98baf2a1) SHA1(df7bd1a743ad0a6e067641e2b7a352c466875ef6) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 ) /* sound samples */
	ROM_LOAD( "rom4",         0x000000, 0x080000, CRC(c2d3d7ad) SHA1(3178096741583cfef1ca8f53e6efa0a59e1d5cb6) )

	ROM_REGION( 0x100000, "ymsnd", 0 ) /* sound samples */
	ROM_LOAD( "rom3",         0x000000, 0x100000, CRC(7f8f066f) SHA1(5e051d5feb327ac818e9c7f7ac721dada3a102b6) )
ROM_END


GAME( 1991, f1gp,  0,    f1gp,  f1gp, driver_device,  0, ROT90, "Video System Co.", "F-1 Grand Prix", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, f1gpb, f1gp, f1gpb, f1gp, driver_device,  0, ROT90, "bootleg (Playmark)", "F-1 Grand Prix (Playmark bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // PCB marked 'Super Formula II', manufactured by Playmark.

GAME( 1992, f1gp2, 0,    f1gp2, f1gp2, driver_device, 0, ROT90, "Video System Co.", "F-1 Grand Prix Part II", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
