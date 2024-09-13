// license:BSD-3-Clause
// copyright-holders:AJR,Valley Bell
/****************************************************************************

    Skeleton driver for Roland R-8 drum machine.

****************************************************************************/

/*
PCM Card Sample Table Format
----------------------------
Note: 2-byte values are stored in Little Endian.

General Layout
--------------
Pos   Len   Description
   80    01 card ID (00 for SN-R8-01, 01 for SN-R8-02, etc.)
   81    01 number of tones
   82    0E padding (byte 0xFF)
   90    70 tone offsets, 2 bytes per offset, relative to 0x100, each value points to the start of a "tone" definition
  100   500 tone list
  600   200 demo song header?
  800   800 ??
 1000  2000 demo song data?
 3000 7D000 sample data

Tone List (address: 0x100)
---------
Pos Len Description
00  08  tone name
08  01  ?? (always 0x8F)
09  01  ??
0A  01  frequency (0xE0 == 44100 Hz)
0B  02  ??
0C  02  sample A bank/flags
        sample cards:
            Bit 10 (0x0400) = bank (0 = card offsets 0x00000..0x3FFFF, 1 = card offsets 0x40000..0x7FFFF)
        R8 internal ROM:
            Bit 10 (0x0400) = bank bit 0 (0 = offsets 0x00000..0x3FFFF, 1 = offsets 0x40000..0x7FFFF)
            Bit 12 (0x1000) = bank bit 1 (0 = offsets 0x00000..0x7FFFF, 1 = offsets 0x80000..0xFFFFF)
        Bits 14-15 (0xC000) - play/loop mode
            0 = normal loop
            1 = no loop? (not used)
            2 = ping-pong 1? (forwards, backwards, forwards, backwards, ...)
            3 = ping-pong 2? (backwards, forwards, backwards, forwards, ...)
                -> when this is used, then the start address is usually set to (end address) or (end address+1)
0E  02  sample A start address (high word, i.e. address bits 2..17)
10  02  sample A start address, fraction (2.14 fixed point, i.e. 1 byte = 0x4000)
12  02  sample A loop address (high word)
14  02  sample A end address (high word)
16  02  ??
18  02  ??
1A  02  ??
1C  02  ??
1E  02  sample B bank and flags?
20  02  sample B start address (high word)
22  02  sample B start address, fraction
24  02  sample B loop address (high word)
26  02  sample B end address (high word)
28  02  ??
2A  02  ??
2C  02  ??
2E  02  ??
-> 30h bytes per entry

"sample A" is what is heard. The use for "sample B" is unknown.
The 8 unknwon consecutive bytes (0x16..0x1D and 0x28..0x2F) are probably related to the envelope generation.


R8/R8M internal tone list offsets: 0x018C10
R8/R8M internal tone list data: 0x018CA0
R8 mkII doesn't seem to store the tone list in the program ROM.

*/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/upd78k/upd78k2.h"
#include "machine/nvram.h"
#include "sound/roland_lp.h"

#include "softlist_dev.h"
#include "speaker.h"

#include <array>


namespace {

// PCM card address line scrambling
#define UNSCRAMBLE_ADDR_EXT(_offset) \
	bitswap<19>(_offset,18,17, 8, 9,16,11,12, 7,14,10,13,15, 3, 2, 1, 6, 4, 5, 0)

#define UNSCRAMBLE_DATA(_data) \
	bitswap<8>(_data,1,2,7,3,5,0,4,6)

// PCM card offsets in PCM chip memory space
// Right now this is an assumption based on the existing sample tables and wasn't confirmed with the firmware yet.
static const offs_t PCMCARD_SIZE = 0x080000;    // 512 KB
static const std::array<offs_t, 3> PCMCARD_OFFSETS = {0x080000, 0x180000, 0x280000};

class roland_r8_base_state : public driver_device
{
public:
	roland_r8_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pcm(*this, "pcm")
	{
	}

	void r8_common(machine_config &config);
	void init_r8();

protected:
	void mk1_map(address_map &map);
	void mk2_map(address_map &map);

	std::pair<std::error_condition, std::string> pcmrom_load(generic_slot_device* pcmcard, int card_id, device_image_interface &image);
	void pcmrom_unload(int card_id);
	void descramble_rom_external(u8* dst, const u8* src);

