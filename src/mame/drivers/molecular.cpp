// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***********************************************************************************

MOLECULAR Computer

Notes:

File Processor i/os
0x00 DMA
0x10-0x13 CTC
0x20-0x23 CTC (Serial A Clock, Serial B Clock, Parity Error IRQ, Bus IRQ)
0x40-0x43 SIO (Serial A Data, Serial A Control, Serial B Data, Serial B Control)
0x60-0x63 FDC (Control, Cylinder, Sector, Data)
0x70 PLOW Lower Parallel i/o
0x71 PHI Upper Parallel i/o
0x80-0x87 HDD (Status, Data, Result 0-5)

Application Processor i/os
0x00-0x03 PIO (Parallel A Data, Parallel A Control, Parallel B Data, Parallel B Control)
0x20 DMA
0x30-0x33 CTC (Serial A Clock, Serial B Clock, Parity Error IRQ, Bus IRQ)
0x60-0x63 SIO (Serial A Data, Serial A Control, Serial B Data, Serial B Control)

File Processor irqs:
0xf80c PIO Ch A
0xf80e PIO Ch B
0xf810-0xf81e SIO Vector N
0xf820-0xf826 CTC A, Channel N
0xf828-0xf82e CTC B, Channel N (do not use)
0xf830 DMA (do not use)

Application Processor irqs:
0xf80c PIO Ch A
0xf80e PIO Ch B
0xf810 DMA (do not use)
0xf820-0xf82e SIO Vector N

App Processor puts on sio port:
"??250 INITIALIZING..."

File Processor puts on sio port:
"??FP RESTART"

TODO:
- Info on HW ports is pretty scarse, other than a trim i/o documentation and schematics

***********************************************************************************/


#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
//#include "sound/ay8910.h"

#define I86_CLOCK XTAL_24MHz
#define Z80_CLOCK XTAL_16MHz

class molecula_state : public driver_device
{
public:
	molecula_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_filecpu(*this, "filecpu")
	{ }

	// devices
	required_device<cpu_device> m_filecpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 *m_file_rom;
	UINT8 *m_app_rom;
	std::unique_ptr<UINT8[]> m_file_ram;
	std::unique_ptr<UINT8[]> m_app_ram;
	DECLARE_READ8_MEMBER(file_r);
	DECLARE_WRITE8_MEMBER(file_w);

	DECLARE_READ8_MEMBER(app_r);
	DECLARE_WRITE8_MEMBER(app_w);

	DECLARE_WRITE8_MEMBER(file_output_w);
	DECLARE_WRITE8_MEMBER(app_output_w);

	DECLARE_READ8_MEMBER( sio_r );
	DECLARE_WRITE8_MEMBER( sio_w );

	UINT8 app_ram_enable;
	UINT8 file_ram_enable;

	DECLARE_PALETTE_INIT(molecula);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
};

void molecula_state::video_start()
{
}

UINT32 molecula_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

READ8_MEMBER(molecula_state::file_r)
{
	if(file_ram_enable)
		return m_file_ram[offset];

	return m_file_rom[offset & 0x7ff];
}


WRITE8_MEMBER(molecula_state::file_w)
{
	m_file_ram[offset] = data;
}

READ8_MEMBER(molecula_state::app_r)
{
	if(app_ram_enable)
		return m_app_ram[offset];

	return m_app_rom[offset & 0x7ff];
}


WRITE8_MEMBER(molecula_state::app_w)
{
	m_app_ram[offset] = data;
}

WRITE8_MEMBER(molecula_state::file_output_w)
{
	if(offset == 0)
		file_ram_enable = (data & 0x80) >> 7;

	if(data & 0x7f || offset)
		printf("FILE output -> %02x %02x\n",data,offset);
}


WRITE8_MEMBER(molecula_state::app_output_w)
{
	app_ram_enable = (data & 0x80) >> 7;

	if(data & 0x7f)
		printf("APP 0x10 -> %02x\n",data);
}

READ8_MEMBER( molecula_state::sio_r)
{
	if(offset == 1)
		return 4;

	return 0;
}

WRITE8_MEMBER( molecula_state::sio_w)
{
	if(offset == 0)
		printf("%c",data);
}

