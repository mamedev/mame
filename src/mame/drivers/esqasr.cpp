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

#include "cpu/es5510/es5510.h"
#include "machine/68340.h"
#include "machine/esqvfd.h"
#include "machine/upd765.h"
#include "sound/es5506.h"
#include "sound/esqpump.h"

#include "speaker.h"


class esqasr_state : public driver_device
{
public:
	esqasr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_esp(*this, "esp")
		, m_pump(*this, "pump")
		, m_sq1vfd(*this, "sq1vfd")
	{ }

	void asrx(machine_config &config);
	void asr(machine_config &config);

	void init_asr();

private:
	required_device<cpu_device> m_maincpu;
	optional_device<es5510_device> m_esp;
	optional_device<esq_5505_5510_pump_device> m_pump;
	required_device<esq2x40_sq1_device> m_sq1vfd;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER(esq5506_otto_irq);
	DECLARE_READ16_MEMBER(esq5506_read_adc);
	void asr_map(address_map &map);
	void asrx_map(address_map &map);
};

void esqasr_state::machine_start()
{
	if (m_pump.found())
	{
		// tell the pump about the ESP chips
		if (m_esp.found())
			m_pump->set_esp(m_esp);
	}
}

void esqasr_state::machine_reset()
{
}

void esqasr_state::asr_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0xf00000, 0xffffff).ram();
}

void esqasr_state::asrx_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom().region("maincpu", 0);
	map(0x08000000, 0x081fffff).ram();
	map(0x0be00000, 0x0befffff).ram();
}

WRITE_LINE_MEMBER(esqasr_state::esq5506_otto_irq)
{
}

READ16_MEMBER(esqasr_state::esq5506_read_adc)
{
	return 0;
}

MACHINE_CONFIG_START(esqasr_state::asr)
	MCFG_DEVICE_ADD("maincpu", M68000, XTAL(16'000'000)) // actually MC68302
	MCFG_DEVICE_PROGRAM_MAP(asr_map)

	MCFG_DEVICE_ADD("esp", ES5510, XTAL(10'000'000))
	MCFG_DEVICE_DISABLE()

	ESQ2X40_SQ1(config, m_sq1vfd, 60);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("pump", ESQ_5505_5510_PUMP, XTAL(16'000'000) / (16 * 32))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("ensoniq", ES5506, XTAL(16'000'000))
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(4)          /* channels, Not verified from real hardware */
	MCFG_ES5506_IRQ_CB(WRITELINE(*this, esqasr_state, esq5506_otto_irq)) /* irq */
	MCFG_ES5506_READ_PORT_CB(READ16(*this, esqasr_state, esq5506_read_adc))
	MCFG_SOUND_ROUTE(0, "pump", 1.0, 0)
	MCFG_SOUND_ROUTE(1, "pump", 1.0, 1)
	MCFG_SOUND_ROUTE(2, "pump", 1.0, 2)
	MCFG_SOUND_ROUTE(3, "pump", 1.0, 3)
	MCFG_SOUND_ROUTE(4, "pump", 1.0, 4)
	MCFG_SOUND_ROUTE(5, "pump", 1.0, 5)
	MCFG_SOUND_ROUTE(6, "pump", 1.0, 6)
	MCFG_SOUND_ROUTE(7, "pump", 1.0, 7)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(esqasr_state::asrx)
	M68340(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &esqasr_state::asrx_map);

	MCFG_DEVICE_ADD("esp", ES5510, XTAL(10'000'000)) // Actually ES5511
	MCFG_DEVICE_DISABLE()

	ESQ2X40_SQ1(config, m_sq1vfd, 60);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("pump", ESQ_5505_5510_PUMP, XTAL(16'000'000) / (16 * 32)) // Actually ES5511
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("ensoniq", ES5506, XTAL(16'000'000))
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(1)          /* channels */
	MCFG_ES5506_IRQ_CB(WRITELINE(*this, esqasr_state, esq5506_otto_irq)) /* irq */
	MCFG_ES5506_READ_PORT_CB(READ16(*this, esqasr_state, esq5506_read_adc))
	MCFG_SOUND_ROUTE(0, "pump", 1.0, 0)
	MCFG_SOUND_ROUTE(1, "pump", 1.0, 1)
	MCFG_SOUND_ROUTE(2, "pump", 1.0, 2)
	MCFG_SOUND_ROUTE(3, "pump", 1.0, 3)
	MCFG_SOUND_ROUTE(4, "pump", 1.0, 4)
	MCFG_SOUND_ROUTE(5, "pump", 1.0, 5)
	MCFG_SOUND_ROUTE(6, "pump", 1.0, 6)
	MCFG_SOUND_ROUTE(7, "pump", 1.0, 7)
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

void esqasr_state::init_asr()
{
}

CONS( 1992, asr10, 0, 0, asr, asr, esqasr_state, init_asr, "Ensoniq", "ASR-10", MACHINE_NOT_WORKING )
CONS( 1997, asrx,  0, 0, asrx,asr, esqasr_state, init_asr, "Ensoniq", "ASR-X",  MACHINE_NOT_WORKING )
