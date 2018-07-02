// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

QTY     Type    clock   position    function
2x  2636            Programmable Video Interface
1x  2650    OSC/2 = 1.7897725 MHz       8-bit Microprocessor - main
1x  oscillator  3.579545 MHz

ROMs
QTY     Type    position    status
4x  2708    6F,6H,6L,6N     dumped
1x  N82S115     2B  dumped

RAMs
QTY     Type    position
2x  2101

*/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class subhuntr_state : public driver_device
{
public:
	subhuntr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void subhuntr(machine_config &config);

private:
	INTERRUPT_GEN_MEMBER(subhuntr_interrupt);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(subhuntr);
	uint32_t screen_update_subhuntr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void subhuntr_data_map(address_map &map);
	void subhuntr_io_map(address_map &map);
	void subhuntr_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(subhuntr_state, subhuntr)
{
}

uint32_t subhuntr_state::screen_update_subhuntr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void subhuntr_state::video_start()
{
}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void subhuntr_state::subhuntr_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1c00, 0x1fff).ram();
}

void subhuntr_state::subhuntr_io_map(address_map &map)
{
}

void subhuntr_state::subhuntr_data_map(address_map &map)
{
//  AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READWRITE( ,  )
//  AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE( ,  )
}

/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( subhuntr )
INPUT_PORTS_END


/***************************************************************************

  Machine Config/Interface

***************************************************************************/

void subhuntr_state::machine_start()
{
}

void subhuntr_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(subhuntr_state::subhuntr_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x03);
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( gfx_subhuntr )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END


MACHINE_CONFIG_START(subhuntr_state::subhuntr)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", S2650, 14318180/4/2)
	MCFG_DEVICE_PROGRAM_MAP(subhuntr_map)
	MCFG_DEVICE_IO_MAP(subhuntr_io_map)
	MCFG_DEVICE_DATA_MAP(subhuntr_data_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", subhuntr_state, subhuntr_interrupt)
	MCFG_S2650_SENSE_INPUT(READLINE("screen", screen_device, vblank))

//  MCFG_DEVICE_ADD("s2636", S2636, 0)
//  MCFG_S2636_WORKRAM_SIZE(0x100)
//  MCFG_S2636_OFFSETS(3, -21)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 29*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(subhuntr_state, screen_update_subhuntr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_subhuntr)

	MCFG_PALETTE_ADD("palette", 26)
	MCFG_PALETTE_INIT_OWNER(subhuntr_state, subhuntr)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* discrete sound */
MACHINE_CONFIG_END



/******************************************************************************/

ROM_START( subhuntr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mr21.6f",  0x0000, 0x0400, CRC(27847939) SHA1(e6b41b511fefac1e1e207eff2dac8c2963d47c5c) )
	ROM_LOAD( "mr22.6g",  0x0400, 0x0400, CRC(e9af1ee8) SHA1(451e88407a120444377a58b06b65152c57503533) )
	ROM_LOAD( "mr25.6l",  0x0800, 0x0400, CRC(8271c975) SHA1(c7192658b50d781ab1b94c2e8cb75c5be3539820) )
	ROM_LOAD( "mr24.6n",  0x0c00, 0x0400, CRC(385c4944) SHA1(84050b0356c9a3a36528dba768f2684e28c6c7c4) )

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "82s115.2b",   0x0000, 0x0200, CRC(6946c9de) SHA1(956b4bebe6960a73609deb75e1493c4127fd7f77) ) // ASCII, not much else
ROM_END

GAME(1979, subhuntr,  0,        subhuntr, subhuntr, subhuntr_state, empty_init, ROT0, "Model Racing", "Sub Hunter (Model Racing)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
