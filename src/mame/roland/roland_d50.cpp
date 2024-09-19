// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland D-50/D-550.

****************************************************************************/

#include "emu.h"
#include "cpu/upd78k/upd78k3.h"
#include "machine/bankdev.h"
#include "mb63h149.h"
#include "machine/nvram.h"


namespace {

class roland_d50_state : public driver_device
{
public:
	roland_d50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_eram(*this, "eram")
	{
	}

	void d50(machine_config &config);
	void d550(machine_config &config);

private:
	void d50_mem_map(address_map &map) ATTR_COLD;
	void d550_mem_map(address_map &map) ATTR_COLD;
	void eram_map(address_map &map) ATTR_COLD;

	required_device<upd78312_device> m_maincpu;
	required_device<address_map_bank_device> m_eram;
};


void roland_d50_state::d50_mem_map(address_map &map)
{
	// Internal ROM is enabled at 0000–1FFF (+5V pullup on EA pin)
	map(0x2000, 0x7fff).rom().region("progrom", 0x2000);
	map(0x8000, 0xbfff).m(m_eram, FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xdfff).ram();
	map(0xf400, 0xf7ff).rw("keyscan", FUNC(mb63h149_device::read), FUNC(mb63h149_device::write));
}

void roland_d50_state::d550_mem_map(address_map &map)
{
	// Internal ROM is enabled at 0000–1FFF (+5V pullup on EA pin)
	map(0x2000, 0x7fff).rom().region("progrom", 0x2000);
	map(0x8000, 0xbfff).m(m_eram, FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xdfff).ram();
}

void roland_d50_state::eram_map(address_map &map)
{
	map(0x00000, 0x07fff).ram().share("toneram");
	//map(0x08000, 0x0ffff).rw(FUNC(roland_d50_state::memcard_r), FUNC(roland_d50_state::memcard_w));
	map(0x10000, 0x13fff).mirror(0x4000).rom().region("progrom", 0x8000);
	map(0x18000, 0x1bfff).mirror(0x4000).rom().region("progrom", 0xc000);
}


static INPUT_PORTS_START(d50)
INPUT_PORTS_END


void roland_d50_state::d50(machine_config &config)
{
	UPD78312(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_d50_state::d50_mem_map);

	ADDRESS_MAP_BANK(config, m_eram);
	m_eram->set_addrmap(0, &roland_d50_state::eram_map);
	m_eram->set_data_width(8);
	m_eram->set_endianness(ENDIANNESS_LITTLE);
	m_eram->set_addr_width(17);
	m_eram->set_stride(0x4000);

	NVRAM(config, "toneram", nvram_device::DEFAULT_ALL_0); // HM62256LP-12 + battery

	// LCD unit is LM402802 (D-50) or LM402551 (D-550)

	MB63H149(config, "keyscan", 32.768_MHz_XTAL / 2); // on Dyna Scan Board
	//keyscan.int_callback().set_inputline(m_maincpu, upd78312_device::INT2_LINE);

	//MB87136(config, "synthe", 32.768_MHz_XTAL);
}

void roland_d50_state::d550(machine_config &config)
{
	d50(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_d50_state::d550_mem_map);

	config.device_remove("keyscan");
}

// the internal date format for the external program roms is as such:
// DDMY where DD is the day within the month, M is a month where
// January == A,February == B, etc, and Y is the last digit of the year
// (the first 3 digits are '198'); note that the date was not updated for
// some minor revisions.

// the portion after the data is in an unknown format, but is presumably
// version information for certain subprograms on the rom itself.

ROM_START(d50) // Newer PCB with silkscreen "Roland || D-50, D-550 || MAIN BOARD" and a checkbox for which board it is intended for "[O]D-50 | ASSY 76180090 || [ ]D-550 | ASSY 79379050" and trace layer ID "22925445 05" and a QFP LA32 "R15229896 || LA32 || 8816Q26" at IC31; the board supports either the split or the combined PCM roms.
	ROM_REGION(0x10000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v2.22", "ROM Version 2.22") // "        D-50    Ver 2.22                Thanks to Eric & Adrian.  14J7 D.420/7.9"
	ROMX_LOAD("d-50_222.ic22", 0x00000, 0x10000, CRC(e92c69f9) SHA1(fd783abc7fbd4abe2dedfe89d59e396b5e687a27), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2.21", "ROM Version 2.21") // "        D-50    Ver 2.21                Thanks to Eric & Adrian.  14J7 D.420/7.6"
	ROMX_LOAD("roland_d-50__r15179866-01__lh531452.v2.21.27c512.ic22", 0x0000, 0x10000, CRC(3e72bdf0) SHA1(6e7f8cd3b3ce385350ca2eda6aee73f5bbc433ef), ROM_BIOS(1)) // "Roland D-50 // R15179866-01 // LH531452" Mask ROM (23c512 equivalent)
	ROM_SYSTEM_BIOS(2, "v2.20cfw", "ROM Version 2.20 (CFW, patched)") // "        D-50    Ver 2.21                Thanks to Ricard W.       14J7 D.420/7.5"
	ROMX_LOAD("roland_d50_220rw_cfw_patched.ic22", 0x00000, 0x10000, CRC(8901640e) SHA1(abbc0026c64c55a6cbaca5e3e5dcb1a7536e91b3), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v2.20", "ROM Version 2.20") // "        D-50    Ver 2.20                Thanks to Eric & Adrian.  14J7 D.420/7.5"
	ROMX_LOAD("roland_d-50__r15179866__lh531306__8803_d.v2.20.27c512.ic22", 0x0000, 0x10000, CRC(5db7c340) SHA1(ada0e65bf04ec24188c01b03ad4e0087d88439a8), ROM_BIOS(3)) // "Roland D-50 // R15179866 // LH531306 // 8803 D" Mask ROM (23c512 equivalent)
	ROM_SYSTEM_BIOS(4, "v2.10", "ROM Version 2.10") // "        D-50    Ver 2.10                Thanks to Eric & Adrian.  16G7 D.400/7.4"
	ROMX_LOAD("d50__v.2.10.27c512.ic22", 0x00000, 0x10000, CRC(d1387c54) SHA1(904cf9daf296a66d3c4969266541983081e19471), ROM_BIOS(4))
	// missing 2.00

	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("d78312g-022_15179266.ic25", 0x0000, 0x2000, CRC(9564903f) SHA1(f68ed97a06764ee000fe6e9d7b39017165f0efc4)) // 8-digit Roland part number not printed on IC
	ROM_COPY("progrom", 0x0000, 0x0000, 0x2000)

	ROM_REGION(0x80000, "pcm", 0)
	ROM_LOAD("roland__r15179858_8801ebi__tc534000p-7477.ic30", 0x00000, 0x80000, CRC(e2aed2d9) SHA1(e9f5b38b9b5fce04beb4cf871401e821a42edacb)) // A+B "Roland || R15179858 8801EBI || TC534000P-7477" 512KiB Mask ROM @ ic30
	// ic29 is empty on boards with tc534000-sized Mask ROMs
ROM_END

ROM_START(d50o) // Older PCB with silkscreen "Roland || D-50 MAIN BOARD || ASSY 76180090" and trace layer ID "22925445 00" and a PGA LA32 "R15229851 || MB87136A || 8706 E02" at IC31, though the oldest service manual supposedly claims it could have an "MB87136" with no A as well. This board only supports the split PCM roms unless hand-modified. This board has a factory greenwire fix for a missing trace from the IC8 Chorus ASIC MB87137-001 pin 59 to the IC1 DRAM D41416 pin 17.
	ROM_REGION(0x10000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.10cfw", "ROM Version 1.10 maybe user patched") // "        D-50    Ver 1.10                Thanks to Eric & Adrian.  30G7 C.410/7.4"
	ROMX_LOAD("roland__1-11__2-1-96_greg.v1.10.ic22", 0x00000, 0x10000, CRC(7fc199c5) SHA1(3fc889d6c44f8b40de6619a3427d0c89730dd9b3), ROM_BIOS(0)) // this particular dump came from a handwritten label eprom chip labeled "Roland || 1-11 2/1/96 Greg" so it could be original code, but it was a hand-made update chip by some operator in the past. It is possible this code has a user-bugfix applied to it, though, given the 1-11 in the label.
	// missing 1.0.7
	ROM_SYSTEM_BIOS(1, "v1.06", "ROM Version 1.06") // "Thanks to Eric & Adrian.  01E7 C.36a/5.4        D-50    Ver 1.06                "
	ROMX_LOAD("d50-v1.06.ic22", 0x00000, 0x10000, CRC(ccba4e46) SHA1(ce56321226dbbf7dbfac2ad344da447ad6448ee8), ROM_BIOS(1))
	// missing 1.0.5
	ROM_SYSTEM_BIOS(2, "v1.04", "ROM Version 1.04") // "Thanks to Eric & Adrian.  08D7 C.35r/5.3        D-50    Ver 1.04                "
	ROMX_LOAD("d-50__1.0.4.mbm27c512.ic22", 0x00000, 0x10000, CRC(d871451e) SHA1(e692f3553ce6d61633c7551e98ad86f5c40e6449), ROM_BIOS(2)) // "D-50 // 1.0.4" on an MBM27c512 @ ic22 (original roland sticker? not sure, no pic)

	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("d78312g-017_15179261.ic25", 0x0000, 0x2000, NO_DUMP) // not compatible with newer firmware
	ROM_COPY("progrom", 0x0000, 0x0000, 0x2000)

	ROM_REGION(0x80000, "pcm", 0)
	ROM_LOAD("roland_a__r15179835_8710eai__tc532000p-7469.ic30", 0x00000, 0x40000, CRC(1461c0fb) SHA1(55257e70bcf003439b76f96b7ae7e45a3cf24276)) // A "Roland A || R15179835 8710EAI || TC532000P-7469" 256KiB Mask ROM @ ic30; this is identical to the first half of the 512KiB ROM; verified from original ic; also seen with 8736 datecode
	ROM_LOAD("roland_b__r15179836_8710eai__tc532000p-7470.ic29", 0x40000, 0x40000, CRC(e50599bf) SHA1(487c62f2ef7baccf2421059a940fa707e27aefb2)) // B "Roland B || R15179836 8710EAI || TC532000P-7470" 256KiB Mask ROM @ ic29; this is identical to the second half of the 512KiB ROM; verified from original ic; also seen with 8736 datecode
ROM_END

ROM_START(d550) // Newer PCB with silkscreen "Roland || D-50, D-550 || MAIN BOARD" and a checkbox for which board it is intended for "[ ]D-50 | ASSY 76180090 || [O]D-550 | ASSY 79379050" and trace layer ID "22925445 05" and a QFP LA32 "R15229896 || LA32 || 8816Q26" at IC31, This board supports either the split or the combined PCM roms.
	ROM_REGION(0x10000, "progrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.02", "ROM Version 1.02") // "        D-550   Ver 1.02                Thanks to Eric & Adrian.  20G7 D.000/1.0"
	ROMX_LOAD("roland_d-550_v1.02_eprom_firmware.ic22", 0x00000, 0x10000, CRC(11b54d24) SHA1(d11bdb50c49edababec73a936346a6f918dd0949), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.01", "ROM Version 1.01") // "        D-550   Ver 1.01                Thanks to Eric & Adrian.  20G7 D.000/0.9"
	ROMX_LOAD("roland_d-550_ver_1.01.ic22", 0x00000, 0x10000, CRC(c267019f) SHA1(314616d14b91e8c8733aee5dd64736fda8ff6904), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v1.0.0", "ROM Version 1.0.0") // "        D-550   Ver 1.00                Thanks to Eric & Adrian.  20G7 D.000/0.5"
	ROMX_LOAD("d-550__1.0.0.mbm27c512.ic22", 0x00000, 0x10000, CRC(f8ec84c3) SHA1(00b2b74009bdc0747e11bf57d4dc6c39424c7669), ROM_BIOS(2))

	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("d78312g-022_15179266.ic25", 0x0000, 0x2000, NO_DUMP)
	ROM_COPY("progrom", 0x0000, 0x0000, 0x2000)

	ROM_REGION(0x80000, "pcm", 0)
	ROM_LOAD("roland__r15179858_8801ebi__tc534000p-7477.ic30", 0x00000, 0x80000, CRC(e2aed2d9) SHA1(e9f5b38b9b5fce04beb4cf871401e821a42edacb)) // A+B "Roland || R15179858 8801EBI || TC534000P-7477" 512KiB Mask ROM @ ic30
	// ic29 is empty on boards with tc534000-sized Mask ROMs
ROM_END

} // anonymous namespace


SYST(1987, d50,  0,   0, d50,  d50, roland_d50_state, empty_init, "Roland", "D-50 Linear Synthesizer (Ver. 2.xx)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(1987, d50o, d50, 0, d50,  d50, roland_d50_state, empty_init, "Roland", "D-50 Linear Synthesizer (Ver. 1.xx)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(1987, d550, d50, 0, d550, d50, roland_d50_state, empty_init, "Roland", "D-550 Linear Synthesizer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
