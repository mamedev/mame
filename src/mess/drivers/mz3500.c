/***************************************************************************

Template for skeleton drivers

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
//#include "sound/ay8910.h"

#define MAIN_CLOCK XTAL_8MHz

class mz3500_state : public driver_device
{
public:
	mz3500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_master(*this, "master"),
			m_slave(*this, "slave")
	{ }

	// devices
	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
	UINT8 *m_ipl_rom;
	UINT8 *m_basic_rom;
	UINT8 *m_work_ram;
	UINT8 *m_shared_ram;

	DECLARE_READ8_MEMBER(mz3500_master_mem_r);
	DECLARE_WRITE8_MEMBER(mz3500_master_mem_w);
	DECLARE_READ8_MEMBER(mz3500_ipl_r);
	DECLARE_READ8_MEMBER(mz3500_basic_r);
	DECLARE_READ8_MEMBER(mz3500_work_ram_r);
	DECLARE_WRITE8_MEMBER(mz3500_work_ram_w);
	DECLARE_READ8_MEMBER(mz3500_shared_ram_r);
	DECLARE_WRITE8_MEMBER(mz3500_shared_ram_w);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void mz3500_state::video_start()
{
}

UINT32 mz3500_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

READ8_MEMBER(mz3500_state::mz3500_ipl_r)
{
	return m_ipl_rom[offset];
}

READ8_MEMBER(mz3500_state::mz3500_basic_r)
{
	return m_basic_rom[offset];
}

READ8_MEMBER(mz3500_state::mz3500_work_ram_r)
{
	return m_work_ram[offset];
}

WRITE8_MEMBER(mz3500_state::mz3500_work_ram_w)
{
	m_work_ram[offset] = data;
}


READ8_MEMBER(mz3500_state::mz3500_master_mem_r)
{
	if((offset & 0xe000) == 0x0000) { return mz3500_ipl_r(space,(offset & 0xfff) | 0x1000); }
	if((offset & 0xe000) == 0x2000) { return mz3500_basic_r(space,offset & 0x1fff); }
	if((offset & 0xe000) == 0x4000) { return mz3500_work_ram_r(space,(offset & 0x1fff) | 0x4000); }
	if((offset & 0xe000) == 0x6000) { return mz3500_work_ram_r(space,(offset & 0x1fff) | 0x6000); }
	if((offset & 0xe000) == 0x8000) { return mz3500_work_ram_r(space,(offset & 0x1fff) | 0x8000); }
	if((offset & 0xe000) == 0xa000) { return mz3500_work_ram_r(space,(offset & 0x1fff) | 0xa000); }
	if((offset & 0xe000) == 0xc000) { return mz3500_work_ram_r(space,(offset & 0x1fff) | 0xc000); }
	if((offset & 0xe000) == 0xe000) { return mz3500_work_ram_r(space,(offset & 0x1fff) | 0xe000); }

	return 0xff;
}

WRITE8_MEMBER(mz3500_state::mz3500_master_mem_w)
{
	if((offset & 0xe000) == 0x4000) { mz3500_work_ram_w(space,(offset & 0x1fff) | 0x4000,data); return; }
	if((offset & 0xe000) == 0x6000) { mz3500_work_ram_w(space,(offset & 0x1fff) | 0x6000,data); return; }
	if((offset & 0xe000) == 0x8000) { mz3500_work_ram_w(space,(offset & 0x1fff) | 0x8000,data); return; }
	if((offset & 0xe000) == 0xa000) { mz3500_work_ram_w(space,(offset & 0x1fff) | 0xa000,data); return; }
	if((offset & 0xe000) == 0xc000) { mz3500_work_ram_w(space,(offset & 0x1fff) | 0xc000,data); return; }
	if((offset & 0xe000) == 0xe000) { mz3500_work_ram_w(space,(offset & 0x1fff) | 0xe000,data); return; }

}

READ8_MEMBER(mz3500_state::mz3500_shared_ram_r)
{
	return m_shared_ram[offset];
}

WRITE8_MEMBER(mz3500_state::mz3500_shared_ram_w)
{
	m_shared_ram[offset] = data;
}

static ADDRESS_MAP_START( mz3500_master_map, AS_PROGRAM, 8, mz3500_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mz3500_master_mem_r,mz3500_master_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz3500_master_io, AS_IO, 8, mz3500_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz3500_slave_map, AS_PROGRAM, 8, mz3500_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("ipl", 0)
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(mz3500_shared_ram_r, mz3500_shared_ram_w)
	AM_RANGE(0x4000, 0x5fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz3500_slave_io, AS_IO, 8, mz3500_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( mz3500 )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( mz3500 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void mz3500_state::machine_start()
{
	m_ipl_rom = memregion("ipl")->base();
	m_basic_rom = memregion("basic")->base();
	m_work_ram = auto_alloc_array_clear(machine(), UINT8, 0x40000);
	m_shared_ram = auto_alloc_array_clear(machine(), UINT8, 0x800);
}

void mz3500_state::machine_reset()
{
}


void mz3500_state::palette_init()
{
}

static MACHINE_CONFIG_START( mz3500, mz3500_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("master",Z80,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(mz3500_master_map)
	MCFG_CPU_IO_MAP(mz3500_master_io)

	MCFG_CPU_ADD("slave",Z80,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(mz3500_slave_map)
	MCFG_CPU_IO_MAP(mz3500_slave_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(mz3500_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_GFXDECODE(mz3500)

	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mz3500 )
	ROM_REGION( 0x2000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "mz-3500_ipl-rom_2-0a_m5l2764k.bin", 0x000000, 0x002000, CRC(119708b9) SHA1(de81979608ba6ab76f09088a92bfd1a5bc42530e) )

	ROM_REGION( 0x8000, "basic", ROMREGION_ERASE00 )
	ROM_LOAD( "basic.rom", 0x00000, 0x8000, NO_DUMP )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "mz-3500_cg-rom_2-b_m5l2764k.bin", 0x000000, 0x002000, CRC(29f2f80a) SHA1(64b307cd9de5a3327e3ec9f3d0d6b3485706f436) )
ROM_END

GAME( 198?, mz3500,  0,   mz3500,  mz3500, driver_device,  0,       ROT0, "Sharp",      "MZ-3500", GAME_IS_SKELETON )
