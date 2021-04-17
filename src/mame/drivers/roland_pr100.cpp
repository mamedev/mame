// license:BSD-3-Clause
// copyright-holders:Valley Bell
/****************************************************************************

    Skeleton driver for Roland PR-100 Digital Sequencer.

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"

class roland_pr100_state : public driver_device
{
public:
	roland_pr100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mpu(*this, "maincpu")
	{
	}

	void pr100(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<hd64180rp_device> m_mpu;
};

void roland_pr100_state::mem_map(address_map &map)
{
	map(0x0000, 0x9fff).rom().region("mpurom", 0);
	map(0xb000, 0xefff).ram();	// stack pointer is initialized to 0xF000, the routine at 0x0325 writes to 0xB003
	// But the routine at 0x02AD also writes to all bytes from 0x8000 to 0xFFFF (does "OUT0 ($38), ## select some RAM bank?)
	// TODO: map(0xf000, 0xffff) seems to be one of the ROM banks 0xA000/0xB000/0xC000/0xD000/0xE000/0xF000
}

void roland_pr100_state::io_map(address_map &map)
{
	map(0x0000, 0x003f).noprw(); // internal registers?
	// registers in the 0x80..0x9F range seem to be important
}


static INPUT_PORTS_START(pr100)
INPUT_PORTS_END

void roland_pr100_state::pr100(machine_config &config)
{
	HD64180RP(config, m_mpu, 10_MHz_XTAL);	// HD64B180R0P
	m_mpu->set_addrmap(AS_PROGRAM, &roland_pr100_state::mem_map);
	m_mpu->set_addrmap(AS_IO, &roland_pr100_state::io_map);
	
	// other components:
	// - QD ("quick disk") drive: D-281
	// - QD controller: MB87013
	// - LCD unit: DM0815
}

ROM_START(pr100)
	ROM_REGION(0x10000, "mpurom", 0)
	ROM_LOAD("roland_mbm27c512-20.ic10", 0x00000, 0x10000, CRC(41160b69) SHA1(11e5fb001dd004a5625d9a75fb1acac4ade614c8))
ROM_END

SYST(1995, pr100, 0, 0, pr100, pr100, roland_pr100_state, empty_init, "Roland", "PR-100 Digital Sequencer", MACHINE_IS_SKELETON)
