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
	if (m_rom_region->bytes() != 0x100000)
	{
		fatalerror("Memory region '%s' is not the correct size for the FS-A1FM firmware\n", m_rom_region.finder_tag());
	}

	m_ram.resize(RAM_SIZE);
	m_empty_bank.resize(8 * 1024);
	for (int i = 0; i < 8 * 1024; i++)
		m_empty_bank[i] = 0xff;
	m_rom = m_rom_region->base() + m_region_offset;

	save_item(NAME(m_selected_bank));
	save_item(NAME(m_control));
	save_item(NAME(m_ram_active));
	save_pointer(m_ram.data(), "ram", RAM_SIZE);
}


void msx_slot_fsa1fm2_device::device_reset()
{
	for (int i = 0; i < 6 ; i++)
	{
		m_selected_bank[i] = 0xa8;
		m_ram_active[i] = false;
	}

	map_all_banks();
}


void msx_slot_fsa1fm2_device::device_post_load()
{
	map_all_banks();
}


void msx_slot_fsa1fm2_device::map_all_banks()
{
	for (int i = 0; i < 6; i++)
		map_bank(i);
}


void msx_slot_fsa1fm2_device::map_bank(int i)
{
	m_ram_active[i] = false;
	if ((m_selected_bank[i] & 0xf4) == 0x80) // 0x80-0x83 and 0x88-0x8b
	{
		m_bank_base[i] = m_empty_bank.data();
	}
	else if ((m_selected_bank[i] & 0xf4) == 0x84) // 0x84-0x87 and 0x8c-0x8f
	{
		m_ram_active[i] = true;
		m_bank_base[i] = m_ram.data();
	}
	else
	{
		m_bank_base[i] = m_rom + ((m_selected_bank[i] * 0x2000) & (m_rom_region->bytes() - 1));
	}
}


uint8_t msx_slot_fsa1fm2_device::read(offs_t offset)
{
	if (offset < 0x2000)
	{
		return m_bank_base[0][offset & 0x1fff];
	}
	if (offset < 0x4000)
	{
		return m_bank_base[1][offset & 0x1fff];
	}
	if (offset < 0x6000)
	{
		return m_bank_base[2][offset & 0x1fff];
	}
	if (offset < 0x8000)
	{
		if (offset >= 0x7ff0 && offset < 0x7ff8 && BIT(m_control, 2))
		{
			if (offset >= 0x7ff6)
				return 0;
			return m_selected_bank[offset - 0x7ff0];
		}
		return m_bank_base[3][offset & 0x1fff];
	}
	if (offset < 0xa000)
	{
		return m_bank_base[4][offset & 0x1fff];
	}
	if (offset < 0xc000)
	{
		return m_bank_base[5][offset & 0x1fff];
	}
	return 0xff;
}


void msx_slot_fsa1fm2_device::write(offs_t offset, uint8_t data)
{
	if (offset < 0x2000)
	{
		if (m_ram_active[0])
			m_ram[offset & 0x1fff] = data;
	}
	if (offset < 0x4000)
	{
		if (m_ram_active[1])
			m_ram[offset & 0x1fff] = data;
	}
	if (offset < 0x6000)
	{
		if (m_ram_active[2])
			m_ram[offset & 0x1fff] = data;
	}
	if (offset < 0x8000)
	{
		if (offset == 0x6000) // 4000 - 5fff
		{
			m_selected_bank[2] = data;
			map_bank(2);
		}
		if (offset == 0x6400) // 0000 - 1fff
		{
			m_selected_bank[0] = data;
			map_bank(0);
		}
		if (offset == 0x6800) // 6000 - 7fff
		{
			m_selected_bank[3] = data;
			map_bank(3);
		}
		if (offset == 0x6c00) // 2000 - 3fff
		{
			m_selected_bank[1] = data;
			map_bank(1);
		}
		if (offset == 0x7000) // 8000 - 9fff
		{
			m_selected_bank[4] = data;
			map_bank(4);
		}
		if (offset == 0x7800) // a000 - bfff
		{
			m_selected_bank[5] = data;
			map_bank(5);
		}
		if (offset == 0x7ff9)
		{
			// writing $04 enables read back of banking registers at 7ff0-7ff5
			m_control = data;
		}
		if (m_ram_active[3])
			m_ram[offset & 0x1fff] = data;
	}
	if (offset < 0xa000)
	{
		if (m_ram_active[4])
			m_ram[offset & 0x1fff] = data;
	}
	if (offset < 0xc000)
	{
		if (m_ram_active[5])
			m_ram[offset & 0x1fff] = data;
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
	, m_bank_base(nullptr)
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

	// Unknown how these are connected
	I8251(config, m_i8251, 0);

	I8255(config, m_i8255);
	m_i8255->out_pa_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_a_w));
	m_i8255->out_pb_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_b_w));
	m_i8255->in_pc_callback().set(FUNC(msx_slot_fsa1fm_device::i8255_port_c_r));
}


void msx_slot_fsa1fm_device::device_start()
{
	// Sanity checks
	if (m_rom_region->bytes() != 0x100000)
	{
		fatalerror("Memory region '%s' is not the correct size for the FS-A1FM firmware\n", m_rom_region.finder_tag());
	}

	m_rom = m_rom_region->base();
	m_sram.resize(SRAM_SIZE);
	m_nvram->set_base(m_sram.data(), SRAM_SIZE);

	save_item(NAME(m_selected_bank));

	m_selected_bank = 0x00;

	map_bank();
}


void msx_slot_fsa1fm_device::device_reset()
{
	m_selected_bank = 0x00;

	map_bank();
}


void msx_slot_fsa1fm_device::device_post_load()
{
	map_bank();
}


void msx_slot_fsa1fm_device::map_bank()
{
	m_bank_base = m_rom + ((m_selected_bank & 0x0f) * 0x2000);
}


void msx_slot_fsa1fm_device::i8255_port_a_w(uint8_t data)
{
	// 0xc0 enables sram?
	logerror("port A write %02x\n", data);
	m_selected_bank = data;
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
	if (offset >= 0x4000 && offset < 0x8000)
	{
		if (offset == 0x7fc0)
		{
			return m_i8251->data_r();
		}
		if (offset >= 0x7fc4 && offset <= 0x7fc7)
		{
			return m_i8255->read(offset & 0x03);
		}
		if (offset >= 0x6000)
			return m_sram[offset & 0x1fff];
		return m_bank_base[offset & 0x1fff];
	}
	return 0xff;
}


void msx_slot_fsa1fm_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0x7ee7)
	{
		// unknown
	}
	if (offset == 0x7ee8)
	{
		// unknown
	}
	if (offset == 0x7ee9)
	{		
		// unknown
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
		// unknown
	}
	if (offset == 0x7fcc)
	{
		// unknown
	}
	if (offset >= 0x6000 && offset <= 0x8000)
	{
		m_sram[offset & 0x1fff] = data;
		return;
	}
}
