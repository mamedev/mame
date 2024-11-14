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
 *   Bt479   80MHz   8 bit   4 bit    3     8 bit  1024 palette entries, 16 windows, compatibility mode
 *
 * Not emulated:
 *
 *   Bt473   80MHz  24 bit   4 bit    3     8 bit  true color/passthrough/bypass modes
 *   Bt474   85MHz 4x8 bit 4x4 bit    3     8 bit  sleep mode, vga mode
 *
 * Sources:
 *   - Brooktree Product Databook 1991
 *
 * TODO:
 *   - remaining devices
 *   - bt479 window mode, overlay mask, 10-bit modes
 */

#include "emu.h"
#include "bt47x.h"

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
DEFINE_DEVICE_TYPE(BT479, bt479_device, "bt479", "Brooktree Bt479 1024 Color RAMDAC")

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

u8 bt47x_device_base::read(offs_t offset)
{
	u8 data = 0;
	switch (offset)
	{
	case 0: data = address_r(); break;
	case 1: data = palette_r(); break;
	case 2: data = mask_r(); break;
	case 3: data = address_r(); break;
	case 4: if (m_overlay_colors) data = address_r(); break;
	case 5: if (m_overlay_colors) data = overlay_r(); break;
	case 7: if (m_overlay_colors) data = address_r(); break;
	}

	return data;
}

void bt47x_device_base::write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: address_w(data); break;
	case 1: palette_w(data); break;
	case 2: mask_w(data); break;
	case 3: address_w(data); break;
	case 4: if (m_overlay_colors) address_w(data); break;
	case 5: if (m_overlay_colors) overlay_w(data); break;
	case 7: if (m_overlay_colors) address_w(data); break;
	}
}

void bt475_device_base::map(address_map &map)
{
	bt47x_device_base::map(map);

	map(0x06, 0x06).rw(FUNC(bt475_device_base::command_r), FUNC(bt475_device_base::command_w));
}

u8 bt475_device_base::read(offs_t offset)
{
	if (offset == 6)
		return command_r();
	else
		return bt47x_device_base::read(offset);
}

void bt475_device_base::write(offs_t offset, u8 data)
{
	if (offset == 6)
		command_w(data);
	else
		bt47x_device_base::write(offset, data);
}

void bt479_device::map(address_map &map)
{
	bt47x_device_base::map(map);

	map(0x06, 0x06).rw(FUNC(bt479_device::control_r), FUNC(bt479_device::control_w));
}

u8 bt479_device::read(offs_t offset)
{
	if (offset == 6)
		return control_r();
	else
		return bt47x_device_base::read(offset);
}

void bt479_device::write(offs_t offset, u8 data)
{
	if (offset == 6)
		control_w(data);
	else
		bt47x_device_base::write(offset, data);
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

bt479_device::bt479_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: bt47x_device_base(mconfig, BT479, tag, owner, clock, 1024, 16, 8)
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

void bt479_device::device_start()
{
	bt47x_device_base::device_start();

	save_item(NAME(m_window));
	save_item(NAME(m_command));
	save_item(NAME(m_flood));

	m_command[0] = 0;
	m_command[1] = 0;
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

void bt47x_device_base::increment_address(bool const rgb, bool const side_effects)
{
	if (!machine().side_effects_disabled() || side_effects)
	{
		if (rgb)
		{
			// increment component index and address register
			m_address_rgb = (m_address_rgb + 1) % 3;
			if (m_address_rgb == 0)
				m_address++;
		}
		else
			m_address++;
	}
}

u8 bt47x_device_base::palette_r()
{
	u8 const data = m_color_ram[address()][m_address_rgb];

	increment_address(true);

	LOGMASKED(LOG_READS, "palette_r 0x%02x\n", data);

	return data;
}

void bt47x_device_base::palette_w(u8 data)
{
	LOG("palette_w 0x%02x\n", data);

	m_color_ram[address()][m_address_rgb] = data;

	// update the mame palette to match the device
	if (m_address_rgb == 2)
		set_pen_color(address(), rgb_t(
			m_color_ram[address()][0] << (8 - color_bits()),
			m_color_ram[address()][1] << (8 - color_bits()),
			m_color_ram[address()][2] << (8 - color_bits())));

	increment_address(true, true);
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

	increment_address(true);

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

	increment_address(true, true);
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

enum bt479_command_0_mask : u8
{
	BT479_CR0_OL = 0x0f, // overlay read mask
	BT479_CR0_CP = 0xf0, // color palette select
};

u8 bt479_device::control_r()
{
	unsigned const address = bt47x_device_base::address();
	u8 data = 0;

	switch (address)
	{
	case 0x82:
	case 0x83:
		data = m_command[address & 1];
		break;
	case 0x84:
	case 0x85:
		data = m_flood[address & 1];
		break;
	default:
		if (address < 0x80)
			data = m_window[address];
		break;
	}

	LOGMASKED(LOG_READS, "control_r 0x%02x\n", data);
	increment_address(false);
	return data;
}

void bt479_device::control_w(u8 data)
{
	unsigned const address = bt47x_device_base::address();
	LOG("control_w address 0x%02x data 0x%02x\n", address, data);

	switch (address)
	{
	case 0x82:
	case 0x83:
		LOG("control_w command%d 0x%02x\n", address & 1, data);
		m_command[address & 1] = data;
		break;
	case 0x84:
	case 0x85:
		LOG("control_w flood%d 0x%02x\n", address & 1, data);
		m_flood[address & 1] = data;
		break;
	default:
		if (address < 0x80)
			m_window[address] = data;
		break;
	}

	increment_address(false, true);
}
