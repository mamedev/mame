// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Brooktree Bt45x family (except the Bt459) of RAMDAC
 * devices. The 453, 454 and 455 are simpler, and have no command or control
 * registers, whereas the others in the family use these to support additional
 * options such as multiplexing, blinking etc. Otherwise the devices are quite
 * similar, and vary only in terms of maximum clock speed, number of color or
 * overlay bits, and quantity and number bits of resolution of DAC output.
 *
 *           Max.       Input          Output
 *   Part    Clock  Color  Overlay  Num.  Levels  Other
 *   Bt451  165MHz  8 bit   2 bit    3     4 bit  blinking, multiplexing
 *   Bt453   66MHz  8 bit   2 bit    3     8 bit
 *   Bt454  170MHz  4 bit   1 bit    3     4 bit
 *   Bt455  170MHz  4 bit   1 bit    1     4 bit
 *   Bt457  165Mhz  8 bit   2 bit    1     8 bit  blinking, multiplexing
 *   Bt458  165MHz  8 bit   2 bit    3     8 bit  blinking, multiplexing
 *   Bt467  220MHz  8 bit   2 bit    3     8 bit  blinking, multiplexing [NOTE: Specs are assumed based on Bt458 compatibility)
 *
 * Reference: http://www.bitsavers.org/components/brooktree/_dataBooks/1991_Brooktree_Product_Databook.pdf
 *
 * The bt45x_mono_device_base uses the standard red/green/blue read/write
 * cycles defined in the databook, with color data active on the green cycle.
 *
 * The Bt467 is specified in its datasheet as register-compatible with the Bt458.
 * As such, it is currently implemented as a simple alias of the Bt458.
 *
 * TODO
 *   - refactor to separate devices with registers
 *   - implement blinking and rgb device overlay
 *   - unsure about address masking when accessing overlay colors
 */

#include "emu.h"
#include "bt45x.h"

#define LOG_GENERAL (1U << 0)
#define LOG_READS   (1U << 1)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(BT451, bt451_device, "bt451", "Brooktree Bt451 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT453, bt453_device, "bt453", "Brooktree Bt453 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT454, bt454_device, "bt454", "Brooktree Bt454 16 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT455, bt455_device, "bt455", "Brooktree Bt455 16 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT457, bt457_device, "bt457", "Brooktree Bt457 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT458, bt458_device, "bt458", "Brooktree Bt458 256 Color RAMDAC")
DEFINE_DEVICE_TYPE(BT467, bt467_device, "bt467", "Brooktree Bt467 256 Color RAMDAC")

void bt45x_device_base::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt45x_device_base::address_r), FUNC(bt45x_device_base::address_w));
	map(0x01, 0x01).rw(FUNC(bt45x_device_base::palette_r), FUNC(bt45x_device_base::palette_w));
	map(0x02, 0x02).rw(FUNC(bt45x_device_base::register_r), FUNC(bt45x_device_base::register_w));
	map(0x03, 0x03).rw(FUNC(bt45x_device_base::overlay_r), FUNC(bt45x_device_base::overlay_w));
}

void bt453_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt453_device::address_r), FUNC(bt453_device::address_w));
	map(0x01, 0x01).rw(FUNC(bt453_device::palette_r), FUNC(bt453_device::palette_w));
	map(0x02, 0x02).rw(FUNC(bt453_device::address_r), FUNC(bt453_device::address_w));
	map(0x03, 0x03).rw(FUNC(bt453_device::overlay_r), FUNC(bt453_device::overlay_w));
}

void bt454_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt454_device::address_r), FUNC(bt454_device::address_w));
	map(0x01, 0x01).rw(FUNC(bt454_device::palette_r), FUNC(bt454_device::palette_w));
	// FIXME: not clear what happens here
	//map(0x02, 0x02).rw(FUNC(bt454_device::address_r), FUNC(bt454_device::address_w));
	map(0x03, 0x03).rw(FUNC(bt454_device::overlay_r), FUNC(bt454_device::overlay_w));
}

void bt455_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt455_device::address_r), FUNC(bt455_device::address_w));
	map(0x01, 0x01).rw(FUNC(bt455_device::palette_r), FUNC(bt455_device::palette_w));
	// FIXME: not clear what happens here
	//map(0x02, 0x02).rw(FUNC(bt455_device::address_r), FUNC(bt455_device::address_w));
	map(0x03, 0x03).rw(FUNC(bt455_device::overlay_r), FUNC(bt455_device::overlay_w));
}

bt45x_device_base::bt45x_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const u32 palette_colors, const u32 overlay_colors)
	: device_t(mconfig, type, tag, owner, clock)
	, m_palette_colors(palette_colors)
	, m_overlay_colors(overlay_colors)
{
}

bt45x_rgb_device_base::bt45x_rgb_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const u32 palette_colors, const u32 overlay_colors)
	: bt45x_device_base(mconfig, type, tag, owner, clock, palette_colors, overlay_colors)
	, device_palette_interface(mconfig, *this)
{
}

