// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*

  Emulation of the firmware mapper in the Panasonic FS-A1FM.

*/

#include "emu.h"
#include "fsa1fm.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_FSA1FM, msx_slot_fsa1fm_device, "msx_slot_fs1afm", "MSX Internal FS-A1FM")
DEFINE_DEVICE_TYPE(MSX_SLOT_FSA1FM2, msx_slot_fsa1fm2_device, "msx_slot_fs1afm2", "MSX Internal FS-A1FM part 2")


msx_slot_fsa1fm2_device::msx_slot_fsa1fm2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_FSA1FM2, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
{
}

void msx_slot_fsa1fm2_device::device_start()
{
	// Sanity checks
	// There is also a 0x200000 sized rom?
	if (m_rom_region->bytes() != 0x100000)
	{
		fatalerror("Memory region '%s' is not the correct size for the FS-A1FM firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base() + m_region_offset;

	save_item(NAME(m_selected_bank2));

	m_selected_bank2[0] = 0x00;
	m_selected_bank2[1] = 0x00;
	m_selected_bank2[2] = 0x00;
	m_selected_bank2[3] = 0x00;

	map_bank();
}


void msx_slot_fsa1fm2_device::device_reset()
{
	m_selected_bank2[0] = 0x00;
	m_selected_bank2[1] = 0x00;
	m_selected_bank2[2] = 0x00;
	m_selected_bank2[3] = 0x00;

	map_bank();
}


void msx_slot_fsa1fm2_device::device_post_load()
{
	map_bank();
}


void msx_slot_fsa1fm2_device::map_bank()
{
	const uint32_t size_mask = m_rom_region->bytes() - 1;

	m_bank_base2[0] = m_rom + ((m_selected_bank2[0] * 0x2000) & size_mask);
	m_bank_base2[1] = m_rom + ((m_selected_bank2[1] * 0x2000) & size_mask);
	m_bank_base2[2] = m_rom + ((m_selected_bank2[2] * 0x2000) & size_mask);
	m_bank_base2[3] = m_rom + ((m_selected_bank2[3] * 0x2000) & size_mask);
}


uint8_t msx_slot_fsa1fm2_device::read(offs_t offset)
{
	if (offset < 0x4000)
	{
		return m_rom[offset & 0x3fff];
	}
	if (offset < 0x6000)
	{
		return m_bank_base2[0][offset & 0x1fff];
	}
	if (offset < 0x8000)
	{
		return m_bank_base2[1][offset & 0x1fff];
	}
	if (offset < 0xa000)
	{
		return m_bank_base2[2][offset & 0x1fff];
	}
	if (offset < 0xc000)
	{
		return m_bank_base2[3][offset & 0x1fff];
	}
	return 0xff;
}


void msx_slot_fsa1fm2_device::write(offs_t offset, uint8_t data)
{
	logerror("%s: write %04x : %02x\n", machine().describe_context(), offset, data);
	if (offset == 0x6000)
	{
		m_selected_bank2[0] = data;
		map_bank();
	}
	if (offset == 0x6800)
	{
		m_selected_bank2[1] = data;
		map_bank();
	}
	if (offset == 0x7000)
	{
		m_selected_bank2[2] = data;
		map_bank();
	}
	if (offset == 0x7800)
	{
		m_selected_bank2[3] = data;
		map_bank();
	}
	if (offset == 0x7ff9)
	{
		// writing $04 enables banking registers 6000 / 6800 / 7000 / 7800 ?
		// followed by read from 7ff2??
	}
}




msx_slot_fsa1fm_device::msx_slot_fsa1fm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_FSA1FM, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_i8251(*this, "i8251")
	, m_i8255(*this, "i8255")
	, m_switch_port(*this, "SWITCH")
	, m_rom_region(*this, finder_base::DUMMY_TAG)
	, m_region_offset(0)
	, m_rom(nullptr)
	, m_bank_base_0000(nullptr)
	, m_bank_base_4000(nullptr)
	, m_bank_base_8000(nullptr)
{
}


static INPUT_PORTS_START(fsa1fm)
	PORT_START("SWITCH")
	PORT_CONFNAME(0x04, 0x00, "Firmware is")
	PORT_CONFSETTING(0x04, "disabled")
	PORT_CONFSETTING(0x00, "enabled")
INPUT_PORTS_END


ioport_constructor msx_slot_fsa1fm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(fsa1fm);
}


void msx_slot_fsa1fm_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	I8251(config, m_i8251, 0);
	// TODO

	I8255(config, m_i8255);
	m_i8255->out_pa_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_a_w));
	m_i8255->out_pb_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_b_w));
	m_i8255->in_pc_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_c_r));
	// TODO
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

	m_selected_bank[0] = 0x00;
	m_selected_bank[1] = 0x00;
	m_selected_bank[2] = 0x00;

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
	const uint32_t size_mask = m_rom_region->bytes() - 1;
	// Odd to have this bank per 0x2000?
	m_bank_base_0000 = m_rom + ((m_selected_bank[0] * 0x2000) & size_mask);
	m_bank_base_4000 = m_rom + ((m_selected_bank[1] * 0x2000) & size_mask);
	m_bank_base_8000 = m_rom + ((m_selected_bank[2] * 0x2000) & size_mask);
}


void msx_slot_fsa1fm_device::i8255_port_a_w(uint8_t data)
{
	logerror("port A write %02x\n", data);
	m_selected_bank[1] = data & 0x3f;
	map_bank();
}


void msx_slot_fsa1fm_device::i8255_port_b_w(uint8_t data)
{
	logerror("port B write %02x\n", data);
}


uint8_t msx_slot_fsa1fm_device::i8255_port_c_r()
{
	logerror("port C read\n");
	return m_switch_port->read();
}


uint8_t msx_slot_fsa1fm_device::read(offs_t offset)
{
	if (offset < 0x4000)
	{
		return m_bank_base_0000[offset & 0x3fff];
	}
	if (offset < 0x8000)
	{
		// TODO: How much of the sram is visible?
		if (offset >= 0x6000 && offset < 0x6400)
			return m_sram[offset & 0x3ff];
		if (offset == 0x7fc0)
		{
			return m_i8251->data_r();
		}
		if (offset >= 0x7fc4 && offset <= 0x7fc7)
		{
			return m_i8255->read(offset & 0x03);
		}
		return m_bank_base_4000[offset & 0x3fff];
	}
	if (offset < 0xc000)
	{
		return m_bank_base_8000[offset & 0x3fff];
	}
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
	logerror("%s: write %04x : %02x\n", machine().describe_context(), offset, data);
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
	if (offset == 0x7ee7)
	{
	}
	if (offset == 0x7ee8)
	{
	}
	if (offset == 0x7ee9)
	{		
	}
	if (offset == 0x7fc0)
	{
		m_i8251->data_w(data);
	}
	if (offset == 0x7fc1)
	{
		m_i8251->control_w(data);
	}
	if (offset >= 0x7fc4 && offset <= 0x7fc7)
	{
		m_i8255->write(offset & 0x03, data);
	}
	if (offset == 0x7fc8)
	{
	}
	if (offset == 0x7fcc)
	{
	}
}
