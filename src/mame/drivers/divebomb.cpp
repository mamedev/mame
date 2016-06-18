// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Bennett
/***************************************************************************

    Kyuukoukabakugekitai - Dive Bomber Squad
 
    (C) Konami 1989

****************************************************************************
 
 PCB Layout
 ----------
 
 GX840 PWB35165
|--------------------------------------------------------------------------|
|                                                                          |
|  U15                               Z80B                  Z80B     DSW1   |
|                                                                   DSW2   |
|                  HM6116AP-15                                      DSW3   |
|  U16                                U21                           DSW4   |
|             MN53060KGB        8464A-15L                   U20            |
|  U17         (QFP148)                                8464A-15L           |
|                                                                          |
|                                                                       J  |
|                                                                       A  |
|                                                                       M  |
|  U18               24.0MHz     051316        Z80B                     M  |
|                                                                       A  |
|                                 U83    U34    U19                        |
|  MB81C86-55 MB81C86-55          U82    U33    8464A-15L                  |
|  MB81C86-55 MB81C86-55          U81    U22                               |
|                                                                          |
|                                 051316                                   |
|                                                                          |
|                           CN1                          CN2   SN76489     |
|                                                              SN76489     |
|                                                              SN76489     |
|                                                              SN76489     |
| HM6116AP-15  U23                                             SN76489     |
|              U22                                             SN76489  VR |
|                                                                          |
|--------------------------------------------------------------------------|
 
 Notes:
     There are numerous wire modificationss connecting pins of U15-U18 to
     various other components on the PCB
 
     MB81C86    - 64kx4 SRAM
     HM6116AP   - 2kx8 SRAM
     8464A      - 8kx8 SRAM
     MN53060KGB - Panasonic CMOS gate array (6016 gates)
     051316     - Konami PSAC
     CN1/CN2    - ROM board connectors
 
 
 ROM Board
 ---------
 
 GX840 PWB451716
 |--------------------------|
 |                          |
 |            U8            |
 |                          |
 |    U3      U7     U10    |
 |                          |
 |    U2      U6     U10    |
 |                          |
 |    U41     U5     U9     |
 |                          |
 |--------------------------|
 
 Notes:
     All ROMs are 27512 EPROMs
 

To do:
 * Verify layer alignment
 * Deduce unknown DIP switch settings
 
To verify against original HW:
 * Game has no sprites when you reach level 4 and becomes unplayable
 * Game often hangs (sprites no longer animate, screen scrolls/rotates endlessly) due to a communication breakdown
   Data sent from one CPU to another is occasionally overwritten before the recipient can read the first value
 * Game hangs if you die from hitting a regular enemy at the same time a boss dies
 * Bosses can't kill you directly
 * Cocktail mode is broken
 * UFOs that appear during the boss battle wrap around when leaving the right side of the screen
 * Sometimes a stray, bad explosion sprite appears in the top left corner after you respawn
 * Lives counter is corrupt for > 4 lives

****************************************************************************/

#include "emu.h"
#include "includes/divebomb.h"



/*************************************
 *
 *  Main CPU
 *
 *************************************/

