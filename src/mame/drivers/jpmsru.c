/**********************************************************************

    JPM S.R.U Hardware

    The Stepper Reel Unit (SRU) was the first JPM hardware platform
    to use CPU based technology as opposed to a purely mechanical
    setup.

    It really shows that this is the antecedent of System 80,
    the sound hardware seems to be a basic discrete circuit (complete
    with tone pot), and the hardware uses the older TMS9980A as a CPU.

    TODO: Everything!

**********************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9900l.h"

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
	MCFG_CPU_ADD("maincpu", TMS9980L, MAIN_CLOCK)

	MCFG_CPU_PROGRAM_MAP(jpmsru_map)
	MCFG_CPU_IO_MAP(jpmsru_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( jpmsru_4, jpmsru_state )
	MCFG_CPU_ADD("maincpu", TMS9980L, MAIN_CLOCK)

	MCFG_CPU_PROGRAM_MAP(jpmsru_4_map)
	MCFG_CPU_IO_MAP(jpmsru_io)
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



GAME(198?, j_ewnud	,0			,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "Barcrest?","Each Way Nudger (Barcrest?, set 1)",						GAME_IS_SKELETON_MECHANICAL )
GAME(198?, j_ewnda	,j_ewnud	,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "Barcrest?","Each Way Nudger (Barcrest?, set 2)",						GAME_IS_SKELETON_MECHANICAL )
GAME(198?, j_luckac	,0			,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "<unknown>","Lucky Aces (Unk)",						GAME_IS_SKELETON_MECHANICAL )
GAME(198?, j_super2	,0			,jpmsru,jpmsru, jpmsru_state,jpmsru,ROT0,   "JPM","Super 2 (JPM)",						GAME_IS_SKELETON_MECHANICAL )

// this one is different again?
GAME(198?, j_plus2	,0			,jpmsru_4,jpmsru, jpmsru_state,jpmsru,ROT0,   "JPM","Plus 2 (JPM)",						GAME_IS_SKELETON_MECHANICAL )
