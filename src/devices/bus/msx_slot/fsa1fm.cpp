// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper in the Panasonic FS-A1FM.

*/

#include "emu.h"
#include "fsa1fm.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_FSA1FM, msx_slot_fsa1fm_device, "msx_slot_fs1afm", "MSX Internal FS-A1FM")


msx_slot_fsa1fm_device::msx_slot_fsa1fm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_FSA1FM, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
	, m_bank_base_0000(nullptr)
	, m_bank_base_4000(nullptr)
	, m_bank_base_8000(nullptr)
{
}


void msx_slot_fsa1fm_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}


void msx_slot_fsa1fm_device::device_start()
{
	// Sanity checks
	// There is also a 0x200000 sized rom?
	if (m_rom_region->bytes() != 0x100000)
	{
		fatalerror("Memory region '%s' is not the correct size for the FS-A1FM firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base() + m_region_offset;
	m_sram.resize(SRAM_SIZE);
	m_nvram->set_base(m_sram.data(), SRAM_SIZE);

	save_item(NAME(m_selected_bank));

	map_bank();
}


void msx_slot_fsa1fm_device::device_reset()
{
	m_selected_bank[0] = 0x00;
	m_selected_bank[1] = 0x00;
	m_selected_bank[2] = 0x00;

	map_bank();
}


void msx_slot_fsa1fm_device::device_post_load()
{
	map_bank();
}


void msx_slot_fsa1fm_device::map_bank()
{
	m_bank_base_0000 = m_rom + m_selected_bank[0] * 0x2000;
	m_bank_base_4000 = m_rom + m_selected_bank[1] * 0x2000;
	m_bank_base_8000 = m_rom + m_selected_bank[2] * 0x2000;
}


uint8_t msx_slot_fsa1fm_device::read(offs_t offset)
{
	if (offset < 0x4000)
		return m_bank_base_0000[offset & 0x3fff];
	if (offset < 0x8000)
	{
		// TODO: How much of the sram is visible?
		if (offset >= 0x6000 && offset < 0x6400)
			return m_sram[offset & 0x3ff];
		return m_bank_base_4000[offset & 0x3fff];
	}
	if (offset < 0xc000)
		return m_bank_base_8000[offset & 0x3fff];
	return 0xff;
}


void msx_slot_fsa1fm_device::write(offs_t offset, uint8_t data)
{
	// TODO: How much of the sram is visible?
	if (offset >= 0x6000 && offset <= 0x6400)
	{
		m_sram[offset & 0x3ff] = data;
		return;
	}
	logerror("write %04x : %02x\n", offset, data);
	// Should switch to bank #0 in slot #1
	// 7fc7 <- 89
	// 7fc4 <- c0

	// SRAM should already be enabled here (is it always enabled?)
	// read 619c
	// 60a3 <- c0
	// 7fc4 <- c0
	// 60a4 <- 7b
	// 7fc5 <- 7b
	// clear 6000 - 6363
	// 7ee7 <- 00
	// 6372 <- 00
	// 7ee7 <- 0000
	// 6374 <- 0000
	// 60a3 <- c0
	// 7fc4 <- c0
	// 60a4 <- 7b
	// 7fc5 <- 7b
	// copy $47 bytes from 4604 to 6000
	// copy $0d bytes from 464b to 6065
	// 60a9 <- ff
	// call 6000

	// at 4627 write to 7fc4 followed by jp (hl)

	// 4e35: ld a,(7fc6) bit 6,a
	// 4e41: ld a,(7fc6) bit 7,a
	// 4f4c: ld a,(7fc1) and $02
	// 4f55: ld a,(7fc0) ld hl,6084 and (hl)
	// 51cb: ld (7fc0),a
	// 524f: ld (7fc0),a
	// 540d: ld (60a3),a ld (7fc4),a ret
	// 5415: ld (60a4),a ld (7fc5),a ret
	// 5496: ld a,03 di ld (7fc8),a pop af ld (7fcc),a
	// 54a3: ld a,02 di ld (7fc8),a ld a,(7fcc) ret
	// 54b1: ld a,(7fc1) and 05 cp 05
	// 54bc: ld (7fc1),a ld (6083),a ret
	// 54c3: xor a ld (7fc1),a push af pop af ld (7fc1),a push af pop af ld (7fc1),a  push af pop af ld a,40 ld (7fc1),a push af pop af ld a,(6082) ld (7fc1),a push af pop af ld a,(7fc0)
	if (offset == 0x7fc4) {
		m_selected_bank[1] = data & 0x3f;
		map_bank();
	}
}
