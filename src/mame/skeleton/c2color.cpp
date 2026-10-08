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
	- access cart through bus/c2color slot device

*******************************************************************************/

#include "emu.h"
#include "bus/c2color/slot.h"
#include "bus/c2color/carts.h"

#include "machine/generic_spi_flash.h"
#include "sound/dac.h"

#include "rendutil.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "ioprocs.h"

#include "c2color_companion.h"
#include "c2color_cpu.h"

#include <vector>

#define LOG_REGS (1U << 1)
#define LOG_DMA  (1U << 2)
#define LOG_SPI  (1U << 3)

#define VERBOSE (LOG_REGS)
#include "logmacro.h"


namespace {

class c2_color_state : public driver_device
{
public:
	c2_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_flash(*this, "flash%u", 1U)
		, m_xram(*this, "xram")
		, m_dram_dword_out_data(*this, "dram_dword_out_data")
	    , m_dram_dword_in_data(*this, "dram_dword_in_data")
		, m_timer_val(*this, "timer_val")
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

	void clear_state();

	// helper functions
	u8 code_r(offs_t offset);
	void spi_select();
	u8 spi_exchange(u8 data);
	void dma(unsigned channel);
	u8 dma_r(u8 source, u32 address);
	void dma_w(u8 destination, u32 address, u8 data);
	void dram_access(u8 data);
	void jpeg_decode();
	void audio_control();
	void update_irq();
	u8 read_reg_swapped(auto &reg, offs_t offset);
	void write_reg_swapped(auto &reg, offs_t offset, u8 data);

