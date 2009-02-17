/***************************************************************************

PACHI FEVER / SANKI DENSHI KOGYO

DORE GA CPU ? <- where's the CPU?

GEN6480830 (TEXAS INSTRUMENTS)
XTAL:12.000MHZ
RY050012   (TEXAS INSTRUMENTS)
XTAL:10.738MHZ

SOUND   :MSM5205 & ?

DIP SWITCH:8BIT x 3

============================================================================

Skeleton driver, the main issue is to decrypt/identify the main CPU.
ADPCM & remaining ROM space is pretty much identical to Jantotsu, maybe
there's a correlation? Or the dump is bad/missing something?

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"

VIDEO_START( pachifev )
{

}

VIDEO_UPDATE( pachifev )
{
	return 0;
}

static ADDRESS_MAP_START( pachifev_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pachifev_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static INPUT_PORTS_START( pachifev )
INPUT_PORTS_END

static UINT32 adpcm_pos;
//static UINT8 adpcm_idle;

static void pf_adpcm_int(const device_config *device)
{
	static UINT8 trigger,adpcm_data;

	if (adpcm_pos >= 0x4000/* || adpcm_idle*/)
	{
		//adpcm_idle = 1;
		msm5205_reset_w(0,1);
		trigger = 0;
	}
	else
	{
		UINT8 *ROM = memory_region(device->machine, "adpcm");

		adpcm_data = ((trigger ? (ROM[adpcm_pos] & 0x0f) : (ROM[adpcm_pos] & 0xf0)>>4) );
		msm5205_data_w(0,adpcm_data & 0xf);
		trigger^=1;
		if(trigger == 0)
		{
			adpcm_pos++;
			//if((ROM[adpcm_pos] & 0xff) == 0xff)
			//  adpcm_idle = 1;
		}
	}
}

static const msm5205_interface msm5205_config =
{
	pf_adpcm_int,	/* interrupt function */
	MSM5205_S48_4B	/* 8kHz */
};

static MACHINE_RESET( pachifev )
{
	adpcm_pos = 0;
}

static MACHINE_DRIVER_START( pachifev )

	/* basic machine hardware */
	MDRV_CPU_ADD("main",Z80,8000000/2)
	MDRV_CPU_PROGRAM_MAP(pachifev_map,0)
	MDRV_CPU_IO_MAP(pachifev_io,0)
//  MDRV_CPU_FLAGS(CPU_DISABLE)
//  MDRV_CPU_VBLANK_INT("main",irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_LENGTH(512)
	MDRV_MACHINE_RESET(pachifev)

	MDRV_VIDEO_START(pachifev)
	MDRV_VIDEO_UPDATE(pachifev)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("adpcm", MSM5205, 288000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pachifev )
	ROM_REGION( 0xc000, "main", 0 )
	ROM_LOAD( "ic42.00",   0x0000, 0x2000, CRC(9653546e) SHA1(0836d01118241d38bbf61732275afe3ae47d0622) )
	ROM_LOAD( "ic43.01",   0x2000, 0x2000, CRC(5572dce5) SHA1(fad45b33e095ac6e3ed3d7cdc3d8678c153a1b38) )
	ROM_LOAD( "ic44.02",   0x4000, 0x2000, CRC(98b3841f) SHA1(0563139877bf01e1673767ee1798bbcf68adadea) )
	ROM_LOAD( "ic45.03",   0x6000, 0x2000, CRC(6b76e6fa) SHA1(5be10ab0b76e2061fc7e9c77649572955bee7661) )
	ROM_LOAD( "ic46.04",   0x8000, 0x2000, CRC(1c8c66d7) SHA1(3b9b05f35b20d798651c7d5fdb35e6af956615a1) )
	ROM_LOAD( "ic48.50",   0xa000, 0x2000, CRC(1c8c66d7) SHA1(3b9b05f35b20d798651c7d5fdb35e6af956615a1) )

	ROM_REGION( 0x4000, "adpcm", 0 ) //msm code
	ROM_LOAD( "ic66.10",   0x0000, 0x2000, CRC(217c573e) SHA1(6fb90865d1d81f5ea00fa7916d0ccb6756ef5ce5) )
ROM_END

GAME( 1983, pachifev,  0,       pachifev,  pachifev,  0, ROT0, "Sanki Denshi Kogyo", "Pachi Fever", GAME_NOT_WORKING )