bt45x_mono_device_base::bt45x_mono_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const u32 palette_colors, const u32 overlay_colors)
	: bt45x_device_base(mconfig, type, tag, owner, clock, palette_colors, overlay_colors)
{
}

bt451_device::bt451_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bt45x_rgb_device_base(mconfig, BT451, tag, owner, clock, 256, 4)
{
}

bt453_device::bt453_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bt45x_rgb_device_base(mconfig, BT453, tag, owner, clock, 256, 4)
{
}

bt454_device::bt454_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bt45x_rgb_device_base(mconfig, BT454, tag, owner, clock, 16, 1)
{
}

bt455_device::bt455_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bt45x_mono_device_base(mconfig, BT455, tag, owner, clock, 16, 1)
{
}
bt457_device::bt457_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bt45x_mono_device_base(mconfig, BT457, tag, owner, clock, 256, 4)
{
}

bt458_device::bt458_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: bt45x_rgb_device_base(mconfig, type, tag, owner, clock, 256, 4)
{
}

bt458_device::bt458_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bt458_device(mconfig, BT458, tag, owner, clock)
{
}

bt467_device::bt467_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bt458_device(mconfig, BT467, tag, owner, clock)
{
}

void bt45x_device_base::device_start()
{
	save_item(NAME(m_address));
	save_item(NAME(m_address_rgb));

	save_item(NAME(m_read_mask));
	save_item(NAME(m_blink_mask));
	save_item(NAME(m_command));
	save_item(NAME(m_control));

	save_item(NAME(m_blink_start));
}

void bt45x_rgb_device_base::device_start()
{
	bt45x_device_base::device_start();

	m_color_ram = std::make_unique<std::array<u8, 3>[]>(m_palette_colors + m_overlay_colors);

	save_pointer(NAME(m_color_ram), m_palette_colors + m_overlay_colors);
}

void bt45x_mono_device_base::device_start()
{
	bt45x_device_base::device_start();

	m_color_ram = std::make_unique<u8[]>(m_palette_colors + m_overlay_colors);

	save_pointer(NAME(m_color_ram), m_palette_colors + m_overlay_colors);
}

void bt45x_device_base::device_reset()
{
	m_blink_start = -1;
}

READ8_MEMBER(bt45x_device_base::address_r)
{
	LOGMASKED(LOG_READS, "address_r 0x%02x\n", m_address & (m_palette_colors - 1));

	if (!machine().side_effects_disabled())
		m_address_rgb = 0;

	return m_address & (m_palette_colors - 1);
}

WRITE8_MEMBER(bt45x_device_base::address_w)
{
	LOG("address_w 0x%02x\n", data);
	m_address_rgb = 0;

	m_address = data & (m_palette_colors - 1);
}

void bt45x_device_base::increment_address(const bool side_effects)
{
	if (!machine().side_effects_disabled() || side_effects)
	{
		// increment component index and address register
		m_address_rgb = (m_address_rgb + 1) % 3;
		if (m_address_rgb == 0)
			m_address = (m_address + 1) & (m_palette_colors - 1);
	}
}

