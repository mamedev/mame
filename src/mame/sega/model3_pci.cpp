// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Sega Model 3 Real3D PCI frontend
    Emulation by R. Belmont

    TODO: verify BAR decoding, status timing and the 0x9c buffer registers.
    DMA burst and query timings are guesses.
 */

#include "emu.h"
#include "model3_pci.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(MODEL3_REAL3D_PCI, model3_real3d_pci_device, "model3_real3d_pci", "Model 3 Real3D PCI Memory Interface")
DEFINE_DEVICE_TYPE(MODEL3_REAL3D_DMA_PCI, model3_real3d_dma_pci_device, "model3_real3d_dma_pci", "Model 3 Step 2 Real3D DMA PCI Interface")

model3_real3d_pci_device::model3_real3d_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pci_device(mconfig, MODEL3_REAL3D_PCI, tag, owner, clock)
	, m_step(step::STEP_1_5)
	, m_reset_command(0)
	, m_reset_base(0)
	, m_texture_callback(*this)
	, m_vrom_callback(*this)
	, m_submit_callback(*this)
	, m_status_callback(*this, 0xffffffff)
	, m_display_list_ram(*this, "display_list", DISPLAY_WORDS * 4, ENDIANNESS_LITTLE)
	, m_culling_ram(*this, "culling", CULLING_WORDS * 4, ENDIANNESS_LITTLE)
	, m_polygon_ram(*this, "polygon", POLYGON_WORDS * 4, ENDIANNESS_LITTLE)
	, m_texture_fifo(*this, "texture_fifo", FIFO_WORDS * 4, ENDIANNESS_LITTLE)
	, m_fifo_count(0)
	, m_vrom_words{}
	, m_vrom_count(0)
	, m_fifo_overflow(false)
	, m_buffer_config{}
	, m_graphics_base(0)
{
	set_ids(0x11db16c3, 0, 0, 0);
}

void model3_real3d_pci_device::device_start()
{
	pci_device::device_start();
	command_mask = 0x0042; // observed command bits
	m_texture_callback.resolve_safe();
	m_vrom_callback.resolve_safe();
	m_submit_callback.resolve_safe();

	save_item(NAME(m_fifo_count));
	save_item(NAME(m_vrom_words));
	save_item(NAME(m_vrom_count));
	save_item(NAME(m_fifo_overflow));
	save_item(NAME(m_buffer_config));
	save_item(NAME(m_graphics_base));
}

void model3_real3d_pci_device::device_reset()
{
	pci_device::device_reset();
	command = m_reset_command & command_mask;
	status = 0;
	m_fifo_count = 0;
	m_vrom_count = 0;
	m_vrom_words[0] = m_vrom_words[1] = 0;
	m_fifo_overflow = false;
	std::fill(std::begin(m_buffer_config), std::end(m_buffer_config), 0);
	m_graphics_base = m_reset_base;
	remap_cb();
}

void model3_real3d_pci_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
	u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	if (!(command & 2))
	{
		return;
	}

	// Keep the holes unmapped and stay within the upstream bridge window.
	auto install = [this, memory_space, memory_window_start, memory_window_end, memory_offset]
		(u32 start, u32 end, void (model3_real3d_pci_device::*map)(address_map &), const char *name)
	{
		if (start >= memory_window_start && end <= memory_window_end)
		{
			address_map_constructor delegate(map, name, this);
			memory_space->install_device_delegate(start + memory_offset, end + memory_offset, *this, delegate);
		}
	};
	install(0x84000000, 0x8400003f, FUNC(model3_real3d_pci_device::status_map));
	install(0x88000000, 0x88000007, FUNC(model3_real3d_pci_device::command_map));
	install(0x8c000000, 0x8c3fffff, FUNC(model3_real3d_pci_device::culling_map));
	install(0x8e000000, 0x8e0fffff, FUNC(model3_real3d_pci_device::display_list_map));
	install(0x90000000, 0x90ffffff, FUNC(model3_real3d_pci_device::vrom_map));
	install(0x94000000, 0x94ffffff, FUNC(model3_real3d_pci_device::fifo_map));
	install(0x98000000, 0x983fffff, FUNC(model3_real3d_pci_device::polygon_map));
	install(0x9c000000, 0x9cffffff, FUNC(model3_real3d_pci_device::buffer_config_map));
}

