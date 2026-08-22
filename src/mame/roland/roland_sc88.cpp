// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland SC-88 MIDI sound generator.

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8510.h"
//#include "cpu/m6502/m38881.h"
#include "machine/nvram.h"


namespace {

class roland_sc88_state : public driver_device
{
public:
	roland_sc88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void sc88(machine_config &config);
	void sc88vl(machine_config &config);
	void sc88pro(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;
	void pro_map(address_map &map) ATTR_COLD;

	required_device<h8510_device> m_maincpu;
};


void roland_sc88_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("progrom", 0);
	map(0x080000, 0x08ffff).ram().share("nvram");
}

void roland_sc88_state::pro_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("progrom", 0);
	map(0x100000, 0x10ffff).ram().share("nvram");	// TODO: probably incorrect
}

static INPUT_PORTS_START(sc88)
INPUT_PORTS_END

static INPUT_PORTS_START(sc88vl)
INPUT_PORTS_END

static INPUT_PORTS_START(sc88pro)
INPUT_PORTS_END

void roland_sc88_state::sc88(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc88_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // ??

	//M38881M2(config, "subcpu", 20_MHz_XTAL / 2);
}

void roland_sc88_state::sc88vl(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc88_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // SRM2A256SLM-70 x2 + battery

	//M38881M2(config, "subcpu", 20_MHz_XTAL / 2);
}

void roland_sc88_state::sc88pro(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc88_state::pro_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // ??

	//M38881M2(config, "subcpu", 20_MHz_XTAL / 2);
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))
#define ROM_LOAD16_WORD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

ROM_START(sc88)
	ROM_REGION16_BE(0x80000, "progrom", 0)
	ROM_DEFAULT_BIOS("102")
	ROM_SYSTEM_BIOS(0, "102", "Control ROM 1.0.2")	// v1.02, 1994-06-16, 18:43 (offset 0x7cff6)
	ROM_LOAD16_WORD_SWAP_BIOS(0, "roland_sc88-control-1.02-hn27c4096h.ic17", 0x00000, 0x80000, CRC(9e9c56f9) SHA1(9291a4e4ac0fea9fb5a39777be501ab3d34877c8))
	ROM_SYSTEM_BIOS(1, "103", "Control ROM 1.0.3")	// v1.03, 1994-07-01, 10:48 (offset 0x7d136)
	ROM_LOAD16_WORD_SWAP_BIOS(1, "roland_sc88-control-1.03-hn27c4096hg-85.ic17", 0x00000, 0x80000, CRC(cf953f71) SHA1(6a25d46b608b317d2604b6424fea7b50978b5d32)) // HN27C4096HG-85
	ROM_SYSTEM_BIOS(2, "104", "Control ROM 1.0.4")	// v1.04, 1994-08-04, 12:41 (offset 0x7d1a6)
	// This ROM dump is circulating labelled as "Roland SC88 Version 1.01", but it really is 1.04.
	ROM_LOAD16_WORD_BIOS(2, "roland_sc88-control-1.04.ic17", 0x00000, 0x80000, CRC(979b6c09) SHA1(3bc9a68703bd09459283b6b45d01d08feaffb744)) // dumper comment: original chip was a 27C4096

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("subcpu.ic11", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION16_LE(0x800000, "waverom", 0)
	ROM_LOAD("sc88-pcm-ic-325.ic14", 0x000000, 0x200000, CRC(f9dd9e49) SHA1(860dcd9804ded4cd46aab38bfc3764a1ad7a6b65))
	ROM_LOAD("sc88-pcm-ic-326.ic8",  0x200000, 0x200000, CRC(05f939f2) SHA1(ac95c26c46c40aacb944f5d45634c93bee9e6d90))
	ROM_LOAD("sc88-pcm-ic-327.ic7",  0x400000, 0x200000, CRC(a6fc7393) SHA1(d88bf13c3d74097991b783295d95ccfae2c9282d))
	ROM_LOAD("sc88-pcm-ic-328.ic6",  0x600000, 0x200000, CRC(7bc514aa) SHA1(03e70ae2efd190f41af24be01b1abaa84bfa93d9))
ROM_END

ROM_START(sc88vl)
	ROM_REGION16_BE(0x80000, "progrom", 0)
	ROM_LOAD16_WORD_SWAP("roland_sc88_vl-1.04.ic29", 0x00000, 0x80000, CRC(66aa5762) SHA1(3a20f8f8cefd0d5e1edb103046f6fe94bb73ac7a))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("roland-r00232667-m38881m2-150gp.ic23", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION16_LE(0x800000, "waverom", 0)
	ROM_LOAD("roland-r00785356-hn624316fbc25.ic10", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD("roland-r00785367-hn624316fbc26.ic7",  0x200000, 0x200000, NO_DUMP)
	ROM_LOAD("roland-r00788489-hn624316fbc27.ic4",  0x400000, 0x200000, NO_DUMP)
	ROM_LOAD("roland-r00788490-hn624316fbc28.ic2",  0x600000, 0x200000, NO_DUMP)
ROM_END

ROM_START(sc88pro)
	ROM_REGION16_BE(0x100000, "progrom", 0)
	ROM_LOAD16_WORD_SWAP("roland-r01780078.ic26", 0x00000, 0x100000, CRC(6cf8fc8b) SHA1(c07cbbd026781428ae83f4b897a51cb7bcdafeb6))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("subcpu.ic10", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION16_LE(0x1400000, "waverom", 0)
	// source unverified - the SC-88Pro service manual lists IC20..24 but the dumped files were called:
	// "R01567167 301 (Samples A).bin", "R01567178 302 (Samples B).BIN" and "R01233667 314 (Samples C).BIN"
	ROM_LOAD("roland-r01567167-301.ic20-21", 0x0000000, 0x800000, CRC(5754EE2E) SHA1(5cc1700b3f41921ed81fe3b92ba9ec28bd6649c9))
	ROM_LOAD("roland-r01567178-302.ic22-23", 0x0800000, 0x800000, CRC(E2B57861) SHA1(78b37ce0e735ad2c3e51338e74219a103d7fadba))
	ROM_LOAD("roland-r01233667-314.ic24",    0x1000000, 0x400000, CRC(93541E95) SHA1(e2ad5f0b2e5bdad84e42d080fc2e2fad523cb84b))
ROM_END

} // anonymous namespace


SYST(1994, sc88, 0, 0, sc88, sc88, roland_sc88_state, empty_init, "Roland", "SoundCanvas SC-88", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1995, sc88vl, 0, 0, sc88vl, sc88vl, roland_sc88_state, empty_init, "Roland", "SoundCanvas SC-88VL", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1996, sc88pro, 0, 0, sc88pro, sc88pro, roland_sc88_state, empty_init, "Roland", "SoundCanvas SC-88Pro", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