	// memory map access
	u8 companion_r() { u8 data = m_companion_2002; return (data & ~2) | ((BIT(data, 1) && m_companion_sda) ? 2 : 0); }
	void companion_w(u8 data);
	void timer_ctrl_w(u8 data);
	u8 adc_reg1_r() { return m_adc_reg1; }
	void adc_reg1_w(u8 data) { m_adc_reg1 = data; }
	u8 adc_r() { return m_adc_reg2; }
	void adc_w(u8 data);
	u8 adc_reg3_r() { return m_adc_reg3; }
	void adc_reg3_w(u8 data) { m_adc_reg3 = data; }
	u8 adc_reg4_r() { return m_adc_reg4; }
	void adc_reg4_w(u8 data) { m_adc_reg4 = data; }
	u8 xram_control_r() { return m_xram_control; }
	void xram_control_w(u8 data) { m_xram_control = data; }
	u8 ram_access_upper_r() { return m_ram_access_upper; }
	void ram_access_upper_w(u8 data) { m_ram_access_upper = data; }
	u8 osd_codes_r() { return m_osd_codes_reg; }
	void osd_codes_w(u8 data);
	void lcd_ctrl_w(u8 data);
	void jpeg_decode_trigger_w(u8 data);
	u8 quant_ctrl_r() {	return m_quant_ctrl; }
	void quant_ctrl_w(u8 data);
	void quant_data_w(u8 data) { m_quant[BIT(m_quant_ctrl, 2) ? 0 : 1][m_quant_pos++ & 0x7f] = data; }
	u8 spi_select_2042_r() { return m_spi_select_2042; }
	void spi_select_2042_w(u8 data) { m_spi_select_2042 = data;	spi_select(); }
	u8 buttons_r() { u8 data = m_buttons_reg; return (data & 0x03) | (m_buttons->read() & 0xfc); }
	void buttons_w(u8 data) { m_buttons_reg = data; }
	u8 timer_ctrl_r() {	return m_timer_ctrl; }
	u8 audiocontrol_208c_r() { return m_audiocontrol_208c; }
	void audiocontrol_208c_w(u8 data) {	m_audiocontrol_208c = data;	audio_control(); }
	u8 audiocontrol_2097_r() { return m_audiocontrol_2097; }
	void audiocontrol_2097_w(u8 data) {	m_audiocontrol_2097 = data;	audio_control(); }
	u8 dramstop_r() { u8 data = m_dramstop;	return (data & 0x3f) | (BIT(data, 2) ? 0x80 : 0x40); /* DRAM stop / resume acknowledgement. */ }
	void dramstop_w(u8 data) { m_dramstop = data; }
	u8 spi_select_2152_r() { return (m_spi_select_2152 & 0x7f) | (BIT(m_buttons->read(), 0) ? 0x80 : 0); }
	void spi_select_2152_w(u8 data) { m_spi_select_2152 = data; spi_select(); }
	u8 spi_status_r() {	return m_spi_status |= 0x18; /* SPI transmit / receive ready; transfers currently complete immediately. */ }
	void spi_status_w(u8 data) { m_spi_status = data; }
	u8 spi_select_2155_r() { return m_spi_select_2155; }
	void spi_select_2155_w(u8 data) { m_spi_select_2155 = data;	spi_select(); }
	void spi_exchange0_w(u8 data) {	spi_exchange(data); }
	u8 spi_exchange1_r() { return m_spi_exchange1; }
	void spi_exchange1_w(u8 data) {	m_spi_exchange1 = spi_exchange(data); }
	u8 audiocontrol_246d_r() { return m_audiocontrol_246d; }
	void audiocontrol_246d_w(u8 data) {	m_audiocontrol_246d = data; audio_control(); }
	u8 dramaccess_ctrl_r() { return m_dramaccess_ctrl; }
	void dramaccess_ctrl_w(u8 data) { m_dramaccess_ctrl = data; dram_access(data); }
	template <uint8_t Reg> u8 irqack_r() { return m_irqack[Reg]; }
	template <uint8_t Reg> u8 irqenable_r() { return m_irqenable[Reg]; }
	template <uint8_t Reg> void irqenable_w(u8 data) {m_irqenable[Reg] = data; update_irq(); }
	template <uint8_t Reg> u8 irqstatus_r() { return m_irqstatus[Reg]; }
	template <uint8_t Reg> void irqstatus_w(u8 data) { m_irqstatus[Reg] = data; }
	template <uint8_t Reg> void irqack_w(u8 data) { m_irqack[Reg] = data; m_irqstatus[Reg] &= ~data; update_irq(); }
	u8 dram_dword_out_address_r(offs_t offset) { return read_reg_swapped(m_dram_dword_out_address, offset); }
	void dram_dword_out_address_w(offs_t offset, u8 data) { write_reg_swapped(m_dram_dword_out_address, offset, data); }
	u8 dram_dword_in_address_r(offs_t offset) { return read_reg_swapped(m_dram_dword_in_address, offset); }
	void dram_dword_in_address_w(offs_t offset, u8 data) { write_reg_swapped(m_dram_dword_in_address, offset, data); }
	u8 jpeg_src_r(offs_t offset) { return read_reg_swapped(m_jpeg_src, offset); }
	void jpeg_src_w(offs_t offset, u8 data) { write_reg_swapped(m_jpeg_src, offset, data); }
	u8 jpeg_dst_r(offs_t offset) { return read_reg_swapped(m_jpeg_dst, offset); }
	void jpeg_dst_w(offs_t offset, u8 data) { write_reg_swapped(m_jpeg_dst, offset, data); }
	u8 jpeg_len_r(offs_t offset) { return read_reg_swapped(m_jpeg_len, offset); }
	void jpeg_len_w(offs_t offset, u8 data) { write_reg_swapped(m_jpeg_len, offset, data); }
	u8 jpeg_width_r(offs_t offset) { return read_reg_swapped(m_jpeg_width, offset); }
	void jpeg_width_w(offs_t offset, u8 data) { write_reg_swapped(m_jpeg_width, offset, data); }
	u8 jpeg_height_r(offs_t offset) { return read_reg_swapped(m_jpeg_height, offset); }
	void jpeg_height_w(offs_t offset, u8 data) { write_reg_swapped(m_jpeg_height, offset, data); }
	u8 render_base_r(offs_t offset) { return read_reg_swapped(m_render_base, offset); }
	void render_base_w(offs_t offset, u8 data) { write_reg_swapped(m_render_base, offset, data); }
	u8 render_overlay_r(offs_t offset) { return read_reg_swapped(m_render_overlay, offset); }
	void render_overlay_w(offs_t offset, u8 data) { write_reg_swapped(m_render_overlay, offset, data); }
	u8 render_font_r(offs_t offset) { return read_reg_swapped(m_render_font, offset); }
	void render_font_w(offs_t offset, u8 data) { write_reg_swapped(m_render_font, offset, data); }
	u8 render_mask_r(offs_t offset) { return read_reg_swapped(m_render_mask, offset); }
	void render_mask_w(offs_t offset, u8 data) { write_reg_swapped(m_render_mask, offset, data); }
	u8 overlay_width_r(offs_t offset) { return read_reg_swapped(m_overlay_width, offset); }
	void overlay_width_w(offs_t offset, u8 data) { write_reg_swapped(m_overlay_width, offset, data); }
	u8 overlay_height_r(offs_t offset) { return read_reg_swapped(m_overlay_height, offset); }
	void overlay_height_w(offs_t offset, u8 data) { write_reg_swapped(m_overlay_height, offset, data); }
	u8 overlay_x_r(offs_t offset) { return read_reg_swapped(m_overlay_x, offset); }
	void overlay_x_w(offs_t offset, u8 data) { write_reg_swapped(m_overlay_x, offset, data); }
	u8 overlay_y_r(offs_t offset) { return read_reg_swapped(m_overlay_y, offset); }
	void overlay_y_w(offs_t offset, u8 data) { write_reg_swapped(m_overlay_y, offset, data); }
	u8 render_osd0_r(offs_t offset) { return read_reg_swapped(m_render_osd0, offset); }
	void render_osd0_w(offs_t offset, u8 data) { write_reg_swapped(m_render_osd0, offset, data); }
	u8 render_osd1_r(offs_t offset) { return read_reg_swapped(m_render_osd1, offset); }
	void render_osd1_w(offs_t offset, u8 data) { write_reg_swapped(m_render_osd1, offset, data); }
	u8 render_osd2_r(offs_t offset) { return read_reg_swapped(m_render_osd2, offset); }
	void render_osd2_w(offs_t offset, u8 data) { write_reg_swapped(m_render_osd2, offset, data); }
	u8 render_osd3_r() { return m_render_osd3; }
	void render_osd3_w(u8 data) { m_render_osd3 = data; }
	u8 render_unknown_r() { return m_render_unknown; }
	void render_unknown_w(u8 data) { m_render_unknown = data; }
	u8 render_columns_r() { return m_render_columns; }
	void render_columns_w(u8 data) { m_render_columns = data; }
	u8 render_rows_r() { return m_render_rows; }
	void render_rows_w(u8 data) { m_render_rows = data; }
	u8 audio_remaining_r(offs_t offset) { return read_reg_swapped(m_audio_remaining_reg, offset); }
	void audio_remaining_w(offs_t offset, u8 data) { write_reg_swapped(m_audio_remaining_reg, offset, data); }
	u8 audio_address_r(offs_t offset) { return read_reg_swapped(m_audio_address_reg, offset); }
	void audio_address_w(offs_t offset, u8 data) { write_reg_swapped(m_audio_address_reg, offset, data); }
	template<int Channel> void dma_unk_w(offs_t offset, u8 data);
	template<int Channel> u8 dma_unk_r(offs_t offset);
	template<int Channel> u8 dma_count_r(offs_t offset) { return read_reg_swapped(m_dma_channel[Channel].m_dma_count, offset); }
	template<int Channel> void dma_count_w(offs_t offset, u8 data) { write_reg_swapped(m_dma_channel[Channel].m_dma_count, offset, data); }
	template<int Channel> u8 dma_source_addr_r(offs_t offset) { return read_reg_swapped(m_dma_channel[Channel].m_dma_source_addr, offset); }
	template<int Channel> void dma_source_addr_w(offs_t offset, u8 data) { write_reg_swapped(m_dma_channel[Channel].m_dma_source_addr, offset, data); }
	template<int Channel> u8 dma_dest_addr_r(offs_t offset) { return read_reg_swapped(m_dma_channel[Channel].m_dma_dest_addr, offset); }
	template<int Channel> void dma_dest_addr_w(offs_t offset, u8 data) { write_reg_swapped(m_dma_channel[Channel].m_dma_dest_addr, offset, data); }
	template<int Channel> u8 dma_source_r() { return m_dma_channel[Channel].m_dma_source; }
	template<int Channel> void dma_source_w(u8 data) { m_dma_channel[Channel].m_dma_source = data; }
	template<int Channel> u8 dma_dest_r() { return m_dma_channel[Channel].m_dma_dest; }
	template<int Channel> void dma_dest_w(u8 data) { m_dma_channel[Channel].m_dma_dest = data; }
	template<int Channel> void dma_fill_w(offs_t offset, u8 data);
	template<int Channel> u8 dma_trigger_r(offs_t offset) { return m_dma_channel[Channel].m_dma_trigger; }
	template<int Channel> void dma_trigger_w(offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(audio_tick);

	void prog_map(address_map &map) ATTR_COLD;
	template<int Channel> void add_dma_map(address_map &map, int base) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	emu_timer *m_audio_timer;
	std::unique_ptr<u8[]> m_flash_data[2]; // set on reset, doesn't need to be saved

	static constexpr u32 DRAM_SIZE = 0x200000;
	std::unique_ptr<u8[]> m_dram;
	std::unique_ptr<u16[]> m_osd_code;
	std::unique_ptr<u8[]> m_osd_attr;
	bool m_lcd_sleep = true;
	bool m_lcd_on = false;
	u8 m_companion_sda;
	u32 m_audio_address;
	u32 m_audio_remaining;
	bool m_audio_enabled;
	u8 m_xram_control;
	u8 m_ram_access_upper;
	u8 m_dramstop;
	u8 m_jpeg_decode_trigger;
	u8 m_quant_ctrl;
	u8 m_dramaccess_ctrl;
	u8 m_spi_select_2152;
	u8 m_spi_select_2155;
	u8 m_spi_status;
	u8 m_spi_exchange1;
	u8 m_osd_codes_reg;
	u8 m_audiocontrol_246d;
	u8 m_audiocontrol_208c;
	u8 m_audiocontrol_2097;
	u8 m_timer_ctrl;
	u8 m_adc_reg1;
	u8 m_adc_reg2;
	u8 m_adc_reg3;
	u8 m_adc_reg4;
	u8 m_spi_select_2042;
	u8 m_companion_2002;
	u8 m_buttons_reg;
	u8 m_irqack[4];
	u8 m_irqenable[4];
	u8 m_irqstatus[4];
	u32 m_dram_dword_out_address;
	u32 m_dram_dword_in_address;
	u32 m_render_base;
	u32 m_jpeg_dst;
	u32 m_jpeg_src;
	u32 m_jpeg_len;
	u32 m_jpeg_width;
	u32 m_jpeg_height;
	u32 m_render_overlay;
	u16 m_render_font;
	u32 m_render_mask;
	u16 m_overlay_width;
	u16 m_overlay_height;
	u16 m_overlay_x;
	u16 m_overlay_y;
	u16 m_render_osd0;
	u16 m_render_osd1;
	u16 m_render_osd2;
	u8 m_render_osd3;
	u8 m_render_columns;
	u8 m_render_rows;
	u8 m_render_unknown;
	u32 m_audio_remaining_reg;
	u32 m_audio_address_reg;
	s8 m_spi_selected;
	u8 m_quant[2][128];
	u8 m_quant_pos;

	struct dma_channel
	{
		u8 m_dma_trigger;
		u32 m_dma_count;
		u32 m_dma_source_addr;
		u8 m_dma_source;
		u32 m_dma_dest_addr;
		u8 m_dma_dest;

		u8 m_dma_fill[4];
		u8 m_dma_fill_pos;
	};

	dma_channel m_dma_channel[2];

	// devices
	required_device<c2_color_cpu_device> m_maincpu;
	required_device<c2color_cartslot_device> m_cart;
	required_device<screen_device> m_screen;
	required_device_array<generic_spi_flash_device, 2> m_flash;
	required_shared_ptr<u8> m_xram;
	required_shared_ptr<u8> m_dram_dword_out_data;
	required_shared_ptr<u8> m_dram_dword_in_data;
	required_shared_ptr<u8> m_timer_val;
	required_device<c2_color_companion_device> m_companion;
	required_device<dac_16bit_r2r_device> m_dac;
	required_ioport m_buttons;
	required_ioport m_battery;
};

u32 c2_color_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	if (m_lcd_sleep || !m_lcd_on)
		return 0;

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
		{
			u32 const address = m_render_base + (y * screen.visible_area().width() + x) * 2;
			u16 const pixel = m_dram[address & (DRAM_SIZE - 1)] | (u16(m_dram[(address + 1) & (DRAM_SIZE - 1)]) << 8);
			bitmap.pix(y, x) = rgb_t(pal5bit(pixel >> 11), pal6bit(pixel >> 5), pal5bit(pixel));
		}
	}