void model3_real3d_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x10, 0x13).rw(FUNC(model3_real3d_pci_device::graphics_base_r), FUNC(model3_real3d_pci_device::graphics_base_w));
}

u32 model3_real3d_pci_device::graphics_base_r()
{
	return m_graphics_base == 0xffffffff ? 0 : m_graphics_base;
}

void model3_real3d_pci_device::graphics_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_graphics_base);
}

void model3_real3d_pci_device::status_map(address_map &map)
{
	map(0x00, 0x3f).r(FUNC(model3_real3d_pci_device::status_r));
}

void model3_real3d_pci_device::command_map(address_map &map)
{
	map(0x00, 0x03).w(FUNC(model3_real3d_pci_device::command_w));
	map(0x04, 0x07).nopw(); // an 8-byte command write submits once
}

void model3_real3d_pci_device::culling_map(address_map &map)
{
	map(0x000000, 0x3fffff).writeonly().share(m_culling_ram);
}

void model3_real3d_pci_device::display_list_map(address_map &map)
{
	map(0x000000, 0x0fffff).writeonly().share(m_display_list_ram);
}

void model3_real3d_pci_device::polygon_map(address_map &map)
{
	map(0x000000, 0x3fffff).writeonly().share(m_polygon_ram);
}

void model3_real3d_pci_device::vrom_map(address_map &map)
{
	map(0x000000, 0xffffff).w(FUNC(model3_real3d_pci_device::vrom_w));
}

void model3_real3d_pci_device::fifo_map(address_map &map)
{
	map(0x000000, 0xffffff).w(FUNC(model3_real3d_pci_device::fifo_w));
}

void model3_real3d_pci_device::buffer_config_map(address_map &map)
{
	map(0x000000, 0x00000f).mirror(0xfffff0).w(FUNC(model3_real3d_pci_device::buffer_config_w));
}

void model3_real3d_pci_device::buffer_config_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_buffer_config[offset]);
}

u32 model3_real3d_pci_device::status_r(offs_t offset, u32 mem_mask)
{
	return m_status_callback(offset, mem_mask);
}

void model3_real3d_pci_device::vrom_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask != 0xffffffff)
	{
		if (mem_mask)
		{
			logerror("Unsupported partial Real3D VROM port write: %08x & %08x\n", data, mem_mask);
		}
		return;
	}
	if (m_step == step::STEP_1_0)
	{
		enqueue_vrom(data & 0x00ffffff);
		return;
	}
	if (m_vrom_count < 2)
	{
		m_vrom_words[m_vrom_count++] = data;
	}
	else
	{
		m_vrom_count = 0;
		// test the raw address before masking, as Supermodel does
		const auto source = m_vrom_words[0] < 0x100000 ? polygon_ram() : m_vrom;
		const u32 address = m_vrom_words[0] & 0x00ffffff;
		if (address < source.size())
		{
			m_vrom_callback(m_vrom_words[0], m_vrom_words[1], data, source.subspan(address));
		}
		else
		{
			logerror("Real3D VROM source address out of bounds: %08x\n", m_vrom_words[0]);
		}
	}
}

void model3_real3d_pci_device::enqueue_vrom(u32 address)
{
	if (m_fifo_overflow)
	{
		return;
	}
	if (address >= m_vrom.size())
	{
		logerror("Real3D VROM record address out of bounds: %08x\n", address);
		return;
	}
	const u32 words = record_words(m_vrom[address]);
	if (words < 2 || words > m_vrom.size() - address)
	{
		logerror("Invalid Real3D VROM record at %08x (%u words)\n", address, words);
		return;
	}
	if (words > FIFO_WORDS - m_fifo_count)
	{
		m_fifo_overflow = true;
		logerror("Real3D texture FIFO overflow copying VROM record\n");
		return;
	}
	std::copy_n(m_vrom.data() + address, words, m_texture_fifo.target() + m_fifo_count);
	m_fifo_count += words;
}

void model3_real3d_pci_device::fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask != 0xffffffff)
	{
		if (mem_mask)
		{
			logerror("Unsupported partial Real3D texture FIFO write: %08x & %08x\n", data, mem_mask);
		}
		return;
	}
	if (m_fifo_overflow)
	{
		return;
	}
	if (m_fifo_count < FIFO_WORDS)
	{
		m_texture_fifo[m_fifo_count++] = data;
	}
	else
	{
		m_fifo_overflow = true;
		logerror("Real3D texture FIFO overflow\n");
	}
}

