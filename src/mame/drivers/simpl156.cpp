// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

  "Simple" 156 based Data East Hardware

 Data East
  DE-0409-1
  DE-0491-1

 Mitchell
  MT5601-0 (slightly different component arrangement to Deco PCBs)


 Games Supported

  Data East:
    Joe & Mac Returns (Japanese version called Caveman Ninja 2 might exist)
    Chain Reaction / Magical Drop / Magical Drop Plus 1

  Mitchell games:
    Charlie Ninja
    Party Time: Gonta the Diver II / Ganbare! Gonta!! 2
    Osman / Cannon Dancer


  The DECO 156 is a 32-bit custom encrypted ARM5 chip connected to
  16-bit hardware. Only ROM and System Work RAM is accessed via all
  32 data lines.

  Info from Charles MacDonald:

  - The sound effects 6295 is clocked at exactly half the rate of the
  music 6295.
  - Both have the SS pin pulled high to select the sample rate of the
  ADPCM data. Depends on the input clock though, the rates are described
  in the data sheet.
  - Both are connected directly to their ROMs with no swapping on the
  address or data lines. The music ROM is a 16-bit ROM in byte mode.

  The 156 data bus has pull-up resistors so reading unused locations will
  return $FFFF.

  I traced out all the connections and confirmed that both video chips (52
  and 141) really are on the lower 16 bits of the 32-bit data bus, same
  with the palette RAM. Just the program ROM and 4K internal RAM to the
  223 should be accessed as 32-bit. Not sure why that part isn't working
  right though.

  Each game has a 512K block of memory that is decoded in the same way.
  One of the PALs controls where this block starts at, for example
  0x380000 for Magical Drop and 0x180000 for Osman:

    000000-00FFFF : Main RAM (16K)
    010000-01FFFF : Sprite RAM (8K)
    020000-02FFFF : Palette RAM (4K)
    030000-03FFFF : Read player inputs, write EEPROM and OKI banking
    040000-04FFFF : PF1,2 control registers
    050000-05FFFF : PF1,2 name tables
    060000-06FFFF : PF1,2 row scroll
    070000-07FFFF : Control register

  The ordering of items within the block does not change and the size of
  each region is always 64K. If any RAM or other I/O has to be mirrored,
  it likely fills out the entire 64K range.

  The control register (marked as MWA_NOP in the driver) pulls one of the
  DE156 pins high for a brief moment and low again. Perhaps it triggers an
  interrupt or reset? It doesn't seem to be connected to anything else, at
  least on my board.

  The sprite chip has 16K RAM but the highest address line is tied to
  ground, so only 8K is available. This is correctly implemented in the
  driver, I'm mentioning it to confirm it isn't banked or anything like that.


  Notes:

  Magical Drop / Magical Drop Plus / Chain Reaction

  Random crashes at 7mhz..

-- check this ---

Are the OKI M6295 clocks from Heavy Smash are correct at least for the Mitchell games:

 - OKI M6295(1) clock: 1.000MHz (28 / 28), sample rate = 1000000 / 132
 - OKI M6295(2) clock: 2.000MHz (28 / 14), sample rate = 2000000 / 132


*/

#include "emu.h"
#include "includes/decocrpt.h"
#include "cpu/arm/arm.h"
#include "includes/simpl156.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

static INPUT_PORTS_START( simpl156 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") // all bits? check..
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


WRITE32_MEMBER(simpl156_state::simpl156_eeprom_w)
{
	//int okibank;

	//okibank = data & 0x07;

	m_okimusic->set_bank_base(0x40000 * (data & 0x7));

	m_eeprom->clk_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->cs_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);
}


/* we need to throw away bits for all ram accesses as the devices are connected as 16-bit */

READ32_MEMBER(simpl156_state::simpl156_spriteram_r)
{
	return m_spriteram[offset] ^ 0xffff0000;
}

WRITE32_MEMBER(simpl156_state::simpl156_spriteram_w)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;

	COMBINE_DATA(&m_spriteram[offset]);
}


READ32_MEMBER(simpl156_state::simpl156_mainram_r)
{
	return m_mainram[offset]^0xffff0000;
}

WRITE32_MEMBER(simpl156_state::simpl156_mainram_w)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;

	COMBINE_DATA(&m_mainram[offset]);
}

READ32_MEMBER(simpl156_state::simpl156_pf1_rowscroll_r)
{
	return m_pf1_rowscroll[offset] ^ 0xffff0000;
}

WRITE32_MEMBER(simpl156_state::simpl156_pf1_rowscroll_w)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;

	COMBINE_DATA(&m_pf1_rowscroll[offset]);
}

READ32_MEMBER(simpl156_state::simpl156_pf2_rowscroll_r)
{
	return m_pf2_rowscroll[offset] ^ 0xffff0000;
}

WRITE32_MEMBER(simpl156_state::simpl156_pf2_rowscroll_w)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;

	COMBINE_DATA(&m_pf2_rowscroll[offset]);
}

/* Memory Map controled by PALs */