void bt457_device::increment_address(const bool side_effects)
{
	if (!machine().side_effects_disabled() || side_effects)
	{
		// check for rgb mode
		if (m_control & RGB)
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

READ8_MEMBER(bt45x_rgb_device_base::palette_r)
{
	const u8 data = m_color_ram[m_address][m_address_rgb];

	increment_address();

	LOGMASKED(LOG_READS, "palette_r 0x%02x\n", data & get_mask());

	return data & get_mask();
}

READ8_MEMBER(bt45x_mono_device_base::palette_r)
{
	u8 data = space.unmap();

	if (m_address_rgb == 1)
		data = m_color_ram[m_address];

	increment_address();

	LOGMASKED(LOG_READS, "palette_r 0x%02x\n", data & get_mask());

	return data & get_mask();
}

READ8_MEMBER(bt457_device::palette_r)
{
	u8 data = space.unmap();

	// normal mode or rgb mode and selected
	if (!(m_control & RGB) || (m_control & RGB) == (1 << m_address_rgb))
		data = m_color_ram[m_address];

	increment_address();

	LOGMASKED(LOG_READS, "palette_r 0x%02x\n", data);

	return data;
}

WRITE8_MEMBER(bt45x_rgb_device_base::palette_w)
{
	LOG("palette_w 0x%02x\n", data);

	m_color_ram[m_address][m_address_rgb] = data & get_mask();

	// update the mame palette to match the device
	if (m_address_rgb == 2)
		set_pen_color(m_address, rgb_t(m_color_ram[m_address][0], m_color_ram[m_address][1], m_color_ram[m_address][2]));

	increment_address(true);
}

WRITE8_MEMBER(bt45x_mono_device_base::palette_w)
{
	LOG("palette_w 0x%02x\n", data);

	if (m_address_rgb == 1)
		m_color_ram[m_address] = data & get_mask();

	increment_address(true);
}

WRITE8_MEMBER(bt457_device::palette_w)
{
	LOG("palette_w 0x%02x\n", data);

	// device in normal mode, or rgb mode and selected
	if (!(m_control & RGB) || (m_control & RGB) == (1 << m_address_rgb))
		m_color_ram[m_address] = data;

	increment_address(true);
}

READ8_MEMBER(bt45x_device_base::register_r)
{
	LOGMASKED(LOG_READS, "register_r 0x%02x\n", m_address);

	switch (m_address)
	{
	case REG_READ_MASK:  return m_read_mask;
	case REG_BLINK_MASK: return m_blink_mask;
	case REG_COMMAND:    return m_command;
	case REG_CONTROL:    return m_control;
	}

	return space.unmap();
}

WRITE8_MEMBER(bt45x_device_base::register_w)
{
	switch (m_address)
	{
	case REG_READ_MASK:
		LOG("read mask 0x%02x\n", data);
		m_read_mask = data;
		break;

	case REG_BLINK_MASK:
		LOG("blink mask 0x%02x\n", data);
		m_blink_mask = data;
		break;

	case REG_COMMAND:
		LOG("command 0x%02x, %d:1 multiplexing, use %s, %s, OL1 %s blinking, OL0 %s blinking, OL1 display %s, OL0 display %s\n",
			data,
			(data & CR7) ? 5 : 4,
			(data & CR6) ? "color palette RAM" : "overlay color 0",
			(data & CR54) == CR54_6464 ? "64 on 64 off (50/50)" :
				(data & CR54) == CR54_3232 ? "32 on 32 off (50/50)" :
				(data & CR54) == CR54_1616 ? "16 on 16 off (50/50)" :
				"16 on 48 off (25/75)",
			(data & CR3) ? "enable" : "disable",
			(data & CR2) ? "enable" : "disable",
			(data & CR1) ? "enable" : "disable",
			(data & CR0) ? "enable" : "disable");
		m_command = data;
		break;

	case REG_CONTROL:
		LOG("control 0x%02x, %s nibble%s%s%s\n",
			data,
			(data & D3) ? "low" : "high",
			(data & D2) ? ", blue channel enable" : "",
			(data & D1) ? ", green channel enable" : "",
			(data & D0) ? ", red channel enable" : "");
		m_control = data & 0xf;
		break;
	}
}

READ8_MEMBER(bt45x_rgb_device_base::overlay_r)
{
	// address is ignored for 1 bit overlay devices
	const u8 address = (m_overlay_colors == 1) ? 0 : m_address;

	u8 data = space.unmap();

	if (address < m_overlay_colors)
		data = m_color_ram[m_palette_colors + address][m_address_rgb];

	increment_address();

	LOGMASKED(LOG_READS, "overlay_r 0x%02x\n", data & get_mask());

	return data & get_mask();
}

READ8_MEMBER(bt45x_mono_device_base::overlay_r)
{
	// address is ignored for 1 bit overlay devices
	const u8 address = (m_overlay_colors == 1) ? 0 : m_address;

	u8 data = space.unmap();

	if (address < m_overlay_colors && m_address_rgb == 1)
		data = m_color_ram[m_palette_colors + address];

	increment_address();

	LOGMASKED(LOG_READS, "overlay_r 0x%02x\n", data & get_mask());

	return data & get_mask();
}

READ8_MEMBER(bt457_device::overlay_r)
{
	u8 data = space.unmap();

	if (m_address < m_overlay_colors)
	{
		// device in normal mode, or rgb mode and selected
		if (!(m_control & RGB) || (m_control & RGB) == (1 << m_address_rgb))
			data = m_color_ram[m_address + m_palette_colors];
	}

	increment_address();

	LOGMASKED(LOG_READS, "overlay_r 0x%02x\n", data);

	return data;
}

WRITE8_MEMBER(bt45x_rgb_device_base::overlay_w)
{
	LOG("overlay_w 0x%02x\n", data);

	// address is ignored for 1 bit overlay devices
	const u8 address = (m_overlay_colors == 1) ? 0 : m_address;

	if (address < m_overlay_colors)
	{
		m_color_ram[m_palette_colors + address][m_address_rgb] = data & get_mask();

		// update the mame palette to match the device
		if (m_address_rgb == 2)
			set_pen_color(m_palette_colors + address, rgb_t(
				m_color_ram[m_palette_colors + address][0],
				m_color_ram[m_palette_colors + address][1],
				m_color_ram[m_palette_colors + address][2]));
	}

	increment_address(true);
}

WRITE8_MEMBER(bt45x_mono_device_base::overlay_w)
{
	LOG("overlay_w 0x%02x\n", data);

	// address is ignored for 1 bit overlay devices
	const u8 address = (m_overlay_colors == 1) ? 0 : m_address;

	if (address < m_overlay_colors)
		m_color_ram[m_palette_colors + address] = data & get_mask();

	increment_address(true);
}

WRITE8_MEMBER(bt457_device::overlay_w)
{
	LOG("overlay_w 0x%02x\n", data);

	if (m_address < m_overlay_colors)
	{
		// device in normal mode, or rgb mode and selected
		if (!(m_control & RGB) || (m_control & RGB) == (1 << m_address_rgb))
			m_color_ram[m_palette_colors + m_address] = data;
	}

	increment_address(true);
}
