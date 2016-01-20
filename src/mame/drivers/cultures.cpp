// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*
    Jibun wo Migaku Culture School Mahjong Hen
    (c)1994 Face

    driver by Pierpaolo Prazzoli

    thanks to David Haywood for some precious advice

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "machine/bankdev.h"

#define MCLK 16000000

class cultures_state : public driver_device
{
public:
	cultures_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vrambank(*this, "vrambank"),
		m_prgbank(*this, "prgbank"),
		m_okibank(*this, "okibank"),
		m_bg1_rom(*this, "bg1"),
		m_bg2_rom(*this, "bg2"),
		m_bg0_videoram(*this, "bg0_videoram"),
		m_bg0_regs_x(*this, "bg0_regs_x"),
		m_bg0_regs_y(*this, "bg0_regs_y"),
		m_bg1_regs_x(*this, "bg1_regs_x"),
		m_bg1_regs_y(*this, "bg1_regs_y"),
		m_bg2_regs_x(*this, "bg2_regs_x"),
		m_bg2_regs_y(*this, "bg2_regs_y")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<address_map_bank_device> m_vrambank;
	required_memory_bank m_prgbank;
	required_memory_bank m_okibank;

	/* memory pointers */
	required_region_ptr<UINT16> m_bg1_rom;
	required_region_ptr<UINT16> m_bg2_rom;

	required_shared_ptr<UINT8> m_bg0_videoram;
	required_shared_ptr<UINT8> m_bg0_regs_x;
	required_shared_ptr<UINT8> m_bg0_regs_y;
	required_shared_ptr<UINT8> m_bg1_regs_x;
	required_shared_ptr<UINT8> m_bg1_regs_y;
	required_shared_ptr<UINT8> m_bg2_regs_x;
	required_shared_ptr<UINT8> m_bg2_regs_y;

	/* video-related */
	tilemap_t  *m_bg0_tilemap;
	tilemap_t  *m_bg1_tilemap;
	tilemap_t  *m_bg2_tilemap;
	int      m_irq_enable;
	int      m_bg1_bank;
	int      m_bg2_bank;
	DECLARE_WRITE8_MEMBER(cpu_bankswitch_w);
	DECLARE_WRITE8_MEMBER(bg0_videoram_w);
	DECLARE_WRITE8_MEMBER(misc_w);
	DECLARE_WRITE8_MEMBER(bg_bank_w);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_cultures(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cultures_interrupt);
};



