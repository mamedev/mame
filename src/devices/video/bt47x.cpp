// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
/*
 * An emulation of the Brooktree Bt47x family of RAMDAC devices.
 *
 *           Max.        Input          Output
 *   Part    Clock   Color  Overlay  Num.  Levels  Other
 *   Bt471   80MHz   8 bit   4 bit    3     6 bit
 *   Bt475   80MHz   8 bit   4 bit    3     6 bit  sleep mode, compatibility mode
 *   Bt476   80Mhz   8 bit            3     6 bit
 *   Bt477   80MHz   8 bit   4 bit    3     8 bit  sleep mode, compatibility mode
 *   Bt478   80MHz   8 bit   4 bit    3     8 bit
 *
 * Not emulated:
 *
 *   Bt473   80MHz  24 bit   4 bit    3     8 bit  true color/passthrough/bypass modes
 *   Bt474   85MHz 4x8 bit 4x4 bit    3     8 bit  sleep mode, vga mode
 *   Bt479   80MHz   8 bit   4 bit    3     8 bit  16 windows, compatibility mode
 *
 * Sources:
 *   - http://www.bitsavers.org/components/brooktree/_dataBooks/1991_Brooktree_Product_Databook.pdf
 *
 * TODO:
 *   - remaining devices
 */

#include "emu.h"
#include "bt47x.h"

#define LOG_GENERAL (1U << 0)
#define LOG_READS   (1U << 1)

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(BT471, bt471_device, "bt471", "Brooktree Bt471 256 Color RAMDAC")
//DEFINE_DEVICE_TYPE(BT473, bt473_device, "bt473", "Brooktree Bt473 True-Color RAMDAC")
//DEFINE_DEVICE_TYPE(BT474, bt474_device, "bt474", "Brooktree Bt474 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT475, bt475_device, "bt475", "Brooktree Bt475 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT476, bt476_device, "bt476", "Brooktree Bt476 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT477, bt477_device, "bt477", "Brooktree Bt477 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT478, bt478_device, "bt478", "Brooktree Bt478 256 Color RAMDAC")
//DEFINE_DEVICE_TYPE(BT479, bt479_device, "bt479", "Brooktree Bt479 1024 Color RAMDAC")

void bt47x_device_base::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt47x_device_base::address_r), FUNC(bt47x_device_base::address_w));
	map(0x01, 0x01).rw(FUNC(bt47x_device_base::palette_r), FUNC(bt47x_device_base::palette_w));
	map(0x02, 0x02).rw(FUNC(bt47x_device_base::mask_r),    FUNC(bt47x_device_base::mask_w));
	map(0x03, 0x03).rw(FUNC(bt47x_device_base::address_r), FUNC(bt47x_device_base::address_w));

	if (m_overlay_colors)
	{
		map(0x04, 0x04).rw(FUNC(bt47x_device_base::address_r), FUNC(bt47x_device_base::address_w));
		map(0x05, 0x05).rw(FUNC(bt47x_device_base::overlay_r), FUNC(bt47x_device_base::overlay_w));
		map(0x07, 0x07).rw(FUNC(bt47x_device_base::address_r), FUNC(bt47x_device_base::address_w));
	}
}

void bt475_device_base::map(address_map &map)
{
	bt47x_device_base::map(map);

	map(0x06, 0x06).rw(FUNC(bt475_device_base::command_r), FUNC(bt475_device_base::command_w));
}

bt47x_device_base::bt47x_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, unsigned const palette_colors, unsigned const overlay_colors, unsigned const color_bits)
	: device_t(mconfig, type, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_palette_colors(palette_colors)
	, m_overlay_colors(overlay_colors)
	, m_color_bits(color_bits)
{
}

bt475_device_base::bt475_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, unsigned const palette_colors, unsigned const overlay_colors, unsigned const color_bits)
	: bt47x_device_base(mconfig, type, tag, owner, clock, palette_colors, overlay_colors, color_bits)
{
}

bt471_device::bt471_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: bt47x_device_base(mconfig, BT471, tag, owner, clock, 256, 16, 6)
{
}

