// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*******************************************************************************************

ZG board (c) 1991 Alba

driver by Angelo Salese

Notes:
-The name of this hardware is "Alba ZG board",a newer revision of the
 "Alba ZC board" used by Hanaroku (albazc.c driver). Test mode says clearly that this is
 from 1991.

TODO:
-Player-2 inputs are unemulated;
-"Custom RAM" emulation: might be a (weak) protection device or related to the "Back-up RAM NG"
 msg that pops up at every start-up.
-Video emulation requires a major conversion to the HD46505SP C.R.T. chip (MC6845 clone),
 there's an heavy x offsetting with the flip screen right now due of that (sets register
 0x0d to 0x80 when the screen is upside-down)
-You can actually configure the coin chutes / coin lockout active high/low (!), obviously
 MAME framework isn't really suitable for it at the current time;

PCB:
- HD46505SP-2 / HD68B45SP Japan
- Mostek MK3880P CPU, Z80 clone
- NEC D8255AC-2
- AY38910A/P
- X1-009 (labeled 8732K5), X1-0198 (or X1-019B, can't read)
- X2-004, X2-003, AX-014 (all with epoxy modules apparently)
- X1-007
- CR-203 lithium battery, near X1-009 and X1-0198. There is also a switch near it
- Xtal 12 MHz at top right corner

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "machine/i8255.h"

#define MASTER_CLOCK XTAL_12MHz

class albazg_state : public driver_device
{
public:
	albazg_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_cus_ram(*this, "cus_ram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_cus_ram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	UINT8 m_mux_data;
	int m_bank;
	UINT8 m_prot_lock;
	DECLARE_WRITE8_MEMBER(yumefuda_vram_w);
	DECLARE_WRITE8_MEMBER(yumefuda_cram_w);
	DECLARE_READ8_MEMBER(custom_ram_r);
	DECLARE_WRITE8_MEMBER(custom_ram_w);
	DECLARE_WRITE8_MEMBER(prot_lock_w);
	DECLARE_READ8_MEMBER(mux_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(yumefuda_output_w);
	TILE_GET_INFO_MEMBER(y_get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_yumefuda(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};

TILE_GET_INFO_MEMBER(albazg_state::y_get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			code + ((color & 0xf8) << 3),
			color & 0x7,
			0);
}


void albazg_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(albazg_state::y_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 albazg_state::screen_update_yumefuda(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/***************************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( yumefuda )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,   0, 8 )
GFXDECODE_END


WRITE8_MEMBER(albazg_state::yumefuda_vram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(albazg_state::yumefuda_cram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/*Custom RAM (Thrash Protection)*/
READ8_MEMBER(albazg_state::custom_ram_r)
{
//  logerror("Custom RAM read at %02x PC = %x\n", offset + 0xaf80, space.device().safe_pc());
	return m_cus_ram[offset];// ^ 0x55;
}

WRITE8_MEMBER(albazg_state::custom_ram_w)
{
//  logerror("Custom RAM write at %02x : %02x PC = %x\n", offset + 0xaf80, data, space.device().safe_pc());
	if(m_prot_lock)
		m_cus_ram[offset] = data;
}

/*this might be used as NVRAM commands btw*/
WRITE8_MEMBER(albazg_state::prot_lock_w)
{
//  logerror("PC %04x Prot lock value written %02x\n", space.device().safe_pc(), data);
	m_prot_lock = data;
}

READ8_MEMBER(albazg_state::mux_r)
{
	switch(m_mux_data)
	{
		case 0x00: return ioport("IN0")->read();
		case 0x01: return ioport("IN1")->read();
		case 0x02: return ioport("IN2")->read();
		case 0x04: return ioport("IN3")->read();
		case 0x08: return ioport("IN4")->read();
		case 0x10: return ioport("IN5")->read();
		case 0x20: return ioport("IN6")->read();
	}

	return 0xff;
}

WRITE8_MEMBER(albazg_state::mux_w)
{
	int new_bank = (data & 0xc0) >> 6;

	//0x10000 "Learn Mode"
	//0x12000 gameplay
	//0x14000 bonus game
	//0x16000 ?
	if( m_bank != new_bank)
	{
		m_bank = new_bank;
		membank("bank1")->set_entry(m_bank);
	}

	m_mux_data = data & ~0xc0;
}

WRITE8_MEMBER(albazg_state::yumefuda_output_w)
{
	machine().bookkeeping().coin_counter_w(0, ~data & 4);
	machine().bookkeeping().coin_counter_w(1, ~data & 2);
	machine().bookkeeping().coin_lockout_global_w(data & 1);
	//data & 0x10 hopper-c (active LOW)
	//data & 0x08 divider (active HIGH)
	flip_screen_set(~data & 0x20);
}

/***************************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, albazg_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa7fc, 0xa7fc) AM_WRITE(prot_lock_w)
	AM_RANGE(0xa7ff, 0xa7ff) AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0xaf80, 0xafff) AM_READWRITE(custom_ram_r, custom_ram_w) AM_SHARE("cus_ram")
	AM_RANGE(0xb000, 0xb07f) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xb080, 0xb0ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_WRITE(yumefuda_vram_w) AM_SHARE("videoram")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(yumefuda_cram_w) AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( port_map, AS_IO, 8, albazg_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x40, 0x40) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

/***************************************************************************************/

static INPUT_PORTS_START( yumefuda )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset SW") //doesn't work?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter SW")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Coin Out")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay Out")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Init SW")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Flip-Flop")  PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coupon") PORT_IMPULSE(2) //coupon
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note") PORT_IMPULSE(2)  //note
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 BET Button") PORT_CODE(KEYCODE_3) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start") PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_START("IN3")
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start") PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 BET Button") PORT_CODE(KEYCODE_3) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	/* Some bits of these three are actually used if you use the Royal Panel type */
	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_START("IN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	/* Unused, on the PCB there's just one bank */
	PORT_START("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/*Added by translating the manual*/
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Learn Mode" )//SW Dip-Switches
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, "Hopper Payout" )
	PORT_DIPSETTING(    0x04, "Hanafuda Type" )//hanaawase
	PORT_DIPSETTING(    0x00, "Royal Type" )
	PORT_DIPNAME( 0x08, 0x08, "Panel Type" )
	PORT_DIPSETTING(    0x08, "Hanafuda Panel" )//hanaawase
	PORT_DIPSETTING(    0x00, "Royal Panel" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )//Screen Orientation
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )//Screen Flip
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )//pressing Flip-Flop button makes the screen flip
INPUT_PORTS_END

