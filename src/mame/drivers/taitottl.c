/***************************************************************************

 Taito Discrete Hardware Games


 Game Name(s)                                Part #'s    Data      PROM/ROM Chip Numbers
 ------------------------------------------+-----------+---------+---------------------------------------
 Acrobat (1978)                                          UNKNOWN
 Astro Race (1973)                                       UNKNOWN
 Avenger (1976) - Vertical                   EG-1020     UNKNOWN
 Attack (1976)                                           UNKNOWN
 Attack UFO (1974)                                       UNKNOWN
 Barricade II (1977)                                     UNKNOWN
 Basketball (1974)                                       UNKNOWN
 Cisco/Fisco 400 (1977)                                  UNKNOWN
 Crashing Race (1976)                                    UNKNOWN
 Crossfire (1977)                                        UNKNOWN - AKA Bazooka
 Davis Cup (1973)                                        UNKNOWN
 Elepong (1973)                                          UNKNOWN
 Flying Fortress/Flying Fortress II (1977)               UNKNOWN
 Gunman (1977)                                           YES        8 x 32bytes (or 11? )
 Interceptor (1976)                                      UNKNOWN
 Pro Hockey (1973)                                       UNKNOWN
 Road Champion (1978)                                    UNKNOWN
 Sky Fighter 2 (1970)                                    UNKNOWN
 Soccer (1973)                                           UNKNOWN
 Speed Race (1974)                                       UNKNOWN
 Speed Race Twin (1976)                                  UNKNOWN
 Speed Race GP-5 (1980)                                  UNKNOWN
 Sub Hunter (1977)                                       UNKNOWN
 Super Block (1978) - Vertical                           UNKNOWN
 Super Speed Race 5 (1978)                               UNKNOWN
 Super Speed Race GP-V (1980)                            UNKNOWN
 Top Bowler (1978)                                       UNKNOWN
 T. T. Block (1977)                                      YES        1 x 2048bytes
 Trampoline (1978)                                       UNKNOWN
 Wall Block (1978)                                       UNKNOWN
 Western Gun (1975)                                      UNKNOWN
 Zun Zun Block (1979)                                    YES        3 - (2 x 512bytes, 1 x 32bytes)

***************************************************************************/


#include "emu.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "video/fixfreq.h"
#include "astring.h"

// copied by Pong, not accurate for this driver!
// start
#define MASTER_CLOCK    7159000
#define V_TOTAL         (0x105+1)       // 262
#define H_TOTAL         (0x1C6+1)       // 454

#define HBSTART                 (H_TOTAL)
#define HBEND                   (80)
#define VBSTART                 (V_TOTAL)
#define VBEND                   (16)

#define HRES_MULT                   (1)

fixedfreq_interface fixedfreq_mode_taito = {
	MASTER_CLOCK,
	H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL,
	V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL,
	1,  /* non-interlaced */
	0.30
};
// end


class taitottl_state : public driver_device
{
public:
	taitottl_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "fixfreq")
	{
	}

	// devices
	required_device<netlist_mame_device_t> m_maincpu;
	required_device<fixedfreq_device> m_video;

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

private:

};


static NETLIST_START(taitottl)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

	//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
	//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void taitottl_state::machine_start()
{
}

void taitottl_state::machine_reset()
{
}


void taitottl_state::video_start()
{
}

static MACHINE_CONFIG_START( taitottl, taitottl_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(taitottl)

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_taito)
MACHINE_CONFIG_END


/***************************************************************************

 Game driver(s)

 ***************************************************************************/

/*

 label  loc. Part #
 ==========================================================
 CR11   N5  74s287      yes, it says CRxx not GNxx
 GN10   H5  5623
 GN09   N6  soldered in
 GN08   J7  IM5200CJG
 GN07   L9  soldered in
 GN06   H6  7621
 GN05   G9  74s287
 GN04   F9  7643
 GN03   D9  7621
 GN02   C9  74s287
 GN01   B9  7643

*/

ROM_START( gunman )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "cr11.n5",     0x0000, 0x0200, CRC(8e2ec052) SHA1(b849080c9ed325a478739fbdf315c8a6c2bb7bd7) )
	ROM_LOAD( "gn10.h5",     0x0000, 0x0200, CRC(a976c4c7) SHA1(f2fb56ccec99ceb19d607bee3b4cca31d6db58fd) )
	ROM_LOAD( "gn06.h6",     0x0000, 0x0400, CRC(17376de6) SHA1(4ef660da9d37024a13925af12a7e102a49c67ff6) )
	ROM_LOAD( "gn05.g9",     0x0000, 0x0200, CRC(f3d7f6cb) SHA1(7abf445cc814dc242ebf2393e9c1335ba4aedb90) )
	ROM_LOAD( "gn04.f9",     0x0000, 0x0800, CRC(dec942db) SHA1(165d8594e07837fd4a87895f0a350d5ed8112ead) )
	ROM_LOAD( "gn03.d9",     0x0000, 0x0400, CRC(b0e6e473) SHA1(9665608b7ec1dfb39e4e1df74adbfc773e9a6aa7) )
	ROM_LOAD( "gn02.c9",     0x0000, 0x0200, CRC(897ae7b8) SHA1(2e91bc8336f8620faedd5329ce7d9734f19a2cfa) )
	ROM_LOAD( "gn01.b9",     0x0000, 0x0800, CRC(69474a3a) SHA1(7396f1be991a0e6207eaa79e0206a5286a8d615d) )
ROM_END


ROM_START( ttblock )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0800, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "tl01.4a",     0x0000, 0x0800, CRC(65b730f7) SHA1(f82931c9a128021c97d1d41b5eac05df55dd5994) ) // MMI 6353
ROM_END


ROM_START( zzblock )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "tz01.1e",     0x0000, 0x0200, CRC(bd1e59e0) SHA1(038ad0b1b629c6c04f4d885fce6fe1c1f72b33e5) )
	ROM_LOAD( "tz02.1d",     0x0000, 0x0200, CRC(befc8ad3) SHA1(80ce2a9978d22d45ccbe055c7dafba5c0ad6cf3c) )
	ROM_LOAD( "tz03.6f",     0x0000, 0x0020, CRC(1ba385b1) SHA1(b70c356174a1748723c0116ae8dd74b25e1fe6b4) )
ROM_END


GAME( 1977, gunman,   0, taitottl, 0, driver_device,  0, ROT0, "Taito", "Gunman [TTL]", GAME_IS_SKELETON )
GAME( 1977, ttblock,  0, taitottl, 0, driver_device,  0, ROT0, "Taito", "T. T. Block [TTL]", GAME_IS_SKELETON )
GAME( 1979, zzblock,  0, taitottl, 0, driver_device,  0, ROT0, "Taito", "Zun Zun Block [TTL]", GAME_IS_SKELETON )