bt475_device::bt475_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: bt475_device_base(mconfig, BT475, tag, owner, clock, 256, 16, 6)
{
}

bt476_device::bt476_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: bt47x_device_base(mconfig, BT476, tag, owner, clock, 256, 0, 6)
{
}

bt477_device::bt477_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: bt475_device_base(mconfig, BT477, tag, owner, clock, 256, 16, 8)
{
}

bt478_device::bt478_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: bt47x_device_base(mconfig, BT478, tag, owner, clock, 256, 16, 8)
{
}

void bt47x_device_base::device_start()
{
	save_item(NAME(m_address));
	save_item(NAME(m_address_rgb));
	save_item(NAME(m_read_mask));

	m_color_ram = std::make_unique<std::array<u8, 3>[]>(m_palette_colors + m_overlay_colors);

	save_pointer(NAME(m_color_ram), m_palette_colors + m_overlay_colors);
}

void bt475_device_base::device_start()
{
	bt47x_device_base::device_start();

	save_item(NAME(m_command));
}

u8 bt47x_device_base::address_r()
{
	LOGMASKED(LOG_READS, "address_r 0x%02x\n", m_address);

	if (!machine().side_effects_disabled())
		m_address_rgb = 0;

	return m_address;
}

void bt47x_device_base::address_w(u8 data)
{
	LOG("address_w 0x%02x\n", data);
	m_address_rgb = 0;

	m_address = data;
}

void bt47x_device_base::increment_address(bool const side_effects)
{
	if (!machine().side_effects_disabled() || side_effects)
	{
		// increment component index and address register
		m_address_rgb = (m_address_rgb + 1) % 3;
		if (m_address_rgb == 0)
			m_address++;
	}
}

u8 bt47x_device_base::palette_r()
{
	u8 const data = m_color_ram[m_address][m_address_rgb];

	increment_address();

	LOGMASKED(LOG_READS, "palette_r 0x%02x\n", data);

	return data;
}

void bt47x_device_base::palette_w(u8 data)
{
	LOG("palette_w 0x%02x\n", data);

	m_color_ram[m_address][m_address_rgb] = data;

	// update the mame palette to match the device
	if (m_address_rgb == 2)
		set_pen_color(m_address, rgb_t(
			m_color_ram[m_address][0] << (8 - color_bits()),
			m_color_ram[m_address][1] << (8 - color_bits()),
			m_color_ram[m_address][2] << (8 - color_bits())));

	increment_address(true);
}

u8 bt47x_device_base::mask_r()
{
	LOGMASKED(LOG_READS, "mask_r 0x%02x\n", m_read_mask);

	return m_read_mask;
}

void bt47x_device_base::mask_w(u8 data)
{
	LOG("mask_w 0x%02x\n", data);

	m_read_mask = data;
}

u8 bt47x_device_base::overlay_r()
{
	unsigned const index = m_palette_colors + (m_address & (m_overlay_colors - 1));
	u8 const data = m_color_ram[index][m_address_rgb];

	increment_address();

	LOGMASKED(LOG_READS, "overlay_r 0x%02x\n", data);

	return data;
}

void bt47x_device_base::overlay_w(u8 data)
{
	LOG("overlay_w 0x%02x\n", data);

	unsigned const index = m_palette_colors + (m_address & (m_overlay_colors - 1));
	m_color_ram[index][m_address_rgb] = data;

	// update the mame palette to match the device
	if (m_address_rgb == 2)
		set_pen_color(index, rgb_t(
			m_color_ram[index][0] << (8 - color_bits()),
			m_color_ram[index][1] << (8 - color_bits()),
			m_color_ram[index][2] << (8 - color_bits())));

	increment_address(true);
}

u8 bt475_device_base::command_r()
{
	LOGMASKED(LOG_READS, "command_r 0x%02x\n", m_command);

	return m_command;
}

void bt475_device_base::command_w(u8 data)
{
	LOG("command_w 0x%02x\n", data);

	m_command = data;
}