void model3_real3d_pci_device::flush_textures()
{
	u32 position = 0;
	while (m_fifo_count - position >= 2)
	{
		const u32 length = m_texture_fifo[position];
		const u32 words = record_words(length);
		const u32 available = m_fifo_count - position;
		if (words < 2 || words > FIFO_WORDS || words > available + 1)
		{
			break;
		}
		// the final record may be one word short (srally2)
		const u32 consumed = std::min(words, available);
		m_texture_callback(m_texture_fifo[position + 1], length, { &m_texture_fifo[position + 2], consumed - 2 });
		position += consumed;
	}
	if (position != m_fifo_count)
	{
		logerror("Discarding incomplete Real3D texture record (%u words)\n", m_fifo_count - position);
	}
	m_fifo_count = 0;
	m_fifo_overflow = false;
}

void model3_real3d_pci_device::command_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask)
	{
		flush_textures();
		m_submit_callback();
	}
}


model3_real3d_dma_pci_device::model3_real3d_dma_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pci_card_device(mconfig, MODEL3_REAL3D_DMA_PCI, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_config_space("graphics_config", ENDIANNESS_LITTLE, 32, 8, 0, address_map_constructor(FUNC(model3_real3d_dma_pci_device::downstream_config_map), this))
	, m_status_space("graphics_status", ENDIANNESS_LITTLE, 32, 6, 0, address_map_constructor(FUNC(model3_real3d_dma_pci_device::downstream_status_map), this))
	, m_graphics(*this, "graphics")
	, m_status_query_callback(*this, 0xffffffff)
	, m_dma_timer(nullptr)
	, m_query_timer(nullptr)
	, m_dma_base(0)
	, m_selector(0)
	, m_cache_latency(0)
	, m_dma_source(0)
	, m_dma_destination(0)
	, m_dma_words(0)
	, m_status_result(0)
	, m_dma_control(0)
	, m_transfer_control(0)
	, m_dma_active(false)
	, m_query_busy(false)
	, m_irq_pending(false)
{
	set_ids(0x11db1786, 0, 0, 0);
	set_irq_map(0);
}

void model3_real3d_dma_pci_device::device_add_mconfig(machine_config &config)
{
	MODEL3_REAL3D_PCI(config, m_graphics);
	m_graphics->set_step(model3_real3d_pci_device::step::STEP_2_0);
	m_graphics->set_reset_command(0x0042);
	m_graphics->set_reset_base(0x8000'0000);
}

device_memory_interface::space_config_vector model3_real3d_dma_pci_device::memory_space_config() const
{
	return { { AS_PROGRAM, &m_config_space }, { AS_IO, &m_status_space } };
}

void model3_real3d_dma_pci_device::device_start()
{
	pci_card_device::device_start();
	command_mask = 0x014e; // observed command bits
	intr_pin = 1;
	m_dma_timer = timer_alloc(FUNC(model3_real3d_dma_pci_device::dma_tick), this);
	m_query_timer = timer_alloc(FUNC(model3_real3d_dma_pci_device::query_done), this);
	save_item(NAME(m_dma_base));
	save_item(NAME(m_selector));
	save_item(NAME(m_cache_latency));
	save_item(NAME(m_dma_source));
	save_item(NAME(m_dma_destination));
	save_item(NAME(m_dma_words));
	save_item(NAME(m_status_result));
	save_item(NAME(m_dma_control));
	save_item(NAME(m_transfer_control));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_query_busy));
	save_item(NAME(m_irq_pending));
}

void model3_real3d_dma_pci_device::device_reset()
{
	pci_card_device::device_reset();
	command = 0;
	status = 0;
	m_dma_base = 0;
	m_selector = 0;
	m_cache_latency = 0;
	m_dma_source = m_dma_destination = m_dma_words = 0;
	m_status_result = 0;
	m_dma_control = m_transfer_control = 0;
	m_dma_active = m_query_busy = m_irq_pending = false;
	m_dma_timer->adjust(attotime::never);
	m_query_timer->adjust(attotime::never);
	update_irq();
	remap_cb();
}

