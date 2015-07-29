// license:BSD-3-Clause
// copyright-holders:Charles MacDonald, David Haywood
/***************************************************************************

 Sega Monaco GP

 1980


 Board 96598-P is an oscillator board that generates the game sounds.  It is activated by outputs from Board Assy A (96577X).

 Board Assy's A (96577X) and B (96578X) are the main guts of the gameplay and contain the roms (all sprites).
 Board Assy A accepts all of the game inputs (Coin, wheel, shift, accelerator pedal), sends the     signals for the sounds to the Oscillator board, outputs to the L.E.D. score display board, and  directly  interacts with Board Assy B
 Board Assy B outputs the video, outputs to the L.E.D. score display board. and directly interacts with Board Assy A.

 On the second set, boards A and B have different part numbers (97091X and 97092X) and two less roms.  These boards were made later then the first set.
 These boards  seem to be interchangable, the only difference seems to be the rom size, which only 2 of the roms utilize the full 1024 bytes (second set).

 ROM SET A  <type 7461, 24 pin> ROM SET B   <type MB7132E, 24 pin>
 Board 96577X               Board 97091X
 ROM ID         IC#     ROM ID
 PRa125 (Light data)    IC59        PRb-01  (identical to PRa125)
 PRa126 (Explosion) IC65        PRb-02  (identical to PRa126)
 PRa131 (????)      IC71        PRb-04  (identical to PRa131)
 PRa127 (car(2))        IC77        PRb-13  (identical to PRa127)
 PRa128 (car(2)rotated) IC84        PRb-03  (identical to PRa128)
 PRa129 (car)       IC91        -----
 PRa130 (car(2) spinout)    IC98        PRb-14  (contains PRa130 and half of PRa129)
 PRa132 (car(2)(main))  IC111       PRb-15  (identical to PRa132)

 PRm-39         IC105       PRm-39          <both type 6331-1, 16 pin>
 PRm-38         IC115       PRm-38      <both type 6331-1, 16 pin>

 Board 96578X               Board 97092X
 ROM ID         IC#     ROM ID
 PRa140 (brdge-water)   IC12        -----
 PRa141 (brdge-pillar)  IC17        PRb-16  (contains both PRa140 and PRa141)
 PRa138 (firetruck) IC30        PRb-10  (identical to PRa138)
 PRa139 (car+bridge symb)IC51       PRb-11  (identical to PRa139)
 PRa133 (text(4)        IC64        PRb-05  (identical to PRa133)
 PRa136 (house)     IC99        PRb-08  (identical to PRa136)
 PRa135 (shrub)     IC106       PRb-07  (identical to PRa135)
 PRa134 (tree, grass)   IC113       PRb-06  (identical to PRa134)
 PRa137 (tunnel,oil slip)IC120      PRb-09  (identical to PRa137)

 Oscillator Board 96598
 ROM ID     IC#
 PRm-40     IC21            PRm-40      <both type 6331-1, 16 pin>

 --------------------------------------------------------

  7641

     512*8
    +------+
 A7 |1   24| Vcc
 A6 |2   23| A8
 A5 |3   22| NC
 A4 |4   21| CE1/
 A3 |5   20| CE2/
 A2 |6   19| CE3
 A1 |7   18| CE4
 A0 |8   17| D7
 D0 |9   16| D6
 D1 |10  15| D5
 D2 |11  14| D4
 GND|12  13| D3
    +------+
 -----------------------------------------------

  6331-1 PROM

           32*8
         +------+
 O1     |1     16| Vcc
 O2     |2     15| CE/
 O3     |3     14| A4
 O4     |4     13| A3
 O5     |5     12| A2
 O6     |6     11| A1
 O7     |7     10| A0
 GND    |8      9| O8
         +------+

 -----------------------------------------------

  7132:

     1024*8
    +------+
 A7 |1   24| Vcc
 A6 |2   23| A8
 A5 |3   22| A9
 A4 |4   21| CE1/
 A3 |5   20| CE2/
 A2 |6   19| CE3
 A1 |7   18| CE4
 A0 |8   17| D7
 D0 |9   16| D6
 D1 |10  15| D5
 D2 |11  14| D4
 GND|12  13| D3
    +------+


 ***************************************************************************/


#include "emu.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "video/fixfreq.h"

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
// end


