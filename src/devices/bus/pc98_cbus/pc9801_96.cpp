// license:BSD-3-Clause
// copyright-holders: BlueRain
#include "emu.h"
#include "pc9801_96.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(PC9801_96, pc9801_96_device, "pc9801_96", "NEC PC-9801-96 Window Accelerator Board")

pc9801_96_device::pc9801_96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_96, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_vga(*this, "vga")
	, m_wab_space_config("wab_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(pc9801_96_device::wab_map), this))
	, m_reg_index(0)
	, m_vram_window_addr(0xf00000)
	, m_prev_vram_window_addr(0)
	, m_linear_vram_addr(0)
	, m_relay_ctrl(0)
	, m_video_enable(0)
{
	memset(m_reg_data, 0, sizeof(m_reg_data));
}

device_memory_interface::space_config_vector pc9801_96_device::memory_space_config() const
{
	return space_config_vector {
			std::make_pair(0, &m_wab_space_config) };
}

void pc9801_96_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(FUNC(pc9801_96_device::screen_update));

	CIRRUS_GD5428_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(256 * 1024 * 4); // 1MB VRAM
}

void pc9801_96_device::device_start()
{
	save_item(NAME(m_reg_index));
	save_item(NAME(m_reg_data));
	save_item(NAME(m_vram_window_addr));
	save_item(NAME(m_prev_vram_window_addr));
	save_item(NAME(m_linear_vram_addr));
	save_item(NAME(m_relay_ctrl));
	save_item(NAME(m_video_enable));
}

void pc9801_96_device::device_reset()
{
	m_reg_index = 0;
	m_vram_window_addr = 0xf00000;
	m_prev_vram_window_addr = 0;
	m_linear_vram_addr = 0;
	m_relay_ctrl = 0;
	m_video_enable = 0;

	m_bus->remap(AS_PROGRAM, 0, 0xffffff);
}

void pc9801_96_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &pc9801_96_device::io_map);
	}
	else if (space_id == AS_PROGRAM)
	{
		address_space &program_space = m_bus->space(AS_PROGRAM);

		if (m_prev_vram_window_addr != 0)
		{
			program_space.unmap_readwrite(m_prev_vram_window_addr, m_prev_vram_window_addr + 0xffff);
			m_prev_vram_window_addr = 0;
		}

		if (m_vram_window_addr != 0)
		{
			program_space.install_readwrite_handler(
					m_vram_window_addr, m_vram_window_addr + 0xffff,
					read8sm_delegate(*this, FUNC(pc9801_96_device::vram_r)),
					write8sm_delegate(*this, FUNC(pc9801_96_device::vram_w)));
			m_prev_vram_window_addr = m_vram_window_addr;
		}
	}
}

uint8_t pc9801_96_device::vram_r(offs_t offset)
{
	return m_vga->mem_r(offset & 0xffff);
}

void pc9801_96_device::vram_w(offs_t offset, uint8_t data)
{
	m_vga->mem_w(offset & 0xffff, data);
}

void pc9801_96_device::io_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(cirrus_gd5428_vga_device::io_map));

	map(0x0902, 0x0902).rw(FUNC(pc9801_96_device::video_enable_r), FUNC(pc9801_96_device::video_enable_w));

	map(0xfa2, 0xfa2).mirror(8).rw(FUNC(pc9801_96_device::reg_index_r), FUNC(pc9801_96_device::reg_index_w));
	map(0xfa3, 0xfa3).mirror(8).rw(FUNC(pc9801_96_device::reg_data_r), FUNC(pc9801_96_device::reg_data_w));

	map(0x0c50, 0x0c5f).rw(FUNC(pc9801_96_device::vga_read_3c0), FUNC(pc9801_96_device::vga_write_3c0));
	map(0x0b54, 0x0b55).rw(FUNC(pc9801_96_device::vga_read_3b4), FUNC(pc9801_96_device::vga_write_3b4));
	map(0x0b5a, 0x0b5a).rw(FUNC(pc9801_96_device::vga_read_3ba), FUNC(pc9801_96_device::vga_write_3ba));
	map(0x0d54, 0x0d55).rw(FUNC(pc9801_96_device::vga_read_3d4), FUNC(pc9801_96_device::vga_write_3d4));
	map(0x0d5a, 0x0d5a).rw(FUNC(pc9801_96_device::vga_read_3da), FUNC(pc9801_96_device::vga_write_3da));
}

uint8_t pc9801_96_device::vga_read_3c0(offs_t offset)
{
	return m_bus->space(AS_IO).read_byte(0x03c0 + offset);
}

void pc9801_96_device::vga_write_3c0(offs_t offset, uint8_t data)
{
	m_bus->space(AS_IO).write_byte(0x03c0 + offset, data);
}