	// A second RGB565 plane has a separate packed, LSB-first opacity mask.
	// DMA constructs the plane in DRAM; the display controller composites it.
	// TODO: Configuration latch timing, signed positions and non-byte-aligned widths.
	if (BIT(m_render_unknown, 0) && m_overlay_width && m_overlay_height)
	{
		rectangle area(m_overlay_x, m_overlay_x + m_overlay_width - 1, m_overlay_y, m_overlay_y + m_overlay_height - 1);
		area &= cliprect;
		for (int y = area.min_y; y <= area.max_y; ++y)
		{
			for (int x = area.min_x; x <= area.max_x; ++x)
			{
				u32 const index = (y - m_overlay_y) * m_overlay_width + x - m_overlay_x;
				if (BIT(m_dram[(m_render_mask + (index >> 3)) & (DRAM_SIZE - 1)], index & 7))
				{
					u32 const address = m_render_overlay + index * 2;
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
			if (BIT(m_osd_codes_reg, 0) && x / 16 < m_render_columns && y / 20 < m_render_rows)
			{
				u16 const cell = (y / 20) * m_render_columns + x / 16;
				u32 const glyph = (m_render_font << 9) + m_osd_code[cell] * 80 + (y % 20) * 4 + (x % 16) / 4;
				u8 const ink = BIT(m_dram[glyph & (DRAM_SIZE - 1)], (x & 3) * 2, 2);
				// TODO: Decode the OSD palette, attributes and blending controls.
				if (ink)
					bitmap.pix(y, x) = rgb_t(ink * 85, ink * 85, ink * 85);
			}
		}
	}
	return 0;
}

void c2_color_state::clear_state()
{
	m_companion_sda = 1;
	std::fill(std::begin(m_dma_channel[0].m_dma_fill), std::end(m_dma_channel[0].m_dma_fill), 0);
	std::fill(std::begin(m_dma_channel[1].m_dma_fill), std::end(m_dma_channel[1].m_dma_fill), 0);

	for (int i = 0; i < 2; i++)
	{
		m_dma_channel[i].m_dma_trigger = 0;
		m_dma_channel[i].m_dma_source = 0;
		m_dma_channel[i].m_dma_dest = 0;
		m_dma_channel[i].m_dma_fill_pos = 0;
		m_dma_channel[i].m_dma_source_addr = 0;
		m_dma_channel[i].m_dma_dest_addr = 0;
		m_dma_channel[i].m_dma_count = 0;

		for (int j = 0; j < 4; j++)
		{
			m_dma_channel[i].m_dma_fill[j] = 0;
		}
	}

	m_spi_selected = -1;

	for (auto& table : m_quant)
		std::fill(std::begin(table), std::end(table), 0);
	m_quant_pos = 0;

	m_lcd_sleep = true;
	m_lcd_on = false;
	m_audio_address = m_audio_remaining = 0;
	m_audio_enabled = false;

	m_dram_dword_out_address = 0;
	m_dram_dword_in_address = 0;

	m_jpeg_src = 0;
	m_jpeg_dst = 0;
	m_jpeg_len = 0;
	m_jpeg_width = 0;
	m_jpeg_height = 0;

	m_render_overlay = 0;
	m_render_base = 0;
	m_render_mask = 0;
	m_overlay_width = 0;
	m_overlay_height = 0;
	m_overlay_x = 0;
	m_overlay_y = 0;

	m_render_osd0 = 0;
	m_render_osd1 = 0;
	m_render_osd2 = 0;
	m_render_osd3 = 0;
	m_render_columns = 0;
	m_render_rows = 0;
	m_render_unknown = 0;

	m_audio_remaining_reg = 0;
	m_audio_address_reg = 0;

	m_adc_reg1 = 0;
	m_adc_reg2 = 0;
	m_adc_reg3 = 0;
	m_adc_reg4 = 0;

}

void c2_color_state::machine_start()
{
	m_dram = std::make_unique<u8[]>(DRAM_SIZE);
	m_osd_code = std::make_unique<u16[]>(0x10000);
	m_osd_attr = std::make_unique<u8[]>(0x10000);

	clear_state();

	save_pointer(NAME(m_dram), DRAM_SIZE);
	save_pointer(NAME(m_osd_code), 0x10000);
	save_pointer(NAME(m_osd_attr), 0x10000);

	save_item(NAME(m_lcd_sleep));
	save_item(NAME(m_lcd_on));
	save_item(NAME(m_companion_sda));
	save_item(NAME(m_audio_address));
	save_item(NAME(m_audio_remaining));
	save_item(NAME(m_audio_enabled));
	save_item(NAME(m_dramstop));
	save_item(NAME(m_jpeg_decode_trigger));
	save_item(NAME(m_quant_ctrl));
	save_item(NAME(m_dramaccess_ctrl));
	save_item(NAME(m_spi_select_2152));
	save_item(NAME(m_spi_select_2155));
	save_item(NAME(m_spi_status));
	save_item(NAME(m_spi_exchange1));
	save_item(NAME(m_osd_codes_reg));
	save_item(NAME(m_audiocontrol_246d));
	save_item(NAME(m_audiocontrol_208c));
	save_item(NAME(m_audiocontrol_2097));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_adc_reg1));
	save_item(NAME(m_adc_reg2));
	save_item(NAME(m_adc_reg3));
	save_item(NAME(m_adc_reg4));
	save_item(NAME(m_spi_select_2042));
	save_item(NAME(m_companion_2002));
	save_item(NAME(m_buttons_reg));
	save_item(NAME(m_dram_dword_out_address));
	save_item(NAME(m_dram_dword_in_address));
	save_item(NAME(m_render_base));
	save_item(NAME(m_jpeg_dst));
	save_item(NAME(m_jpeg_src));
	save_item(NAME(m_jpeg_len));
	save_item(NAME(m_jpeg_width));
	save_item(NAME(m_jpeg_height));
	save_item(NAME(m_render_overlay));
	save_item(NAME(m_render_font));
	save_item(NAME(m_render_mask));
	save_item(NAME(m_overlay_width));
	save_item(NAME(m_overlay_height));
	save_item(NAME(m_overlay_x));
	save_item(NAME(m_overlay_y));
	save_item(NAME(m_render_osd0));
	save_item(NAME(m_render_osd1));
	save_item(NAME(m_render_osd2));
	save_item(NAME(m_render_osd3));
	save_item(NAME(m_render_columns));
	save_item(NAME(m_render_rows));
	save_item(NAME(m_render_unknown));
	save_item(NAME(m_audio_remaining_reg));
	save_item(NAME(m_audio_address_reg));
	save_item(NAME(m_xram_control));
	save_item(NAME(m_ram_access_upper));
	save_item(NAME(m_irqack));
	save_item(NAME(m_irqenable));
	save_item(NAME(m_irqstatus));
	save_item(NAME(m_spi_selected));
	save_item(NAME(m_quant));
	save_item(NAME(m_quant_pos));

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_dma_channel[i].m_dma_trigger), i);
		save_item(NAME(m_dma_channel[i].m_dma_count), i);
		save_item(NAME(m_dma_channel[i].m_dma_source_addr), i);
		save_item(NAME(m_dma_channel[i].m_dma_source), i);
		save_item(NAME(m_dma_channel[i].m_dma_dest_addr), i);
		save_item(NAME(m_dma_channel[i].m_dma_dest), i);
		save_item(NAME(m_dma_channel[i].m_dma_fill), i);
		save_item(NAME(m_dma_channel[i].m_dma_fill_pos), i);
	}

	m_audio_timer = timer_alloc(FUNC(c2_color_state::audio_tick), this);
	machine().save().register_postload(save_prepost_delegate(FUNC(c2_color_state::update_irq), this));

	for (unsigned i = 0; i != 2; ++i)
	{
		memory_region *const region = memregion(i ? "spi2" : "spi1");
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
	clear_state();
	m_audio_timer->adjust(attotime::never);
	m_dac->write(0x8000);
	m_ram_access_upper = 1;
	m_spi_select_2042 = 0x10;
	m_companion_2002 = 0x03;

	m_dramstop = 0;
	m_jpeg_decode_trigger = 0;
	m_quant_ctrl = 0;
	m_dramaccess_ctrl = 0;
	m_spi_select_2152 = 0x20;
	m_spi_select_2155 = 0;
	m_spi_status = 0;
	m_spi_exchange1 = 0;
	m_osd_codes_reg = 0;

	for (int i = 0; i < 4; i++)
	{
		m_irqack[i] = 0;
		m_irqenable[i] = 0;
		m_irqstatus[i] = 0;
	}

	update_irq();

	// The internal boot ROM is undumped.  Substitute its initial load of the
	// built-in firmware into DRAM, skipping the SPI image's 32-byte header.
	// The header specifies a 0x40000-byte initial load.
	// Subsequent resource transfers use the emulated SPI and DMA controllers.
	std::copy_n(m_flash_data[0].get() + 0x20, 0x40000, m_dram.get());
}