class monacogp_state : public driver_device
{
public:
	monacogp_state(const machine_config &mconfig, device_type type, const char *tag)
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


static NETLIST_START(monacogp)
	SOLVER(Solver, 48000)
//  PARAM(Solver.FREQ, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	// schematics
	//...

//  NETDEV_ANALOG_CALLBACK(sound_cb, sound, exidyttl_state, sound_cb, "")
//  NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()



void monacogp_state::machine_start()
{
}

void monacogp_state::machine_reset()
{
}


void monacogp_state::video_start()
{
}

static MACHINE_CONFIG_START( monacogp, monacogp_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(monacogp)

	/* video hardware */
	MCFG_FIXFREQ_ADD("fixfreq", "screen")
	MCFG_FIXFREQ_MONITOR_CLOCK(MASTER_CLOCK)
	MCFG_FIXFREQ_HORZ_PARAMS(H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL)
	MCFG_FIXFREQ_VERT_PARAMS(V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL)
	MCFG_FIXFREQ_FIELDCOUNT(1)
	MCFG_FIXFREQ_SYNC_THRESHOLD(0.30)
MACHINE_CONFIG_END


/***************************************************************************

 Game driver(s)

 ***************************************************************************/


ROM_START( monacogp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "pra125.ic59",     0x0000, 0x0200, CRC(7a66ed4c) SHA1(514e129c334a551b931c90b063b073a9b4bdffc3) )
	ROM_LOAD( "pra126.ic65",     0x0000, 0x0200, CRC(5d7a8f12) SHA1(b4f0d21b91a7cf7002f99c08788669c7c38be51d) )
	ROM_LOAD( "pra131.ic71",     0x0000, 0x0200, CRC(ff31eb01) SHA1(fd6bcd92c4bd919bb1a96ca97688d46cb310b39d) )
	ROM_LOAD( "pra127.ic77",     0x0000, 0x0200, CRC(8ffdc2f0) SHA1(05cc3330c067965b8b90b5d27119fe9f26580a13) )
	ROM_LOAD( "pra128.ic84",     0x0000, 0x0200, CRC(dde29dea) SHA1(34c413edff991297471bd0bc193c4bd8ede4e468) )
	ROM_LOAD( "pra129.ic91",     0x0000, 0x0200, CRC(7b18af26) SHA1(3d1ff2610813544c3b9b65182f081272a9537640) )
	ROM_LOAD( "pra130.ic98",     0x0000, 0x0200, CRC(9ef1913b) SHA1(58830121781b8a13532eaf8ea13ec07f10522320) )
	ROM_LOAD( "pra132.ic111",    0x0000, 0x0200, CRC(6b8ad9bc) SHA1(be36e3b6b647d3a9565bc45903027c791dc889e5) )

	ROM_LOAD( "prm39.ic105",     0x0000, 0x0020, CRC(6acfa0da) SHA1(1e56da4cdf71a095eac29878969b831babac222b) )
	ROM_LOAD( "prm38.ic115",     0x0000, 0x0020, CRC(82dd0a0f) SHA1(3e7e475c3270853d70c1fe90a773172532b60cfb) )

	ROM_LOAD( "pra140.ic12",     0x0000, 0x0200, CRC(48e9971b) SHA1(c0c265cdc08727e3caaf49cdfe728a91c4c46ba2) )
	ROM_LOAD( "pra141.ic17",     0x0000, 0x0200, CRC(99934236) SHA1(ec271f3e690d5c57ead9132b22b9b1b966e4d170) )
	ROM_LOAD( "pra138.ic30",     0x0000, 0x0200, CRC(058e53cf) SHA1(7c3aaaca5a9e9ce3a3badd0dcc8360342673a397) )
	ROM_LOAD( "pra139.ic51",     0x0000, 0x0200, CRC(e8ba0794) SHA1(eadd7425134f26b1c126bbcd3d3dabf4b2e1fe70) )
	ROM_LOAD( "pra133.ic64",     0x0000, 0x0200, CRC(d50641d9) SHA1(bf399e9830c88e4d8f8fb386305f54ef766946d9) )
	ROM_LOAD( "pra136.ic99",     0x0000, 0x0200, CRC(ecc5d1a2) SHA1(33bff7381785557a85e4c8bdd74679b59e0ed9d5) )
	ROM_LOAD( "pra135.ic106",    0x0000, 0x0200, CRC(986eda32) SHA1(73fa539d4c83748952d9339985208520fec955f3) )
	ROM_LOAD( "pra134.ic113",    0x0000, 0x0200, CRC(8ebd50bb) SHA1(98d51f503753d4d7191a09b509d26c1e049e981a) )
	ROM_LOAD( "pra137.ic120",    0x0000, 0x0200, CRC(ddd9004e) SHA1(5229c34578e66d9c51a05439a516513946ba69ed) )

	ROM_LOAD( "prm40.ic21",      0x0000, 0x0020, CRC(87d12d57) SHA1(54682ce464449a3084cba29a82ff80288c87ad36) )
ROM_END


ROM_START( monacogpa )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x0400, "roms", ROMREGION_ERASE00 )
	ROM_LOAD( "prb-01.ic59",     0x0000, 0x0200, CRC(7a66ed4c) SHA1(514e129c334a551b931c90b063b073a9b4bdffc3) )
	ROM_LOAD( "prb-02.ic65",     0x0000, 0x0200, CRC(5d7a8f12) SHA1(b4f0d21b91a7cf7002f99c08788669c7c38be51d) )
	ROM_LOAD( "prb-04.ic71",     0x0000, 0x0200, CRC(ff31eb01) SHA1(fd6bcd92c4bd919bb1a96ca97688d46cb310b39d) )
	ROM_LOAD( "prb-13.ic77",     0x0000, 0x0200, CRC(8ffdc2f0) SHA1(05cc3330c067965b8b90b5d27119fe9f26580a13) )
	ROM_LOAD( "prb-03.ic84",     0x0000, 0x0200, CRC(dde29dea) SHA1(34c413edff991297471bd0bc193c4bd8ede4e468) )
	// nothing at IC91
	ROM_LOAD( "prb-14.ic98",     0x0000, 0x0400, CRC(8ec80107) SHA1(8182c52dae83a6fc876d484c543894fa768896ca) )
	ROM_LOAD( "prb-15.ic111",    0x0000, 0x0200, CRC(6b8ad9bc) SHA1(be36e3b6b647d3a9565bc45903027c791dc889e5) )

