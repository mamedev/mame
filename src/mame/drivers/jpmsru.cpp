// license:BSD-3-Clause
// copyright-holders:David Haywood
/**********************************************************************

    JPM S.R.U Hardware

    The Stepper Reel Unit (SRU) was the first JPM hardware platform
    to use CPU based technology as opposed to a purely mechanical
    setup.

    It really shows that this is the antecedent of System 80,
    the sound hardware seems to be a basic discrete circuit (complete
    with tone pot), and the hardware uses the older TMS9980A as a CPU.

    TODO: Everything!, there are 10 different SRU subtypes, including
    one purely for export, though this is a layout change rather than
    anything significant.

**********************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "jpmsru.lh"

class jpmsru_state : public driver_device
{
public:
	jpmsru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
public:
	DECLARE_DRIVER_INIT(jpmsru);
};

// blind guess
#define MAIN_CLOCK 6000000

/* System with RAM at 0x0c00 */

static ADDRESS_MAP_START( jpmsru_map, AS_PROGRAM, 8, jpmsru_state )
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0eff) AM_RAM
ADDRESS_MAP_END

/* System with RAM at 0x0e00 */

static ADDRESS_MAP_START( jpmsru_4_map, AS_PROGRAM, 8, jpmsru_state )
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0eff) AM_RAM
	AM_RANGE(0x0f00, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jpmsru_io, AS_IO, 8, jpmsru_state )
ADDRESS_MAP_END


static INPUT_PORTS_START( jpmsru )
INPUT_PORTS_END

static MACHINE_CONFIG_START( jpmsru, jpmsru_state )
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, MAIN_CLOCK, jpmsru_map, jpmsru_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( jpmsru_4, jpmsru_state )
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, MAIN_CLOCK, jpmsru_4_map, jpmsru_io)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(jpmsru_state,jpmsru)
{
}