u8 c2_color_state::code_r(offs_t offset)
{
	if (BIT(m_xram_control, 0) && offset >= 0x4000 && offset < 0x4200)
		return m_xram[offset - 0x4000];
	u32 const address = offset < 0x8000 ? offset : ((u32)(m_ram_access_upper) << 15) | (offset & 0x7fff);
	return m_dram[address & (DRAM_SIZE - 1)];
}

void c2_color_state::spi_select()
{
	// The first flash uses an active-high select; the other two are active-low.
	s8 const selected = BIT(m_spi_select_2155, 5) ? 0 : !BIT(m_spi_select_2152, 5) ? 1 : !BIT(m_spi_select_2042, 4) ? 2 : -1;
	if (selected == m_spi_selected)
		return;
	for (unsigned i = 0; i != 2; ++i)
		m_flash[i]->cs_w(selected != int(i));
	m_spi_selected = selected;
	LOGMASKED(LOG_SPI, "%s: SPI select %d\n", machine().describe_context(), selected);
}

u8 c2_color_state::spi_exchange(u8 data)
{
	if (m_spi_selected < 0 || (m_spi_selected == 2))
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
	u8 const source = m_dma_channel[channel].m_dma_source & 0x0f;
	u8 const destination = m_dma_channel[channel].m_dma_dest & 0x0f;
	u32 const source_address = m_dma_channel[channel].m_dma_source_addr;
	u32 const destination_address = m_dma_channel[channel].m_dma_dest_addr;
	u32 const count = m_dma_channel[channel].m_dma_count;
	bool const fill = BIT(m_dma_channel[channel].m_dma_trigger, 1);
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
	{
		dma_w(destination, destination_address + i, fill ? m_dma_channel[channel].m_dma_fill[i & 3] : dma_r(source, source_address + i));
	}

	m_irqstatus[0] |= 0x40 << channel;
	if (destination == 2 || destination == 3)
	{
		m_irqstatus[0] |= 0x01;
		m_irqstatus[1] |= 0x40;
	}
	update_irq();
}

