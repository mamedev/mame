// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland U-20 & related synthesizers.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs96/i8x9x.h"
#include "machine/nvram.h"
#include "sound/roland_lp.h"
#include "speaker.h"


namespace {

class roland_u220_state : public driver_device
{
public:
	roland_u220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rams(*this, "rams")
		, m_maincpu(*this, "maincpu")
		, m_pcm(*this, "pcm")
	{
	}

	void u110(machine_config &config);
	void u20(machine_config &config);
	void u220(machine_config &config);

private:
	void mem_map_u110(address_map &map) ATTR_COLD;
	void mem_map_u220(address_map &map) ATTR_COLD;

	required_device<nvram_device> m_rams;
	required_device<i8x9x_device> m_maincpu;
	required_device<mb87419_mb87420_device> m_pcm;
};


void roland_u220_state::mem_map_u110(address_map &map)
{
	// memory map according to U-110 service notes, pages 4 and 6
	map(0x0000, 0x0fff).rom().region("progrom", 0x0000);    // program memory (CS7)
	map(0x1000, 0x107f).ram(); // ? (CS1)
	map(0x1080, 0x10ff).ram(); // Reverb (CS2)
	map(0x1100, 0x11ff).ram(); // LCD
	map(0x1200, 0x12ff).ram(); // LED
	map(0x1300, 0x13ff).ram(); // SW Scan
	map(0x1400, 0x1bff).ram(); // Piano (CS3)
	map(0x1c00, 0x1dff).ram(); // EXT (CS4)
	map(0x1e00, 0x1eff).ram(); // TVF (CS5)
	map(0x1f00, 0x1fff).ram(); // Output (CS6)
	map(0x2000, 0x20ff).rom().region("progrom", 0x2000);    // program memory (CS7)
	map(0x2100, 0x3fff).ram().share("rams");                // backup memory (CS8)
	map(0x4000, 0xdfff).rom().region("progrom", 0x4000);    // program memory (CS7)
	map(0xe000, 0xffff).rom().region("progrom", 0xe000);    // program memory (CS7) or external memory (CS9), depending on Bank Select
}

void roland_u220_state::mem_map_u220(address_map &map)
{
	map(0x1800, 0x1fff).ram().share("rams"); // TODO: more RAM than this is present
	map(0x2000, 0xffff).rom().region("progrom", 0x2000); // TODO: banking
}

static INPUT_PORTS_START(u110)
INPUT_PORTS_END

static INPUT_PORTS_START(u20)
INPUT_PORTS_END

static INPUT_PORTS_START(u220)
INPUT_PORTS_END

void roland_u220_state::u110(machine_config &config)
{
	P8098(config, m_maincpu, 12_MHz_XTAL); // 8097BH
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_u220_state::mem_map_u110);

	// Battery-backed main ram
	NVRAM(config, m_rams, nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "speaker", 2).front();

	MB87419_MB87420(config, m_pcm, 34.816_MHz_XTAL); // clock according to service notes
	m_pcm->int_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);
	m_pcm->set_device_rom_tag("waverom");
	m_pcm->add_route(0, "speaker", 1.0, 0);
	m_pcm->add_route(1, "speaker", 1.0, 1);
}

void roland_u220_state::u20(machine_config &config)
{
	P8098(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_u220_state::mem_map_u220);

	// Battery-backed main ram
	NVRAM(config, m_rams, nvram_device::DEFAULT_ALL_0);

	//R15239124(config, "keyscan", 12_MHz_XTAL);

	SPEAKER(config, "speaker", 2).front();

	MB87419_MB87420(config, m_pcm, 32.768_MHz_XTAL);
	m_pcm->int_callback().set_inputline(m_maincpu, i8x9x_device::EXTINT_LINE);
	m_pcm->set_device_rom_tag("waverom");
	m_pcm->add_route(0, "speaker", 1.0, 0);
	m_pcm->add_route(1, "speaker", 1.0, 1);
}

void roland_u220_state::u220(machine_config &config)
{
	u20(config);

	//config.device_remove("keyscan");
	m_pcm->int_callback().set_inputline(m_maincpu, i8x9x_device::HSI0_LINE);
}

ROM_START(u110)
	ROM_REGION(0x10000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "203", "Version 2.03")
	ROMX_LOAD("roland_r15179960_lh531467.ic9", 0x00000, 0x10000, CRC(73e70eae) SHA1(ec0d76d325f27eddd7d8ac5530221137dbde6db1), ROM_BIOS(0)) // R15179960 LH531467 8921 E, marking under chip footprint: 27C512

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD("roland-a_r15179892f_mb834000a-20_226-aa.bin", 0x000000, 0x080000, CRC(d5475560) SHA1(55d36199ea489969a729782afe4cbff5b1cede0e))   // IC18
	// 0x080000 .. 0x0fffff is reserved for PCM card 1
	ROM_LOAD("roland-b_r15179893f_mb834000a-20_227-aa.bin", 0x100000, 0x080000, CRC(43a5b26c) SHA1(8b8067fbc6fa7b2e8694a3785f123627ea458d1f))   // IC19
	// 0x180000 .. 0x1fffff is reserved for PCM card 2
	ROM_LOAD("roland-c_r15179894f_mb834000a-20_228-aa.bin", 0x200000, 0x080000, CRC(e79ee88a) SHA1(8dd9826d7d3e67b9a568eadeaaf7856a05f3e068))   // IC20
	// 0x280000 .. 0x2fffff is reserved for PCM card 3
	ROM_LOAD("roland-d_r15179895f_mb834000a-20_229-aa.bin", 0x300000, 0x080000, CRC(da9fbf6d) SHA1(35b98a0fb5cf643fb7b761a74581486eee3bd8c3))   // IC21
	// 0x380000 .. 0x3fffff is reserved for PCM card 4
