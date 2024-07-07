// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland U-20 & related synthesizers.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs96/i8x9x.h"
#include "sound/roland_lp.h"
#include "speaker.h"


namespace {

class roland_u20_state : public driver_device
{
public:
	roland_u20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pcm(*this, "pcm")
	{
	}

	void u20(machine_config &config);
	void u220(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<i8x9x_device> m_maincpu;
	required_device<mb87419_mb87420_device> m_pcm;
};


void roland_u20_state::mem_map(address_map &map)
{
	map(0x1800, 0x1fff).ram(); // TODO: more RAM than this is present
	map(0x2000, 0xffff).rom().region("progrom", 0x2000); // TODO: banking
}

static INPUT_PORTS_START(u20)
INPUT_PORTS_END

void roland_u20_state::u20(machine_config &config)
{
	P8098(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_u20_state::mem_map);

	//R15239124(config, "keyscan", 12_MHz_XTAL);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MB87419_MB87420(config, m_pcm, 32.768_MHz_XTAL);
	m_pcm->int_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);
	m_pcm->set_device_rom_tag("waverom");
	m_pcm->add_route(0, "lspeaker", 1.0);
	m_pcm->add_route(1, "rspeaker", 1.0);
}

void roland_u20_state::u220(machine_config &config)
{
	u20(config);

	//config.device_remove("keyscan");
	m_pcm->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);
}

ROM_START(u20)
	ROM_REGION(0x20000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v303", "Version 3.03")
	ROMX_LOAD("u-20-v303.bin", 0x00000, 0x20000, CRC(28ce7fca) SHA1(d4186a7034a0646e1c76a3a7f5c3cf5f165ace80), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v103", "Version 1.03")
	ROMX_LOAD("u-20-v103.bin", 0x00000, 0x20000, CRC(eb94054f) SHA1(1127e21ba94cc629eb00355c0be6ace6ca7759d6), ROM_BIOS(1)) // M5M27C100P

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD("roland-a_r15179892f_mb834000a-20_226-aa.ic27", 0x000000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-b_r15179893f_mb834000a-20_227-aa.ic28", 0x080000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-c_r15179894f_mb834000a-20_228-aa.ic29", 0x100000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-d_r15179895f_mb834000a-20_229-aa.ic30", 0x180000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-e_r15179947_mb834000a-20_3a1-aa.ic31",  0x200000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-f_r15179948_mb834000a-20_3a2-aa.ic32",  0x280000, 0x080000, NO_DUMP)
ROM_END

ROM_START(u220)
	ROM_REGION(0x20000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v102", "Version 1.02")
	ROMX_LOAD("u-220_roland_1-0-2.ic8", 0x00000, 0x20000, CRC(d5492e9b) SHA1(9d72b1688a173505b5e1c5ddac01efa78ee76c7f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v101", "Version 1.01")
	ROMX_LOAD("u-220_roland_1-0-1.ic8", 0x00000, 0x20000, CRC(893afa9c) SHA1(d0b66c0ea0e3af284a1806226aed79d8da2f3dd4), ROM_BIOS(1)) // HN27C101G-20

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD("roland-a_r15179892f_mb834000a-20_226-aa.ic19", 0x000000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-b_r15179893f_mb834000a-20_227-aa.ic20", 0x080000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-c_r15179894f_mb834000a-20_228-aa.ic21", 0x100000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-d_r15179895f_mb834000a-20_229-aa.ic22", 0x180000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-e_r15179947_mb834000a-20_3a1-aa.ic23",  0x200000, 0x080000, NO_DUMP)
	ROM_LOAD("roland-f_r15179948_mb834000a-20_3a2-aa.ic24",  0x280000, 0x080000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1989, u20,  0, 0, u20,  u20, roland_u20_state, empty_init, "Roland", "U-20 RS-PCM Keyboard", MACHINE_IS_SKELETON)
SYST(1989, u220, 0, 0, u220, u20, roland_u20_state, empty_init, "Roland", "U-220 RS-PCM Sound Module", MACHINE_IS_SKELETON)
