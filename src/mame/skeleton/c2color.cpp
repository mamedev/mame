// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    basic information
    https://gbatemp.net/threads/the-c2-color-game-console-an-obscure-chinese-handheld.509320/

    "The C2 is a glorious console with a D-Pad, Local 2.4GHz WiFi, Cartridge slot, A, B, and C buttons,
     and has micro usb power! Don't be fooled though, there is no lithium battery, so you have to put in
     3 AA batteries if you don't want to play with it tethered to a charger.

     It comes with a built in game based on the roco kingdom characters.

     In addition, there is a slot on the side of the console allowing cards to be swiped through. Those
     cards can add characters to the game. The console scans the barcode and a new character or item appears in the game for you to use.

     The C2 comes with 9 holographic game cards that will melt your eyes."

    also includes a link to the following video
    https://www.youtube.com/watch?v=D3XO4aTZEko

    TODO:
    identify CPU type - It's an i8051 derived CPU, and seems to be "Mars Semiconductor Corp" related, there is a MARS-PCCAM string
                        amongst other things, this is a known USB identifier for the "Discovery Kids Digital Camera"
                        Possibly a MR97327B, which is listed as RISC-51 in places, but little information can be found in English
    - Dump the internal boot ROM; its initial SPI-to-DRAM load is substituted below.
    - Establish CPU/peripheral clocks, DMA/codec timing and the remaining interrupt sources.
    - Identify the companion and implement commands beyond the boot challenge.
    - Complete LCD/OSD palettes, display latching, JPEG formats and audio controls.
    - Barcode reader, radio, USB and cartridge boot selection are not implemented.
    - Flash erase/program behaviour depends on the incomplete generic SPI flash model.

*******************************************************************************/

#include "emu.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/mcs51/i8052.h"
#include "machine/generic_spi_flash.h"
#include "machine/i2chle.h"
#include "sound/dac.h"

#include "rendutil.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "ioprocs.h"

#include <vector>

#define LOG_REGS (1U << 1)
#define LOG_DMA  (1U << 2)
#define LOG_SPI  (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

// The exact SoC is still unidentified.  Use the ROM-less 8052 configuration
// with the additional arithmetic registers exercised by the firmware.
class c2_color_cpu_device : public i8052_device
{
public:
	c2_color_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sfr_map(address_map &map) override ATTR_COLD;

private:
	u8 math_r(offs_t offset) { return m_math[offset]; }
	void math_w(offs_t offset, u8 data);
	u8 unknown_r() { return m_unknown; }
	void unknown_w(u8 data) { m_unknown = data; }

	u8 m_math[6] = {};
	u8 m_math_written = 0;
	u8 m_unknown = 0;
};

DEFINE_DEVICE_TYPE_PRIVATE(C2_COLOR_CPU, c2_color_cpu_device, c2_color_cpu_device, "c2_color_cpu", "C2 Color 8051-based CPU")

c2_color_cpu_device::c2_color_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, C2_COLOR_CPU, tag, owner, clock, 0)
{
}

void c2_color_cpu_device::device_start()
{
	i8052_device::device_start();
	save_item(NAME(m_math));
	save_item(NAME(m_math_written));
	save_item(NAME(m_unknown));
}

void c2_color_cpu_device::device_reset()
{
	i8052_device::device_reset();
	std::fill(std::begin(m_math), std::end(m_math), 0);
	m_math_written = 0;
	m_unknown = 0;
}

void c2_color_cpu_device::sfr_map(address_map &map)
{
	i8052_device::sfr_map(map);
	map(0x8e, 0x8e).rw(FUNC(c2_color_cpu_device::unknown_r), FUNC(c2_color_cpu_device::unknown_w));
	map(0xe9, 0xee).rw(FUNC(c2_color_cpu_device::math_r), FUNC(c2_color_cpu_device::math_w));
}

