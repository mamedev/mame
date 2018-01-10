// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for M6800-based display terminals by Qume.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
//#include "video/mc6845.h"

class qvt6800_state : public driver_device
{
public:
	qvt6800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

static ADDRESS_MAP_START( qvt102_mem_map, AS_PROGRAM, 8, qvt6800_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2800, 0x2803) AM_DEVWRITE("ctc", z80ctc_device, write)
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	//AM_RANGE(0x8000, 0x8000) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	//AM_RANGE(0x8001, 0x8001) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x9800, 0x9801) AM_DEVREADWRITE("acia", acia6850_device, read, write)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( qvt190_mem_map, AS_PROGRAM, 8, qvt6800_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2500, 0x2501) AM_DEVREADWRITE("acia1", acia6850_device, read, write)
	AM_RANGE(0x2600, 0x2601) AM_DEVREADWRITE("acia2", acia6850_device, read, write)
	//AM_RANGE(0x2800, 0x2800) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	//AM_RANGE(0x2801, 0x2801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( qvt6800 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( qvt102 )
	MCFG_CPU_ADD("maincpu", M6800, XTAL_16_6698MHz / 18)
	MCFG_CPU_PROGRAM_MAP(qvt102_mem_map)

	MCFG_NVRAM_ADD_0FILL("nvram") // 2x TC5514-APL + 3V battery

	//MCFG_DEVICE_ADD("crtc", MC6845, XTAL_16_6698MHz / 9)

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_16_6698MHz / 9)
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("acia", acia6850_device, write_txc))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("acia", acia6850_device, write_rxc))

	MCFG_DEVICE_ADD("ctcclk", CLOCK, XTAL_16_6698MHz / 18) // OR of CRTC CLK and ϕ1
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg1))

	MCFG_CPU_ADD("kbdmcu", I8748, XTAL_6MHz)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( qvt190 )
	MCFG_CPU_ADD("maincpu", M6800, 1'000'000)
	MCFG_CPU_PROGRAM_MAP(qvt190_mem_map)

	MCFG_DEVICE_ADD("acia1", ACIA6850, 0)

	MCFG_DEVICE_ADD("acia2", ACIA6850, 0)
MACHINE_CONFIG_END



/**************************************************************************************************************

Qume QVT-102.
Chips: HD46800DP (6800), HD46505SP (6845), HD46850P (6850), M58725P-15 (16k RAM), LH0082 (Z80CTC), Button battery
Crystals: 16.6698
Keyboard: D8748D, 6.000, Beeper

***************************************************************************************************************/

ROM_START( qvt102 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "t205m.u8",   0x0000, 0x2000, CRC(59cc04f6) SHA1(ee2e3a3ea7b57a231483fcc74266f0f3f51204af) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "c3205m.u32", 0x0000, 0x1000, CRC(f6d86e87) SHA1(c0885e4a35095a730d760bf91a1cf4e8edd6a2bb) )

	ROM_REGION(0x0400, "kbdmcu", 0)
	ROM_LOAD( "k301.u302",  0x0000, 0x0400, CRC(67564b20) SHA1(5897ff920f8fae4aa498d3a4dfd45b58183c041d) )
ROM_END

COMP( 1983, qvt102, 0, 0, qvt102, qvt6800, qvt6800_state, 0, "Qume", "QVT-102", MACHINE_IS_SKELETON )



/**************************************************************************************************************

Qume QVT-190.
Chips: MC68B00P, 2x MC68B50P, MC68B45P, V61C16P55L, M5M5165P-70L, ABHGA101006, button battery, 7-DIL-jumper
Crystal: unreadable

***************************************************************************************************************/

ROM_START( qvt190 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "95987-267.u19", 0x0000, 0x8000, CRC(78894d8e) SHA1(0a0f6883dd18872bddeb3ed18ebe496080e6591b) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "95864-304.u17", 0x0000, 0x2000, CRC(2792e99b) SHA1(4a84d029d0e63975fc95dc7056d2523193dff986) )
ROM_END

COMP( 1987, qvt190, 0, 0, qvt190, qvt6800, qvt6800_state, 0, "Qume", "QVT-190", MACHINE_IS_SKELETON )