ROM_START( j_ewnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn1a26", 0x0000, 0x000400, CRC(a92760b7) SHA1(dfef0dab7799a4b4975755c1584efca81a3798c4) )
	ROM_LOAD( "ewn26.2", 0x0400, 0x000400, CRC(bd24e59e) SHA1(038ed23283a7b61e873f543de32b685630fcdb97) )
	ROM_LOAD( "ewn26.3", 0x0800, 0x000400, CRC(a3280b35) SHA1(2771c81735c69ae3efb02715ac97901dae434e72) )
ROM_END

ROM_START( j_ewnda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn1.bin", 0x0000, 0x000400, CRC(84ce735e) SHA1(98bae928246050ae88518ca511447fbef5c810f5) )
	ROM_LOAD( "ewn2.bin", 0x0400, 0x000400, CRC(4c121f5e) SHA1(1221ff91ff9e352efeabb26a60eab93aae5bca5e) )
	ROM_LOAD( "ewn3.bin", 0x0800, 0x000400, CRC(bef3a938) SHA1(6a6844203c6361b65f5b07853d9dbe18a29ebc44) )
ROM_END

ROM_START( j_ewnd20 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn20.1", 0x0000, 0x000400, CRC(e90f686b) SHA1(aec88647c6289b01149b2816845a568481b1d37f) )
	ROM_LOAD( "ewn20.2", 0x0400, 0x000400, CRC(c02a2427) SHA1(57144443a03db56a803b19e14e868b1ccd222f37) )
	ROM_LOAD( "ewn20.3", 0x0800, 0x000400, CRC(a64e4df7) SHA1(1512c3c85e100dadd5ff67fed731feb69cc8575e) )
ROM_END

ROM_START( j_ews )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews13c1.bin", 0x0000, 0x000400, CRC(2eec7c4d) SHA1(a1740d27e60192659392ba7602b9b62947c4f6db) )
	ROM_LOAD( "ews13b2.bin", 0x0400, 0x000400, CRC(b84b7858) SHA1(90fd64881d52e1f4362ccbcb9434dbf7b25b97f9) )
	ROM_LOAD( "ews13.3",     0x0800, 0x000400, CRC(4d8e197a) SHA1(1569327f0e4b5d7632658b69abf59076effb2600) )
ROM_END

ROM_START( j_ews8a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews8a.1", 0x0000, 0x000400, CRC(52e9709a) SHA1(0b437834f48ca7718e0b30303916eed00c7fb4c9) )
	ROM_LOAD( "ews8a.2", 0x0400, 0x000400, CRC(ee4a4809) SHA1(292a12a5ddc5a22c8568016b34dfec7959f49027) )
	ROM_LOAD( "ews8a.3", 0x0800, 0x000400, CRC(3700a7a3) SHA1(cf24a54e6aa3a3a86ff75f6e8bcb692d0cfd0e80) )
ROM_END

ROM_START( j_luckac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la1.bin", 0x0000, 0x000400, CRC(21076280) SHA1(d5cf25d289f03c743f4428273ac002df3164c344) )
	ROM_LOAD( "la2.bin", 0x0400, 0x000400, CRC(cae10bc1) SHA1(a740946437a3b277b714f13d001783987f57bc77) )
	ROM_LOAD( "la3.bin", 0x0800, 0x000400, CRC(cb9362ac) SHA1(a16d43ba01b24e1b515881957c1559d33a03bcc4) )
ROM_END



ROM_START( j_plus2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plus2_1.bin", 0x0000, 0x000400, CRC(f635174d) SHA1(9478aabc0eaa25d4ae44d2385e738584f03f6647) )
	ROM_LOAD( "plus2_2.bin", 0x0400, 0x000400, CRC(0999d32f) SHA1(e08c852f8f3aff8ab7b73e9c0b0502ab91f9e844) )
	ROM_LOAD( "plus2_3.bin", 0x0800, 0x000400, CRC(d3dfd6ab) SHA1(4cf0f8977fb2c023bf2ccc8d9d74352ce32206bf) )
	ROM_LOAD( "plus2_4.bin", 0x0c00, 0x000400, CRC(8b6922b4) SHA1(7b7fc7b0708bf96846860254fea957bcbc952923) )
ROM_END



ROM_START( j_super2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super2_1.bin", 0x0000, 0x000400, CRC(a1df2719) SHA1(eed80329c14ef6c272a8c622e8a4bc7d14ac87e6) )
	ROM_LOAD( "super2_2.bin", 0x0400, 0x000400, CRC(0fd5ddd0) SHA1(e8d31b009b29486d36d11052af857c609a7f1f84) )
	ROM_LOAD( "super2_3.bin", 0x0800, 0x000400, CRC(ddd998d3) SHA1(5964da70ae4c2f174dc3d1494fc67579c221a7b7) )
ROM_END

ROM_START( j_luck2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lt_9.1", 0x0000, 0x000400, CRC(97236ce3) SHA1(f71861576f33daec3e1d371c670b535e6fd32b5e) )
	ROM_LOAD( "lt_9.2", 0x0400, 0x000400, CRC(6e1cd083) SHA1(17edaa9880ae2a6d6d99e771e41b985527d5ed3b) )
	ROM_LOAD( "lt_9.3", 0x0800, 0x000400, CRC(d6881e6f) SHA1(42a83f01d67a8f530ca2a10ffeff30237bdfba94) )
ROM_END

ROM_START( j_nuddup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ndu10_1.p1", 0x0000, 0x000400, CRC(f2210c00) SHA1(34a18028661a5ac0064b8c5c2f09d3918942be6e) )
	ROM_LOAD( "ndu10_1.p2", 0x0400, 0x000400, CRC(69243c04) SHA1(958791fbd515ab6e2b38391527b611977303ad10) )
	ROM_LOAD( "ndu10_1.p3", 0x0800, 0x000400, CRC(9f67e2f7) SHA1(f850655ba5d3651ff91f624431deb0e008fab57e) )
ROM_END

ROM_START( j_nuddup2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nduset1-1.bin", 0x0000, 0x000400, CRC(66445282) SHA1(8614b5330d72ed28141974e60a2238e003f4bce1) )
	ROM_LOAD( "nduset1-2.bin", 0x0400, 0x000400, CRC(2945e808) SHA1(e306b5f9cc9f4999b9b4b8536101f2b69728f6ca) )
	ROM_LOAD( "nduset1-3.bin", 0x0800, 0x000400, CRC(f4359851) SHA1(43c17c147a96aba901435154de657594fbec6008) )
ROM_END

ROM_START( j_unk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sruunk1.p1", 0x0000, 0x000400, CRC(be7d3b79) SHA1(3304dcc69e93eca2e6e89df0b18afc6874ebacf0) )
	ROM_LOAD( "sruunk1.p2", 0x0400, 0x000400, CRC(bf19cd60) SHA1(77b0b439628589cb0db1b74a760b652519c20991) )
	ROM_LOAD( "sruunk1.p3", 0x0800, 0x000400, CRC(25138e03) SHA1(644fc6144ea74f08dc892f106ad494ba364afe86) )
ROM_END

GAME(198?, j_ewnud  ,0          ,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "Barcrest?","Each Way Nudger (Barcrest?, set 1)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j_ewnda  ,j_ewnud    ,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "Barcrest?","Each Way Nudger (Barcrest?, set 2)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j_ewnd20 ,j_ewnud    ,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "Barcrest?","Each Way Nudger (Barcrest?, set 3, version 20?)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j_ews    ,0          ,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "Barcrest?","Each Way Shifter (Barcrest?, set 1, version 16)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j_ews8a  ,j_ews      ,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "Barcrest?","Each Way Shifter (Barcrest?, set 2, version 8a)",                      MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j_luckac ,0          ,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "<unknown>","Lucky Aces (Unk)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j_super2 ,0          ,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "JPM","Super 2 (JPM)",                      MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j_luck2  ,0          ,jpmsru_4,jpmsru, jpmsru_state,jpmsru,ROT0,   "<unknown>","Lucky Twos?",                        MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j_nuddup ,0          ,jpmsru_4,jpmsru, jpmsru_state,jpmsru,ROT0,   "JPM","Nudge Double Up (JPM SRU, set 1)",                     MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j_nuddup2,j_nuddup   ,jpmsru_4,jpmsru, jpmsru_state,jpmsru,ROT0,   "JPM","Nudge Double Up (JPM SRU, set 2)",                     MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j_unk    ,0          ,jpmsru_4,jpmsru, jpmsru_state,jpmsru,ROT0,   "JPM?","unknown SRU Game (JPM?)",                     MACHINE_IS_SKELETON_MECHANICAL )

// this one is different again?
GAME(198?, j_plus2  ,0          ,jpmsru_4,jpmsru, jpmsru_state,jpmsru,ROT0,   "JPM","Plus 2 (JPM)",                     MACHINE_IS_SKELETON_MECHANICAL )