static ADDRESS_MAP_START( divebomb_fgcpu_map, AS_PROGRAM, 8, divebomb_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(fgram_w) AM_SHARE("fgram")
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( divebomb_fgcpu_iomap, AS_IO, 8, divebomb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("sn0", sn76489_device, write)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("sn1", sn76489_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("sn2", sn76489_device, write)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("sn3", sn76489_device, write)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("sn4", sn76489_device, write)
	AM_RANGE(0x05, 0x05) AM_DEVWRITE("sn5", sn76489_device, write)
	AM_RANGE(0x10, 0x10) AM_READWRITE(fgcpu_roz_comm_r, fgcpu_roz_comm_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(fgcpu_spr_comm_r, fgcpu_spr_comm_w)
	AM_RANGE(0x30, 0x30) AM_READ_PORT("IN0")
	AM_RANGE(0x31, 0x31) AM_READ_PORT("IN1")
	AM_RANGE(0x32, 0x32) AM_READ_PORT("DSW1")
	AM_RANGE(0x33, 0x33) AM_READ_PORT("DSW2")
	// 34/35 aren't read but PCB has 4 banks of dips populated, so logically they would map here even if unused
	AM_RANGE(0x34, 0x34) AM_READ_PORT("DSW3")
	AM_RANGE(0x35, 0x35) AM_READ_PORT("DSW4")
	AM_RANGE(0x36, 0x36) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x37, 0x37) AM_READ(fgcpu_comm_flags_r)
ADDRESS_MAP_END


READ8_MEMBER(divebomb_state::fgcpu_spr_comm_r)
{
    has_fromsprite = false;
    update_irqs();
    return from_sprite;
}


WRITE8_MEMBER(divebomb_state::fgcpu_spr_comm_w)
{
    m_spritecpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
    to_spritecpu = data;
}


READ8_MEMBER(divebomb_state::fgcpu_roz_comm_r)
{
    has_fromroz = false;
    update_irqs();
    return from_roz;
}


WRITE8_MEMBER(divebomb_state::fgcpu_roz_comm_w)
{
    m_rozcpucpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
    to_rozcpu = data;
}


READ8_MEMBER(divebomb_state::fgcpu_comm_flags_r)
{
    UINT8 result = 0;
    
    if (has_fromroz)
        result |= 1;
    if (has_fromsprite)
        result |= 2;
    
    return result;
}



/*************************************
 *
 *  Sprite CPU
 *
 *************************************/

static ADDRESS_MAP_START( divebomb_spritecpu_map, AS_PROGRAM, 8, divebomb_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram")
    AM_RANGE(0xc800, 0xdfff) AM_WRITENOP
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( divebomb_spritecpu_iomap, AS_IO, 8, divebomb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(spritecpu_port00_w)
	AM_RANGE(0x80, 0x80) AM_READWRITE(spritecpu_comm_r, spritecpu_comm_w)
ADDRESS_MAP_END


READ8_MEMBER(divebomb_state::spritecpu_comm_r)
{
    m_spritecpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
    return to_spritecpu;
}


WRITE8_MEMBER(divebomb_state::spritecpu_comm_w)
{
    from_sprite = data;
    has_fromsprite = true;
    update_irqs();
}


WRITE8_MEMBER(divebomb_state::spritecpu_port00_w)
{
    // Written with 0x00 on reset
    // Written with 0x34 7 times in succession on occasion (see PC:0x00E3)
}



/*************************************
 *
 *  ROZ CPU
 *
 *************************************/

static ADDRESS_MAP_START( divebomb_rozcpu_map, AS_PROGRAM, 8, divebomb_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1") 
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_DEVREADWRITE("k051316_1", k051316_device, read, write)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_DEVREADWRITE("k051316_2", k051316_device, read, write)
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( divebomb_rozcpu_iomap, AS_IO, 8, divebomb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(rozcpu_bank_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(rozcpu_wrap2_enable_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(rozcpu_enable1_w)
	AM_RANGE(0x13, 0x13) AM_WRITE(rozcpu_enable2_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(rozcpu_wrap1_enable_w)
	AM_RANGE(0x20, 0x2f) AM_DEVWRITE("k051316_1", k051316_device, ctrl_w)
	AM_RANGE(0x30, 0x3f) AM_DEVWRITE("k051316_2", k051316_device, ctrl_w)
	AM_RANGE(0x40, 0x40) AM_READWRITE(rozcpu_comm_r, rozcpu_comm_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(rozcpu_pal_w)
ADDRESS_MAP_END


WRITE8_MEMBER(divebomb_state::rozcpu_bank_w)
{
    UINT32 bank = BITSWAP8(data, 4, 5, 6, 7, 3, 2, 1, 0) >> 4;
    m_bank1->set_entry(bank);

    if (data & 0x0f)
        logerror("rozcpu_bank_w %02x\n", data);
}


READ8_MEMBER(divebomb_state::rozcpu_comm_r)
{
    m_rozcpucpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
    return to_rozcpu;
}


WRITE8_MEMBER(divebomb_state::rozcpu_comm_w)
{
    from_roz = data;
    has_fromroz = true;
    update_irqs();
}



/*************************************
 *
 *  IRQs
 *
 *************************************/

void divebomb_state::update_irqs()
{
    if (has_fromsprite || has_fromroz)
        m_fgcpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
    else
        m_fgcpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( divebomb )
	PORT_START("DSW1") // If the first 4 are on it engages freeplay
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "9" )
    PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:3")
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
    PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
    PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
    PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
    PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
    PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
    PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
    PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
    PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

    PORT_START("DSW4")
    PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW4:1" )
    PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW4:2" )
    PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3" )
    PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4" )
    PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" )
    PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
    PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
    PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 8,0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};


static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24,16,8,0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 32,33,34,35,36,37,38,39 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	64*16
};


static GFXDECODE_START( divebomb )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0x400+0x400, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16_layout, 0x400+0x400+0x400, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( divebomb, divebomb_state )

	MCFG_CPU_ADD("fgcpu", Z80,XTAL1/4) // ?
	MCFG_CPU_PROGRAM_MAP(divebomb_fgcpu_map)
	MCFG_CPU_IO_MAP(divebomb_fgcpu_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", divebomb_state,  nmi_line_pulse)

	MCFG_CPU_ADD("spritecpu", Z80,XTAL1/4) // ?
	MCFG_CPU_PROGRAM_MAP(divebomb_spritecpu_map)
	MCFG_CPU_IO_MAP(divebomb_spritecpu_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", divebomb_state,  nmi_line_pulse)

	MCFG_CPU_ADD("rozcpu", Z80,XTAL1/4) // ?
	MCFG_CPU_PROGRAM_MAP(divebomb_rozcpu_map)
	MCFG_CPU_IO_MAP(divebomb_rozcpu_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", divebomb_state,  nmi_line_pulse)

	MCFG_QUANTUM_PERFECT_CPU("fgcpu")

	MCFG_DEVICE_ADD("k051316_1", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_BPP(8)
	MCFG_K051316_WRAP(0)
	MCFG_K051316_OFFSETS(-88, -16)
	MCFG_K051316_CB(divebomb_state, zoom_callback_1)

	MCFG_DEVICE_ADD("k051316_2", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_BPP(8)
	MCFG_K051316_WRAP(0)
	MCFG_K051316_OFFSETS(-88, -16)
	MCFG_K051316_CB(divebomb_state, zoom_callback_2)

    MCFG_MACHINE_START_OVERRIDE(divebomb_state, divebomb)
    MCFG_MACHINE_RESET_OVERRIDE(divebomb_state, divebomb)

    MCFG_VIDEO_START_OVERRIDE(divebomb_state, divebomb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1-32)
	MCFG_SCREEN_UPDATE_DRIVER(divebomb_state, screen_update_divebomb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", divebomb)
	MCFG_PALETTE_ADD("palette", 0x400+0x400+0x400+0x100)

	MCFG_PALETTE_INIT_OWNER(divebomb_state, divebomb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

    // All frequencies unverified
	MCFG_SOUND_ADD("sn0", SN76489, XTAL1/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
	MCFG_SOUND_ADD("sn1", SN76489, XTAL1/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
	MCFG_SOUND_ADD("sn2", SN76489, XTAL1/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
	MCFG_SOUND_ADD("sn3", SN76489, XTAL1/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
	MCFG_SOUND_ADD("sn4", SN76489, XTAL1/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
	MCFG_SOUND_ADD("sn5", SN76489, XTAL1/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
MACHINE_CONFIG_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

ROM_START( divebomb )
	ROM_REGION( 0x08000, "fgcpu", 0 )
	ROM_LOAD( "U20.27256", 0x00000, 0x08000, CRC(89a30c82) SHA1(ecb3dcc00192b9646dab55ec6c102ab1d2e403e8) )
	
	ROM_REGION( 0x08000, "spritecpu", 0 )
	ROM_LOAD( "U21.27256", 0x00000, 0x08000, CRC(3896d3f6) SHA1(86d10c907bf00977656d8fa426b2f1ac211acdb3) )

	ROM_REGION( 0x08000, "rozcpu", 0 )
	ROM_LOAD( "U19.27256", 0x00000, 0x08000, CRC(16e26fb9) SHA1(9602c79f947f5267e5f4f4f8dedc4697861a519b) )

	ROM_REGION( 0x40000, "rozcpudata", 0 ) // on sub-board
	ROM_LOAD( "U9.27512",  0x00000, 0x10000, CRC(c842f831) SHA1(badb38908a285e54b85e369342fd6ecb8fd68bbb) )
	ROM_LOAD( "U10.27512", 0x10000, 0x10000, CRC(c77f3574) SHA1(06b7576d5949906ee3209c011cd30e7066bb20cc) )
	ROM_LOAD( "U11.27512", 0x20000, 0x10000, CRC(8d46be7d) SHA1(7751df1f39b208169f04a5b904cb63e9fb53bba8) )
	// u12 not populated

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "U22.27256", 0x00000, 0x08000, CRC(f816f9c5) SHA1(b8e136463a1b4c81960c6b7350472d82af0fb1fb) )
	ROM_LOAD16_BYTE( "U23.27256", 0x00001, 0x08000, CRC(d2600570) SHA1(a7f7e182670e7b95321c4ec8278ce915bbe2b5ca) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "U15.27C100", 0x00000, 0x20000, CRC(ccba7fa0) SHA1(5eb4c1e458e7810e0f9db92946474d6da65f1a1b) )
	ROM_LOAD32_BYTE( "U16.27C100", 0x00001, 0x20000, CRC(16891fef) SHA1(a4723958509bccc73138306e58c355325ec342a3) )
	ROM_LOAD32_BYTE( "U17.27C100", 0x00002, 0x20000, CRC(f4cbc97f) SHA1(1e13bc18db128575ca8e6998e9dd6f7dc37a99b8) )
	ROM_LOAD32_BYTE( "U18.27C100", 0x00003, 0x20000, CRC(91ab9d89) SHA1(98454df4638bb831babb1796b095169851e6cf40) )

	ROM_REGION( 0x30000, "k051316_1", ROMREGION_INVERT ) // on sub-board
	ROM_LOAD( "U1.27512", 0x00000, 0x10000, CRC(99af1e18) SHA1(5d63130313fdd85c58e1d6b59e42b75a15328a6b) )
	ROM_LOAD( "U2.27512", 0x10000, 0x10000, CRC(99c8d516) SHA1(6205907bd526181542f4d58d442667595aec9730) )
	ROM_LOAD( "U3.27512", 0x20000, 0x10000, CRC(5ab4af3c) SHA1(ab8632e37a42f2f0db9b22c8577c4f09718ccc7c) )
	// u4 not populated

	ROM_REGION( 0x40000, "k051316_2", ROMREGION_INVERT ) // on sub-board
	ROM_LOAD( "U5.27512", 0x00000, 0x10000, CRC(6726d022) SHA1(314fcec87f3bf335356b24224158233d91012675) )
	ROM_LOAD( "U6.27512", 0x10000, 0x10000, CRC(756b8a12) SHA1(2869c18ef1592d00dbc340facac2002d21adf6bc) )
	ROM_LOAD( "U7.27512", 0x20000, 0x10000, CRC(01c07d84) SHA1(45e05f15e94a32adbd488a4f77a9619e7e6b63f3) )
	ROM_LOAD( "U8.27512", 0x30000, 0x10000, CRC(5b9e7caa) SHA1(85510c0b861bad6a1411afc1d628bc7c448c9fef) )

	// there are 12 PROMs, 4x banks of 3
	ROM_REGION( 0xc00, "fg_proms", 0 ) // text layer palette
	ROM_LOAD( "u35.MB7122.bin", 0x800, 0x400, CRC(e890259d) SHA1(f96e00da6de3b868e50f2347b83f8e5727b85d9b) )
	ROM_LOAD( "u36.MB7122.bin", 0x400, 0x400, CRC(0c1ecdb5) SHA1(4ba9e283c747d6a760fe2e0a8b2bfdfcc55a3969) )
	ROM_LOAD( "u37.MB7122.bin", 0x000, 0x400, CRC(55c17465) SHA1(153e99a09604467ddd8c641d60c5f1b8d9a205b4) )

    ROM_REGION( 0xc00, "k051316_1_pr", 0 ) // front roz ( k051316_1 )
    // looks like banks 0,1,2 are valid
    ROM_LOAD( "u29.MB7122.bin", 0x000, 0x400, CRC(8b3d60d2) SHA1(a9ac4d3dd5e72522717dd18c32d3f88b75bb077c) ) // the last 0x100 block is invalid (copy of data in u33)
    ROM_LOAD( "u30.MB7122.bin", 0x400, 0x400, CRC(0aeb1a88) SHA1(7b00e85ced210a5f7dfc100c15baa9e1735c7c80) ) // the last 0x100 block is empty
    ROM_LOAD( "u31.MB7122.bin", 0x800, 0x400, CRC(75cf5f3d) SHA1(ec02b99ab65e561596b773918569b28313f21835) ) // the last 0x100 block is invalid (copy of data in u33)

	ROM_REGION( 0xc00, "k051316_2_pr", 0 ) // back roz ( k051316_2 )
	// 4 valid palettes but our code is only using 1
    ROM_LOAD( "u34.MB7122.bin", 0x000, 0x400, CRC(e0e2d93b) SHA1(9043929520abde15727e9333c153cf97104c9003) )
    ROM_LOAD( "u33.MB7122.bin", 0x400, 0x400, CRC(4df75f4f) SHA1(0157d6e268cdd797622c712648eb2e88214b12d9) )
    ROM_LOAD( "u32.MB7122.bin", 0x800, 0x400, CRC(e2e4b443) SHA1(e97a4e2988e29f992c5dec6f817a783b14535742) )

	ROM_REGION( 0x300, "spr_proms", 0 ) // sprite layer palette
	ROM_LOAD( "u83.MB7114.bin", 0x000, 0x100, CRC(d216110d) SHA1(3de935dbf876f82864b8b69049c8681101619411) )
	ROM_LOAD( "u82.MB7114.bin", 0x100, 0x100, CRC(52637774) SHA1(919727e337a716dbd18b51e26e45025b82aeba79) )
	ROM_LOAD( "u81.MB7114.bin", 0x200, 0x100, CRC(c59b0857) SHA1(aea4cb8d1ab59b54f90edb96d4ac42b1dd6bdcbe) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

MACHINE_START_MEMBER(divebomb_state, divebomb)
{
    m_bank1->configure_entries(0, 16, memregion("rozcpudata")->base(), 0x4000);
    
    save_item(NAME(roz1_enable));
    save_item(NAME(roz2_enable));
    save_item(NAME(roz1_wrap));
    save_item(NAME(roz2_wrap));
    save_item(NAME(to_spritecpu));
    save_item(NAME(to_rozcpu));
    save_item(NAME(has_fromsprite));
    save_item(NAME(has_fromroz));
    save_item(NAME(from_sprite));
    save_item(NAME(from_roz));
    save_item(NAME(roz_pal));
}


MACHINE_RESET_MEMBER(divebomb_state, divebomb)
{
    roz1_enable = false;
    roz2_enable = false;
    roz1_wrap = false;
    roz2_wrap = false;
    has_fromsprite = false;
    has_fromroz = false;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

// According to a flyer, the world release was to be called 'Gaia'. The Gaia title graphics are present in the ROMs.
GAME( 1989, divebomb, 0, divebomb, divebomb, driver_device, 0, ROT270, "Konami", "Kyuukoukabakugekitai - Dive Bomber Squad (Japan, prototype)", MACHINE_IS_INCOMPLETE | MACHINE_SUPPORTS_SAVE )