void c2_color_cpu_device::math_w(offs_t offset, u8 data)
{
	if (!offset)
		m_math_written = 0;
	m_math[offset] = data;
	m_math_written |= 1U << offset;
	if (offset != 5)
		return;

	// Multiplication writes E9, ED, EA, EE.  Division writes all six bytes
	// in ascending order, supplying a 32-bit dividend and 16-bit divisor.
	u32 const operand = u32(m_math[0]) | (u32(m_math[1]) << 8);
	u32 const factor = u32(m_math[4]) | (u32(m_math[5]) << 8);
	u32 result = operand * factor;
	if (m_math_written == 0x3f)
	{
		u32 const dividend = operand | (u32(m_math[2]) << 16) | (u32(m_math[3]) << 24);
		result = factor ? dividend / factor : 0xffffffff;
		u16 const remainder = factor ? dividend % factor : 0;
		m_math[4] = remainder;
		m_math[5] = remainder >> 8;
	}
	for (unsigned i = 0; i != 4; ++i)
		m_math[i] = result >> (8 * i);
	// Arithmetic timing and overflow/division-by-zero status are unknown.
}


// High-level model of the unidentified companion at I2C address 0x53.
// Only the observed three-byte challenge and two-byte response are understood.
// TODO: Identify the device and the remaining commands.
class c2_color_companion_device : public device_t, public i2c_hle_interface
{
public:
	c2_color_companion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual const char *get_tag() override { return tag(); }

private:
	u8 m_challenge[3]{};
	u8 m_received = 0;
	u8 m_response = 0;
};

DEFINE_DEVICE_TYPE_PRIVATE(C2_COLOR_COMPANION, c2_color_companion_device, c2_color_companion_device, "c2_color_companion", "C2 Color companion (HLE)")

c2_color_companion_device::c2_color_companion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, C2_COLOR_COMPANION, tag, owner, clock)
	, i2c_hle_interface(mconfig, *this, 0x53)
{
}

void c2_color_companion_device::device_start()
{
	save_item(NAME(m_challenge));
	save_item(NAME(m_received));
	save_item(NAME(m_response));
}

void c2_color_companion_device::device_reset()
{
	std::fill(std::begin(m_challenge), std::end(m_challenge), 0);
	m_received = m_response = 0;
}

void c2_color_companion_device::write_data(u16 offset, u8 data)
{
	// Command 1 is followed by three challenge bytes.
	if (offset == 1)
		m_received = 0;
	if ((offset >= 1) && (offset <= 3) && (offset == m_received + 1))
	{
		m_challenge[m_received++] = data;
		if (m_received == 3)
		{
			u8 const a = bitswap<8>(m_challenge[0], 3, 2, 1, 0, 7, 6, 5, 4) ^ 0x19;
			u8 const b = bitswap<8>(m_challenge[1], 0, 1, 2, 3, 4, 5, 6, 7) ^ 0xac;
			u8 const c = bitswap<8>(m_challenge[2], 6, 7, 4, 5, 2, 3, 0, 1) ^ 0x58;
			m_response = a + b + c;
		}
	}
}

u8 c2_color_companion_device::read_data(u16 offset)
{
	// Firmware writes command 2, then issues STOP and a two-byte read.
	if (m_received == 3)
	{
		if (offset == 0)
			return 1;
		if (offset == 1)
			return m_response;
	}
	return 0xff;
}


class c2_color_state : public driver_device
{
public:
	c2_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
		, m_screen(*this, "screen")
		, m_flash(*this, "flash%u", 1U)
		, m_xram(*this, "xram")
		, m_companion(*this, "companion")
		, m_dac(*this, "dac")
		, m_buttons(*this, "BUTTONS")
		, m_battery(*this, "BATTERY")
	{ }

	void c2_color(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	u8 code_r(offs_t offset);
	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);
	u8 &reg(u16 address) { return m_regs[address - 0x2000]; }
	u32 reg32(u16 address) const;
	void spi_select();
	u8 spi_exchange(u8 data);
	void dma(unsigned channel);
	u8 dma_r(u8 source, u32 address);
	void dma_w(u8 destination, u32 address, u8 data);
	void dram_access(u8 data);
	void jpeg_decode();
	void audio_control();
	void update_irq();
	TIMER_CALLBACK_MEMBER(audio_tick);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<c2_color_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
	required_device<screen_device> m_screen;
	required_device_array<generic_spi_flash_device, 3> m_flash;
	required_shared_ptr<u8> m_xram;
	required_device<c2_color_companion_device> m_companion;
	required_device<dac_16bit_r2r_device> m_dac;
	required_ioport m_buttons;
	required_ioport m_battery;
	u8 m_companion_sda = 1;
	emu_timer *m_audio_timer = nullptr;
	u32 m_audio_address = 0;
	u32 m_audio_remaining = 0;
	bool m_audio_enabled = false;