/* Joe and Mac Returns */
static ADDRESS_MAP_START( joemacr_map, AS_PROGRAM, 32, simpl156_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x107fff) AM_READWRITE(simpl156_mainram_r, simpl156_mainram_w) AM_SHARE("mainram") // main ram
	AM_RANGE(0x110000, 0x111fff) AM_READWRITE(simpl156_spriteram_r, simpl156_spriteram_w)
	AM_RANGE(0x120000, 0x120fff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0x130000, 0x130003) AM_READ_PORT("IN1") AM_WRITE(simpl156_eeprom_w)
	AM_RANGE(0x140000, 0x14001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x150000, 0x151fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x152000, 0x153fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x154000, 0x155fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x160000, 0x161fff) AM_READWRITE(simpl156_pf1_rowscroll_r, simpl156_pf1_rowscroll_w)
	AM_RANGE(0x164000, 0x165fff) AM_READWRITE(simpl156_pf2_rowscroll_r, simpl156_pf2_rowscroll_w)
	AM_RANGE(0x170000, 0x170003) AM_READONLY AM_WRITENOP // ?
	AM_RANGE(0x180000, 0x180003) AM_DEVREADWRITE8("okisfx", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x1c0000, 0x1c0003) AM_DEVREADWRITE8("okimusic", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x200000, 0x200003) AM_READ_PORT("IN0")
	AM_RANGE(0x201000, 0x201fff) AM_RAM AM_SHARE("systemram") // work ram (32-bit)
ADDRESS_MAP_END


/* Chain Reaction */
static ADDRESS_MAP_START( chainrec_map, AS_PROGRAM, 32, simpl156_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_ROM // rom (32-bit)
	AM_RANGE(0x200000, 0x200003) AM_READ_PORT("IN0")
	AM_RANGE(0x201000, 0x201fff) AM_RAM AM_SHARE("systemram") // work ram (32-bit)
	AM_RANGE(0x3c0000, 0x3c0003) AM_DEVREADWRITE8("okimusic", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x400000, 0x407fff) AM_READWRITE(simpl156_mainram_r, simpl156_mainram_w) AM_SHARE("mainram") // main ram?
	AM_RANGE(0x410000, 0x411fff) AM_READWRITE(simpl156_spriteram_r, simpl156_spriteram_w)
	AM_RANGE(0x420000, 0x420fff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0x430000, 0x430003) AM_READ_PORT("IN1") AM_WRITE(simpl156_eeprom_w)
	AM_RANGE(0x440000, 0x44001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x450000, 0x451fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x452000, 0x453fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x454000, 0x455fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x460000, 0x461fff) AM_READWRITE(simpl156_pf1_rowscroll_r, simpl156_pf1_rowscroll_w)
	AM_RANGE(0x464000, 0x465fff) AM_READWRITE(simpl156_pf2_rowscroll_r, simpl156_pf2_rowscroll_w)
	AM_RANGE(0x470000, 0x470003) AM_READONLY AM_WRITENOP // ??
	AM_RANGE(0x480000, 0x480003) AM_DEVREADWRITE8("okisfx", okim6295_device, read, write, 0x000000ff)
ADDRESS_MAP_END


/* Magical Drop */
static ADDRESS_MAP_START( magdrop_map, AS_PROGRAM, 32, simpl156_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x200003) AM_READ_PORT("IN0")
	AM_RANGE(0x201000, 0x201fff) AM_RAM AM_SHARE("systemram") // work ram (32-bit)
	AM_RANGE(0x340000, 0x340003) AM_DEVREADWRITE8("okimusic", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x380000, 0x387fff) AM_READWRITE(simpl156_mainram_r, simpl156_mainram_w) AM_SHARE("mainram") // main ram?
	AM_RANGE(0x390000, 0x391fff) AM_READWRITE(simpl156_spriteram_r, simpl156_spriteram_w)
	AM_RANGE(0x3a0000, 0x3a0fff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0x3b0000, 0x3b0003) AM_READ_PORT("IN1") AM_WRITE(simpl156_eeprom_w)
	AM_RANGE(0x3c0000, 0x3c001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x3d0000, 0x3d1fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x3d2000, 0x3d3fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x3d4000, 0x3d5fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x3e0000, 0x3e1fff) AM_READWRITE(simpl156_pf1_rowscroll_r, simpl156_pf1_rowscroll_w)
	AM_RANGE(0x3e4000, 0x3e5fff) AM_READWRITE(simpl156_pf2_rowscroll_r, simpl156_pf2_rowscroll_w)
	AM_RANGE(0x3f0000, 0x3f0003) AM_READONLY AM_WRITENOP //?
	AM_RANGE(0x400000, 0x400003) AM_DEVREADWRITE8("okisfx", okim6295_device, read, write, 0x000000ff)
ADDRESS_MAP_END


/* Magical Drop Plus 1 */
static ADDRESS_MAP_START( magdropp_map, AS_PROGRAM, 32, simpl156_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x200003) AM_READ_PORT("IN0")
	AM_RANGE(0x201000, 0x201fff) AM_RAM AM_SHARE("systemram") // work ram (32-bit)
	AM_RANGE(0x4c0000, 0x4c0003) AM_DEVREADWRITE8("okimusic", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x680000, 0x687fff) AM_READWRITE(simpl156_mainram_r, simpl156_mainram_w) AM_SHARE("mainram") // main ram?
	AM_RANGE(0x690000, 0x691fff) AM_READWRITE(simpl156_spriteram_r, simpl156_spriteram_w)
	AM_RANGE(0x6a0000, 0x6a0fff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0x6b0000, 0x6b0003) AM_READ_PORT("IN1") AM_WRITE(simpl156_eeprom_w)
	AM_RANGE(0x6c0000, 0x6c001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x6d0000, 0x6d1fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x6d2000, 0x6d3fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x6d4000, 0x6d5fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x6e0000, 0x6e1fff) AM_READWRITE(simpl156_pf1_rowscroll_r, simpl156_pf1_rowscroll_w)
	AM_RANGE(0x6e4000, 0x6e5fff) AM_READWRITE(simpl156_pf2_rowscroll_r, simpl156_pf2_rowscroll_w)
	AM_RANGE(0x6f0000, 0x6f0003) AM_READONLY AM_WRITENOP // ?
	AM_RANGE(0x780000, 0x780003) AM_DEVREADWRITE8("okisfx", okim6295_device, read, write, 0x000000ff)
ADDRESS_MAP_END


/* Mitchell MT5601-0 PCB (prtytime, charlien, osman) */
static ADDRESS_MAP_START( mitchell156_map, AS_PROGRAM, 32, simpl156_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100003) AM_DEVREADWRITE8("okisfx", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x140000, 0x140003) AM_DEVREADWRITE8("okimusic", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x180000, 0x187fff) AM_READWRITE(simpl156_mainram_r, simpl156_mainram_w) AM_SHARE("mainram") // main ram
	AM_RANGE(0x190000, 0x191fff) AM_READWRITE(simpl156_spriteram_r, simpl156_spriteram_w)
	AM_RANGE(0x1a0000, 0x1a0fff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0x1b0000, 0x1b0003) AM_READ_PORT("IN1") AM_WRITE(simpl156_eeprom_w)
	AM_RANGE(0x1c0000, 0x1c001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x1d0000, 0x1d1fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x1d2000, 0x1d3fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x1d4000, 0x1d5fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x1e0000, 0x1e1fff) AM_READWRITE(simpl156_pf1_rowscroll_r, simpl156_pf1_rowscroll_w)
	AM_RANGE(0x1e4000, 0x1e5fff) AM_READWRITE(simpl156_pf2_rowscroll_r, simpl156_pf2_rowscroll_w)
	AM_RANGE(0x1f0000, 0x1f0003) AM_READONLY AM_WRITENOP // ?
	AM_RANGE(0x200000, 0x200003) AM_READ_PORT("IN0")
	AM_RANGE(0x201000, 0x201fff) AM_RAM AM_SHARE("systemram") // work ram (32-bit)
ADDRESS_MAP_END


static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 256,257,258,259,260,261,262,263,0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	32*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24,8,16,0 },
	{ 512,513,514,515,516,517,518,519, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	32*32
};

