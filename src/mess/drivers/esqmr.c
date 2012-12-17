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
#include "cpu/m68000/m68000.h"
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

    required_device<m68340_device> m_maincpu;
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
//  AM_RANGE(0x200000, 0x20003f) AM_DEVREADWRITE8_LEGACY("ensoniq", es5506_r, es5506_w, 0xffffffff)
//  AM_RANGE(0x240000, 0x24003f) AM_DEVREADWRITE8_LEGACY("ensoniq2", es5506_r, es5506_w, 0xffffffff)
//    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static void esq5506_otto_irq(device_t *device, int state)
{
}

static UINT16 esq5506_read_adc(device_t *device)
{
	return 0;
}

static const es5506_interface es5506_config =
{
	"waverom",	/* Bank 0 */
	"waverom2",	/* Bank 1 */
	"waverom3",	/* Bank 0 */
	"waverom4",	/* Bank 1 */
	esq5506_otto_irq, /* irq */
    esq5506_read_adc
};

static const es5506_interface es5506_2_config =
{
	"waverom",	/* Bank 0 */
	"waverom2",	/* Bank 1 */
	"waverom3",	/* Bank 0 */
	"waverom4",	/* Bank 1 */
	NULL,
	NULL
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

CONS( 1996, mrrack, 0, 0, mr, mr, esqmr_state, mr, "Ensoniq", "MR-Rack", GAME_NOT_WORKING )