	static constexpr u32 DRAM_SIZE = 0x200000;
	std::unique_ptr<u8[]> m_dram;
	std::unique_ptr<u16[]> m_osd_code;
	std::unique_ptr<u8[]> m_osd_attr;
	bool m_lcd_sleep = true;
	bool m_lcd_on = false;
	std::unique_ptr<u8[]> m_flash_data[3];
	u8 m_regs[0x600] = {};
	u8 m_dma_fill[4] = {};
	u8 m_dma_fill_pos = 0;
	s8 m_spi_selected = -1;
	u8 m_quant[2][128] = {};
	u8 m_quant_pos = 0;
};

u32 c2_color_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	if (m_lcd_sleep || !m_lcd_on)
		return 0;

	u32 const base = reg32(0x2460) & 0xffffff;
	u32 const font = (u32(reg(0x21a4)) | (u32(reg(0x21a5)) << 8)) << 9;
	u16 const columns = reg(0x2186);
	u16 const rows = reg(0x2187);
	u32 const overlay = reg32(0x246f);
	u32 const mask = reg32(0x2473);
	u16 const overlay_width = reg32(0x2477) & 0xffff;
	u16 const overlay_height = reg32(0x2479) & 0xffff;
	u16 const overlay_x = reg32(0x247b) & 0xffff;
	u16 const overlay_y = reg32(0x247d) & 0xffff;
	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			u32 const address = base + (y * screen.visible_area().width() + x) * 2;
			u16 const pixel = m_dram[address & (DRAM_SIZE - 1)] | (u16(m_dram[(address + 1) & (DRAM_SIZE - 1)]) << 8);
			bitmap.pix(y, x) = rgb_t(pal5bit(pixel >> 11), pal6bit(pixel >> 5), pal5bit(pixel));
		}
	}

	// A second RGB565 plane has a separate packed, LSB-first opacity mask.
	// DMA constructs the plane in DRAM; the display controller composites it.
	// TODO: Configuration latch timing, signed positions and non-byte-aligned widths.
	if (BIT(reg(0x246e), 0) && overlay_width && overlay_height)
	{
		rectangle area(overlay_x, overlay_x + overlay_width - 1, overlay_y, overlay_y + overlay_height - 1);
		area &= cliprect;
		for (int y = area.min_y; y <= area.max_y; ++y)
		{
			for (int x = area.min_x; x <= area.max_x; ++x)
			{
				u32 const index = (y - overlay_y) * overlay_width + x - overlay_x;
				if (BIT(m_dram[(mask + (index >> 3)) & (DRAM_SIZE - 1)], index & 7))
				{
					u32 const address = overlay + index * 2;
					u16 const pixel = m_dram[address & (DRAM_SIZE - 1)] | (u16(m_dram[(address + 1) & (DRAM_SIZE - 1)]) << 8);
					bitmap.pix(y, x) = rgb_t(pal5bit(pixel >> 11), pal6bit(pixel >> 5), pal5bit(pixel));
				}
			}
		}
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			if (BIT(reg(0x2185), 0) && x / 16 < columns && y / 20 < rows)
			{
				u16 const cell = (y / 20) * columns + x / 16;
				u32 const glyph = font + m_osd_code[cell] * 80 + (y % 20) * 4 + (x % 16) / 4;
				u8 const ink = BIT(m_dram[glyph & (DRAM_SIZE - 1)], (x & 3) * 2, 2);
				// TODO: Decode the OSD palette, attributes and blending controls.
				if (ink)
					bitmap.pix(y, x) = rgb_t(ink * 85, ink * 85, ink * 85);
			}
		}
	}
	return 0;
}