static GFXDECODE_START( simpl156 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0,     32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0,     32 )    /* Tiles (16x16) */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x200, 32)    /* Sprites (16x16) */

GFXDECODE_END

INTERRUPT_GEN_MEMBER(simpl156_state::simpl156_vbl_interrupt)
{
	device.execute().set_input_line(ARM_IRQ_LINE, HOLD_LINE);
}


DECO16IC_BANK_CB_MEMBER(simpl156_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

DECOSPR_PRIORITY_CB_MEMBER(simpl156_state::pri_callback)
{
	switch (pri & 0xc000)
	{
		case 0x0000: return 0;
		case 0x4000: return 0xf0;
		case 0x8000: return 0xf0 | 0xcc;
		case 0xc000: return 0xf0 | 0xcc;; /*  or 0xf0|0xcc|0xaa ? */
	}

	return 0;
}


static MACHINE_CONFIG_START( chainrec, simpl156_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM, 28000000 /* /4 */) /*DE156*/ /* 7.000 MHz */ /* measured at 7.. seems to need 28? */
	MCFG_CPU_PROGRAM_MAP(chainrec_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", simpl156_state,  simpl156_vbl_interrupt)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")  // 93C45

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(800))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(simpl156_state, screen_update_simpl156)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_MEMBITS(16)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", simpl156)

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x10)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(simpl156_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(simpl156_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_PRIORITY_CB(simpl156_state, pri_callback)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("okisfx", 32220000/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.6)

	MCFG_OKIM6295_ADD("okimusic", 32220000/16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.2)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( magdrop, chainrec )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(magdrop_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( magdropp, chainrec )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(magdropp_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( joemacr, chainrec )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(joemacr_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mitchell156, chainrec )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mitchell156_map)

	MCFG_OKIM6295_REPLACE("okimusic", 32220000/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.2)
MACHINE_CONFIG_END


/*

Joe and Mac Returns
Data East 1994

This is a higher quality bootleg made with genuine DECO chips/parts.

+-------------------------------+
|        07.u46           04.u12|
| VOL  M6296 M6295    52  03.u11|
|J  06.u45      62256           |
|A              62256     02.u8h|
|M    PAL  141            01.u8l|
|M     62256                    |
|A 223 62256  PAL  28MHz        |
|             62256         156 |
|  SW1        62256   223       |
|      93C45  05.u29            |
+-------------------------------+

All roms are socketed eproms, no labels, just a number in pencel.

05.u29  27c4096
01.u8l  27c4096
02.u8h  27c4096
03.u11  27c4096
04.u12  27c4096
06.u45  27c160
07.u46  27c020

*/

ROM_START( joemacr )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "05.u29",    0x000000, 0x080000,  CRC(74e9a158) SHA1(eee447303ac0884e152b89f59a9694afade87336) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "01.u8l",    0x000000, 0x080000,  CRC(4da4a2c1) SHA1(1ed4bd4337d8b185b56e326e662a8715e4d09e17) )
	ROM_LOAD( "02.u8h",    0x080000, 0x080000,  CRC(642c08db) SHA1(9a541fd56ae34c24f803e08869702be6fafd81d1) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbn01",    0x000001, 0x080000, CRC(a3a37353) SHA1(c4509c8268afb647c20e71b42ae8ebd2bdf075e6) ) /* 03.u11 */
	ROM_LOAD16_BYTE( "mbn02",    0x000000, 0x080000, CRC(aa2230c5) SHA1(43b7ac5c69cde1840a5255a8897e1c5d5f89fd7b) ) /* 04.u12 */

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "mbn04",    0x00000, 0x40000,  CRC(dcbd4771) SHA1(2a1ab6b0fc372333c7eb17aab077fe1ca5ba1dea) ) /* 07.u46 */

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mbn03",    0x00000, 0x200000, CRC(70b71a2a) SHA1(45851b0692de73016fc9b913316001af4690534c) ) /* 06.u45 */
ROM_END

/*

Joe and Mac Returns
Data East 1994

DE-0491-1

156         MW00
      223              223
             141

  MBN00

  MBN01   52   MBN03  M6295
  MBN02        MBN04  M6295

*/

ROM_START( joemacra )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "mw00",    0x000000, 0x080000,  CRC(e1b78f40) SHA1(e611c317ada5a049a5e05d69c051e22a43fa2845) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // rebuilt with roms from other set
	ROM_LOAD( "mbn00",    0x000000, 0x100000, CRC(11b2dac7) SHA1(71a50f606caddeb0ef266e2d3df9e429a4873f21) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbn01",    0x000001, 0x080000, CRC(a3a37353) SHA1(c4509c8268afb647c20e71b42ae8ebd2bdf075e6) )
	ROM_LOAD16_BYTE( "mbn02",    0x000000, 0x080000, CRC(aa2230c5) SHA1(43b7ac5c69cde1840a5255a8897e1c5d5f89fd7b) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "mbn04",    0x00000, 0x40000,  CRC(dcbd4771) SHA1(2a1ab6b0fc372333c7eb17aab077fe1ca5ba1dea) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mbn03",    0x00000, 0x200000, CRC(70b71a2a) SHA1(45851b0692de73016fc9b913316001af4690534c) )
ROM_END

ROM_START( joemacrj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "my00-.e1",    0x000000, 0x080000,  CRC(2c184981) SHA1(976fbb554de96aa6405f6f64cd75b633439fe583) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // rebuilt with roms from other set
	ROM_LOAD( "mbn00",    0x000000, 0x100000, CRC(11b2dac7) SHA1(71a50f606caddeb0ef266e2d3df9e429a4873f21) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbn01",    0x000001, 0x080000, CRC(a3a37353) SHA1(c4509c8268afb647c20e71b42ae8ebd2bdf075e6) )
	ROM_LOAD16_BYTE( "mbn02",    0x000000, 0x080000, CRC(aa2230c5) SHA1(43b7ac5c69cde1840a5255a8897e1c5d5f89fd7b) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "mbn04",    0x00000, 0x40000,  CRC(dcbd4771) SHA1(2a1ab6b0fc372333c7eb17aab077fe1ca5ba1dea) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mbn03",    0x00000, 0x200000, CRC(70b71a2a) SHA1(45851b0692de73016fc9b913316001af4690534c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD( "jmrj_ak93c45.h3", 0x00, 0x80, CRC(31f17522) SHA1(88fc10ffa2e0e3887ebaeafc0b132507e4c77d0d) )
ROM_END

/*

Chain Reaction
Data East 1995

DEC-22VO
DE-0409-1

156           E1
       223    2063     93C45
              2063
                              223
               141
       28MHz         5864
                     5864
            6264
            6264
  MCC-00

  U5* U6*    52      MCC-03   AD-65
  U3* U4*            MCC-04   AD-65

* NOTE: These 4 roms are on a sub-board connecting through mcc-01 & mcc-02 sockets

*/

ROM_START( chainrec )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "e1",    0x000000, 0x080000, CRC(8a8340ef) SHA1(4aaee56127b73453b862ff2a33dc241eeabf5658) ) /* No DECO ID number on label */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mcc-00",    0x000000, 0x100000, CRC(646b03ec) SHA1(9a2fc11b1575032b5a784d88c3a90913068d1e69) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "u3",    0x000000, 0x080000, CRC(92659721) SHA1(b446ce98ec9c2c16375ef00639cfb463b365b8f7) ) /* No DECO ID numbers on labels */
	ROM_LOAD32_BYTE( "u4",    0x000002, 0x080000, CRC(e304eb32) SHA1(61a647ec89695a6b25ff924bdc6d29cbd7aca82b) )
	ROM_LOAD32_BYTE( "u5",    0x000001, 0x080000, CRC(1b6f01ea) SHA1(753fc670707432e317d035b09b0bad0762fea731) )
	ROM_LOAD32_BYTE( "u6",    0x000003, 0x080000, CRC(531a56f2) SHA1(89602bb873a3b110bffc216f921ba228e53380f9) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "mcc-04",    0x00000, 0x40000,  CRC(86ee6ade) SHA1(56ad3f432c7f430f19fcba7c89940c63da165906) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mcc-03",    0x00000, 0x100000, CRC(da2ebba0) SHA1(96d31dea4c7226ee1d386b286919fa334388c7a1) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD( "eeprom-chainrec.bin", 0x00, 0x80, CRC(b6da3fbf) SHA1(bdc2bdeabf8686c1454737faf785b0d7501f9bde) )
ROM_END

/*

Magical Drop / Magical Drop Plus 1
Data East 1995

DE-0409-1

156           E1
       223    2063     93C45
              2063
                              223
               141
       28MHz         5864
                     5864
            6264
            6264
  MCC-00

  mcc-01     52      MCC-03   AD-65
  mcc-02             MCC-04   AD-65


*/

ROM_START( magdrop )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "re00-2.e1",    0x000000, 0x080000,  CRC(7138f10f) SHA1(ca93c3c2dc9a7dd6901c8429a6bf6883076a9b8f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mcc-00",    0x000000, 0x100000, CRC(646b03ec) SHA1(9a2fc11b1575032b5a784d88c3a90913068d1e69) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mcc-01.a13",    0x000001, 0x100000, CRC(13d88745) SHA1(0ce4ec1481f31be860ee80322de6e32f9a566229) )
	ROM_LOAD16_BYTE( "mcc-02.a14",    0x000000, 0x100000, CRC(d0f97126) SHA1(3848a6f00d0e57aaf383298c4d111eb63a88b073) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "mcc-04",    0x00000, 0x40000,  CRC(86ee6ade) SHA1(56ad3f432c7f430f19fcba7c89940c63da165906) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mcc-03",    0x00000, 0x100000, CRC(da2ebba0) SHA1(96d31dea4c7226ee1d386b286919fa334388c7a1) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD_SWAP( "93c45.2h",    0x00, 0x80, CRC(16ce8d2d) SHA1(1a6883c75d34febbd92a16cfe204ff12550c85fd) )
ROM_END

ROM_START( magdropp )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "rz00-1.e1",    0x000000, 0x080000,  CRC(28caf639) SHA1(a17e792c82e65009e21680094acf093c0c4f1021) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mcc-00",    0x000000, 0x100000, CRC(646b03ec) SHA1(9a2fc11b1575032b5a784d88c3a90913068d1e69) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mcc-01.a13",    0x000001, 0x100000, CRC(13d88745) SHA1(0ce4ec1481f31be860ee80322de6e32f9a566229) )
	ROM_LOAD16_BYTE( "mcc-02.a14",    0x000000, 0x100000, CRC(d0f97126) SHA1(3848a6f00d0e57aaf383298c4d111eb63a88b073) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "mcc-04",    0x00000, 0x40000,  CRC(86ee6ade) SHA1(56ad3f432c7f430f19fcba7c89940c63da165906) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mcc-03",    0x00000, 0x100000, CRC(da2ebba0) SHA1(96d31dea4c7226ee1d386b286919fa334388c7a1) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD_SWAP( "eeprom.2h",    0x00, 0x80, CRC(d13d9edd) SHA1(e98ee2b696f0a7a8752dc30ef8b41bfe6598cbe4) )
ROM_END

/*

Charlie Ninja
(c)1995 Mitchell Corp.

DEC-22VO (board is manufactured by DECO)
MT5601-0
------------------------------------------------------------
|                 MBR03.14H                    MBR-01.14A  |
|                                                          |
|     OKI M6295   ND01-0.13H            52                 |
|                                                          |
--|   OKI M6295    MBR02.12F                               |
  |                                                        |
--|                               CY7C185-25   MBR-00.9A   |
|                                 CY7C185-25               |
|                                                          |
|                                                          |
|J                     GAL16V8           28.000MHz         |
|A                  CY7C185-25                             |
|M                  CY7C185-25     141                     |
|M                                                         |
|A          223                                            |
|                                             GAL16V8      |
|                                GAL16V8                   |
|                             CY7C185-25   223             |
--|                93C45.2H   CY7C185-25                   |
  |                                                  156   |
--| TESTSW                  ND00-1.1E                      |
|                                                          |
------------------------------------------------------------

eprom 1e   27c4002  labeled ND 00-1
eprom 13h  27c2001  labeled ND 01-0

maskrom 14a  27c800  labeled MBR-01
maskrom 14h  27c800  labeled MBR-03
maskrom 12f  27c800  labeled MBR-02
maskrom 9a   27c160  labeled MBR-00

*/

ROM_START( charlien )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "nd00-1.1e",    0x000000, 0x080000,  CRC(f18f4b23) SHA1(cb0c159b4dde3a3c5f295f270485996811e5e4d2) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mbr-00.9a",    0x000000, 0x080000, CRC(ecf2c7f0) SHA1(3c735a4eef2bc49f16ac9365a5689101f43c13e9) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbr-01.14a",    0x000001, 0x100000, CRC(46c90215) SHA1(152acdeea34ec1db3f761066a0c1ff6e43e47f9d) )
	ROM_LOAD16_BYTE( "mbr-03.14h",    0x000000, 0x100000, CRC(c448a68a) SHA1(4b607dfee269abdfeb710b74b73ef87dc2b30e8c) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "nd01-0.13h",    0x00000, 0x40000,  CRC(635a100a) SHA1(f6ec70890892e7557097ccd519de37247bb8c98d) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mbr-02.12f",    0x00000, 0x100000, CRC(4f67d333) SHA1(608f921bfa6b7020c0ce72e5229b3f1489208b23) ) // 00, 01, 04, 05
ROM_END


/*

Party Time: Gonta the Diver II / Ganbare! Gonta!! 2
(c)1995 Mitchell Corp.

DEC-22VO (board is manufactured by DECO)
MT5601-0
------------------------------------------------------------
|                 MCB05.14H     MCB-03.14D     MCB-02.14A  |
|                                                          |
|     OKI M6295   PZ01-0.13H            52     MCB-01.13A  |
|                                                          |
--|   OKI M6295    MCB04.12F                               |
  |                                                        |
--|                               CY7C185-25   MCB-00.9A   |
|                                 CY7C185-25               |
|                                                          |
|                                                          |
|J                     GAL16V8           28.000MHz         |
|A                  CY7C185-25                             |
|M                  CY7C185-25     141                     |
|M                                                         |
|A          223                                            |
|                                             GAL16V8      |
|                                GAL16V8                   |
|                             CY7C185-25   223             |
--|                93C45.2H   CY7C185-25                   |
  |                                                  156   |
--| TESTSW                  PZ00-0.1E                      |
|                                                          |
------------------------------------------------------------

CPU  : DECO 156 100pin PQFP custom encrypted ARM
Sound: M6295x2
OSC  : 28.0000MHz

ROMs:
pz_00-0.1e - Main program (27c4096) Party Time: Gonta the Diver II
rd_00-0.1e - Main program (27c4096) Ganbare! Gonta!! 2

mcb-00.9a  - Graphics (23c16000)

mcb-01.13a - Graphics (23c16000)
mcb-02.14a |
mcb-03.14d |
mcb-05.14h /

pz_01-0.13h - Samples (27c020) Party Time: Gonta the Diver II
rd_01-0.13h - Samples (27c020) Ganbare! Gonta!! 2

mcb-04.12f - Samples (23c16000)

GALs (16V8B, not dumped):
vz-00.5c
vz-01.4e
vz-02.8f

*/

ROM_START( prtytime )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "pz_00-0.1e",    0x000000, 0x080000, CRC(ec715c87) SHA1(c9f28399d59b37977f31a5c67cb97af6c58947ae) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mcb-00.9a",    0x000000, 0x080000, CRC(c48a4f2b) SHA1(2dee5f8507b2a7e6f7e44b14f9abca36d0ebf78b) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mcb-02.14a",    0x000001, 0x200000, CRC(423cfb38) SHA1(b8c772a8ab471c365a11a88c85e1c8c7d2ad6e80) )
	ROM_LOAD16_BYTE( "mcb-05.14h",    0x000000, 0x200000, CRC(81540cfb) SHA1(6f7bc62c3c4d4a29eb1e0cfb261ace461bbca57c) )
	ROM_LOAD16_BYTE( "mcb-01.13a",    0x400001, 0x200000, CRC(06f40a57) SHA1(896f1d373e911dcff7223bf21756ad35b28b4c5d) )
	ROM_LOAD16_BYTE( "mcb-03.14d",    0x400000, 0x200000, CRC(0aef73af) SHA1(76cf13f53da5202da80820f98660edee1eef7f1a) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "pz_01-0.13h",    0x00000, 0x40000,  CRC(8925bce2) SHA1(0ff2d5db7a24a2af30bd753eba274572c32cc2e7) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mcb-04.12f",    0x00000, 0x200000, CRC(e23d3590) SHA1(dc8418edc525f56e84f26e9334d5576000b14e5f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD( "eeprom-prtytime.bin", 0x00, 0x80, CRC(105700da) SHA1(650d11236c5692c1454d7b823d5be4d3278d6576) )
ROM_END

ROM_START( gangonta )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "rd_00-0.1e",    0x000000, 0x080000, CRC(f80f43bb) SHA1(f9d26829eb90d41a6c410d4d673fe9595f814868) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mcb-00.9a",    0x000000, 0x080000, CRC(c48a4f2b) SHA1(2dee5f8507b2a7e6f7e44b14f9abca36d0ebf78b) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mcb-02.14a",    0x000001, 0x200000, CRC(423cfb38) SHA1(b8c772a8ab471c365a11a88c85e1c8c7d2ad6e80) )
	ROM_LOAD16_BYTE( "mcb-05.14h",    0x000000, 0x200000, CRC(81540cfb) SHA1(6f7bc62c3c4d4a29eb1e0cfb261ace461bbca57c) )
	ROM_LOAD16_BYTE( "mcb-01.13a",    0x400001, 0x200000, CRC(06f40a57) SHA1(896f1d373e911dcff7223bf21756ad35b28b4c5d) )
	ROM_LOAD16_BYTE( "mcb-03.14d",    0x400000, 0x200000, CRC(0aef73af) SHA1(76cf13f53da5202da80820f98660edee1eef7f1a) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "rd_01-0.13h",    0x00000, 0x40000,  CRC(70fd18c6) SHA1(368cd8e10c5f5a13eb3813974a7e6b46a4fa6b6c) )

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mcb-04.12f",    0x00000, 0x200000, CRC(e23d3590) SHA1(dc8418edc525f56e84f26e9334d5576000b14e5f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD( "eeprom-gangonta.bin", 0x00, 0x80, CRC(27ba60a5) SHA1(1f65be84cfecda5545e7083bcea9db1681425279) )
ROM_END

/*

Osman / Cannon Dancer
(c)1996 Mitchell Corp.

The game is very similar to Strider as it was programmed by some of the same programmers.

PCB Layout
----------

DEC-22VO (board is manufactured by DECO)
MT5601-0
------------------------------------------------------------
|                 MCF04.14H     MCF-03.14D     MCF-02.14A  |
|                                                          |
|     OKI M6295   SA01-0.13H            52     MCF-01.13A  |
|                                                          |
--|   OKI M6295    MCF05.12F                               |
  |                                                        |
--|                               CY7C185-25   MCF-00.9A   |
|                                 CY7C185-25               |
|                                                          |
|                                                          |
|J                     GAL16V8           28.000MHz         |
|A                  CY7C185-25                             |
|M                  CY7C185-25     141                     |
|M                                                         |
|A          223                                            |
|                                             GAL16V8      |
|                                GAL16V8                   |
|                             CY7C185-25   223             |
--|                93C45.2H   CY7C185-25                   |
  |                                                  156   |
--| TESTSW                  SA00-0.1E                      |
|                                                          |
------------------------------------------------------------

*/

ROM_START( osman )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "sa00-0.1e",    0x000000, 0x080000, CRC(ec6b3257) SHA1(10a42a680ce122ab030eaa2ccd99d302cb77854e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mcf-00.9a",    0x000000, 0x080000, CRC(247712dc) SHA1(bcb765afd7e756b68131c97c30d210de115d6b50) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mcf-02.14a",    0x000001, 0x200000, CRC(21251b33) SHA1(d252fe5c6eef8cbc9327e4176b4868b1cb17a738) )
	ROM_LOAD16_BYTE( "mcf-04.14h",    0x000000, 0x200000, CRC(4fa55577) SHA1(e229ba9cce46b92ce255aa33b974e19b214c4017) )
	ROM_LOAD16_BYTE( "mcf-01.13a",    0x400001, 0x200000, CRC(83881e25) SHA1(ae82cf0f704e6efea94c6c1d276d4e3e5b3ebe43) )
	ROM_LOAD16_BYTE( "mcf-03.14d",    0x400000, 0x200000, CRC(faf1d51d) SHA1(675dbbfe15b8010d54b2b3af26d42cdd753c2ce2) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "sa01-0.13h",    0x00000, 0x40000,  CRC(cea8368e) SHA1(1fcc641381fdc29bd50d3a4b23e67647f79e505a))

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mcf-05.12f",    0x00000, 0x200000, CRC(f007d376) SHA1(4ba20e5dabeacc3278b7f30c4462864cbe8f6984) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD( "eeprom-osman.bin", 0x00, 0x80, CRC(509552b2) SHA1(b506692180ccd67f6ad48503a36ebfc4819817b1) )
ROM_END

/* NOTE: Cannon Dancer uses IDENTICAL roms to Osman. Region is contained in the eeprom settings which we set in the INIT function */

ROM_START( candance )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD( "sa00-0.1e",    0x000000, 0x080000, CRC(ec6b3257) SHA1(10a42a680ce122ab030eaa2ccd99d302cb77854e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mcf-00.9a",    0x000000, 0x080000, CRC(247712dc) SHA1(bcb765afd7e756b68131c97c30d210de115d6b50) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mcf-02.14a",    0x000001, 0x200000, CRC(21251b33) SHA1(d252fe5c6eef8cbc9327e4176b4868b1cb17a738) )
	ROM_LOAD16_BYTE( "mcf-04.14h",    0x000000, 0x200000, CRC(4fa55577) SHA1(e229ba9cce46b92ce255aa33b974e19b214c4017) )
	ROM_LOAD16_BYTE( "mcf-01.13a",    0x400001, 0x200000, CRC(83881e25) SHA1(ae82cf0f704e6efea94c6c1d276d4e3e5b3ebe43) )
	ROM_LOAD16_BYTE( "mcf-03.14d",    0x400000, 0x200000, CRC(faf1d51d) SHA1(675dbbfe15b8010d54b2b3af26d42cdd753c2ce2) )

	ROM_REGION( 0x80000, "okisfx", 0 ) /* Oki samples */
	ROM_LOAD( "sa01-0.13h",    0x00000, 0x40000,  CRC(cea8368e) SHA1(1fcc641381fdc29bd50d3a4b23e67647f79e505a))

	ROM_REGION( 0x200000, "okimusic", 0 ) /* samples? (banked?) */
	ROM_LOAD( "mcf-05.12f",    0x00000, 0x200000, CRC(f007d376) SHA1(4ba20e5dabeacc3278b7f30c4462864cbe8f6984) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) /* eeprom */
	ROM_LOAD16_WORD( "eeprom-candance.bin", 0x00, 0x80, CRC(0a0a8f6b) SHA1(4e07138ba7d615291c67162958c889540615e06f) )
ROM_END


/*
  Cannon Dancer / Osman are odd, they don't init their own Eeprom...
  Roms on boths games are identical, and the Eeprom contains several settings the user isn't
  permitted to change, including region and button config..  the first 2 bytes must match the
  last first 2 bytes in the last block of 8 bytes, and the at 0x20 there must be 0088 or the
  game won't boot

  The second byte of Eeprom contains the following

  (Switch 0 in test mode)

   -bss llfr (byte 0x01 / 0x79 of eeprom)

  *- = unknown / unused  (no observed effect)
  *b = button config (2 buttons, or 3 buttons)
   s = number of special weapons (bombs)
   l = lives
   f = flip (screen rotation)
  *r = region


  other settings

  Switch 1  (byte 0x00 / 0x78 of eeprom)

  ppdd -ecs

  p = number of players (health bar?)
  d = difficulty
 *- = unknown / unused  (no observed effect)
  e = extend score
  c = continue
  s = demo sound

 Switch 2  (byte 0x7d of eeprom)

  cccC CCxb

  ccc = coin1
  CCC = coin 2
 *x = breaks attract mode / game..
  b = blood


  items marked * the user can't configure in test mode

  I don't know if any of the other bytes in the eeprom are tested / used.

  1 in eeprom is 0 in test mode

  Both games are currently configured to 3 buttons, its possible the game was never
  released with a 2 button configuration.

   NOTE: an actual read of the eeprom appears to be byteswapped vs. this data / the file
         MAME outputs

*/


DRIVER_INIT_MEMBER(simpl156_state,simpl156)
{
	UINT8 *rom = memregion("okimusic")->base();
	int length = memregion("okimusic")->bytes();
	dynamic_buffer buf1(length);

	UINT32 x;

	/* hmm low address line goes to banking chip instead? */
	for (x = 0; x < length; x++)
	{
		UINT32 addr;

		addr = BITSWAP24 (x,23,22,21,0, 20,
							19,18,17,16,
							15,14,13,12,
							11,10,9, 8,
							7, 6, 5, 4,
							3, 2, 1 );

		buf1[addr] = rom[x];
	}

	memcpy(rom, &buf1[0], length);

	deco56_decrypt_gfx(machine(), "gfx1");
	deco156_decrypt(machine());
}

/* Everything seems more stable if we run the CPU speed x4 and use Idle skips.. maybe it has an internal multipler? */
READ32_MEMBER(simpl156_state::joemacr_speedup_r)
{
	if (space.device().safe_pc() == 0x284)
		space.device().execute().spin_until_time(attotime::from_usec(400));
	return m_systemram[0x18/4];
}


DRIVER_INIT_MEMBER(simpl156_state,joemacr)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0201018, 0x020101b, read32_delegate(FUNC(simpl156_state::joemacr_speedup_r),this));
	DRIVER_INIT_CALL(simpl156);
}

READ32_MEMBER(simpl156_state::chainrec_speedup_r)
{
	if (space.device().safe_pc() == 0x2d4)
		space.device().execute().spin_until_time(attotime::from_usec(400));
	return m_systemram[0x18/4];
}

DRIVER_INIT_MEMBER(simpl156_state,chainrec)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0201018, 0x020101b, read32_delegate(FUNC(simpl156_state::chainrec_speedup_r),this));
	DRIVER_INIT_CALL(simpl156);
}

READ32_MEMBER(simpl156_state::prtytime_speedup_r)
{
	if (space.device().safe_pc() == 0x4f0)
		space.device().execute().spin_until_time(attotime::from_usec(400));
	return m_systemram[0xae0/4];
}

DRIVER_INIT_MEMBER(simpl156_state,prtytime)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0201ae0, 0x0201ae3, read32_delegate(FUNC(simpl156_state::prtytime_speedup_r),this));
	DRIVER_INIT_CALL(simpl156);
}


READ32_MEMBER(simpl156_state::charlien_speedup_r)
{
	if (space.device().safe_pc() == 0xc8c8)
		space.device().execute().spin_until_time(attotime::from_usec(400));
	return m_systemram[0x10/4];
}

DRIVER_INIT_MEMBER(simpl156_state,charlien)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0201010, 0x0201013, read32_delegate(FUNC(simpl156_state::charlien_speedup_r),this));
	DRIVER_INIT_CALL(simpl156);
}

