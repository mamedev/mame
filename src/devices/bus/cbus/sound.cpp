// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* Internal sound slot options */

#include "emu.h"
#include "sound.h"

DEFINE_DEVICE_TYPE(SOUND_PC9821CE, sound_pc9821ce_device, "sound_pc9821ce", "NEC PC-9821CE built-in sound")

sound_pc9821ce_device::sound_pc9821ce_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_86_device(mconfig, SOUND_PC9821CE, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_io_config("pnp_io", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sound_pc9821ce_device::pnp_io_map), this))
	, m_eeprom(*this, "eeprom")
{
}

device_memory_interface::space_config_vector sound_pc9821ce_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_io_config)
	};
}

// TODO: blank it for now (needs to load from driver machine_config instead)
ROM_START( sound_pc9821ce )
	ROM_REGION( 0x4000, "sound_bios", ROMREGION_ERASEFF )
ROM_END

const tiny_rom_entry *sound_pc9821ce_device::device_rom_region() const
{
	return ROM_NAME( sound_pc9821ce );
}

void sound_pc9821ce_device::device_add_mconfig(machine_config &config)
{
	pc9801_86_config(config);
	EEPROM_93C06_16BIT(config, m_eeprom);

}


void sound_pc9821ce_device::device_start()
{
	pc9801_86_device::device_start();

	m_bus->install_device(0xac6c, 0xac6f, *this, &sound_pc9821ce_device::pnp_map);
}

void sound_pc9821ce_device::device_reset()
{
	pc9801_86_device::device_reset();
}

// expects writes with 0x54xx for index, 0x41xx for data
void sound_pc9821ce_device::pnp_map(address_map &map)
{
	map(0x00, 0x01).lrw16(
		NAME([this] (offs_t offset) {
			return 0x5400 | m_index;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (mem_mask != 0xffff || (data & 0xff00) != 0x5400)
				logerror("sound_pc9821ce: warning access index with %04x & %04x (blocked?)\n", data, mem_mask);
			m_index = data & 0xff;
		})
	);
	map(0x02, 0x03).lrw16(
		NAME([this] (offs_t offset) {
			return 0x4100 | this->space(0).read_byte(m_index);
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			if (mem_mask != 0xffff || (data & 0xff00) != 0x4100)
				logerror("sound_pc9821ce: warning access data with %04x & %04x (blocked?)\n", data, mem_mask);
			this->space(0).write_byte(m_index, data & 0xff);
		})
	);

}

void sound_pc9821ce_device::pnp_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			return m_eeprom->do_read() << 0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_eeprom->cs_write(BIT(data, 2));
			m_eeprom->clk_write(BIT(data, 1));
			m_eeprom->di_write(BIT(data, 0));
		})
	);
	// CPU flag (dip switch?)
	// -100 High
	// -010 Middle
	// -001 Low
	map(0x50, 0x50).lr8(NAME([] () { return 4; }));
}