void c2_color_state::machine_start()
{
	// The cartridge is a third SPI source; the built-in firmware remains present.
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}

	m_dram = std::make_unique<u8[]>(DRAM_SIZE);
	m_osd_code = std::make_unique<u16[]>(0x10000);
	m_osd_attr = std::make_unique<u8[]>(0x10000);
	save_pointer(NAME(m_dram), DRAM_SIZE);
	save_pointer(NAME(m_osd_code), 0x10000);
	save_pointer(NAME(m_osd_attr), 0x10000);
	save_item(NAME(m_lcd_sleep));
	save_item(NAME(m_lcd_on));
	save_item(NAME(m_regs));
	save_item(NAME(m_dma_fill));
	save_item(NAME(m_dma_fill_pos));
	save_item(NAME(m_spi_selected));
	save_item(NAME(m_quant));
	save_item(NAME(m_quant_pos));
	save_item(NAME(m_companion_sda));
	save_item(NAME(m_audio_address));
	save_item(NAME(m_audio_remaining));
	save_item(NAME(m_audio_enabled));
	m_audio_timer = timer_alloc(FUNC(c2_color_state::audio_tick), this);
	machine().save().register_postload(save_prepost_delegate(FUNC(c2_color_state::update_irq), this));

	for (unsigned i = 0; i != 3; ++i)
	{
		memory_region *const region = i == 2 ? m_cart_region : memregion(i ? "spi2" : "spi1");
		u32 const length = region ? region->bytes() : 1;
		m_flash_data[i] = std::make_unique<u8[]>(length);
		if (region)
			std::copy_n(region->base(), length, m_flash_data[i].get());
		else
			m_flash_data[i][0] = 0xff;
		m_flash[i]->set_rom_ptr(m_flash_data[i].get());
		m_flash[i]->set_rom_size(length);
		save_pointer(NAME(m_flash_data[i]), length, i);
	}
}

void c2_color_state::machine_reset()
{
	std::fill_n(&m_xram[0], 0x2000, 0);
	std::fill_n(m_dram.get(), DRAM_SIZE, 0);
	std::fill_n(m_osd_code.get(), 0x10000, 0);
	std::fill_n(m_osd_attr.get(), 0x10000, 0);
	m_lcd_sleep = true;
	m_lcd_on = false;
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_dma_fill), std::end(m_dma_fill), 0);
	m_dma_fill_pos = 0;
	m_spi_selected = -1;
	for (auto &table : m_quant)
		std::fill(std::begin(table), std::end(table), 0);
	m_quant_pos = 0;
	m_audio_address = m_audio_remaining = 0;
	m_audio_enabled = false;
	m_audio_timer->adjust(attotime::never);
	m_dac->write(0x8000);
	reg(0x2144) = 1;
	reg(0x2152) = 0x20;
	reg(0x2042) = 0x10;
	reg(0x2002) = 3;
	update_irq();

	// The internal boot ROM is undumped.  Substitute its initial load of the
	// built-in firmware into DRAM, skipping the SPI image's 32-byte header.
	// The header specifies a 0x40000-byte initial load.
	// Subsequent resource transfers use the emulated SPI and DMA controllers.
	std::copy_n(m_flash_data[0].get() + 0x20, 0x40000, m_dram.get());
}