/***************************************************************************************/

void albazg_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x2000);

	save_item(NAME(m_mux_data));
	save_item(NAME(m_bank));
	save_item(NAME(m_prot_lock));
}

void albazg_state::machine_reset()
{
	m_mux_data = 0;
	m_bank = -1;
	m_prot_lock = 0;
}

static MACHINE_CONFIG_START( yumefuda, albazg_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80 , MASTER_CLOCK/2) /* xtal is 12 Mhz, unknown divider*/
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(port_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", albazg_state,  irq0_line_hold)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_WATCHDOG_VBLANK_INIT(8) // timing is unknown

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(albazg_state, mux_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("SYSTEM"))
	MCFG_I8255_IN_PORTC_CB(READ8(albazg_state, mux_r))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(albazg_state, screen_update_yumefuda)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", H46505, "screen", MASTER_CLOCK/16)   /* hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", yumefuda )
	MCFG_PALETTE_ADD("palette", 0x80)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/16) /* guessed to use the same xtal as the crtc */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(albazg_state, yumefuda_output_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/***************************************************************************************/


ROM_START( yumefuda )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* code */
	ROM_LOAD("zg004y02.u43", 0x00000, 0x8000, CRC(974c543c) SHA1(56aeb318cb00445f133246dfddc8c24bb0c23f2d))
	ROM_LOAD("zg004y01.u42", 0x10000, 0x8000, CRC(ae99126b) SHA1(4ae2c1c804bbc505a013f5e3d98c0bfbb51b747a))

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD("zg001006.u6", 0x0000, 0x4000, CRC(a5df443c) SHA1(a6c088a463c05e43a7b559c5d0afceddc88ef476))
	ROM_LOAD("zg001005.u5", 0x4000, 0x4000, CRC(158b6cde) SHA1(3e335b7dc1bbae2edb02722025180f32ab91f69f))
	ROM_LOAD("zg001004.u4", 0x8000, 0x4000, CRC(d8676435) SHA1(9b6df5378948f492717e1a4d9c833ddc5a9e8225))
	ROM_LOAD("zg001003.u3", 0xc000, 0x4000, CRC(5822ff27) SHA1(d40fa0790de3c912f770ef8f610bd8c42bc3500f))

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD("zg1-007.u13", 0x000, 0x100, NO_DUMP ) //could be either PROM or PAL
ROM_END

GAME( 1991, yumefuda, 0, yumefuda, yumefuda, driver_device, 0, ROT0, "Alba", "Yumefuda [BET]", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
