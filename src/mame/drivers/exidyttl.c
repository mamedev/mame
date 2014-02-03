/***************************************************************************

Exidy discrete hardware games

 Alley Rally (1975)
 Attack (1977)
 Death Race (1976)
 Destruction Derby (1975)
 Football (1978)
 Old Time Basketball (1976)
 Spiders From Space (1976)
 Score (1977)
 Super Death Chase (1977)
 Table Football (1975)
 Tv Pinball (1974)
 
***************************************************************************/


#include "emu.h"

#include "machine/rescap.h"
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

fixedfreq_interface fixedfreq_mode_attack = {
	MASTER_CLOCK,
	H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL,
	V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL,
	1,  /* non-interlaced */
	0.30
};
// end


class exidyttl_state : public driver_device
{
public:
	exidyttl_state(const machine_config &mconfig, device_type type, const char *tag)
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


static NETLIST_START(attack)
	SOLVER(Solver)
	PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//	NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//	NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void exidyttl_state::machine_start()
{
}

void exidyttl_state::machine_reset()
{
}


void exidyttl_state::video_start()
{
}

static MACHINE_CONFIG_START( attack, exidyttl_state )

	/* basic machine hardware */
    MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(attack)

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_attack)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( attckexd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "attack.a1",     0x0000, 0x0200, CRC(5afd5aff) SHA1(051915c43b33f476597b2adddda7ae2f5d4fe214) )
	ROM_LOAD( "attack.b1",     0x0000, 0x0200, CRC(92d0fbf4) SHA1(31f7e04c4cd1fb36404b22a26d7e62939b43d941) )
	ROM_LOAD( "attack.c1",     0x0000, 0x0200, CRC(25625d6e) SHA1(d61ff867a226a01781e689c124b93159a92dc057) )
	ROM_LOAD( "attack.d1",     0x0000, 0x0200, CRC(2ff8dd6b) SHA1(21921faa26f95414070df12d30a9bcdc0b674c00) )
	ROM_LOAD( "attack.j6",     0x0000, 0x0200, CRC(21f87c1a) SHA1(1881142ecceee1175f1837e63b7258a08ed293d6) )
	ROM_LOAD( "attack.k6",     0x0000, 0x0200, CRC(ba5115b3) SHA1(1679b07e8a9376789c9cf15fe16e97003e9267be) )
ROM_END


GAME( 1977, attckexd,  0, attack, 0, driver_device,  0, ROT0, "Exidy", "Attack [TTL]", GAME_IS_SKELETON )