	required_device<upd78k2_device> m_maincpu;
	required_device<mb87419_mb87420_device> m_pcm;
	//required_device<generic_slot_device> m_ramcard;   // TODO
};

class roland_r8_state : public roland_r8_base_state
{
public:
	roland_r8_state(const machine_config &mconfig, device_type type, const char *tag)
		: roland_r8_base_state(mconfig, type, tag)
		, m_pcmcard(*this, "pcmcard")
	{
	}

	void r8(machine_config &config);

private:
	DEVICE_IMAGE_LOAD_MEMBER(pcmcard_load) { return pcmrom_load(m_pcmcard, 0, image); }
	DEVICE_IMAGE_UNLOAD_MEMBER(pcmcard_unload) { pcmrom_unload(0); }

	required_device<generic_slot_device> m_pcmcard;
};

class roland_r8m_state : public roland_r8_base_state
{
public:
	roland_r8m_state(const machine_config &mconfig, device_type type, const char *tag)
		: roland_r8_base_state(mconfig, type, tag)
		, m_pcmcards(*this, "pcmcard%u", 0U)
	{
	}

	void r8m(machine_config &config);

private:
	DEVICE_IMAGE_LOAD_MEMBER(pcmcard0_load) { return pcmrom_load(m_pcmcards[0], 0, image); }
	DEVICE_IMAGE_UNLOAD_MEMBER(pcmcard0_unload) { pcmrom_unload(0); }
	DEVICE_IMAGE_LOAD_MEMBER(pcmcard1_load) { return pcmrom_load(m_pcmcards[1], 1, image); }
	DEVICE_IMAGE_UNLOAD_MEMBER(pcmcard1_unload) { pcmrom_unload(1); }
	DEVICE_IMAGE_LOAD_MEMBER(pcmcard2_load) { return pcmrom_load(m_pcmcards[2], 2, image); }
	DEVICE_IMAGE_UNLOAD_MEMBER(pcmcard2_unload) { pcmrom_unload(2); }

	required_device_array<generic_slot_device, 3> m_pcmcards;
};

class roland_r8mk2_state : public roland_r8_base_state
{
public:
	roland_r8mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: roland_r8_base_state(mconfig, type, tag)
		, m_pcmcard(*this, "pcmcard")
	{
	}

	void r8mk2(machine_config &config);

private:
	DEVICE_IMAGE_LOAD_MEMBER(pcmcard_load) { return pcmrom_load(m_pcmcard, 0, image); }
	DEVICE_IMAGE_UNLOAD_MEMBER(pcmcard_unload) { pcmrom_unload(0); }

	required_device<generic_slot_device> m_pcmcard;
};


std::pair<std::error_condition, std::string> roland_r8_base_state::pcmrom_load(generic_slot_device *pcmcard, int card_id, device_image_interface &image)
{
	uint32_t const size = pcmcard->common_get_size("rom");
	if (size > PCMCARD_SIZE)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size (maximum supported is 512K)");

	pcmcard->rom_alloc(PCMCARD_SIZE, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	pcmcard->common_load_rom(pcmcard->get_rom_base(), size, "rom");
	u8 *base = pcmcard->get_rom_base();
	if (size < PCMCARD_SIZE)
	{
		uint32_t mirror = (1 << (31 - count_leading_zeros_32(size)));
		if (mirror < 0x020000)  // due to how address descrambling works, we can currently only do mirroring for 128K pages
			mirror = 0x020000;
		for (uint32_t ofs = mirror; ofs < PCMCARD_SIZE; ofs += mirror)
			memcpy(base + ofs, base, mirror);
	}

	offs_t pcm_addr = PCMCARD_OFFSETS[card_id];
	u8 *src = reinterpret_cast<u8 *>(memregion("pcmorg")->base());
	u8 *dst = reinterpret_cast<u8 *>(memregion("pcm")->base());
	memcpy(&src[pcm_addr], base, PCMCARD_SIZE);
	// descramble PCM card ROM
	descramble_rom_external(&dst[pcm_addr], &src[pcm_addr]);
	//pcmard_loaded[card_id] = true;

	return std::make_pair(std::error_condition(), std::string());
}

void roland_r8_base_state::pcmrom_unload(int card_id)
{
	u8 *src = reinterpret_cast<u8 *>(memregion("pcmorg")->base());
	u8 *dst = reinterpret_cast<u8 *>(memregion("pcm")->base());
	offs_t pcm_addr = PCMCARD_OFFSETS[card_id];
	memset(&src[pcm_addr], 0xff, PCMCARD_SIZE);
	memset(&dst[pcm_addr], 0xff, PCMCARD_SIZE);
	//pcmard_loaded[card_id] = false;
}


void roland_r8_base_state::mk1_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("maincpu", 0);
	map(0x20000, 0x27fff).ram().share("nvram");
	map(0x70000, 0x7001f).rw(m_pcm, FUNC(mb87419_mb87420_device::read), FUNC(mb87419_mb87420_device::write));
}

void roland_r8_base_state::mk2_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("maincpu", 0);
	map(0x20000, 0x27fff).ram().share("nvram1");
	map(0x28000, 0x2ffff).ram().share("nvram2");
	map(0x70000, 0x7001f).rw(m_pcm, FUNC(mb87419_mb87420_device::read), FUNC(mb87419_mb87420_device::write));
}