void model3_real3d_dma_pci_device::set_remap_cb(mapper_cb callback)
{
	pci_device::set_remap_cb(callback);
	m_graphics->set_remap_cb(callback);
}

void model3_real3d_dma_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x04, 0x05).rw(FUNC(model3_real3d_dma_pci_device::command_r), FUNC(model3_real3d_dma_pci_device::command_w));
	map(0x0c, 0x0d).rw(FUNC(model3_real3d_dma_pci_device::cache_latency_r), FUNC(model3_real3d_dma_pci_device::cache_latency_w));
	map(0x10, 0x13).rw(FUNC(model3_real3d_dma_pci_device::dma_base_r), FUNC(model3_real3d_dma_pci_device::dma_base_w));
	map(0x3d, 0x3d).lw8(NAME([] (u8 data) { })); // INTA is fixed
}

void model3_real3d_dma_pci_device::command_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old = command;
	pci_card_device::command_w(offset, data, mem_mask);
	if (m_dma_active && ((old ^ command) & 4))
	{
		m_dma_timer->adjust((command & 4) ? attotime::from_usec(10) : attotime::never);
	}
}

u32 model3_real3d_dma_pci_device::dma_base_r()
{
	return m_dma_base == 0xffffffff ? 0 : m_dma_base;
}

void model3_real3d_dma_pci_device::dma_base_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_base);
}

void model3_real3d_dma_pci_device::downstream_config_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0xff).m(m_graphics, FUNC(model3_real3d_pci_device::config_map));
}

void model3_real3d_dma_pci_device::config_port_map(address_map &map)
{
	map(0x00, 0x07).rw(FUNC(model3_real3d_dma_pci_device::parameter_r), FUNC(model3_real3d_dma_pci_device::parameter_w));
	map(0x08, 0x0b).rw(FUNC(model3_real3d_dma_pci_device::length_r), FUNC(model3_real3d_dma_pci_device::length_w));
	map(0x0c, 0x0c).r(FUNC(model3_real3d_dma_pci_device::dma_status_r));
	map(0x0d, 0x0d).w(FUNC(model3_real3d_dma_pci_device::acknowledge_w));
	map(0x0e, 0x0e).rw(FUNC(model3_real3d_dma_pci_device::control_r), FUNC(model3_real3d_dma_pci_device::control_w));
	map(0x10, 0x13).rw(FUNC(model3_real3d_dma_pci_device::selector_r), FUNC(model3_real3d_dma_pci_device::selector_w));
	map(0x14, 0x17).rw(FUNC(model3_real3d_dma_pci_device::data_r), FUNC(model3_real3d_dma_pci_device::data_w));
}

void model3_real3d_dma_pci_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
	u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	// the graphics function has its own command register
	m_graphics->map_device(memory_window_start, memory_window_end, memory_offset, memory_space,
		io_window_start, io_window_end, io_offset, io_space);
	if ((command & 2) && memory_window_start <= 0xc2000000 && memory_window_end >= 0xc200001f)
	{
		memory_space->install_device(0xc2000000 + memory_offset, 0xc200001f + memory_offset, *this, &model3_real3d_dma_pci_device::config_port_map);
	}
}

void model3_real3d_dma_pci_device::downstream_status_map(address_map &map)
{
	map(0x00, 0x3f).m(m_graphics, FUNC(model3_real3d_pci_device::status_map));
}

void model3_real3d_dma_pci_device::selector_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_selector);
	m_query_busy = config_selected() || status_selected();
	if (status_selected())
	{
		const offs_t reg = m_selector & 0x3c;
		m_status_result = m_status_query_callback.isunset() ? space(AS_IO).read_dword(reg) : m_status_query_callback(reg >> 2);
	}
	m_query_timer->adjust(m_query_busy ? attotime::from_usec(50) : attotime::never);
}

TIMER_CALLBACK_MEMBER(model3_real3d_dma_pci_device::query_done)
{
	m_query_busy = false;
}

u32 model3_real3d_dma_pci_device::data_r(offs_t offset, u32 mem_mask)
{
	// status queries return the result latched by selector_w
	if (config_selected())
	{
		return space(AS_PROGRAM).read_dword(m_selector & 0xfc, mem_mask);
	}
	return status_selected() ? m_status_result : 0xffffffff;
}