READ32_MEMBER(simpl156_state::osman_speedup_r)
{
	if (space.device().safe_pc() == 0x5974)
		space.device().execute().spin_until_time(attotime::from_usec(400));
	return m_systemram[0x10/4];
}

DRIVER_INIT_MEMBER(simpl156_state,osman)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0201010, 0x0201013, read32_delegate(FUNC(simpl156_state::osman_speedup_r),this));
	DRIVER_INIT_CALL(simpl156);

}

/* Data East games running on the DE-0409-1 or DE-0491-1 PCB */
GAME( 1994, joemacr,  0,        joemacr,     simpl156, simpl156_state, joemacr,  ROT0, "Data East", "Joe & Mac Returns (World, Version 1.1, 1994.05.27)", MACHINE_SUPPORTS_SAVE ) /* bootleg board with genuine DECO parts */
GAME( 1994, joemacra, joemacr,  joemacr,     simpl156, simpl156_state, joemacr,  ROT0, "Data East", "Joe & Mac Returns (World, Version 1.0, 1994.05.19)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, joemacrj, joemacr,  joemacr,     simpl156, simpl156_state, joemacr,  ROT0, "Data East", "Joe & Mac Returns (Japan, Version 1.2, 1994.06.06)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, chainrec, 0,        chainrec,    simpl156, simpl156_state, chainrec, ROT0, "Data East", "Chain Reaction (World, Version 2.2, 1995.09.25)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, magdrop,  chainrec, magdrop,     simpl156, simpl156_state, chainrec, ROT0, "Data East", "Magical Drop (Japan, Version 1.1, 1995.06.21)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, magdropp, chainrec, magdropp,    simpl156, simpl156_state, chainrec, ROT0, "Data East", "Magical Drop Plus 1 (Japan, Version 2.1, 1995.09.12)", MACHINE_SUPPORTS_SAVE )

/* Mitchell games running on the DEC-22VO / MT5601-0 PCB */
GAME( 1995, charlien, 0,        mitchell156, simpl156, simpl156_state, charlien, ROT0,  "Mitchell", "Charlie Ninja" , MACHINE_SUPPORTS_SAVE ) /* language in service mode */
GAME( 1995, prtytime, 0,        mitchell156, simpl156, simpl156_state, prtytime, ROT90, "Mitchell", "Party Time: Gonta the Diver II / Ganbare! Gonta!! 2 (World Release)", MACHINE_SUPPORTS_SAVE ) /* language in service mode */
GAME( 1995, gangonta, prtytime, mitchell156, simpl156, simpl156_state, prtytime, ROT90, "Mitchell", "Ganbare! Gonta!! 2 / Party Time: Gonta the Diver II (Japan Release)", MACHINE_SUPPORTS_SAVE ) /* language in service mode */
GAME( 1996, osman,    0,        mitchell156, simpl156, simpl156_state, osman,    ROT0,  "Mitchell", "Osman (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, candance, osman,    mitchell156, simpl156, simpl156_state, osman,    ROT0,  "Mitchell (Atlus license)", "Cannon Dancer (Japan)", MACHINE_SUPPORTS_SAVE )