DEVICE_IMAGE_LOAD_MEMBER(c2_color_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");
	if (!size || (size & (size - 1)) || size > 0x1000000)
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridge size must be a power of two, no larger than 16 MiB");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

u8 c2_color_state::code_r(offs_t offset)
{
	if (BIT(reg(0x2141), 0) && offset >= 0x4000 && offset < 0x4200)
		return m_xram[offset - 0x4000];
	u32 const address = offset < 0x8000 ? offset : (u32(reg(0x2144)) << 15) | (offset & 0x7fff);
	return m_dram[address & (DRAM_SIZE - 1)];
}

u32 c2_color_state::reg32(u16 address) const
{
	u8 const *const bytes = &m_regs[address - 0x2000];
	return u32(bytes[0]) | (u32(bytes[1]) << 8) | (u32(bytes[2]) << 16) | (u32(bytes[3]) << 24);
}

void c2_color_state::spi_select()
{
	// The first flash uses an active-high select; the other two are active-low.
	s8 const selected = BIT(reg(0x2155), 5) ? 0 : !BIT(reg(0x2152), 5) ? 1 : !BIT(reg(0x2042), 4) ? 2 : -1;
	if (selected == m_spi_selected)
		return;
	for (unsigned i = 0; i != 3; ++i)
		m_flash[i]->cs_w(selected != int(i));
	m_spi_selected = selected;
	LOGMASKED(LOG_SPI, "%s: SPI select %d\n", machine().describe_context(), selected);
}

u8 c2_color_state::spi_exchange(u8 data)
{
	if (m_spi_selected < 0 || (m_spi_selected == 2 && !m_cart_region))
		return 0xff;
	m_flash[m_spi_selected]->write(data);
	return m_flash[m_spi_selected]->read();
}

u8 c2_color_state::dma_r(u8 source, u32 address)
{
	switch (source)
	{
	case 0:
		return m_xram[address & 0x1fff];
	case 2:
	case 3:
		return m_dram[address & (DRAM_SIZE - 1)];
	case 9:
		return spi_exchange(0xff);
	default:
		return 0xff;
	}
}

void c2_color_state::dma_w(u8 destination, u32 address, u8 data)
{
	switch (destination)
	{
	case 0:
		m_xram[address & 0x1fff] = data;
		break;
	case 2:
	case 3:
		m_dram[address & (DRAM_SIZE - 1)] = data;
		break;
	}
}

void c2_color_state::dma(unsigned channel)
{
	u16 const base = 0x2200 + channel * 0x3a;
	u8 const source = reg(base + 0x10) & 0x0f;
	u8 const destination = reg(base + 0x25) & 0x0f;
	u32 const source_address = reg32(base + 0x12);
	u32 const destination_address = reg32(base + 0x27);
	u32 const count = reg32(base + 1);
	bool const fill = BIT(reg(base), 1);
	LOGMASKED(LOG_DMA, "%s: DMA %u %x:%08x -> %x:%08x, %08x bytes%s\n", machine().describe_context(), channel,
		source, source_address, destination, destination_address, count, fill ? " (fill)" : "");

	// The observed transfers fit within DRAM.  Other modes, peripheral targets
	// and transfer timing still need investigation.
	if (count > DRAM_SIZE || (!fill && source != 0 && source != 2 && source != 3 && source != 9)
		|| (destination != 0 && destination != 2 && destination != 3))
	{
		LOGMASKED(LOG_DMA, "%s: unsupported DMA transfer\n", machine().describe_context());
		return;
	}
	for (u32 i = 0; i != count; ++i)
		dma_w(destination, destination_address + i, fill ? m_dma_fill[i & 3] : dma_r(source, source_address + i));

	reg(0x2147) |= 0x40 << channel;
	if (destination == 2 || destination == 3)
	{
		reg(0x2147) |= 0x01;
		reg(0x2148) |= 0x40;
	}
	update_irq();
}

void c2_color_state::dram_access(u8 data)
{
	// Firmware first writes 03, then requests a four-byte read or write.
	if (data == 0x07)
	{
		u32 const address = reg32(0x2429);
		for (unsigned i = 0; i != 4; ++i)
			m_dram[(address + i) & (DRAM_SIZE - 1)] = reg(0x242d + i);
		reg(0x2405) |= 0x08;
	}
	else if (data == 0x13)
	{
		u32 const address = reg32(0x2431);
		for (unsigned i = 0; i != 4; ++i)
			reg(0x2435 + i) = m_dram[(address + i) & (DRAM_SIZE - 1)];
		reg(0x2405) |= 0x20;
	}
}

void c2_color_state::jpeg_decode()
{
	u16 const width = reg32(0x2024) & 0xffff;
	u16 const height = reg32(0x202a) & 0xffff;
	u32 const source = reg32(0x244e);
	u32 const destination = reg32(0x244a) & 0xffffff;
	u32 const length = reg32(0x2452) & 0xffffff;
	if (!width || !height || u32(width) * height > DRAM_SIZE / 2 || !length || length > DRAM_SIZE)
		return;

	// The hardware receives quantisation tables and a baseline 4:2:2 scan
	// separately.  Supply the JPEG framing expected by the existing decoder;
	// libjpeg supplies the standard Huffman tables when DHT is omitted.
	// TODO: Establish whether DRAM holds RGB565 or YUV422 converted on display.
	std::vector<u8> stream{ 0xff, 0xd8, 0xff, 0xdb, 0x00, 0x84 };
	for (unsigned table = 0; table != 2; ++table)
	{
		stream.push_back(table);
		for (unsigned i = 0; i != 64; ++i)
			stream.push_back(m_quant[table][i * 2]);
	}
	u8 const header[] = {
		0xff, 0xc0, 0x00, 0x11, 8, u8(height >> 8), u8(height), u8(width >> 8), u8(width),
		3, 1, 0x21, 0, 2, 0x11, 1, 3, 0x11, 1,
		0xff, 0xda, 0x00, 0x0c, 3, 1, 0, 2, 0x11, 3, 0x11, 0, 0x3f, 0
	};
	stream.insert(stream.end(), std::begin(header), std::end(header));
	for (u32 i = 0; i != length; ++i)
		stream.push_back(m_dram[(source + i) & (DRAM_SIZE - 1)]);
	stream.push_back(0xff);
	stream.push_back(0xd9);
	bitmap_argb32 decoded;
	auto input = util::ram_read(stream.data(), stream.size());
	if (input)
		render_load_jpeg(decoded, *input);
	if (!decoded.valid())
		return;
	for (u32 y = 0; y != decoded.height(); ++y)
	{
		for (u32 x = 0; x != decoded.width(); ++x)
		{
			rgb_t const color(decoded.pix(y, x));
			u16 const pixel = ((color.r() >> 3) << 11) | ((color.g() >> 2) << 5) | (color.b() >> 3);
			u32 const address = destination + (y * width + x) * 2;
			m_dram[address & (DRAM_SIZE - 1)] = pixel;
			m_dram[(address + 1) & (DRAM_SIZE - 1)] = pixel >> 8;
		}
	}
	reg(0x2148) |= 0x30;
	update_irq();
}

void c2_color_state::update_irq()
{
	m_maincpu->set_input_line(MCS51_INT0_LINE, ((reg(0x2147) & reg(0x2145)) | (reg(0x2148) & reg(0x2146))) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(MCS51_INT1_LINE, ((reg(0x214d) & reg(0x214b)) | (reg(0x214e) & reg(0x214c))) ? ASSERT_LINE : CLEAR_LINE);
}

void c2_color_state::audio_control()
{
	bool const enabled = BIT(reg(0x208c), 0) && BIT(reg(0x2097), 1) && BIT(reg(0x246d), 1);
	if (enabled && !m_audio_enabled)
	{
		// Addresses and lengths are in 16-bit samples.  The observed setting
		// (20ab bits 2:1 clear) plays signed little-endian mono PCM at 8 kHz.
		// TODO: Other rates, formats, volume and output filtering.
		m_audio_address = (reg32(0x20a5) & 0xffffff) * 2;
		m_audio_remaining = reg32(0x2099) & 0xffffff;
		m_audio_timer->adjust(attotime::zero, 0, attotime::from_hz(8000));
	}
	else if (!enabled)
	{
		m_audio_timer->adjust(attotime::never);
		m_dac->write(0x8000);
	}
	m_audio_enabled = enabled;
}

TIMER_CALLBACK_MEMBER(c2_color_state::audio_tick)
{
	if (m_audio_remaining)
	{
		u16 const sample = m_dram[m_audio_address & (DRAM_SIZE - 1)]
			| (u16(m_dram[(m_audio_address + 1) & (DRAM_SIZE - 1)]) << 8);
		m_dac->write(sample ^ 0x8000);
		m_audio_address += 2;
		--m_audio_remaining;
	}
	else
	{
		m_dac->write(0x8000);
		m_audio_timer->adjust(attotime::never);
		reg(0x214e) |= 0x10;
		update_irq();
	}
}

u8 c2_color_state::io_r(offs_t offset)
{
	u16 const address = 0x2000 + offset;
	u8 data = reg(address);
	switch (address)
	{
	case 0x2002:
		data = (data & ~2) | ((BIT(data, 1) && m_companion_sda) ? 2 : 0);
		break;
	case 0x2053:
		data = (data & 0x03) | (m_buttons->read() & 0xfc);
		break;
	case 0x2152:
		data = (data & 0x7f) | (BIT(m_buttons->read(), 0) ? 0x80 : 0);
		break;
	case 0x2156:
		data |= 0x18; // SPI transmit/receive ready; transfers currently complete immediately.
		break;
	case 0x2400:
		data = (data & 0x3f) | (BIT(data, 2) ? 0x80 : 0x40); // DRAM stop/resume acknowledgement.
		break;
	}
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGS, "%s: read %04x = %02x\n", machine().describe_context(), address, data);
	return data;
}

void c2_color_state::io_w(offs_t offset, u8 data)
{
	u16 const address = 0x2000 + offset;
	LOGMASKED(LOG_REGS, "%s: write %04x = %02x\n", machine().describe_context(), address, data);
	u8 const previous = reg(address);
	reg(address) = data;
	switch (address)
	{
	case 0x2002:
		if (!BIT(data, 0))
			m_companion->scl_write(0);
		m_companion->sda_write(BIT(data, 1));
		if (BIT(data, 0))
			m_companion->scl_write(1);
		break;
	case 0x2042:
	case 0x2152:
	case 0x2155:
		spi_select();
		break;
	case 0x2064:
		if (BIT(data, 1))
		{
			// The millisecond unit is inferred from the LCD Sleep Out delay.
			// TODO: Identify the timer clock and divider controls.
			u32 const ticks = machine().time().as_ticks(1000);
			for (unsigned i = 0; i != 4; ++i)
				reg(0x205f + i) = ticks >> (8 * i);
			reg(address) &= ~0x02;
		}
		break;
	case 0x208c:
	case 0x2097:
	case 0x246d:
		audio_control();
		break;
	case 0x20ad:
		reg(address) = (data & ~9) | (previous & 8);
		if (BIT(data, 0))
		{
			// Channel 0 measures the batteries.  Voltage scaling, other inputs
			// and conversion timing are unknown; use representative raw levels.
			u16 const sample = (reg(0x20ac) & 3) == 0 ? m_battery->read() : 0;
			reg(0x20ae) = sample >> 8;
			reg(0x20af) = sample;
			reg(address) |= 8;
		}
		break;
	case 0x2149:
		reg(0x2147) &= ~data;
		update_irq();
		break;
	case 0x214a:
		reg(0x2148) &= ~data;
		update_irq();
		break;
	case 0x214f:
		reg(0x214d) &= ~data;
		update_irq();
		break;
	case 0x2150:
		reg(0x214e) &= ~data;
		update_irq();
		break;
	case 0x2145:
	case 0x2146:
	case 0x214b:
	case 0x214c:
		update_irq();
		break;
	case 0x2157:
		// Bit 6 of 2155 also enables a debug output stream on this port.
		// With no flash selected those bytes do not enter a flash command parser.
		spi_exchange(data);
		break;
	case 0x2158:
		reg(address) = spi_exchange(data);
		break;
	case 0x2185:
		// The two table-write strobes are armed separately before both go high.
		if ((data & 6) == 6)
		{
			if (!(previous & 2))
				m_osd_code[reg32(0x219d) & 0xffff] = reg32(0x219f) & 0xffff;
			if (!(previous & 4))
				m_osd_attr[reg32(0x21a1) & 0xffff] = reg(0x21a3);
			reg(address) &= ~6;
		}
		break;
	case 0x21c0:
		switch (data)
		{
		case 0x10: m_lcd_sleep = true; break;
		case 0x11: m_lcd_sleep = false; break;
		case 0x28: m_lcd_on = false; break;
		case 0x29: m_lcd_on = true; break;
		}
		break;
	case 0x2200:
	case 0x223a:
		if (data == 2)
			m_dma_fill_pos = 0;
		if (BIT(data, 0))
			dma(address == 0x2200 ? 0 : 1);
		break;
	case 0x220d:
		m_dma_fill[m_dma_fill_pos++ & 3] = data;
		break;
	case 0x229b:
		if (BIT(previous, 3) && !BIT(data, 3))
			m_quant_pos = 0;
		break;
	case 0x229d:
		m_quant[BIT(reg(0x229b), 2) ? 0 : 1][m_quant_pos++ & 0x7f] = data;
		break;
	case 0x2402:
		if ((data & 0x18) == 0x18 && (previous & 0x18) != 0x18)
			jpeg_decode();
		break;
	case 0x2405:
		dram_access(data);
		break;
	}
}

void c2_color_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(c2_color_state::code_r));
}