void c2_color_state::dram_access(u8 data)
{
	// Firmware first writes 03, then requests a four-byte read or write.
	if (data == 0x07)
	{
		u32 const address = m_dram_dword_out_address;
		for (unsigned i = 0; i != 4; ++i)
			m_dram[(address + i) & (DRAM_SIZE - 1)] = m_dram_dword_out_data[i];
		m_dramaccess_ctrl |= 0x08;
	}
	else if (data == 0x13)
	{
		u32 const address = m_dram_dword_in_address;
		for (unsigned i = 0; i != 4; ++i)
			m_dram_dword_in_data[i] = m_dram[(address + i) & (DRAM_SIZE - 1)];
		m_dramaccess_ctrl |= 0x20;
	}
}

void c2_color_state::jpeg_decode()
{
	u16 const width = m_jpeg_width & 0xffff;
	u16 const height = m_jpeg_height & 0xffff;

	if (!width || !height || u32(width) * height > DRAM_SIZE / 2 || !m_jpeg_len || m_jpeg_len > DRAM_SIZE)
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
	u8 const header[] = { 0xff, 0xc0, 0x00, 0x11, 0x08, u8(height >> 8), u8(height), u8(width >> 8), u8(width), 0x03, 0x01, 0x21, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xff, 0xda, 0x00, 0x0c, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3f, 0x00 };
	stream.insert(stream.end(), std::begin(header), std::end(header));

	for (u32 i = 0; i != m_jpeg_len; ++i)
		stream.push_back(m_dram[(m_jpeg_src + i) & (DRAM_SIZE - 1)]);

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
			u32 const address = m_jpeg_dst + (y * width + x) * 2;
			m_dram[address & (DRAM_SIZE - 1)] = pixel;
			m_dram[(address + 1) & (DRAM_SIZE - 1)] = pixel >> 8;
		}
	}
	m_irqstatus[1] |= 0x30;
	update_irq();
}

u8 c2_color_state::read_reg_swapped(auto &reg, offs_t offset)
{
	return (reg >> (offset * 8)) & 0xff;
}

void c2_color_state::write_reg_swapped(auto &reg, offs_t offset, u8 data)
{
	auto shifteddata = data << (offset * 8);
	auto shiftedmask = 0xff << (offset * 8);
	reg = (reg & ~shiftedmask) | shifteddata;
}