static INPUT_PORTS_START(r8)
INPUT_PORTS_END


void roland_r8_base_state::r8_common(machine_config &config)
{
	UPD78210(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_r8_state::mk1_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // SRM20256LC-12 + battery

	//bu3904s_device &fsk(BU3904S(config, "fsk", 12_MHz_XTAL));
	//fsk.xint_callback().set_inputline(m_maincpu, upd78k2_device::INTP0_LINE);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MB87419_MB87420(config, m_pcm, 33.8688_MHz_XTAL);
	//m_pcm->int_callback().set_inputline(m_maincpu, upd78k2_device::INTP1_LINE);
	m_pcm->set_device_rom_tag("pcm");
	m_pcm->add_route(0, "lspeaker", 1.0);
	m_pcm->add_route(1, "rspeaker", 1.0);
}

void roland_r8_state::r8(machine_config &config)
{
	r8_common(config);

	GENERIC_CARTSLOT(config, m_pcmcard, generic_romram_plain_slot, "r8_card", "bin");
	m_pcmcard->set_device_load(FUNC(roland_r8_state::pcmcard_load));
	m_pcmcard->set_device_unload(FUNC(roland_r8_state::pcmcard_unload));

	SOFTWARE_LIST(config, "card_list").set_original("r8_card");
}

void roland_r8m_state::r8m(machine_config &config)
{
	r8_common(config);

	UPD78213(config.replace(), m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_r8m_state::mk1_map);

	for (auto& pcmcard : m_pcmcards)
		GENERIC_CARTSLOT(config, pcmcard, generic_romram_plain_slot, "r8_card", "bin");
	m_pcmcards[0]->set_device_load(FUNC(roland_r8m_state::pcmcard0_load));
	m_pcmcards[0]->set_device_unload(FUNC(roland_r8m_state::pcmcard0_unload));
	m_pcmcards[1]->set_device_load(FUNC(roland_r8m_state::pcmcard1_load));
	m_pcmcards[1]->set_device_unload(FUNC(roland_r8m_state::pcmcard1_unload));
	m_pcmcards[2]->set_device_load(FUNC(roland_r8m_state::pcmcard2_load));
	m_pcmcards[2]->set_device_unload(FUNC(roland_r8m_state::pcmcard2_unload));
	SOFTWARE_LIST(config, "card_list").set_original("r8_card");
}

void roland_r8mk2_state::r8mk2(machine_config &config)
{
	UPD78213(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_r8mk2_state::mk2_map);

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // SRM20256LC-10 + battery
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // SRM20256LC-10 + battery

	//bu3904s_device &fsk(BU3904S(config, "fsk", 12_MHz_XTAL));
	//fsk.xint_callback().set_inputline(m_maincpu, upd78k2_device::INTP0_LINE);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MB87419_MB87420(config, m_pcm, 33.8688_MHz_XTAL);
	//m_pcm->int_callback().set_inputline(m_maincpu, upd78k2_device::INTP1_LINE);
	m_pcm->set_device_rom_tag("pcm");
	m_pcm->add_route(0, "lspeaker", 1.0);
	m_pcm->add_route(1, "rspeaker", 1.0);

	GENERIC_CARTSLOT(config, m_pcmcard, generic_romram_plain_slot, "r8_card", "bin");
	m_pcmcard->set_device_load(FUNC(roland_r8mk2_state::pcmcard_load));
	m_pcmcard->set_device_unload(FUNC(roland_r8mk2_state::pcmcard_unload));
	SOFTWARE_LIST(config, "card_list").set_original("r8_card");
}

void roland_r8_base_state::init_r8()
{
	u8 *src = static_cast<u8*>(memregion("pcmorg")->base());
	u8 *dst = static_cast<u8*>(memregion("pcm")->base());

	for (offs_t addr = 0; addr < memregion("pcmorg")->bytes(); addr += 0x100000)
	{
		// TODO: descramble internal ROMs
		memcpy(&dst[addr + 0x00000], &src[addr + 0x00000], 0x80000);
		// descramble PCM card ROMs
		descramble_rom_external(&dst[addr + 0x80000], &src[addr + 0x80000]);
	}
}

void roland_r8_base_state::descramble_rom_external(u8* dst, const u8* src)
{
	for (offs_t srcpos = 0x00; srcpos < PCMCARD_SIZE; srcpos ++)
	{
		offs_t dstpos = UNSCRAMBLE_ADDR_EXT(srcpos);
		dst[dstpos] = UNSCRAMBLE_DATA(src[srcpos]);
	}
}


ROM_START(r8)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("roland r-8_2.02_27c010.bin", 0x00000, 0x20000, CRC(45d0f64f) SHA1(55f0831db74cbdeae20cd7f1ff28af27dafba9b9))

	ROM_REGION(0x200000, "pcmorg", ROMREGION_ERASE00) // ROMs before descrambling
	ROM_LOAD("r15179929-mn234000rle.ic30", 0x000000, 0x080000, NO_DUMP)
	// 0x80000..0xFFFFF is reserved for PCM cards, according to the sample list
	ROM_LOAD("r15179930-mn234000rlf.ic31", 0x100000, 0x080000, NO_DUMP)
	ROM_REGION(0x200000, "pcm", ROMREGION_ERASE00) // ROMs after descrambling
ROM_END

ROM_START(r8m)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("rolandr8mv104.bin", 0x00000, 0x20000, CRC(5e95e2f6) SHA1(b4e1a8f15f72a9db9aa8fd41ee3c3ebd10460587))

	ROM_REGION(0x400000, "pcmorg", ROMREGION_ERASE00) // ROMs before descrambling (must be large enough to allow for 3 PCM cards at offsets 0.5M, 1.5M and 2.5M)
	// same ROMs as R-8 assumed
	ROM_LOAD("r15179929-mn234000rle.bin", 0x000000, 0x080000, NO_DUMP)
	// 0x80000..0xFFFFF is reserved for PCM cards, according to the sample list
	ROM_LOAD("r15179930-mn234000rlf.bin", 0x100000, 0x080000, NO_DUMP)
	ROM_REGION(0x400000, "pcm", ROMREGION_ERASE00) // ROMs after descrambling
ROM_END

ROM_START(r8mk2)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("roland r8 mkii eprom v1.0.3.bin", 0x00000, 0x20000, CRC(128a9a0c) SHA1(94bd8c76efe270754219f2899f31b62fc4f9060d))

	ROM_REGION(0x400000, "pcmorg", ROMREGION_ERASE00) // ROMs before descrambling
	ROM_LOAD("r15209440-upd27c8001eacz-025.ic30", 0x000000, 0x080000, NO_DUMP)
	ROM_LOAD("r15209441-upd27c8001eacz-026.ic31", 0x100000, 0x080000, NO_DUMP)
	ROM_LOAD("r15209442-upd27c8001eacz-027.ic82", 0x200000, 0x080000, NO_DUMP)
	ROM_REGION(0x400000, "pcm", ROMREGION_ERASE00) // ROMs after descrambling
ROM_END

} // anonymous namespace


SYST(1989, r8,    0,  0, r8,    r8, roland_r8_state, init_r8, "Roland", "R-8 Human Rhythm Composer (v2.02)", MACHINE_IS_SKELETON)
SYST(1990, r8m,   r8, 0, r8m,   r8, roland_r8m_state, init_r8, "Roland", "R-8M Total Percussion Sound Module (v1.04)", MACHINE_IS_SKELETON)
SYST(1992, r8mk2, 0,  0, r8mk2, r8, roland_r8mk2_state, init_r8, "Roland", "R-8 Mk II Human Rhythm Composer (v1.0.3)", MACHINE_IS_SKELETON)