void c2_color_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("xram");
	map(0x2000, 0x25ff).rw(FUNC(c2_color_state::io_r), FUNC(c2_color_state::io_w));
}

static INPUT_PORTS_START( c2_color )
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Button C")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Button A")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Button B")

	PORT_START("BATTERY")
	PORT_CONFNAME(0x0fff, 0x0200, "Battery")
	PORT_CONFSETTING(0x0200, "Full")
	PORT_CONFSETTING(0x0100, "Low")
	PORT_CONFSETTING(0x0040, "Empty")
INPUT_PORTS_END

void c2_color_state::c2_color(machine_config &config)
{
	C2_COLOR_CPU(config, m_maincpu, 24'000'000); // exact type and clock / opcode cycle counts unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &c2_color_state::prog_map);
	m_maincpu->set_addrmap(AS_DATA, &c2_color_state::ext_map);

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(320, 240);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(c2_color_state::screen_update));

	SPEAKER(config, "speaker").front_center();
	DAC_16BIT_R2R(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 1.0);

	for (unsigned i = 0; i != 3; ++i)
		GENERIC_SPI_FLASH(config, m_flash[i]);
	// The optional cartridge has no per-image NVRAM ownership yet. (should be handled by slot device, not here)
	m_flash[2]->nvram_enable_backup(false);

	C2_COLOR_COMPANION(config, m_companion);
	m_companion->sda_callback().set([this] (int state) { m_companion_sda = state; });

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "c2color_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(c2_color_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("c2color_cart");
}

