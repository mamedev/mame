// license: BSD-3-Clause
// copyright-holders:

/*************************************************************

Skeleton driver for Honeywell CAMIR-F1 wireless passive
infrared motion sensor with built-in colour camera.

Hardware based on a PIC18LF26K22 as main CPU with a Conexant
CX93610 (DIFT JPEG Encoder with a BT.656 Camera Interface and
Optional Microphone Input) and a IS25LQ040 Serial Flash.

PCB:
  _________    _    _________
 /         |__| |__|         \
|                             |
|            _____        _   |
|           |CAMERA------| |  |
|           |SENSOR------| |  |
|           |_____|------|_|  |
|            _____     _____  |
|           |CX93610  |PIC18LF26K22
|           |     |   |    |  |
|           |_____|   |    |  |
|                     |____|  |
|                  _____      |
|                 |IS25LQ040  |
|        Xtal                 |
|       27 MHz                |
|              ____           |
|             /IR  |          |
|            |SENSOR   __     |
|   ___      |____/   |__|    |
|  |___|                      |
|           _________         |
|__________|         |________|

*************************************************************/

#include "emu.h"
#include "cpu/pic16x8x/pic16x8x.h"

namespace {


class camirf1_state : public driver_device
{
public:
	camirf1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void camirf1(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
};

INPUT_PORTS_START( camirf1 )
INPUT_PORTS_END

void camirf1_state::camirf1(machine_config &config)
{
	PIC16F84(config, m_maincpu, 16_MHz_XTAL); // Actually a PIC18LF26K22
}

ROM_START( camirf1 )
	// ID = 00000000ffffffffh, CONFIG = 28130e003d81000fc00fe00fh
	ROM_REGION( 0x04280, "maincpu", 0 )
	ROM_LOAD( "pic18lf26k22_user.bin", 0x00000, 0x01000, CRC(47e4e6c6) SHA1(e0b1d0690cf991803673e9bcf2d244aa15c42feb) )
	ROM_LOAD( "pic18lf26k22_data.bin", 0x00000, 0x00400, CRC(b15001ef) SHA1(6d722abfb433fbf76f2a53b015febc42f4d638c2) )

	ROM_REGION( 0x80000, "sflash", 0 )
	ROM_LOAD( "is25lq040.bin",         0x00000, 0x80000, CRC(4458e03f) SHA1(52cf7001c90ae946ed00f4452a43523c12c81e9d) )
ROM_END

} // anonymous namespace


SYST( 2016, camirf1, 0, 0, camirf1, camirf1, camirf1_state, empty_init, "Honeywell", "CAMIR-F1", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
