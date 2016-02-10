// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    esqasr.c - Ensoniq ASR-10 and ASR-X

    Skeleton driver by R. Belmont

    ASR-10 hardware:
        CPU: 68302 MCU
        Sound: ES5506
        Effects: ES5510
        FDC: NEC uPD72069
        DUART: 2681

    Memory map:
    0x000000-0x03ffff   OS ROM
    0xfb0000-0xfcffff   OS RAM


    ASR-X hardware:
        CPU: 68340 MCU
        Sound: ES5506
        Effects: ES5511
        FDC: NEC uPD72069

    http://www.gweep.net/~shifty/music/asrxhack/

    Memory map:
    0x00000000-0x000fffff   OS ROM
    0x00800000-0x008000ff   ESP2 5511?
    0x00f00000-0x00f007ff   Unknown
    0x08000000-0x08200000   RAM
    0x0be00000-0x0befffff   RAM (size unknown)

    These may want to be separated when they run more.

***************************************************************************/

#include "emu.h"
#include "machine/68340.h"
#include "sound/es5506.h"
#include "cpu/es5510/es5510.h"
#include "machine/upd765.h"

#include "machine/esqvfd.h"

class esqasr_state : public driver_device
{
public:
	esqasr_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_esp(*this, "esp"),
		m_sq1vfd(*this, "sq1vfd")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<es5510_device> m_esp;
	required_device<esq2x40_sq1_t> m_sq1vfd;

	virtual void machine_reset() override;

	DECLARE_DRIVER_INIT(asr);
	DECLARE_WRITE_LINE_MEMBER(esq5506_otto_irq);
	DECLARE_READ16_MEMBER(esq5506_read_adc);
};

void esqasr_state::machine_reset()
{
}

static ADDRESS_MAP_START( asr_map, AS_PROGRAM, 16, esqasr_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0xf00000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( asrx_map, AS_PROGRAM, 32, esqasr_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x08000000, 0x081fffff) AM_RAM
	AM_RANGE(0x0be00000, 0x0befffff) AM_RAM
ADDRESS_MAP_END

WRITE_LINE_MEMBER(esqasr_state::esq5506_otto_irq)
{
}

READ16_MEMBER(esqasr_state::esq5506_read_adc)
{
	return 0;
}

static MACHINE_CONFIG_START( asr, esqasr_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz) // actually MC68302
	MCFG_CPU_PROGRAM_MAP(asr_map)

	MCFG_CPU_ADD("esp", ES5510, XTAL_10MHz)
	MCFG_DEVICE_DISABLE()

	MCFG_ESQ2x40_SQ1_ADD("sq1vfd")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5506, XTAL_16MHz)
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(1)          /* channels */
	MCFG_ES5506_IRQ_CB(WRITELINE(esqasr_state, esq5506_otto_irq)) /* irq */
	MCFG_ES5506_READ_PORT_CB(READ16(esqasr_state, esq5506_read_adc))
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( asrx, esqasr_state )
	MCFG_CPU_ADD("maincpu", M68020, XTAL_16MHz) // unknown, possibly 68340?
	MCFG_CPU_PROGRAM_MAP(asrx_map)

	MCFG_CPU_ADD("esp", ES5510, XTAL_10MHz)
	MCFG_DEVICE_DISABLE()

	MCFG_ESQ2x40_SQ1_ADD("sq1vfd")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5506, XTAL_16MHz)
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(1)          /* channels */
	MCFG_ES5506_IRQ_CB(WRITELINE(esqasr_state, esq5506_otto_irq)) /* irq */
	MCFG_ES5506_READ_PORT_CB(READ16(esqasr_state, esq5506_read_adc))
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_CONFIG_END

static INPUT_PORTS_START( asr )
INPUT_PORTS_END

ROM_START( asr10 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "asr-648c-lo-1.5b.bin", 0x000001, 0x020000, CRC(8e437843) SHA1(418f042acbc5323f5b59cbbd71fdc8b2d851f7d0) )
	ROM_LOAD16_BYTE( "asr-65e0-hi-1.5b.bin", 0x000000, 0x020000, CRC(b37cd3b6) SHA1(c4371848428a628b5e5a50e99be602d7abfc7904) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

ROM_START( asrx )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "asr267lo.bin", 0x000001, 0x080000, CRC(7408d441) SHA1(0113f84b6d224bf1423ad62c173f32a0c95ca715) )
	ROM_LOAD16_BYTE( "asr267hi.bin", 0x000000, 0x080000, CRC(7df14ea7) SHA1(895b99013c0f924edb52612eb93c3e6babb9f053) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

DRIVER_INIT_MEMBER(esqasr_state, asr)
{
}

CONS( 1992, asr10, 0, 0, asr, asr, esqasr_state, asr, "Ensoniq", "ASR-10", MACHINE_NOT_WORKING )
CONS( 1997, asrx,  0, 0, asrx,asr, esqasr_state, asr, "Ensoniq", "ASR-X", MACHINE_NOT_WORKING )
