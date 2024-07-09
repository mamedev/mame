// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland JV-80, JV-880 and related synthesizers.

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8510.h"
#include "cpu/h8500/h8532.h"
#include "machine/nvram.h"
#include "sound/roland_gp.h"


namespace {

class roland_jv80_state : public driver_device
{
public:
	roland_jv80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pcm(*this, "pcm")
	{
	}

	void jv880(machine_config &config);

private:
	void jv880_mem_map(address_map &map);

	required_device<h8532_device> m_maincpu;
	required_device<tc6116_device> m_pcm;
};


void roland_jv80_state::jv880_mem_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("maincpu", 0);
	map(0x08000, 0x0dfff).ram().mirror(0xa0000);
	map(0x0f000, 0x0f3ff).rw(m_pcm, FUNC(tc6116_device::read), FUNC(tc6116_device::write));
	map(0x10000, 0x3ffff).rom().region("progrom", 0x10000);
	map(0x40000, 0x4ffff).rom().region("progrom", 0);
	map(0xa0000, 0xbffff).ram();
	map(0xc0000, 0xd7fff).ram().share("nvram");
	// map(0xe0000, 0xf7fff).ram().share("cardram");
}

static INPUT_PORTS_START(jv880)
INPUT_PORTS_END

void roland_jv80_state::jv880(machine_config &config)
{
	HD6435328(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_jv80_state::jv880_mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // LC36256AML-10 (IC18) + CR2032 battery

	TC6116(config, "pcm", 23.2_MHz_XTAL);
}

class roland_rd500_state : public driver_device
{
public:
	roland_rd500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pcm(*this, "pcm")
	{
	}

	void rd500(machine_config &config);

private:
	void rd500_mem_map(address_map &map);

	u8 keyscan_r(offs_t offset);
	void keyscan_w(offs_t offset, u8 data);

	required_device<h8510_device> m_maincpu;
	required_device<tc6116_device> m_pcm;
};

void roland_rd500_state::rd500_mem_map(address_map &map)
{
	map(0x000000, 0x8fffff).rom().region("progrom", 0);
	map(0x900000, 0x90ffff).ram();
	map(0xa00000, 0xa0ffff).rw(m_pcm, FUNC(tc6116_device::read), FUNC(tc6116_device::write));
	map(0xc00000, 0xc0ffff).rw(FUNC(roland_rd500_state::keyscan_r), FUNC(roland_rd500_state::keyscan_w));
}

u8 roland_rd500_state::keyscan_r(offs_t offset)
{
	return 0;
}

void roland_rd500_state::keyscan_w(offs_t offset, u8 data)
{
}

static INPUT_PORTS_START(rd500)
INPUT_PORTS_END

void roland_rd500_state::rd500(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_rd500_state::rd500_mem_map);

	TC6116(config, "pcm", 23.2_MHz_XTAL);
}

ROM_START(jv880)
	ROM_DEFAULT_BIOS("v1.0.1")
	ROM_SYSTEM_BIOS(0, "v1.0.0", "ROM Version 1.0.0")
	ROM_SYSTEM_BIOS(1, "v1.0.1", "ROM Version 1.0.1")

	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD("roland_r15199810_6435328b86f.ic16", 0x0000, 0x8000, CRC(b3d611b1) SHA1(6c1b59905b361ac3c4803d39589406bc3e1d0647))

	ROM_REGION(0x40000, "progrom", 0)
	ROMX_LOAD("roland_jv-880_v1.00.ic17", 0x00000, 0x40000, CRC(b266efe9) SHA1(282116e8e8471053cf159d22675931592b7f7c8f), ROM_BIOS(0))
	ROMX_LOAD("roland_jv-880_v1.01.ic17", 0x00000, 0x40000, CRC(5f19c95f) SHA1(38ec496f16dfa02d35f934cf32d8302aaf5f236e), ROM_BIOS(1))

	ROM_REGION(0x400000, "waverom", 0)
	ROM_LOAD("roland-a_r15209312_lh5375n2.ic27", 0x000000, 0x200000, CRC(1348c0dc) SHA1(37e28498351fb502f6d43398d288a026c02b446d))
	ROM_LOAD("roland-b_r15209313_lh5375n3.ic25", 0x200000, 0x200000, CRC(d55fcf90) SHA1(963ce75b6668dab377d3a2fd895630a745491be5))
ROM_END

ROM_START(rd500)
	ROM_REGION(0x80000, "progrom", 0)
	ROM_LOAD("rd500_rom.bin", 0x00000, 0x80000, CRC(668fc7e9) SHA1(59e28d3e2190902dd6fd02a9820f96e383781178))

	ROM_REGION(0x400000, "waverom", 0)
	ROM_LOAD("roland-a_r00342978.ic4", 0x000000, 0x200000, CRC(c885bf4f) SHA1(e14f0f4a8181e09fae7db10130e4ed3cd6bf5a34))
	ROM_LOAD("roland-b_r00343012.ic5", 0x200000, 0x200000, CRC(ceb02d33) SHA1(9f6969d94598c68902188085d0c91fb8b300d762))
	ROM_LOAD("roland-c_r00343023.ic6", 0x400000, 0x200000, CRC(f627cdb7) SHA1(7b834fee5db5a7377ec7f66172d0fa3096cefbc9))
	ROM_LOAD("roland-d_r00343034.ic7", 0x600000, 0x200000, CRC(c06be973) SHA1(2e7ce8a91a6f92648f73d7ff8c1d608f62df9aab))
ROM_END

} // anonymous namespace


//SYST(1992, jv80, 0, 0, jv80, jv80, roland_jv80_state, empty_init, "Roland", "JV-80 Multi Timbral Synthesizer", MACHINE_IS_SKELETON)
SYST(1992, jv880, 0, 0, jv880, jv880, roland_jv80_state, empty_init, "Roland", "JV-880 Multi Timbral Synthesizer Module", MACHINE_IS_SKELETON)
SYST(1994, rd500, 0, 0, rd500, rd500, roland_rd500_state, empty_init, "Roland", "RD-500 Digital Piano", MACHINE_IS_SKELETON)