uint8_t pc9801_96_device::vga_read_3b4(offs_t offset)
{
	return m_bus->space(AS_IO).read_byte(0x03b4 + offset);
}

void pc9801_96_device::vga_write_3b4(offs_t offset, uint8_t data)
{
	m_bus->space(AS_IO).write_byte(0x03b4 + offset, data);
}

uint8_t pc9801_96_device::vga_read_3ba()
{
	return m_bus->space(AS_IO).read_byte(0x03ba);
}

void pc9801_96_device::vga_write_3ba(uint8_t data)
{
	m_bus->space(AS_IO).write_byte(0x03ba, data);
}

uint8_t pc9801_96_device::vga_read_3d4(offs_t offset)
{
	return m_bus->space(AS_IO).read_byte(0x03d4 + offset);
}

void pc9801_96_device::vga_write_3d4(offs_t offset, uint8_t data)
{
	m_bus->space(AS_IO).write_byte(0x03d4 + offset, data);
}

uint8_t pc9801_96_device::vga_read_3da()
{
	return m_bus->space(AS_IO).read_byte(0x03da);
}

void pc9801_96_device::vga_write_3da(uint8_t data)
{
	m_bus->space(AS_IO).write_byte(0x03da, data);
}

uint8_t pc9801_96_device::video_enable_r()
{
	return m_video_enable;
}

void pc9801_96_device::video_enable_w(uint8_t data)
{
	m_video_enable = data & 1;
}

uint8_t pc9801_96_device::reg_index_r()
{
	return m_reg_index;
}

void pc9801_96_device::reg_index_w(uint8_t data)
{
	m_reg_index = data;
}

uint8_t pc9801_96_device::reg_data_r()
{
	return space(0).read_byte(m_reg_index);
}

void pc9801_96_device::reg_data_w(uint8_t data)
{
	space(0).write_byte(m_reg_index, data);
}

void pc9801_96_device::wab_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(pc9801_96_device::board_id_r));
	map(0x01, 0x01).rw(FUNC(pc9801_96_device::window_r), FUNC(pc9801_96_device::window_w));
	map(0x02, 0x02).rw(FUNC(pc9801_96_device::linear_addr_r), FUNC(pc9801_96_device::linear_addr_w));
	map(0x03, 0x03).rw(FUNC(pc9801_96_device::relay_r), FUNC(pc9801_96_device::relay_w));
	map(0x04, 0x04).r(FUNC(pc9801_96_device::window_index_r));
}

uint8_t pc9801_96_device::board_id_r() { return 0x60; } // PC-9801-96 (PC-9801B3-E02)

uint8_t pc9801_96_device::window_r()
{
	if (m_vram_window_addr == 0x0b0000)
		return 0x10;
	if (m_vram_window_addr == 0xf20000)
		return 0x80;
	if (m_vram_window_addr == 0xf00000)
		return 0xa0;
	if (m_vram_window_addr == 0xf40000)
		return 0xe0;
	if (m_vram_window_addr == 0xf60000)
		return 0xc0;
	return 0x00;
}

void pc9801_96_device::window_w(uint8_t data)
{
	switch (data)
	{
		case 0x10:
			m_vram_window_addr = 0x0b0000;
			break;
		case 0x80:
			m_vram_window_addr = 0xf20000;
			break;
		case 0xa0:
			m_vram_window_addr = 0xf00000;
			break;
		case 0xc0:
			m_vram_window_addr = 0xf60000;
			break;
		case 0xe0:
			m_vram_window_addr = 0xf40000;
			break;
		default:
			m_vram_window_addr = 0;
			break;
	}
	m_bus->remap(AS_PROGRAM, 0, 0xffffff);
}

uint8_t pc9801_96_device::linear_addr_r()
{
	return (m_linear_vram_addr >> 24) & 0xff;
}

void pc9801_96_device::linear_addr_w(uint8_t data)
{
	m_linear_vram_addr = (data << 24);
	m_bus->remap(AS_PROGRAM, 0, 0xffffff);
}

uint8_t pc9801_96_device::relay_r()
{
	return m_relay_ctrl | m_video_enable;
}

void pc9801_96_device::relay_w(uint8_t data)
{
	m_relay_ctrl = data & ~0x1;
}

uint8_t pc9801_96_device::window_index_r()
{
	if (m_vram_window_addr == 0xf00000) return 0x00;
	if (m_vram_window_addr == 0xf20000) return 0x01;
	if (m_vram_window_addr == 0xf40000) return 0x02;
	if (m_vram_window_addr == 0xf60000) return 0x03;
	return 0x00;
}

uint32_t pc9801_96_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_video_enable)
	{
		return m_vga->screen_update(screen, bitmap, cliprect);
	}
	return 0;
}