void c2_color_state::update_irq()
{
	m_maincpu->set_input_line(MCS51_INT0_LINE, ((m_irqstatus[0] & m_irqenable[0]) | (m_irqstatus[1] & m_irqenable[1])) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(MCS51_INT1_LINE, ((m_irqstatus[2] & m_irqenable[2]) | (m_irqstatus[3] & m_irqenable[3])) ? ASSERT_LINE : CLEAR_LINE);
}

void c2_color_state::audio_control()
{
	bool const enabled = BIT(m_audiocontrol_208c, 0) && BIT(m_audiocontrol_2097, 1) && BIT(m_audiocontrol_246d, 1);
	if (enabled && !m_audio_enabled)
	{
		// Addresses and lengths are in 16-bit samples.  The observed setting
		// (20ab bits 2:1 clear) plays signed little-endian mono PCM at 8 kHz.
		// TODO: Other rates, formats, volume and output filtering.
		m_audio_address = m_audio_address_reg * 2;
		m_audio_remaining = m_audio_remaining_reg;
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
		m_irqstatus[3] |= 0x10;
		update_irq();
	}
}

void c2_color_state::companion_w(u8 data)
{
	m_companion_2002 = data;
	if (!BIT(data, 0))
		m_companion->scl_write(0);
	m_companion->sda_write(BIT(data, 1));
	if (BIT(data, 0))
		m_companion->scl_write(1);
}

void c2_color_state::timer_ctrl_w(u8 data)
{
	m_timer_ctrl = data;
	if (BIT(data, 1))
	{
		// The millisecond unit is inferred from the LCD Sleep Out delay.
		// TODO: Identify the timer clock and divider controls.
		u32 const ticks = machine().time().as_ticks(1000);
		for (unsigned i = 0; i != 4; ++i)
			m_timer_val[i] = ticks >> (8 * i);
		m_timer_ctrl &= ~0x02;
	}
}

void c2_color_state::adc_w(u8 data)
{
	u8 previous = m_adc_reg2;

	m_adc_reg2 = (data & ~9) | (previous & 8);
	if (BIT(data, 0))
	{
		// Channel 0 measures the batteries.  Voltage scaling, other inputs
		// and conversion timing are unknown; use representative raw levels.
		u16 const sample = (m_adc_reg1 & 3) == 0 ? m_battery->read() : 0;
		m_adc_reg3 = sample >> 8;
		m_adc_reg4 = sample;
		m_adc_reg2 |= 8;
	}
}

void c2_color_state::osd_codes_w(u8 data)
{
	u8 previous = m_osd_codes_reg;
	m_osd_codes_reg = data;

	// The two table-write strobes are armed separately before both go high.
	if ((data & 6) == 6)
	{
		if (!(previous & 2))
			m_osd_code[m_render_osd0] = m_render_osd1;
		if (!(previous & 4))
			m_osd_attr[m_render_osd2] = m_render_osd3;
		m_osd_codes_reg &= ~6;
	}

}

void c2_color_state::lcd_ctrl_w(u8 data)
{
	switch (data)
	{
	case 0x10: m_lcd_sleep = true; break;
	case 0x11: m_lcd_sleep = false; break;
	case 0x28: m_lcd_on = false; break;
	case 0x29: m_lcd_on = true; break;
	}
}

void c2_color_state::jpeg_decode_trigger_w(u8 data)
{
	u8 previous = m_jpeg_decode_trigger;
	m_jpeg_decode_trigger = data;

	if ((data & 0x18) == 0x18 && (previous & 0x18) != 0x18)
		jpeg_decode();
}

void c2_color_state::quant_ctrl_w(u8 data)
{
	u8 previous = m_quant_ctrl;
	m_quant_ctrl = data;

	if (BIT(previous, 3) && !BIT(data, 3))
		m_quant_pos = 0;
}

void c2_color_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(c2_color_state::code_r));
}


template<int Channel> void c2_color_state::dma_fill_w(offs_t offset, u8 data)
{
	m_dma_channel[Channel].m_dma_fill[m_dma_channel[Channel].m_dma_fill_pos++ & 3] = data;
}

template<int Channel> void c2_color_state::dma_trigger_w(offs_t offset, u8 data)
{
	m_dma_channel[Channel].m_dma_trigger = data;
	if (data == 2)
		m_dma_channel[Channel].m_dma_fill_pos = 0;
	if (BIT(data, 0))
		dma(Channel);
}

template<int Channel> u8 c2_color_state::dma_unk_r(offs_t offset)
{
	logerror("%s: unhandled DMA read address Channel %d, Offset %02x\n", Channel, offset);
	return 0x00;
}

template<int Channel> void c2_color_state::dma_unk_w(offs_t offset, u8 data)
{
	logerror("%s: unhandled DMA write address Channel %d, Offset %02x Data %02x\n", Channel, offset, data);
}

// 0x2200 - 0x2239 for Channel 0
// 0x223a - 0x224f for Channel 1
template<int Channel> void c2_color_state::add_dma_map(address_map &map, int base)
{
	map(base + 0x00, base + 0x39).rw(FUNC(c2_color_state::dma_unk_r<Channel>), FUNC(c2_color_state::dma_unk_w<Channel>));

	map(base + 0x00, base + 0x00).rw(FUNC(c2_color_state::dma_trigger_r<Channel>), FUNC(c2_color_state::dma_trigger_w<Channel>));
	map(base + 0x01, base + 0x04).rw(FUNC(c2_color_state::dma_count_r<Channel>), FUNC(c2_color_state::dma_count_w<Channel>));
	map(base + 0x0d, base + 0x0d).w(FUNC(c2_color_state::dma_fill_w<Channel>));

	map(base + 0x10, base + 0x10).rw(FUNC(c2_color_state::dma_source_r<Channel>), FUNC(c2_color_state::dma_source_w<Channel>));

	map(base + 0x12, base + 0x15).rw(FUNC(c2_color_state::dma_source_addr_r<Channel>), FUNC(c2_color_state::dma_source_addr_w<Channel>));

	map(base + 0x25, base + 0x25).rw(FUNC(c2_color_state::dma_dest_r<Channel>), FUNC(c2_color_state::dma_dest_w<Channel>));

	map(base + 0x27, base + 0x2a).rw(FUNC(c2_color_state::dma_dest_addr_r<Channel>), FUNC(c2_color_state::dma_dest_addr_w<Channel>));

}