	ROM_LOAD( "prm39.ic105",     0x0000, 0x0020, CRC(6acfa0da) SHA1(1e56da4cdf71a095eac29878969b831babac222b) )
	ROM_LOAD( "prm38.ic115",     0x0000, 0x0020, CRC(82dd0a0f) SHA1(3e7e475c3270853d70c1fe90a773172532b60cfb) )

	// nothing at IC12
	ROM_LOAD( "prb-16.ic17",     0x0000, 0x0400, CRC(719c5ca4) SHA1(103f6f1ecf30a7c81eb0926d794d1c3bf7d5760b) )
	ROM_LOAD( "prb-10.ic30",     0x0000, 0x0200, CRC(058e53cf) SHA1(7c3aaaca5a9e9ce3a3badd0dcc8360342673a397) )
	ROM_LOAD( "prb-11.ic51",     0x0000, 0x0200, CRC(e8ba0794) SHA1(eadd7425134f26b1c126bbcd3d3dabf4b2e1fe70) )
	ROM_LOAD( "prb-05.ic64",     0x0000, 0x0200, CRC(d50641d9) SHA1(bf399e9830c88e4d8f8fb386305f54ef766946d9) )
	ROM_LOAD( "prb-08.ic99",     0x0000, 0x0200, CRC(ecc5d1a2) SHA1(33bff7381785557a85e4c8bdd74679b59e0ed9d5) )
	ROM_LOAD( "prb-07.ic106",    0x0000, 0x0200, CRC(986eda32) SHA1(73fa539d4c83748952d9339985208520fec955f3) )
	ROM_LOAD( "prb-06.ic113",    0x0000, 0x0200, CRC(8ebd50bb) SHA1(98d51f503753d4d7191a09b509d26c1e049e981a) )
	ROM_LOAD( "prb-09.ic120",    0x0000, 0x0200, CRC(ddd9004e) SHA1(5229c34578e66d9c51a05439a516513946ba69ed) )

	ROM_LOAD( "prm40.ic21",      0x0000, 0x0020, CRC(87d12d57) SHA1(54682ce464449a3084cba29a82ff80288c87ad36) )
ROM_END



GAME( 1980, monacogp,  0,         monacogp, 0, driver_device,  0, ROT0, "Sega", "Monaco GP (Set 1) [TTL]", MACHINE_IS_SKELETON )
GAME( 1980, monacogpa, monacogp,  monacogp, 0, driver_device,  0, ROT0, "Sega", "Monaco GP (Set 2) [TTL]", MACHINE_IS_SKELETON )