void model3_real3d_dma_pci_device::data_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (config_selected())
	{
		space(AS_PROGRAM).write_dword(m_selector & 0xfc, data, mem_mask);
	}
}

u32 model3_real3d_dma_pci_device::parameter_r(offs_t offset)
{
	return offset ? m_dma_destination : m_dma_source;
}

void model3_real3d_dma_pci_device::parameter_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_dma_active)
	{
		logerror("Ignoring Step 2 DMA address write while active\n");
		return;
	}
	if (offset)
	{
		COMBINE_DATA(&m_dma_destination);
	}
	else
	{
		COMBINE_DATA(&m_dma_source);
	}
}

void model3_real3d_dma_pci_device::length_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask != 0xffffffff || m_dma_active || ((m_dma_source | m_dma_destination) & 3))
	{
		logerror("Unsupported Step 2 DMA launch: %08x & %08x, source %08x destination %08x active %u\n",
			data, mem_mask, m_dma_source, m_dma_destination, m_dma_active);
		return;
	}
	m_dma_words = data;
	m_transfer_control = m_dma_control;
	m_dma_active = true;
	m_dma_timer->adjust((command & 4) ? attotime::from_usec(10) : attotime::never);
}

TIMER_CALLBACK_MEMBER(model3_real3d_dma_pci_device::dma_tick)
{
	if (!m_dma_active || !(command & 4))
	{
		return;
	}
	address_space &memory = *get_pci_busmaster_space();
	// commit progress before the write, which may restart the transfer
	for (u32 count = 0; count != 256 && m_dma_active && (command & 4) && m_dma_words; ++count)
	{
		u32 data = memory.read_dword(m_dma_source);
		if (BIT(m_transfer_control, 7))
		{
			data = swapendian_int32(data);
		}
		const u32 destination = m_dma_destination;
		m_dma_source += 4;
		m_dma_destination += 4;
		--m_dma_words;
		memory.write_dword(destination, data);
	}
	if (!m_dma_active)
	{
		return;
	}
	if (!m_dma_words)
	{
		m_dma_active = false;
		if (BIT(m_transfer_control, 0))
		{
			m_irq_pending = true;
		}
		update_irq();
	}
	else if (command & 4)
	{
		m_dma_timer->adjust(attotime::from_usec(10));
	}
}

void model3_real3d_dma_pci_device::acknowledge_w(u8 data)
{
	if (BIT(data, 0))
	{
		m_irq_pending = false;
		update_irq();
	}
}

void model3_real3d_dma_pci_device::update_irq()
{
	irq_pin_w(0, m_irq_pending);
}


DEFINE_DEVICE_TYPE(MODEL3_ROM_PCI, model3_rom_pci_device, "model3_rom_pci", "Model 3 PCI ROM Board")

model3_rom_pci_device::model3_rom_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pci_device(mconfig, MODEL3_ROM_PCI, tag, owner, clock)
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_bank_callback(*this)
	, m_bank(0)
{
	set_ids(0x11db1827, 0, 0, 0);
}

void model3_rom_pci_device::device_start()
{
	pci_device::device_start();
	// 8 MiB ROM plus the bank latch; the real aperture size is unknown
	add_map(0x1000000, M_MEM, FUNC(model3_rom_pci_device::memory_map));
	command_mask = 2;
	save_item(NAME(m_bank));
}

void model3_rom_pci_device::device_reset()
{
	pci_device::device_reset();
	command = 0;
	set_map_address(0, 0);
	m_bank = 0;
	m_bank_callback(m_bank);
	remap_cb();
}

void model3_rom_pci_device::memory_map(address_map &map)
{
	map(0x000000, 0x7fffff).r(FUNC(model3_rom_pci_device::rom_r));
	map(0x800000, 0x800000).w(FUNC(model3_rom_pci_device::bank_w));
}

u32 model3_rom_pci_device::rom_r(offs_t offset)
{
	const u32 word = (0x800000 / 8) * (1 + m_bank) + offset / 2;
	if (word >= m_rom.length())
	{
		return 0xffffffff;
	}
	const u64 data = m_rom[word];
	return swapendian_int32(BIT(offset, 0) ? u32(data) : u32(data >> 32));
}

void model3_rom_pci_device::bank_w(u8 data)
{
	m_bank = ~data & 15;
	m_bank_callback(m_bank);
}