TILE_GET_INFO_MEMBER(cultures_state::get_bg1_tile_info)
{
	int const code = m_bg1_rom[0x200000/2 + m_bg1_bank * 0x80000/2 + tile_index];
	SET_TILE_INFO_MEMBER(1, code, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(cultures_state::get_bg2_tile_info)
{
	int const code = m_bg2_rom[0x200000/2 + m_bg2_bank * 0x80000/2 + tile_index];
	SET_TILE_INFO_MEMBER(2, code, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(cultures_state::get_bg0_tile_info)
{
	int const code = m_bg0_videoram[tile_index * 2] + (m_bg0_videoram[tile_index * 2 + 1] << 8);
	SET_TILE_INFO_MEMBER(0, code, code >> 12, 0);
}

void cultures_state::video_start()
{
	m_bg0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cultures_state::get_bg0_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cultures_state::get_bg1_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 512, 512);
	m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cultures_state::get_bg2_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 512, 512);

	m_bg1_tilemap->set_transparent_pen(0);
	m_bg0_tilemap->set_transparent_pen(0);

	m_bg0_tilemap->set_scrolldx(502, -118);
	m_bg1_tilemap->set_scrolldx(502, -118);
	m_bg2_tilemap->set_scrolldx(502, -118);

	m_bg0_tilemap->set_scrolldy(255, -16);
	m_bg1_tilemap->set_scrolldy(255, -16);
	m_bg2_tilemap->set_scrolldy(255, -16);
}

UINT32 cultures_state::screen_update_cultures(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int attr;

	// tilemaps attributes
	attr = (m_bg0_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (m_bg0_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	m_bg0_tilemap->set_flip(attr);

	attr = (m_bg1_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (m_bg1_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	m_bg1_tilemap->set_flip(attr);

	attr = (m_bg2_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (m_bg2_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	m_bg2_tilemap->set_flip(attr);

	// tilemaps scrolls
	m_bg0_tilemap->set_scrollx(0, (m_bg0_regs_x[2] << 8) + m_bg0_regs_x[0]);
	m_bg1_tilemap->set_scrollx(0, (m_bg1_regs_x[2] << 8) + m_bg1_regs_x[0]);
	m_bg2_tilemap->set_scrollx(0, (m_bg2_regs_x[2] << 8) + m_bg2_regs_x[0]);
	m_bg0_tilemap->set_scrolly(0, (m_bg0_regs_y[2] << 8) + m_bg0_regs_y[0]);
	m_bg1_tilemap->set_scrolly(0, (m_bg1_regs_y[2] << 8) + m_bg1_regs_y[0]);
	m_bg2_tilemap->set_scrolly(0, (m_bg2_regs_y[2] << 8) + m_bg2_regs_y[0]);

	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg0_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg1_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

WRITE8_MEMBER(cultures_state::cpu_bankswitch_w)
{
	m_prgbank->set_entry(data & 0x0f);
	m_vrambank->set_bank((data & 0x20)>>5);
}


WRITE8_MEMBER(cultures_state::bg0_videoram_w)
{
	m_bg0_videoram[offset] = data;
	m_bg0_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER(cultures_state::misc_w)
{
	m_okibank->set_entry(data&0x0f);
	m_irq_enable = data & 0x80;
}

WRITE8_MEMBER(cultures_state::bg_bank_w)
{
	if (m_bg1_bank != (data & 3))
	{
		m_bg1_bank = data & 3;
		m_bg1_tilemap->mark_all_dirty();
	}

	if (m_bg2_bank != ((data & 0xc) >> 2))
	{
		m_bg2_bank = (data & 0xc) >> 2;
		m_bg2_tilemap->mark_all_dirty();
	}
	machine().bookkeeping().coin_counter_w(0, data & 0x10);
}


static ADDRESS_MAP_START( oki_map, AS_0, 8, cultures_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END

static ADDRESS_MAP_START( vrambank_map, AS_PROGRAM, 8, cultures_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM_WRITE(bg0_videoram_w) AM_SHARE("bg0_videoram")
	AM_RANGE(0x4000, 0x6fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cultures_map, AS_PROGRAM, 8, cultures_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("prgbank")
	AM_RANGE(0x8000, 0xbfff) AM_DEVICE("vrambank", address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cultures_io_map, AS_IO, 8, cultures_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_RAM
	AM_RANGE(0x10, 0x13) AM_RAM
	AM_RANGE(0x20, 0x23) AM_RAM AM_SHARE("bg0_regs_x")
	AM_RANGE(0x30, 0x33) AM_RAM AM_SHARE("bg0_regs_y")
	AM_RANGE(0x40, 0x43) AM_RAM AM_SHARE("bg1_regs_x")
	AM_RANGE(0x50, 0x53) AM_RAM AM_SHARE("bg1_regs_y")
	AM_RANGE(0x60, 0x63) AM_RAM AM_SHARE("bg2_regs_x")
	AM_RANGE(0x70, 0x73) AM_RAM AM_SHARE("bg2_regs_y")
	AM_RANGE(0x80, 0x80) AM_WRITE(cpu_bankswitch_w)
	AM_RANGE(0x90, 0x90) AM_WRITE(misc_w)
	AM_RANGE(0xa0, 0xa0) AM_WRITE(bg_bank_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xd0, 0xd0) AM_READ_PORT("SW1_A")
	AM_RANGE(0xd1, 0xd1) AM_READ_PORT("SW1_B")
	AM_RANGE(0xd2, 0xd2) AM_READ_PORT("SW2_A")
	AM_RANGE(0xd3, 0xd3) AM_READ_PORT("SW2_B")
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT("KEY0")
	AM_RANGE(0xe1, 0xe1) AM_READ_PORT("KEY1")
	AM_RANGE(0xe2, 0xe2) AM_READ_PORT("KEY2")
	AM_RANGE(0xe3, 0xe3) AM_READ_PORT("KEY3")
	AM_RANGE(0xe4, 0xe4) AM_READ_PORT("KEY4")
	AM_RANGE(0xe5, 0xe5) AM_READ_PORT("START")
	AM_RANGE(0xf0, 0xf0) AM_READ_PORT("UNUSED1")
	AM_RANGE(0xf1, 0xf1) AM_READ_PORT("UNUSED2")
	AM_RANGE(0xf2, 0xf2) AM_READ_PORT("UNUSED3")
	AM_RANGE(0xf3, 0xf3) AM_READ_PORT("UNUSED4")
	AM_RANGE(0xf7, 0xf7) AM_READ_PORT("COINS")
ADDRESS_MAP_END


static INPUT_PORTS_START( cultures )
	PORT_START("SW1_A")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1_B")
	PORT_DIPNAME( 0x01, 0x01, "Auto Mode After Reach" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Attract Mode" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x02, "Partial" )
	PORT_DIPSETTING(    0x00, "Full" )
	PORT_DIPNAME( 0x04, 0x04, "Open Hands After Noten" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Datsui Count After Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x08, "Not Cleared" )
	PORT_DIPSETTING(    0x00, "Cleared" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW2_A")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Background Music" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW2_B")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Machihai Display" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:7" ) // "always off"
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:8" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) // "Test"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

/*** GFX Decode ***/


static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64,
};

static GFXDECODE_START( culture )
	GFXDECODE_ENTRY("bg0", 0, gfxlayout, 0x0000, 16 )
	GFXDECODE_ENTRY("bg1", 0, gfxlayout, 0x1000, 8 )
	GFXDECODE_ENTRY("bg2", 0, gfxlayout, 0x1000, 8 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(cultures_state::cultures_interrupt)
{
	if (m_irq_enable)
		device.execute().set_input_line(0, HOLD_LINE);
}

void cultures_state::machine_start()
{
	m_prgbank->configure_entries(0, 16, memregion("maincpu")->base(), 0x4000);
	m_okibank->configure_entries(0, 0x200000 / 0x20000, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_bg1_bank));
	save_item(NAME(m_bg2_bank));
}

void cultures_state::machine_reset()
{
	m_okibank->set_entry(0);
	m_vrambank->set_bank(1);
	m_irq_enable = 0;
	m_bg1_bank = 0;
	m_bg2_bank = 0;
}



static MACHINE_CONFIG_START( cultures, cultures_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MCLK/2) /* 8.000 MHz */
	MCFG_CPU_PROGRAM_MAP(cultures_map)
	MCFG_CPU_IO_MAP(cultures_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cultures_state,  cultures_interrupt)

	MCFG_DEVICE_ADD("vrambank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(vrambank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(15)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x4000)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cultures_state, screen_update_cultures)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", culture)
	MCFG_PALETTE_ADD("palette", 0x3000/2)
	MCFG_PALETTE_FORMAT(xRGBRRRRGGGGBBBB_bit0)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", MCLK/8, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki_map)

MACHINE_CONFIG_END

/*

Jibun wo Migaku Culture School Mahjong Hen
(c)1994 Face

CPU: Z80
Sound: M6295
OSC: 16.000MHz
EEPROM: 93C46
Custom: FACE AV44 4HA0G (x3)

ROMs:
MA01.U12
PCM.U87
BG0C.U45
BG0C2.U46
BG1C.U80
BG1T.U67
BG2C.U68
BG2T.U79

-----mahjong connector-----
                      empty
amp OSC               BG0C
6295 GAL              BG0C2
G PCM     SS        S
A MA01 G  RR        R
L      A  AA        A
 Z80   L  MM custom M

D GGG
I AAA
P LLL
D
I custom BG2C         BG1C
P custom      custom
93C46    BG1T         BG2T

*/

ROM_START( cultures )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ma01.u12",     0x000000, 0x040000, CRC(f57417b3) SHA1(9a2a50222f54e5da9bc5c66863b8be16e33b171f) )

	ROM_REGION( 0x400000, "bg0", ROMREGION_ERASE00 )
	ROM_LOAD( "bg0c.u45",     0x000000, 0x200000, CRC(ad2e1263) SHA1(b28a3d82aaa0421a7b4df837814147b109e7d1a5) )
	ROM_LOAD( "bg0c2.u46",    0x200000, 0x100000, CRC(97c71c09) SHA1(ffbcee1d9cb39d0824f3aa652c3a24579113cf2e) )
	/* 0x300000 - 0x3fffff empty */

	ROM_REGION16_LE( 0x400000, "bg1", ROMREGION_ERASE00 )
	ROM_LOAD( "bg2c.u68",     0x000000, 0x200000, CRC(fa598644) SHA1(532249e456c34f18a787d5a028df82f2170f604d) )
	ROM_LOAD( "bg1t.u67",     0x200000, 0x100000, CRC(d2e594ee) SHA1(a84b5ab62dec1867d433ccaeb1381e7593958cf0) )
	/* 0x300000 - 0x3fffff empty */

	ROM_REGION16_LE( 0x400000, "bg2", ROMREGION_ERASE00 )
	ROM_LOAD( "bg1c.u80",     0x000000, 0x200000, CRC(9ab99bd9) SHA1(bce41b6f5d83c8262ba8d37b2dfcd5d7a5e7ace7) )
	ROM_LOAD( "bg2t.u79",     0x200000, 0x100000, CRC(0610a79f) SHA1(9fc6b2e5c573ed682b2f7fa462c8f42ff99da5ba) )
	/* 0x300000 - 0x3fffff empty */

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "pcm.u87",      0x000000, 0x200000, CRC(84206475) SHA1(d1423bd5c7425e121fb4e7845cf57801e9afa7b3) )
ROM_END


GAME( 1994, cultures, 0, cultures, cultures, driver_device, 0, ROT0, "Face", "Jibun wo Migaku Culture School Mahjong Hen", MACHINE_SUPPORTS_SAVE )
