// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

    Milton Bradley Phantom chess computer

    TODO:
    - no romdump of MB version yet
    - sort out fphantom memory map

-------------------------------------------------------------------------------

Milton Bradley Phantom (1983):
sold as Milton in Europe, Phantom in UK, Grand Master in USA

Hardware notes:
- CPU: SY6502A, 2MHz?(resonator)
- ROM: 2x8KB?, labels C19679, C19680
- RAM: 2KB 4*2114
- magnetized x/y motor under chessboard, chesspieces have magnet underneath
- piezo speaker, LEDs


-------------------------------------------------------------------------------

Fidelity Phantom (1988, model 6100):

Fidelity bought the design from Milton Bradley and released their own version.
It has a small LCD panel added, the rest looks nearly the same from the outside.

Hardware notes:
- CPU: 6502, 5MHz?
- ROM 2*32KB
- RAM: 8KB?
- LCD driver, display panel for digits
- assume rest is same as MB version

After Fidelity was taken over by H&G, it was rereleased in 1990 as the Mephisto
Phantom. This is assumed to be identical.

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/speaker.h"

// internal artwork
#include "fphantom.lh" // clickable


class phantom_state : public driver_device
{
public:
	phantom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void phantom_state::machine_start()
{
	// zerofill

	// register for savestates
}

void phantom_state::machine_reset()
{
}



/******************************************************************************
    I/O, Memory Maps
******************************************************************************/

static ADDRESS_MAP_START( fphantom_mem, AS_PROGRAM, 8, phantom_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0xffff) AM_ROM
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( fphantom )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( fphantom, phantom_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 5000000) // 5MHz?
	MCFG_CPU_PERIODIC_INT_DRIVER(phantom_state, irq0_line_hold, 600) // guessed
	MCFG_CPU_PROGRAM_MAP(fphantom_mem)

	MCFG_DEFAULT_LAYOUT(layout_fphantom)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fphantom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("4_4.u4",  0x0000, 0x8000, CRC(e4181ba2) SHA1(1f77d1867c6f566be98645fc252a01108f412c96) )
	ROM_LOAD("4_3c.u3", 0x8000, 0x8000, CRC(fb7c38ae) SHA1(a1aa7637705052cb4eec92644dc79aee7ba4d77c) )
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1988, fphantom, 0,      0,      fphantom, fphantom, driver_device, 0, "Fidelity Electronics", "Phantom (Fidelity)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
