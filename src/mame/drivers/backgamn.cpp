// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Backgammon - "COPYRIGHT BY ADP LUEBBECKE GERMANY 1990"

    Skeleton driver.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/mc68681.h"
#include "machine/msm6242.h"
#include "sound/saa1099.h"
#include "speaker.h"

class backgamn_state : public driver_device
{
public:
	backgamn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_pit(*this, "pit")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<pit68230_device> m_pit;

	IRQ_CALLBACK_MEMBER(iack_handler);

private:
	virtual void machine_start() override;
};


void backgamn_state::machine_start()
{
}


IRQ_CALLBACK_MEMBER(backgamn_state::iack_handler)
{
	// TODO: fetch 68230 vector
	return m_duart->get_irq_vector();
}


static ADDRESS_MAP_START( backgamn_mem, AS_PROGRAM, 16, backgamn_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x100000, 0x10003f) AM_DEVREADWRITE8("pit", pit68230_device, read, write, 0x00ff)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE8("duart", mc68681_device, read, write, 0x00ff)
	AM_RANGE(0x300000, 0x300003) AM_DEVWRITE8("saa", saa1099_device, write, 0x00ff) AM_READNOP
	AM_RANGE(0x400000, 0x40001f) AM_DEVREADWRITE8("rtc", msm6242_device, read, write, 0x00ff)
	AM_RANGE(0x500000, 0x503fff) AM_RAM //work RAM
	AM_RANGE(0x600006, 0x600007) AM_NOP //(r) is discarded (watchdog?)
ADDRESS_MAP_END


static INPUT_PORTS_START( backgamn )
INPUT_PORTS_END


static MACHINE_CONFIG_START( backgamn )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(backgamn_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(backgamn_state, iack_handler)

	MCFG_DEVICE_ADD("pit", PIT68230, XTAL_8MHz / 10) // clock not verified

	MCFG_DEVICE_ADD("duart", MC68681, XTAL_3_6864MHz)
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", M68K_IRQ_4))

	MCFG_DEVICE_ADD("rtc", MSM6242, XTAL_32_768kHz)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("saa", SAA1099, XTAL_8MHz / 2) // clock not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END


ROM_START( backgamn )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b_f2_i.bin", 0x00000, 0x10000, CRC(9e42937c) SHA1(85d462a560b85b03ee9d341e18815b7c396118ac) )
	ROM_LOAD16_BYTE( "b_f2_ii.bin", 0x00001, 0x10000, CRC(8e0ee50c) SHA1(2a05c337db1131b873646aa4109593636ebaa356) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b_f1_i.bin", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "b_f1_ii.bin", 0x00001, 0x20000, NO_DUMP )
ROM_END


GAME( 1990, backgamn, 0, backgamn, backgamn, backgamn_state, 0, ROT0, "ADP", "Backgammon", MACHINE_NOT_WORKING )