void c2_color_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("xram");

	//////////////////////////////////////////////
	// 0x2000 region
	//////////////////////////////////////////////

	map(0x2002, 0x2002).rw(FUNC(c2_color_state::companion_r), FUNC(c2_color_state::companion_w));

	//map(0x2004, 0x2004).ram();
	//map(0x200a, 0x200b).ram();

	map(0x2024, 0x2027).rw(FUNC(c2_color_state::jpeg_width_r), FUNC(c2_color_state::jpeg_width_w)); // we only use 16-bits

	map(0x202a, 0x202d).rw(FUNC(c2_color_state::jpeg_height_r), FUNC(c2_color_state::jpeg_height_w)); // we only use 16-bits

	//map(0x2028, 0x2029).nopw();

	//map(0x202e, 0x202f).nopw();

	//map(0x203a, 0x203c).ram();

	//map(0x203f, 0x2041).ram();

	map(0x2042, 0x2042).rw(FUNC(c2_color_state::spi_select_2042_r), FUNC(c2_color_state::spi_select_2042_w));

	//map(0x204b, 0x204c).ram();

	//map(0x2051, 0x2051).ram();

	map(0x2053, 0x2053).rw(FUNC(c2_color_state::buttons_r), FUNC(c2_color_state::buttons_w));

	//map(0x205a, 0x205c).nopr(); // 205a is only read?

	map(0x205f, 0x2062).ram().share("timer_val");

	map(0x2064, 0x2064).rw(FUNC(c2_color_state::timer_ctrl_r), FUNC(c2_color_state::timer_ctrl_w));

	//map(0x2065, 0x2065).ram();
	//map(0x2066, 0x2066).nopw();
	//map(0x2067, 0x2067).ram();

	map(0x208c, 0x208c).rw(FUNC(c2_color_state::audiocontrol_208c_r), FUNC(c2_color_state::audiocontrol_208c_w));

	//map(0x208d, 0x208d).ram();

	//map(0x2093, 0x2093).nopw();

	map(0x2097, 0x2097).rw(FUNC(c2_color_state::audiocontrol_2097_r), FUNC(c2_color_state::audiocontrol_2097_w));

	map(0x2099, 0x209b).rw(FUNC(c2_color_state::audio_remaining_r), FUNC(c2_color_state::audio_remaining_w));
	map(0x20a5, 0x20a7).rw(FUNC(c2_color_state::audio_address_r), FUNC(c2_color_state::audio_address_w));

	//map(0x20ab, 0x20ab).ram();
	map(0x20ac, 0x20ac).rw(FUNC(c2_color_state::adc_reg1_r), FUNC(c2_color_state::adc_reg1_w));
	map(0x20ad, 0x20ad).rw(FUNC(c2_color_state::adc_r), FUNC(c2_color_state::adc_w));
	map(0x20ae, 0x20ae).rw(FUNC(c2_color_state::adc_reg3_r), FUNC(c2_color_state::adc_reg3_w));
	map(0x20af, 0x20af).rw(FUNC(c2_color_state::adc_reg4_r), FUNC(c2_color_state::adc_reg4_w));

	//map(0x20b6, 0x20b9).nopw();

	//////////////////////////////////////////////
	// 0x2100 region
	//////////////////////////////////////////////

	//map(0x2140, 0x2140).ram();
	map(0x2141, 0x2141).rw(FUNC(c2_color_state::xram_control_r), FUNC(c2_color_state::xram_control_w));
	//map(0x2142, 0x2142).ram();

	map(0x2144, 0x2144).rw(FUNC(c2_color_state::ram_access_upper_r), FUNC(c2_color_state::ram_access_upper_w));

	// first IRQ group
	map(0x2145, 0x2145).rw(FUNC(c2_color_state::irqenable_r<0>), FUNC(c2_color_state::irqenable_w<0>));
	map(0x2146, 0x2146).rw(FUNC(c2_color_state::irqenable_r<1>), FUNC(c2_color_state::irqenable_w<1>));
	map(0x2147, 0x2147).rw(FUNC(c2_color_state::irqstatus_r<0>), FUNC(c2_color_state::irqstatus_w<0>));
	map(0x2148, 0x2148).rw(FUNC(c2_color_state::irqstatus_r<1>), FUNC(c2_color_state::irqstatus_w<1>));
	map(0x2149, 0x2149).rw(FUNC(c2_color_state::irqack_r<0>), FUNC(c2_color_state::irqack_w<0>));
	map(0x214a, 0x214a).rw(FUNC(c2_color_state::irqack_r<1>), FUNC(c2_color_state::irqack_w<1>));
	// second IRQ group
	map(0x214b, 0x214b).rw(FUNC(c2_color_state::irqenable_r<2>), FUNC(c2_color_state::irqenable_w<2>));
	map(0x214c, 0x214c).rw(FUNC(c2_color_state::irqenable_r<3>), FUNC(c2_color_state::irqenable_w<3>));
	map(0x214d, 0x214d).rw(FUNC(c2_color_state::irqstatus_r<2>), FUNC(c2_color_state::irqstatus_w<2>));
	map(0x214e, 0x214e).rw(FUNC(c2_color_state::irqstatus_r<3>), FUNC(c2_color_state::irqstatus_w<3>));
	map(0x214f, 0x214f).rw(FUNC(c2_color_state::irqack_r<2>), FUNC(c2_color_state::irqack_w<2>));
	map(0x2150, 0x2150).rw(FUNC(c2_color_state::irqack_r<3>), FUNC(c2_color_state::irqack_w<3>));

	//map(0x2151, 0x2151).nopw();

	map(0x2152, 0x2152).rw(FUNC(c2_color_state::spi_select_2152_r), FUNC(c2_color_state::spi_select_2152_w));

	//map(0x2154, 0x2154).ram();
	map(0x2155, 0x2155).rw(FUNC(c2_color_state::spi_select_2155_r), FUNC(c2_color_state::spi_select_2155_w));
	map(0x2156, 0x2156).rw(FUNC(c2_color_state::spi_status_r), FUNC(c2_color_state::spi_status_w));

	map(0x2157, 0x2157).w(FUNC(c2_color_state::spi_exchange0_w)); /* Bit 6 of 2155 also enables a debug output stream on this port,  With no flash selected those bytes do not enter a flash command parser. */
	map(0x2158, 0x2158).rw(FUNC(c2_color_state::spi_exchange1_r), FUNC(c2_color_state::spi_exchange1_w));
	//map(0x215c, 0x215d).ram();

	//map(0x2184, 0x2184).ram();
	map(0x2185, 0x2185).rw(FUNC(c2_color_state::osd_codes_r), FUNC(c2_color_state::osd_codes_w));

	map(0x2186, 0x2186).rw(FUNC(c2_color_state::render_columns_r), FUNC(c2_color_state::render_columns_w));
	map(0x2187, 0x2187).rw(FUNC(c2_color_state::render_rows_r), FUNC(c2_color_state::render_rows_w));

	//map(0x2188, 0x2288).ram();
	//map(0x218a, 0x228c).ram();
	
	//map(0x228d, 0x229c).nopw();
	map(0x219d, 0x219e).rw(FUNC(c2_color_state::render_osd0_r), FUNC(c2_color_state::render_osd0_w));
	map(0x219f, 0x21a0).rw(FUNC(c2_color_state::render_osd1_r), FUNC(c2_color_state::render_osd1_w));
	map(0x21a1, 0x21a2).rw(FUNC(c2_color_state::render_osd2_r), FUNC(c2_color_state::render_osd2_w));
	map(0x21a3, 0x21a3).rw(FUNC(c2_color_state::render_osd3_r), FUNC(c2_color_state::render_osd3_w));
	map(0x21a4, 0x21a5).rw(FUNC(c2_color_state::render_font_r), FUNC(c2_color_state::render_font_w));
	//map(0x21a6, 0x21bb).nopw();

	//map(0x21bf, 0x21bf).nopw();
	map(0x21c0, 0x21c0).w(FUNC(c2_color_state::lcd_ctrl_w));

	//map(0x21c7, 0x21c7).ram();

	//////////////////////////////////////////////
	// 0x2200 region
	//////////////////////////////////////////////

	add_dma_map<0>(map, 0x2200);
	add_dma_map<1>(map, 0x223a);

	//map(0x229a, 0x229a).nopw();
	map(0x229b, 0x229b).rw(FUNC(c2_color_state::quant_ctrl_r), FUNC(c2_color_state::quant_ctrl_w));

	map(0x229d, 0x229d).w(FUNC(c2_color_state::quant_data_w));

	//map(0x22a1, 0x22a1).ram();

	//////////////////////////////////////////////
	// 0x2300 region
	//////////////////////////////////////////////

	//map(0x2345, 0x2345).ram();
	//map(0x2336, 0x233c).nopw();
	//map(0x234d, 0x234d).ram();

	//////////////////////////////////////////////
	// 0x2400 region
	//////////////////////////////////////////////

	map(0x2400, 0x2400).rw(FUNC(c2_color_state::dramstop_r), FUNC(c2_color_state::dramstop_w));

	map(0x2402, 0x2402).w(FUNC(c2_color_state::jpeg_decode_trigger_w));

	//map(0x2403, 0x2404).ram();
	map(0x2405, 0x2405).rw(FUNC(c2_color_state::dramaccess_ctrl_r), FUNC(c2_color_state::dramaccess_ctrl_w));
	//map(0x2406, 0x2407).nopw();

	map(0x2429, 0x242c).ram().share("dram_dword_out_data");
	map(0x242d, 0x2430).rw(FUNC(c2_color_state::dram_dword_out_address_r), FUNC(c2_color_state::dram_dword_out_address_w));

	map(0x2431, 0x2434).ram().share("dram_dword_in_data");
	map(0x2435, 0x2438).ram().share("dram_dword_in_address");

	//map(0x2446, 0x2449).nopw();

	map(0x244a, 0x244c).rw(FUNC(c2_color_state::jpeg_dst_r), FUNC(c2_color_state::jpeg_dst_w));
	//map(0x244d, 0x244d).ram();
	map(0x244e, 0x2451).rw(FUNC(c2_color_state::jpeg_src_r), FUNC(c2_color_state::jpeg_src_w));
	map(0x2452, 0x2454).rw(FUNC(c2_color_state::jpeg_len_r), FUNC(c2_color_state::jpeg_len_w));
	//map(0x2455, 0x2455).ram();
	
	//map(0x2456, 0x245f).nopw();

	map(0x2460, 0x2462).rw(FUNC(c2_color_state::render_base_r), FUNC(c2_color_state::render_base_w));

	//map(0x2464, 0x2466).nopw();

	//map(0x2468, 0x246a).nopw();

	//map(0x246c, 0x246c).nopw();

	map(0x246d, 0x246d).rw(FUNC(c2_color_state::audiocontrol_246d_r), FUNC(c2_color_state::audiocontrol_246d_w)); // seems out of place for audiocontrol
	
	map(0x246e, 0x246e).rw(FUNC(c2_color_state::render_unknown_r), FUNC(c2_color_state::render_unknown_w));

	map(0x246f, 0x2472).rw(FUNC(c2_color_state::render_overlay_r), FUNC(c2_color_state::render_overlay_w));
	map(0x2473, 0x2476).rw(FUNC(c2_color_state::render_mask_r), FUNC(c2_color_state::render_mask_w));
	map(0x2477, 0x2478).rw(FUNC(c2_color_state::overlay_width_r), FUNC(c2_color_state::overlay_width_w));
	map(0x2479, 0x247a).rw(FUNC(c2_color_state::overlay_height_r), FUNC(c2_color_state::overlay_height_w));
	map(0x247b, 0x247c).rw(FUNC(c2_color_state::overlay_x_r), FUNC(c2_color_state::overlay_x_w));
	map(0x247d, 0x247e).rw(FUNC(c2_color_state::overlay_y_r), FUNC(c2_color_state::overlay_y_w));

	//map(0x24a4, 0x24a4).ram();

	//////////////////////////////////////////////
	// 0x2500 region
	//////////////////////////////////////////////

	//map(0x2541, 0x2541).ram();
	map(0x2542, 0x2546).nopw();

	//map(0x254a, 0x254a).ram();

	map(0x256b, 0x256e).nopw();

	map(0x25e3, 0x25e6).nopw();
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

	GENERIC_SPI_FLASH(config, m_flash[0]);
	GENERIC_SPI_FLASH(config, m_flash[1]);

	C2_COLOR_COMPANION(config, m_companion);
	m_companion->sda_callback().set([this] (int state) { m_companion_sda = state; });

	C2COLOR_CARTSLOT(config, "cartslot", c2color_plain_slot);

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