ROM_START( c2color )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bootloader", 0x0000, 0x4000, NO_DUMP )

	// As with the cartridges, each of these has a 0x20 byte header before the i8051
	// code starts.  This suggests it is unlikely the game runs directly from the SPI
	// ROM and more likely a bootloader copies the code into RAM.
	// The Mainboard has a 2MByte DRAM on it

	// This, the larger of the 2 ROMs contains unique code, it appears to be the base
	// game, and system functions.  It also has some 16-bit signed PCM samples.
	ROM_REGION( 0x800000, "spi1", ROMREGION_ERASEFF )
	ROM_LOAD( "spi.u7", 0x000000, 0x800000, CRC(6a4d2cd2) SHA1(46e109bbd5db206911716919ad13efc080cbdf34) )

	// The smaller ROM is much more similar to the cartridges (actually identical up
	// until the first MRDB resource block at 0x26000 aside from a few bytes in the
	// 0x20 header, and the game number at the 0x20000 mark being 0)
	//
	// This ROM also has a 2nd MRDB resource block, whereas the cartridges only have
	// a single block
	//
	// The code still contains a lot of generic 'firmware' like functions, but it is
	// unclear if any of the code is used, or if these ROMs are used more like skins
	// for the base game, accessing the resource table only
	//
	// The MRDB tables index IMG0 resources containing quantisation tables and
	// baseline JPEG scans.  No sound effects or non-graphical resources have
	// been identified in this flash.

	ROM_REGION( 0x400000, "spi2", ROMREGION_ERASEFF )
	ROM_LOAD( "spi.u16", 0x000000, 0x400000, CRC(9101b02a) SHA1(8c31e7641f4667bd8d5d7cc991cd5976828a0628) )
ROM_END

} // anonymous namespace


//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 201?, c2color,      0,       0,      c2_color,   c2_color, c2_color_state, empty_init, "Baiyi Animation", "C2 Color (China)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
