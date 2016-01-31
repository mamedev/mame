// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/*************************************************************************

  Unknown 'Nibble' game
  
  Preliminary driver by Roberto Fresca.

**************************************************************************
  
  Specs:
  
  1x UM6845
  1x AY38910A/p

  3x HY6264P-12
  2x IMSG171P-50G

  2 Chips with no markings!

  8x 64K Graphics ROMs.
  1x 64K Program ROM.
  1x 128K unknown ROM.

  2x XTAL - 11.98135 KDS9C
  2x 8 DIP switches banks.


**************************************************************************

  Tech notes...

  About the unknown ICs:
  DIP64 CPU with Xtal tied to pins 30 % 31. --> TMS9900? (ROM 9)
  DIP40 CPU or sound IC driving 128k (ROM 10) data? (pin 20 tied to GND)


*************************************************************************/

#define MASTER_CLOCK    XTAL_12MHz

#include "emu.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"


class nibble_state : public driver_device
{
public:
	nibble_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	//  ,m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

//	required_device<cpu_device> m_maincpu;

};

static INPUT_PORTS_START( nibble )
INPUT_PORTS_END


void nibble_state::machine_start()
{
}

void nibble_state::machine_reset()
{
}


static MACHINE_CONFIG_START( nibble, nibble_state )

	/* basic machine hardware */
//  MCFG_CPU_ADD("maincpu", ??, 3000000) // unknown DIP64 CPU
//  MCFG_CPU_PROGRAM_MAP(nibble_map)
//  MCFG_CPU_IO_MAP(nibble_io)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", nibble_state,  irq0_line_hold)

	/* sound hardware */
//	MCFG_SPEAKER_STANDARD_MONO("mono")

//	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/8)
//	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( l9nibble )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "09.U123", 0x00000, 0x10000, CRC(dfef685d) SHA1(0aeb4257e408e8549df629a0cdb5f2b6790e32de) ) // unknown

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "01.U139", 0x00000, 0x10000, CRC(aba06e58) SHA1(5841beec122613eed2ba9f48cb1d51bfa0ff450c) )
	ROM_LOAD( "02.U141", 0x00000, 0x10000, CRC(a1e5d6d1) SHA1(8ec85b0544dd75bcb13600bae503ad2b20978281) )
	ROM_LOAD( "03.U149", 0x00000, 0x10000, CRC(ae66f77c) SHA1(6c9e98cc00b72252cb238f14686c0faef47134df) )
	ROM_LOAD( "04.U147", 0x00000, 0x10000, CRC(f1864094) SHA1(b439f9e8c2cc4575f9edbda45b9e724257015a73) )
	ROM_LOAD( "05.U137", 0x00000, 0x10000, CRC(2e8ae9de) SHA1(5f2831f71b351e34df82af37041c9aa815eb372c) )
	ROM_LOAD( "06.U143", 0x00000, 0x10000, CRC(8a56f324) SHA1(68790a12ca57c999bd7b7f26adc206aab3c06976) )
	ROM_LOAD( "07.U145", 0x00000, 0x10000, CRC(4f757912) SHA1(63e5fc2672552463060680b7a5a94df45f3d4b68) )
	ROM_LOAD( "08.U152", 0x00000, 0x10000, CRC(4f878ee4) SHA1(215f3ead0c358cc09c21515981cbb0a1e58c2ca6) )

	ROM_REGION( 0x20000, "user", 0 )
	ROM_LOAD( "10.U138", 0x00000, 0x20000, CRC(ed831d2a) SHA1(ce5c3b24979d220215d7f0e8d50f45550aec15bd) )

ROM_END


/*    YEAR  NAME      PARENT  MACHINE  INPUT   STATE          INIT  ROT    COMPANY    FULLNAME              FLAGS... */
GAME( 19??, l9nibble, 0,      nibble,  nibble, driver_device, 0,    ROT0, "Nibble?", "Unknown Nibble game", MACHINE_IS_SKELETON )
