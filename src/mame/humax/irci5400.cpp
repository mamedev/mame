// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Humax IRCI-5400 / -5400Z DVB-S set top box

Components:
- SAA7219 main CPU clocked at 81 MHz:
  i2c MPEG2 transport RISC with PR3930 (MIPS16) as instruction set core;
- PIC16C64A for front panel duties (LTC-5623G-12 7-seg);
- Two flash ROMs, 28F160F3B/28F800F3B for main boot block, 28F160B3b-90/28F800B3B for channel data
  and "constant" (?);
- 16 MB system SDRAM + two SDRAMs for MPEG and OSD (both KM416S1020BT-G10);
- SAA7215 EMPEG decoding and A/V signal output;
- UART options, external/internal modem supported;
- STV6411AD TV/VCR/Scart output switching;
- RMUP74055VA RF modulator;
- UDA1320 audio DAC;
- SPDIF;
- CXD1957AQ CI and TS buffer controller;
- SD1228E/LA MK tuner;
- LNBP15SP;
- TDA8004 + SAS004 + 12 MHz clock for the Irdeto duties (N/A for emulation);

TODO:
- everything, starting from the CPU core;

**************************************************************************************************/

#include "emu.h"

#include "cpu/mips/mips3.h"

#define VERBOSE ( 0 )
#include "logmacro.h"

namespace {

class irci5400_state : public driver_device
{
public:
	irci5400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void irci5400(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map) ATTR_COLD;
};

void irci5400_state::main_map(address_map &map)
{
//	map(0x00000000, 0x001fffff).rom().region("flash", 0);
}

void irci5400_state::machine_start()
{

}

void irci5400_state::machine_reset()
{

}

void irci5400_state::irci5400(machine_config &config)
{
	// TODO: placeholder, is 83 MHz from a base XTAL?
	R4000BE(config, m_maincpu, 83'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &irci5400_state::main_map);
	m_maincpu->set_disable();
}


} // anonymous namespace

ROM_START( irci5400 )
	ROM_REGION16_BE( 0x200000, "flash", 0 )
	ROM_LOAD16_WORD_SWAP( "te28f160c3ba90.bin", 0x000000, 0x200000, CRC(97e78cd5) SHA1(ef9d679085ae2949814f11dd6a04f1ca63ee3606) )

	ROM_REGION16_LE( 0x4010, "pic", 0 )
	ROM_LOAD( "pic16c64a", 0, 0x4010, NO_DUMP )
ROM_END

ROM_START( irci5400z )
	ROM_REGION16_BE( 0x200000, "flash", 0 )
	ROM_LOAD16_WORD_SWAP( "te28f160.bin", 0x000000, 0x200000, CRC(968dad4c) SHA1(0dfa141903ce608a56324890e7c59e3fc3ec18cb) )

	// tables at $a0000 looks Astra 1 or 2 channel data
	// useful for validating menu navigation, to be eventually removed in a far future.
	ROM_REGION16_BE( 0x100000, "extra", 0)
	ROM_LOAD16_WORD_SWAP( "te28f800.bin", 0x000000, 0x100000, CRC(2a279ba7) SHA1(c7e72796307966ed392ca33359844e6c9aea320d) )

	ROM_REGION16_LE( 0x4010, "pic", 0 )
	ROM_LOAD( "pic16c64a", 0, 0x4010, NO_DUMP )
ROM_END



SYST( 2000, irci5400,  0,        0,      irci5400, 0, irci5400_state, empty_init, "Humax", "IRCI-5400", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 2000, irci5400z, irci5400, 0,      irci5400, 0, irci5400_state, empty_init, "Humax", "IRCI-5400Z", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
// IRCI-5500
// IRCI-5510
// IRCI-5100
// IRCI-5200