ROM_END

ROM_START(u20)
	ROM_REGION(0x20000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v303", "Version 3.03")
	ROMX_LOAD("u-20-v303.bin", 0x00000, 0x20000, CRC(28ce7fca) SHA1(d4186a7034a0646e1c76a3a7f5c3cf5f165ace80), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v103", "Version 1.03")
	ROMX_LOAD("u-20-v103.bin", 0x00000, 0x20000, CRC(eb94054f) SHA1(1127e21ba94cc629eb00355c0be6ace6ca7759d6), ROM_BIOS(1)) // M5M27C100P

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	// TODO: figure out the proper order of the 0x#80000 ROMs.
	ROM_LOAD("roland-a_r15179892f_mb834000a-20_226-aa.bin", 0x000000, 0x080000, CRC(d5475560) SHA1(55d36199ea489969a729782afe4cbff5b1cede0e))   // IC27
	ROM_LOAD("roland-e_r15179947_mb834000a-20_3a1-aa.bin",  0x080000, 0x080000, CRC(104f3974) SHA1(71b6f97df6dd4e573db5743c10c77e86d67965a3))   // IC31
	ROM_LOAD("roland-b_r15179893f_mb834000a-20_227-aa.bin", 0x100000, 0x080000, CRC(43a5b26c) SHA1(8b8067fbc6fa7b2e8694a3785f123627ea458d1f))   // IC28
	ROM_LOAD("roland-f_r15179948_mb834000a-20_3a2-aa.bin",  0x180000, 0x080000, CRC(af1c2778) SHA1(aea715a0a554c071a2e912894805a8d819802e1d))   // IC32
	ROM_LOAD("roland-c_r15179894f_mb834000a-20_228-aa.bin", 0x200000, 0x080000, CRC(e79ee88a) SHA1(8dd9826d7d3e67b9a568eadeaaf7856a05f3e068))   // IC29
	// 0x280000 .. 0x2fffff is reserved for PCM card 1
	ROM_LOAD("roland-d_r15179895f_mb834000a-20_229-aa.bin", 0x300000, 0x080000, CRC(da9fbf6d) SHA1(35b98a0fb5cf643fb7b761a74581486eee3bd8c3))   // IC30
	// 0x380000 .. 0x3fffff is reserved for PCM card 2
ROM_END

ROM_START(u220)
	ROM_REGION(0x20000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v100", "Version 1.00")
	ROMX_LOAD("u-220_roland_1-0-0.ic8", 0x00000, 0x20000, CRC(82abd055) SHA1(e1c6f1ee29cebe8663cb0c6989b8faebb3d841fd), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v101", "Version 1.01")
	ROMX_LOAD("u-220_roland_1-0-1.ic8", 0x00000, 0x20000, CRC(893afa9c) SHA1(d0b66c0ea0e3af284a1806226aed79d8da2f3dd4), ROM_BIOS(1)) // HN27C101G-20
	ROM_SYSTEM_BIOS(2, "v102", "Version 1.02")
	ROMX_LOAD("u-220_roland_1-0-2.ic8", 0x00000, 0x20000, CRC(d5492e9b) SHA1(9d72b1688a173505b5e1c5ddac01efa78ee76c7f), ROM_BIOS(2)) // 15209245 LH530847

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	// TODO: figure out the proper order of the 0x#80000 ROMs.
	ROM_LOAD("roland-a_r15179892f_mb834000a-20_226-aa.bin", 0x000000, 0x080000, CRC(d5475560) SHA1(55d36199ea489969a729782afe4cbff5b1cede0e))   // IC19, R15179892 62314BPD81 E22 0L1
	ROM_LOAD("roland-e_r15179947_mb834000a-20_3a1-aa.bin",  0x080000, 0x080000, CRC(104f3974) SHA1(71b6f97df6dd4e573db5743c10c77e86d67965a3))   // IC23, R15179947H ?????????? ??? ???
	ROM_LOAD("roland-b_r15179893f_mb834000a-20_227-aa.bin", 0x100000, 0x080000, CRC(43a5b26c) SHA1(8b8067fbc6fa7b2e8694a3785f123627ea458d1f))   // IC20, R15179893 62314BPD82 E23 1A1
	ROM_LOAD("roland-f_r15179948_mb834000a-20_3a2-aa.bin",  0x180000, 0x080000, CRC(af1c2778) SHA1(aea715a0a554c071a2e912894805a8d819802e1d))   // IC24, R15179948H 62314BPD72 G87 0M1
	ROM_LOAD("roland-c_r15179894f_mb834000a-20_228-aa.bin", 0x200000, 0x080000, CRC(e79ee88a) SHA1(8dd9826d7d3e67b9a568eadeaaf7856a05f3e068))   // IC21, R15179894 62314BPD83 E24 ???
	// 0x280000 .. 0x2fffff is reserved for PCM card 1
	ROM_LOAD("roland-d_r15179895f_mb834000a-20_229-aa.bin", 0x300000, 0x080000, CRC(da9fbf6d) SHA1(35b98a0fb5cf643fb7b761a74581486eee3bd8c3))   // IC22, R15179895 62314BPD84 E25 0M1
	// 0x380000 .. 0x3fffff is reserved for PCM card 2
ROM_END

} // anonymous namespace


SYST(1988, u110, u220, 0, u110, u110, roland_u220_state, empty_init, "Roland", "U-110 PCM Sound Module", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1989, u220,    0, 0, u220, u220, roland_u220_state, empty_init, "Roland", "U-220 RS-PCM Sound Module", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1989, u20,  u220, 0, u20,  u20,  roland_u220_state, empty_init, "Roland", "U-20 RS-PCM Keyboard", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