static ADDRESS_MAP_START( molecula_file_map, AS_PROGRAM, 8, molecula_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(file_r,file_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( molecula_file_io, AS_IO, 8, molecula_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x40, 0x43) AM_READWRITE(sio_r,sio_w)
	AM_RANGE(0x72, 0x73) AM_WRITE(file_output_w) // unknown
ADDRESS_MAP_END

static ADDRESS_MAP_START( molecula_app_map, AS_PROGRAM, 8, molecula_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(app_r,app_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( molecula_app_io, AS_IO, 8, molecula_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITE(app_output_w)
	AM_RANGE(0x60, 0x63) AM_READWRITE(sio_r,sio_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( molecula )
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

#if 0
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
#endif

static GFXDECODE_START( molecula )
//  GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void molecula_state::machine_start()
{
	m_file_rom = memregion("fileipl")->base();
	m_app_rom = memregion("appipl")->base();

	m_file_ram = make_unique_clear<UINT8[]>(0x10000);
	m_app_ram = make_unique_clear<UINT8[]>(0x10000);
}

void molecula_state::machine_reset()
{
	app_ram_enable = 0;
	file_ram_enable = 0;
}


PALETTE_INIT_MEMBER(molecula_state, molecula)
{
}

static MACHINE_CONFIG_START( molecula, molecula_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("filecpu",Z80,Z80_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(molecula_file_map)
	MCFG_CPU_IO_MAP(molecula_file_io)
	MCFG_DEVICE_DISABLE()

	MCFG_CPU_ADD("appcpu",Z80,Z80_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(molecula_app_map)
	MCFG_CPU_IO_MAP(molecula_app_io)

//  MCFG_CPU_ADD("sub",I8086,I86_CLOCK/2)
//  MCFG_CPU_PROGRAM_MAP(molecula_map)
//  MCFG_CPU_IO_MAP(molecula_io)
//  MCFG_DEVICE_DISABLE()

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(molecula_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", molecula)

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(molecula_state, molecula)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( molecula )
	ROM_REGION( 0x10000, "fileipl", ROMREGION_ERASE00 )
	ROM_LOAD( "sm32-111782-255-ff.bin", 0x000000, 0x000800, CRC(818f5f5a) SHA1(829b8fef99e6667d04d1944112823f3ea6a1a7e8) )

	ROM_REGION( 0x10000, "appipl", ROMREGION_ERASE00 )
	ROM_LOAD( "ap-111181-fa.bin", 0x000000, 0x000800, CRC(c289fc90) SHA1(2c0a17808971400e0289d65a11ef09e1f5eb90a0) )

	ROM_REGION( 0x1000, "x86ipl", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ap86eve_2716.bin", 0x000000, 0x000800, CRC(d6484a11) SHA1(19cfea72afbc796e5b59c47095529d6d6350dd98) )
	ROM_LOAD16_BYTE( "ap86odd_2716.bin", 0x000001, 0x000800, CRC(a4403c13) SHA1(2aeb07ad1f3381cbd1ec901e1f095ae737ded538) )

	/* same as ap-111181-fa.bin, with a different & bad byte at [3]*/
	ROM_REGION( 0x10000, "user2", ROMREGION_ERASE00 )
	ROM_LOAD( "ap-111181-253-fd.bin", 0x000000, 0x000800, CRC(10f16080) SHA1(10f13a04c525cd4b67e2b6e1a3eed46c619a439c) )

	/* proms */
	ROM_REGION( 0x10000, "user3", ROMREGION_ERASE00 )
	ROM_LOAD( "ms01_82s131.bin", 0x000000, 0x000200, CRC(433b139c) SHA1(32c1e853faacf85b11506a6903789ef129dd6adc) )
	ROM_LOAD( "ms11_82s131.bin", 0x000200, 0x000200, CRC(cff86fed) SHA1(554ec15799afc6305c8cae0f9d55b32004479cdb) )

	ROM_REGION( 0x10000, "misc", ROMREGION_ERASE00 )
	ROM_LOAD( "mem_16r4.jed", 0x000000, 0x00caef, CRC(a8cfbdfe) SHA1(92be58ab3299e0d3ac37bd1e258054c881b9ad7f) )
	ROM_LOAD( "rx_16r8.jed",  0x000000, 0x00caef, CRC(76f11ea8) SHA1(3ec41e5af04a89659f1b30270ee00e1fdf302ad6) )
	ROM_LOAD( "tx_16r8.jed",  0x000000, 0x00caef, CRC(a91dcf0b) SHA1(16d261f064a4af29c3da58f3ae8a867bdc953ce6) )
	ROM_LOAD( "wait_16r4.jed", 0x000000, 0x00caef, CRC(3aacfeb4) SHA1(1af1a8046e5a8a0337c85b55adceaef6e45702b7) )
ROM_END

COMP( 1982, molecula,  0,   0,   molecula,  molecula, driver_device,  0,  "MOLECULAR",      "MOLECULAR Computer", MACHINE_IS_SKELETON )
