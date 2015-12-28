// license:BSD-3-Clause
// copyright-holders:
/*

 Tatsunoko Vs Capcom : Cross Generation of Heroes

 Wii derived hardware

 -- todo, add more hardware info, seems to have been lost at some point

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"

class tvcapcom_state : public driver_device
{
public:
	tvcapcom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update_tvcapcom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<ppc_device> m_maincpu;
};

static ADDRESS_MAP_START( gc_map, AS_PROGRAM, 64, tvcapcom_state )
ADDRESS_MAP_END


void tvcapcom_state::machine_start()
{
}

void tvcapcom_state::video_start()
{
}


UINT32 tvcapcom_state::screen_update_tvcapcom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( tvcapcom )
INPUT_PORTS_END

static MACHINE_CONFIG_START( tvcapcom, tvcapcom_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC603, 72900000) // IBM PowerPC Broadway CPU @ 729 MHz  ? 
	MCFG_CPU_PROGRAM_MAP(gc_map)
	MCFG_DEVICE_DISABLE()  

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(tvcapcom_state, screen_update_tvcapcom)

	MCFG_PALETTE_ADD("palette", 65536)

MACHINE_CONFIG_END

ROM_START( tvcapcom )
	// Bios??

	ROM_REGION( 0x21000898, "flash", ROMREGION_ERASE) // it's possible all these dumps are bad
	
	ROM_LOAD("tvc_read1.u14", 0x00, 0x21000898, CRC(51de96ac) SHA1(11a631a695140efa299e6fe9e68c0026dcebc766) ) // old dump, lots of data repeats, seems to cut out in the middle of a block?!

	// alt attempts at dumping the same rom, same weird size as above.. the same CRC was read consistently each time, but then differed when read again later without being used inbetween, some blocks seem to get swapped around?!
	ROM_LOAD("tvc_read2.u14",  0x00, 0x21000898, CRC(efb5911f) SHA1(555e58c2d3744cfe649b7424937c07a3ce675838))
	ROM_LOAD("tvc_read3.u14",  0x00, 0x21000898, CRC(4a3f143d) SHA1(83bb7abc5f925df9c4e28de0298aed7b3b791e37))

ROM_END

GAME( 2008, tvcapcom,  0, tvcapcom,    tvcapcom, driver_device, 0, ROT0, "Capcom",            "Tatsunoko Vs Capcom : Cross Generation of Heroes", MACHINE_IS_SKELETON )

