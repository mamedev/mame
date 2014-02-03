/***************************************************************************

    esqmr.c - Ensoniq MR-61, MR-76, and MR-Rack

    Skeleton driver by R. Belmont

    Hardware:
        CPU: 68340 MCU
        Sound: 2xES5506
        Effects: ES5511

    Memory map:

    0x000000-0x0FFFFF   OS ROM

***************************************************************************/

#include "emu.h"
#include "machine/68340.h"
#include "sound/es5506.h"

#include "machine/esqvfd.h"

class esqmr_state : public driver_device
{
public:
	esqmr_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sq1vfd(*this, "sq1vfd")
	{ }

	required_device<m68340cpu_device> m_maincpu;
	required_device<esq2x40_sq1_t> m_sq1vfd;

	virtual void machine_reset();

public:
	DECLARE_DRIVER_INIT(mr);
};

void esqmr_state::machine_reset()
{
}

static ADDRESS_MAP_START( mr_map, AS_PROGRAM, 32, esqmr_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_REGION("maincpu", 0)
//  AM_RANGE(0x200000, 0x20003f) AM_DEVREADWRITE8("ensoniq", es5506_device, read, write, 0xffffffff)
//  AM_RANGE(0x240000, 0x24003f) AM_DEVREADWRITE8("ensoniq2", es5506_device, read, write, 0xffffffff)
//    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static void esq5506_otto_irq(device_t *device, int state)
{
}

static READ16_DEVICE_HANDLER(esq5506_read_adc)
{
	return 0;
}

static const es5506_interface es5506_config =
{
	"waverom",  /* Bank 0 */
	"waverom2", /* Bank 1 */
	"waverom3", /* Bank 0 */
	"waverom4", /* Bank 1 */
	1,          /* channels */
	DEVCB_LINE(esq5506_otto_irq), /* irq */
	DEVCB_DEVICE_HANDLER(DEVICE_SELF, esq5506_read_adc)
};

static const es5506_interface es5506_2_config =
{
	"waverom",  /* Bank 0 */
	"waverom2", /* Bank 1 */
	"waverom3", /* Bank 0 */
	"waverom4", /* Bank 1 */
	1,          /* channels */
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( mr, esqmr_state )
	MCFG_CPU_ADD("maincpu", M68340, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(mr_map)

	MCFG_ESQ2x40_SQ1_ADD("sq1vfd")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5506, XTAL_16MHz)
	MCFG_SOUND_CONFIG(es5506_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
	MCFG_SOUND_ADD("ensoniq2", ES5506, XTAL_16MHz)
	MCFG_SOUND_CONFIG(es5506_2_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_CONFIG_END

static INPUT_PORTS_START( mr )
INPUT_PORTS_END

ROM_START( mr61 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "mrw-osf-11af-2.10.bin",  0x000000, 0x080000, CRC(5854314e) SHA1(8fb2e2ee2f5fb12eae8ea33cb18f757efaec6780) )
	ROM_LOAD16_WORD_SWAP( "mrw-romc-32ef-1.20.bin", 0x080000, 0x080000, CRC(68321347) SHA1(56cb96943ba42c35ba2787a49b5f4adf7c8dffb8) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

ROM_START( mrrack )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "mr-rack-150-lo.bin", 0x000001, 0x080000, CRC(b29988a1) SHA1(986c2def11de27fa2b9be55ac32f7fec0c414bca) )
	ROM_LOAD16_BYTE( "mr-rack-150-up.bin", 0x000000, 0x080000, CRC(71511692) SHA1(54744f16f1db1ac5abb2f70b6e04aebf1e0e029d) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

DRIVER_INIT_MEMBER(esqmr_state, mr)
{
}

CONS( 1996, mr61,   0, 0, mr, mr, esqmr_state, mr, "Ensoniq", "MR-61 Workstation", GAME_NOT_WORKING )
CONS( 1996, mrrack, 0, 0, mr, mr, esqmr_state, mr, "Ensoniq", "MR-Rack", GAME_NOT_WORKING )
