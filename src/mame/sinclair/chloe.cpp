// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
	Chloe 280SE
**********************************************************************/

#include "emu.h"

#include "screen_ula.h"
#include "spec128.h"

#include "machine/spi_sdcard.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

#include "screen.h"
#include "speaker.h"


#define LOG_IO    (1U << 1)
#define LOG_MEM   (1U << 2)
#define LOG_WARN  (1U << 3)

#define VERBOSE ( /*LOG_GENERAL | LOG_IO | LOG_MEM |*/ LOG_WARN )
#include "logmacro.h"

#define LOGIO(...)    LOGMASKED(LOG_IO,    __VA_ARGS__)
#define LOGMEM(...)   LOGMASKED(LOG_MEM,   __VA_ARGS__)
#define LOGWARN(...)  LOGMASKED(LOG_WARN,  __VA_ARGS__)


namespace {

#define TIMINGS_PERFECT     0

// Must be 800x525 to match VGA. With below puts odd scanlines to the right in order to avoid the same line redrawing.
static const u16 CYCLES_HORIZ = 800 << 1;
static const u16 CYCLES_VERT = 525 >> 1;
static const rectangle SCR_FULL = { 0, 640 - 1, 0, 240 - 1 };
static const rectangle SCR_256x192 = {  64, 64 + (256 << 1) - 1, 24, 24 + 192 - 1 };

class chloe_state : public spectrum_128_state
{
public:
	chloe_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bank_ram(*this, "bank_ram%u", 0U)
		, m_bank0_view(*this, "bank0_view")
		, m_bank1_view(*this, "bank1_view")
		, m_regs_map(*this, "regs_map")
		, m_palette(*this, "palette")
		, m_ula(*this, "ula")
		, m_sdcard(*this, "sdcard")
		, m_io_line(*this, "IO_LINE%u", 0U)
		, m_io_mouse(*this, "mouse_input%u", 1U)
		, m_ay(*this, "ay%u", 0U)
		, m_covox(*this, "covox")
	{}

	void chloe(machine_config &config);

	INPUT_CHANGED_MEMBER(on_divmmc_nmi);

protected:
	static const u8 BASIC48_ROM = 0x01;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override;

	void map_regs(address_map &map);
	void map_fetch(address_map &map);
	void map_mem(address_map &map);
	void map_io(address_map &map);

	u8 kbd_fe_r(offs_t offset);
	u8 divmmc_neutral_r(offs_t offset);
	u8 divmmc_enable_r(offs_t offset);
	u8 divmmc_disable_r(offs_t offset);
	void dma_reg_w(offs_t offset, u8 data);
	void port_7ffd_w(u8 data);
	void port_f4_w(u8 data);
	void port_ff_w(u8 data);
	void port_e3_w(u8 data);
	void ay_address_w(u8 data);
	u8 spi_data_r();
	void spi_data_w(u8 data);
	void spi_miso_w(u8 data);

	void update_memory();
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void raster_irq_adjust();

private:
	TIMER_CALLBACK_MEMBER(spi_clock);
	TIMER_CALLBACK_MEMBER(raster_irq_on);
	TIMER_CALLBACK_MEMBER(raster_irq_off);
	INTERRUPT_GEN_MEMBER(chloe_interrupt);

	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_uno_regs;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
	memory_bank_array_creator<8> m_bank_ram;
	memory_view m_bank0_view, m_bank1_view;
	required_device<address_map_bank_device> m_regs_map;
	required_device<device_palette_interface> m_palette;
	required_device<screen_ula_plus_device> m_ula;
	required_device<spi_sdcard_sdhc_device> m_sdcard;
	required_ioport_array<8> m_io_line;
	required_ioport_array<3> m_io_mouse;
	required_device_array<ay8912_device, 2> m_ay;
	required_device<dac_byte_interface> m_covox;

	u8 m_timex_mmu;
	u8 m_port_ff_data;
	bool m_core_boot;
	u8 m_reg_selected;
	bool m_divmmc_paged;
	u8 m_divmmc_ctrl;
	u8 m_uno_regs_data[256];
	u8 m_palpen_selected;
	u8 m_ay_selected;
	bool m_dma_hilo;
	u8 m_dma_src_latch;
	u8 m_dma_dst_latch;
	u8 m_dma_pre_latch;
	u8 m_dma_len_latch;
	u8 m_dma_prob_latch;

	emu_timer *m_irq_raster_on_timer;
	emu_timer *m_irq_raster_off_timer;

	emu_timer *m_spi_clock;
	int m_spi_clock_cycles;
	bool m_spi_clock_state;
	u8 m_spi_mosi_dat;
	u8 m_spi_miso_dat;
};


void chloe_state::update_memory()
{
	m_screen->update_now();
	m_ula->ula_shadow_en_w(BIT(m_port_7ffd_data, 3));

	const bool ext = BIT(m_port_ff_data, 7); // 0 - DOC 7xxxx=28+; 1 - EXT 6xxxx=24+
	m_bank0_view.disable();
	m_bank1_view.disable();
	const u8 divmmc_sram_page = BIT(m_divmmc_ctrl, 0, 6);
	const bool divmmc_sram_page_is_valid = BIT(divmmc_sram_page, 4, 2) == 0;
	const bool mapram_mode = BIT(m_divmmc_ctrl, 6);
	const bool conmem = BIT(m_divmmc_ctrl, 7);
	const bool divmmc_rom_active = m_divmmc_paged || conmem;
	for (auto i = 0; i < 8; ++i)
	{
		const bool paged = BIT(m_timex_mmu, i);

		u8 pg;
		if (i == 0 && (divmmc_rom_active || !paged))
		{
			if (divmmc_rom_active)
			{
				pg = (!mapram_mode || conmem) ? 24 : 35;
			}
			else
			{
				pg = (8 + BIT(m_port_7ffd_data, 4)) << 1;
			}
			m_bank0_view.select(0);
			m_bank_ram[0]->set_entry(pg);
		}
		else if (i == 1 && (divmmc_rom_active || !paged))
		{
			if (divmmc_rom_active)
			{
				pg = 32 + (divmmc_sram_page & 0x0f);
				if (!mapram_mode || conmem)
				{
					if (!divmmc_sram_page_is_valid)
					{
						m_bank1_view.select(0);
					}
				}
				else
				{
					if ((mapram_mode && (divmmc_sram_page == 3)) || !divmmc_sram_page_is_valid)
					{
						m_bank1_view.select(0);
					}
				}
			}
			else
			{
				pg = ((8 + BIT(m_port_7ffd_data, 4)) << 1) + 1;
				m_bank1_view.select(0);
			}
			m_bank_ram[1]->set_entry(pg);
		}
		else if (paged)
		{
			pg = ((28 - 4 * ext) << 1) + i;
			m_bank_ram[i]->set_entry(pg);
		}
		else
		{
			if (i < 4)
			{
				pg = 5;
			}
			else if (i < 6)
			{
				pg = 2;
			}
			else
			{
				pg = m_port_7ffd_data & 0x07;
			}

			m_bank_ram[i]->set_entry((pg << 1) + (i & 1));
		}
	}
}

u32 chloe_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip256x192 = SCR_256x192;
	clip256x192 &= cliprect;

	screen.priority().fill(0, cliprect);
	m_ula->draw_border(bitmap, cliprect, m_port_fe_data & 0x07);

	const bool flash = u64(screen.frame_number() / m_frame_invert_count) & 1;
	m_ula->draw(screen, bitmap, clip256x192, flash, 0);

	return 0;
}

void chloe_state::port_7ffd_w(u8 data)
{
	if (m_port_7ffd_data & 0x20)
	{
		return;
	}

	m_port_7ffd_data = data;
	update_memory();
}

void chloe_state::port_ff_w(u8 data)
{
	m_ula->port_ff_reg_w(data);

	m_port_ff_data = data;
	update_memory();
}

void chloe_state::port_f4_w(u8 data)
{
	m_timex_mmu = data;
	update_memory();
}

void chloe_state::port_e3_w(u8 data)
{
	if (m_divmmc_ctrl & 0x40)
	{
		m_divmmc_ctrl = data | 0x40;
	}
	else
	{
		m_divmmc_ctrl = data;
	}
	update_memory();
}

void chloe_state::ay_address_w(u8 data)
{
	if ((data & 0xfe) == 0xfe)
	{
		m_ay_selected = data & 1;
	}
	else
	{
		m_ay[m_ay_selected]->address_w(data);
	}
}

u8 chloe_state::spi_data_r()
{
	u8 din = m_spi_miso_dat;
	if (!machine().side_effects_disabled())
	{
		spi_data_w(0xff);
	}

	return din;
}

void chloe_state::spi_data_w(u8 data)
{
	m_spi_mosi_dat = data;
#if TIMINGS_PERFECT
	m_spi_clock_cycles = 8;
	m_spi_clock->adjust(m_maincpu->clocks_to_attotime(1) / 4, 0, m_maincpu->clocks_to_attotime(1) / 4);
#else
	for (u8 m = 0x80; m; m >>= 1)
	{
		m_sdcard->spi_mosi_w(m_spi_mosi_dat & m ? 1 : 0);
		m_sdcard->spi_clock_w(CLEAR_LINE);
		m_sdcard->spi_clock_w(ASSERT_LINE);
	}
#endif
}

void chloe_state::spi_miso_w(u8 data)
{
	m_spi_miso_dat <<= 1;
	m_spi_miso_dat |= data;
}

TIMER_CALLBACK_MEMBER(chloe_state::spi_clock)
{
	if (m_spi_clock_cycles > 0)
	{
		if (m_spi_clock_state)
		{
			m_sdcard->spi_clock_w(ASSERT_LINE);
			m_spi_clock_cycles--;
		}
		else
		{
			m_sdcard->spi_mosi_w(BIT(m_spi_mosi_dat, m_spi_clock_cycles - 1));
			m_sdcard->spi_clock_w(CLEAR_LINE);
		}

		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		m_spi_clock_state = false;
		m_spi_clock->reset();
	}
}


u8 chloe_state::divmmc_neutral_r(offs_t offset)
{
	return m_program.read_byte(offset);
}

u8 chloe_state::divmmc_enable_r(offs_t offset)
{
	// M1
	if (!machine().side_effects_disabled() && !m_divmmc_paged && (offset >= 0x3d00))
	{
		m_divmmc_paged = 1;
		update_memory();
	}
	const u8 op = m_program.read_byte(offset);
	// after M1
	if (!machine().side_effects_disabled() && !m_divmmc_paged)
	{
		m_divmmc_paged = 1;
		update_memory();
	}
	return op;
}

u8 chloe_state::divmmc_disable_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && m_divmmc_paged && (offset >= 0x4000))
	{
		m_divmmc_paged = 0;
		update_memory();
	}
	const u8 op = m_program.read_byte(offset);
	if (!machine().side_effects_disabled() && m_divmmc_paged)
	{
		m_divmmc_paged = 0;
		update_memory();
	}
	return op;
}

void chloe_state::dma_reg_w(offs_t offset, u8 data)
{
	// TODO dma stub
	m_uno_regs_data[0xa0 + offset] = data;
	m_dma_hilo = !m_dma_hilo;

	switch (offset + 10 * m_dma_hilo)
	{
	case 0: case 10: // DMACTRL
		// 07;
		break;
	case  1: // DMASRC
		//m_dma->((m_dma_src_latch << 8) | data);
		break;
	case 11:
		m_dma_src_latch = data;
		break;
	case  2: // DMADST
		//m_dma->((m_dma_dst_latch << 8) | data);
		break;
	case 12:
		m_dma_dst_latch = data;
		break;
	case  3: // DMAPRE
		//m_dma->((m_dma_pre_latch << 8) | data);
		break;
	case 13:
		m_dma_pre_latch = data;
		break;
	case  4: // DMALEN
		//m_dma->((m_dma_len_latch << 8) | data);
		break;
	case 14:
		m_dma_len_latch = data;
		break;
	case  5: // DMAPROB
		//m_dma->((m_dma_prob_latch << 8) | data);
		break;
	case 15:
		m_dma_prob_latch = data;
		break;
	case 6: case 16: // DMASTAT
		break;
	}
}

void chloe_state::raster_irq_adjust()
{
	if (BIT(m_uno_regs_data[0x0d], 1))
	{
		u16 line = (BIT(m_uno_regs_data[0x0d], 0) << 8) | m_uno_regs_data[0x0c];
		m_irq_raster_on_timer->adjust(m_screen->time_until_pos((SCR_256x192.top() + line) % m_screen->height()));
	}
	else
		m_irq_raster_on_timer->reset();
}

INPUT_CHANGED_MEMBER(chloe_state::on_divmmc_nmi)
{
	if ((newval & 1) && (~m_io_line[0]->read() & 0x8000))
	{
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

TIMER_CALLBACK_MEMBER(chloe_state::raster_irq_on)
{
	m_screen->update_now();
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	m_irq_raster_off_timer->adjust(m_maincpu->clocks_to_attotime(32));
}

TIMER_CALLBACK_MEMBER(chloe_state::raster_irq_off)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	raster_irq_adjust();
}

INTERRUPT_GEN_MEMBER(chloe_state::chloe_interrupt)
{
	if (BIT(~m_uno_regs_data[0x0d], 2))
	{
		m_irq_on_timer->adjust(m_screen->time_until_pos(SCR_256x192.top(), SCR_256x192.left())
			- attotime::from_ticks(14365, m_maincpu->unscaled_clock())); // TODO confirm
	}
}

void chloe_state::map_fetch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(chloe_state::divmmc_neutral_r));
	map(0x0000, 0x0000).lr8(NAME([this]() { return divmmc_enable_r(0x0000); }));
	map(0x0008, 0x0008).lr8(NAME([this]() { return divmmc_enable_r(0x0008); }));
	map(0x0038, 0x0038).lr8(NAME([this]() { return divmmc_enable_r(0x0038); }));
	map(0x0066, 0x0066).lr8(NAME([this]() { return divmmc_enable_r(0x0066); }));
	map(0x04c6, 0x04c6).lr8(NAME([this]() { return divmmc_enable_r(0x04c6); }));
	map(0x0562, 0x0562).lr8(NAME([this]() { return divmmc_enable_r(0x0562); }));
	map(0x1ff8, 0x1fff).lr8(NAME([this](offs_t offset) { return divmmc_disable_r(0x1ff8 + offset); }));
	map(0x3d00, 0x3dff).lr8(NAME([this](offs_t offset) { return divmmc_enable_r(0x3d00 + offset); }));
	map(0x4000, 0xffff).lr8(NAME([this](offs_t offset) { return divmmc_disable_r(0x4000 + offset); }));
}

void chloe_state::map_mem(address_map &map)
{
	for (auto i = 0; i < 8; i++)
		map(0x0000 + i * 0x2000, 0x1fff + i * 0x2000).bankrw(m_bank_ram[i]);

	map(0x0000, 0x1fff).view(m_bank0_view);
	m_bank0_view[0](0x0000, 0x1fff).nopw();
	map(0x2000, 0x3fff).view(m_bank1_view);
	m_bank1_view[0](0x2000, 0x3fff).nopw();
}

void chloe_state::map_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).select(0xfffe).rw(FUNC(chloe_state::kbd_fe_r), FUNC(chloe_state::spectrum_ula_w));
	map(0x00ff, 0x00ff).mirror(0xff00).lr8(NAME([this]() { return m_port_ff_data; })).w(FUNC(chloe_state::port_ff_w));
	map(0x7ffd, 0x7ffd).w(FUNC(chloe_state::port_7ffd_w));
	map(0x1ffd, 0x1ffd).w(FUNC(chloe_state::port_7ffd_w));
	map(0x00f4, 0x00f4).mirror(0xff00).w(FUNC(chloe_state::port_f4_w));

	map(0x00e3, 0x00e3).mirror(0xff00).w(FUNC(chloe_state::port_e3_w));
	map(0x00e7, 0x00e7).mirror(0xff00).lw8(NAME([this](u8 data) { m_sdcard->spi_ss_w(data & 1); }));
	map(0x00eb, 0x00eb).mirror(0xff00).rw(FUNC(chloe_state::spi_data_r), FUNC(chloe_state::spi_data_w));

	map(0x00fd, 0x00fd).lw8(NAME([this](u8 data)
	{
		if (data & 1)
		{
			LOGMEM("Core Boot Off\n");
			m_core_boot = 0;
			reset();
		}
	}));

	map(0xbffd, 0xbffd).lw8(NAME([this](u8 data) { return m_ay[m_ay_selected]->data_w(data); }));
	map(0xfffd, 0xfffd).lr8(NAME([this]()
	{
		return m_ay[m_ay_selected]->data_r();
	})).w(FUNC(chloe_state::ay_address_w));

	map(0xbf3b, 0xbf3b).lw8(NAME([this](u8 data)
	{
		m_palpen_selected = data;
		if ((data & 0xc0) == 0x40)
		{
			port_ff_w(data);
		}
	}));
	map(0xff3b, 0xff3b).lrw8(NAME([this]()
	{
		rgb_t rgb = m_palette->pen_color(0xc0 | m_palpen_selected);
		return ((rgb.g() >> 5) << 5) | ((rgb.r() >> 5) << 2) | ((rgb.b() >> 6) << 0);
	}), NAME([this](u8 data)
		{
			if (m_palpen_selected < 64)
			{
				m_palette->set_pen_color(0xc0 | m_palpen_selected, rgbexpand<3,3,3>((data << 1) | (((data >> 1) | data) & 1), 3, 6, 0));
			}
			else if ((m_palpen_selected & 0xc0) == 0x40)
			{
				m_ula->ulap_en_w(data & 1);
			}
		}));
	map(0xfc3b, 0xfc3b).lrw8(NAME([this]() { return m_reg_selected; })
		, NAME([this](u8 data) { m_dma_hilo = 0; m_reg_selected = data; }));
	map(0xfd3b, 0xfd3b).lrw8(NAME([this]() { return m_uno_regs.read_byte(m_reg_selected); })
		, NAME([this](u8 data) { m_uno_regs.write_byte(m_reg_selected, data); }));

	//map(0x007f, 0x007f).mirror(0xff00).lr8(NAME([]() -> u8 { return 0x00; })); // Fuller
	map(0x001f, 0x001f).mirror(0xff00).lr8(NAME([this]() -> u8 { return m_io_joy1->read() & 0x1f; })); // Kempston 1
	map(0x00df, 0x00df).mirror(0xff00).lrw8(NAME([this]() -> u8 { return m_io_joy2->read() & 0x1f; })  // Kempston 2
		, NAME([this](u8 data) { m_covox->data_w(data); }));
	map(0x00b3, 0x00b3).mirror(0xff00).lw8(NAME([this](u8 data) { m_covox->data_w(data); }));
	map(0xfadf, 0xfadf).lr8(NAME([this]() -> u8 { return 0x80 | (m_io_mouse[2]->read() & 0x07); }));
	map(0xfbdf, 0xfbdf).lr8(NAME([this]() -> u8 { return  m_io_mouse[0]->read(); }));
	map(0xffdf, 0xffdf).lr8(NAME([this]() -> u8 { return ~m_io_mouse[1]->read(); }));

	map(0x00f7, 0x00f7).mirror(0xff00).nopw(); // Audio Mixer. No support for now, using default ACB
	map(0x8e3b, 0x8e3b).nopw(); // PRISMSPEEDCTRL used by software compatible with Prism
}

void chloe_state::map_regs(address_map &map)
{
	map(0x00, 0xff).lrw8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled())
		{
			LOGIO("rREG %02x\n", offset);
		}
		return m_uno_regs_data[offset];
	}), NAME([this](offs_t offset, u8 data)
	{
		LOGIO("wREG %02x = %02x\n", offset, data);
		m_uno_regs_data[offset] = data;
	}));

	map(0x01, 0x01).lw8(NAME([this](u8 data)
	{
		m_uno_regs_data[0x01] = data;
		LOGMEM("UnoMapper %d\n", data);
	}));
	map(0x0b, 0x0b).lw8(NAME([this](u8 data)
	{
		m_uno_regs_data[0x0b] = data;
		m_maincpu->set_clock_scale(1 << BIT(data, 6, 2));
	}));
	map(0x0c, 0x0d).lw8(NAME([this](offs_t offset, u8 data)
	{
		m_uno_regs_data[0x0c + offset] = data;
		raster_irq_adjust();
	}));
	map(0xa0, 0xa6).w(FUNC(chloe_state::dma_reg_w));
}

u8 chloe_state::kbd_fe_r(offs_t offset)
{
	u16 data = 0xffff;
	u16 shifts = 0xffff;

	u8 oi = offset >> 8;
	for (u8 i = 0; i < 8; i++, oi >>= 1)
	{
		u16 line_data = m_io_line[i]->read();
		shifts &= line_data;
		if ((oi & 1) == 0)
		{
			data &= line_data;
		}
	}

	bool shift_hold = ~m_io_line[0]->read() & 0x8000;
	if (shift_hold && ((shifts & 0x1f00) != 0x1f00))
	{
		data >>= 8;
		shifts >>= 8;
	}

	if (((offset & 0x0100) == 0) && BIT(~shifts, 6))
	{
		data &= ~0x01; // CS
	}

	if (((offset & 0x8000) == 0) && BIT(~shifts, 7))
	{
		data &= ~0x02; // SS
	}

	data |= 0xe0;

	/* cassette input from wav */
	if (m_cassette->input() > 0.0038 )
	{
		data &= ~0x40;
	}

	return data;
}

INPUT_PORTS_START(chloe)
	/* PORT_NAME =  KEY Mode    CAPS Mode    SYMBOL Mode  */
	PORT_START("IO_LINE0") /* 0xFEFE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Shift")   PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z   Z   :")    PORT_CODE(KEYCODE_Z)      PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(':')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x   X   `")    PORT_CODE(KEYCODE_X)      PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR('`')
																			PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c   C   ?")    PORT_CODE(KEYCODE_C)      PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR('?')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v   V   /")    PORT_CODE(KEYCODE_V)      PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR('/')
																			PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line0")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line0")     PORT_CODE(KEYCODE_TILDE) PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": (SS+KEY)")   PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("? (SS+KEY)")   PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS")           PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x7520, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE1") /* 0xFDFE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a   A   ~")    PORT_CODE(KEYCODE_A)       PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR('~')
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s   S   |")    PORT_CODE(KEYCODE_S)       PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR('|')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d   D   \\")   PORT_CODE(KEYCODE_D)       PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR('\\')
																			PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f   F   {")    PORT_CODE(KEYCODE_F)       PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR('{')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g   G   }")    PORT_CODE(KEYCODE_G)       PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR('}')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line1")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line1")     PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~ (SS+KEY)")   PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("| (SS+KEY)")   PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{ (SS+KEY)")   PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("} (SS+KEY)")   PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0xe420, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE2") /* 0xFBFE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q   Q   Hom")  PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_HOME) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w   W   Del")  PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_DEL)  PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e   E   End")  PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_END)  PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r   R   <")    PORT_CODE(KEYCODE_R)                         PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('<')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t   T   >")    PORT_CODE(KEYCODE_T)                         PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR('>')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line2")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line2")     PORT_CODE(KEYCODE_HOME) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_END)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("< (SS+KEY)")   PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("> (SS+KEY)")   PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0xe720, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE3") /* 0xF7FE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1   Tab   !")  PORT_CODE(KEYCODE_1)       PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR('1') PORT_CHAR('!')
																			PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_TAB) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2   CLk   @")  PORT_CODE(KEYCODE_2)       PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR('2') PORT_CHAR('@')
																			PORT_CODE(KEYCODE_F2) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3   PgU   #")  PORT_CODE(KEYCODE_3)       PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR('3') PORT_CHAR('#')
																			PORT_CODE(KEYCODE_F3) PORT_CODE(KEYCODE_PGUP) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4   PgD   $")  PORT_CODE(KEYCODE_4)       PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR('4') PORT_CHAR('$')
																			PORT_CODE(KEYCODE_F4) PORT_CODE(KEYCODE_PGDN) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5   Lft   %")  PORT_CODE(KEYCODE_5)       PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR('5') PORT_CHAR('%')
																			PORT_CODE(KEYCODE_F5) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line3")     PORT_CODE(KEYCODE_TAB) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CODE(KEYCODE_PGUP) PORT_CODE(KEYCODE_PGDN) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line3")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("! (SS+KEY)")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ (SS+KEY)")   PORT_CODE(KEYCODE_2)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("# (SS+KEY)")   PORT_CODE(KEYCODE_3)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("$ (SS+KEY)")   PORT_CODE(KEYCODE_4)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("% (SS+KEY)")   PORT_CODE(KEYCODE_5)
	PORT_BIT(0xe020, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE4") /* 0xEFFE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0   BSp   _")  PORT_CODE(KEYCODE_0)       PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CHAR('0') PORT_CHAR('_')
																			PORT_CODE(KEYCODE_F10) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9   Ctr   )")  PORT_CODE(KEYCODE_9)       PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CHAR('9') PORT_CHAR(')')
																			PORT_CODE(KEYCODE_F9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8   Rgt   (")  PORT_CODE(KEYCODE_8)       PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHAR('8') PORT_CHAR('(')
																			PORT_CODE(KEYCODE_F8) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7   Up    '")  PORT_CODE(KEYCODE_7)       PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHAR('7') PORT_CHAR('\'')
																			PORT_CODE(KEYCODE_F7) PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_QUOTE) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6   Dwn   &")  PORT_CODE(KEYCODE_6)       PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHAR('6') PORT_CHAR('&')
																			PORT_CODE(KEYCODE_F6) PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line4")     PORT_CODE(KEYCODE_BACKSPACE)  PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line4")     PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_ (SS+KEY)")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(") (SS+KEY)")   PORT_CODE(KEYCODE_0)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("( (SS+KEY)")   PORT_CODE(KEYCODE_9)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("& (SS+KEY)")   PORT_CODE(KEYCODE_7)
	PORT_BIT(0xe820, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE5") /* 0xDFFE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p   P   \"")   PORT_CODE(KEYCODE_P)       PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('"') 
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o   O   ;")    PORT_CODE(KEYCODE_O)       PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(';')
																			PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i   I   Ins")  PORT_CODE(KEYCODE_I)       PORT_CHAR('i') PORT_CHAR('I')
																			PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u   U   ]")    PORT_CODE(KEYCODE_U)       PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(']')
																			PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y   Y   [")    PORT_CODE(KEYCODE_Y)       PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR('[')
																			PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line5")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line5")     PORT_CODE(KEYCODE_COLON) PORT_CODE(KEYCODE_INSERT) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\" (SS+KEY)")  PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0xfe20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE6") /* 0xBFFE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ret    Cmp   Clr") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
																				PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l    L    =")      PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR('=')
																				PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k    K    +")      PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR('+')
																				PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j    J    -")      PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR('-')
																				PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h    H    ^")      PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR('^')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line6")         PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line6")         PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+ (SS+KEY)")       PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ (SS+KEY)")       PORT_CODE(KEYCODE_6)
	PORT_BIT(0xeb20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE7") /* 0x7FFE */
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Spc   Esc   Hlp")  PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ') PORT_CODE(KEYCODE_ESC)
																				PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Koru")             PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m   M   .")        PORT_CODE(KEYCODE_M)        PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR('.')
																				PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n   N   ,")        PORT_CODE(KEYCODE_N)        PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(',')
																				PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b   B   *")        PORT_CODE(KEYCODE_B)        PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR('*')
																				PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line7")         PORT_CODE(KEYCODE_ESC) PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line7")         PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_ASTERISK) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("* (SS+KEY)")       PORT_CODE(KEYCODE_8)
	PORT_BIT(0xef20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("NMI")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("NMI (SS+KEY)") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, chloe_state, on_divmmc_nmi, 0)


	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1) PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_Y_UP_SWITCH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_X_LEFT_SWITCH)

	PORT_START("JOY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_Y_UP_SWITCH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON1)

	PORT_START("mouse_input1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(30)

	PORT_START("mouse_input2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(30)

	PORT_START("mouse_input3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3)

INPUT_PORTS_END


void chloe_state::machine_start()
{
	spectrum_128_state::machine_start();

	m_spi_clock = timer_alloc(FUNC(chloe_state::spi_clock), this);
	m_irq_raster_on_timer = timer_alloc(FUNC(chloe_state::raster_irq_on), this);
	m_irq_raster_off_timer = timer_alloc(FUNC(chloe_state::raster_irq_off), this);

	m_regs_map->space(AS_PROGRAM).specific(m_uno_regs);
	m_maincpu->space(AS_PROGRAM).specific(m_program);
	m_maincpu->space(AS_IO).specific(m_io);

	for (auto i = 0; i < 8; i++)
	{
		m_bank_ram[i]->configure_entries(0, m_ram->size() / 0x2000, m_ram->pointer(), 0x2000);
		m_bank_ram[i]->configure_entries( 8 << 1, 4, memregion("maincpu")->base(), 0x2000);
		m_bank_ram[i]->configure_entries(12 << 1, 2, memregion("maincpu")->base() + 0x8000, 0x2000);
	}

	memset(m_uno_regs_data, 0 , 256);
	m_core_boot = 1;
	m_divmmc_ctrl = 0;

	// Save
	save_item(NAME(m_timex_mmu));
	save_item(NAME(m_port_ff_data));
	save_item(NAME(m_core_boot));
	save_item(NAME(m_reg_selected));
	save_item(NAME(m_divmmc_paged));
	save_item(NAME(m_divmmc_ctrl));
	save_pointer(NAME(m_uno_regs_data), 256);
	save_item(NAME(m_palpen_selected));
	save_item(NAME(m_ay_selected));
	save_item(NAME(m_dma_hilo));
	save_item(NAME(m_dma_src_latch));
	save_item(NAME(m_dma_dst_latch));
	save_item(NAME(m_dma_pre_latch));
	save_item(NAME(m_dma_len_latch));
	save_item(NAME(m_dma_prob_latch));

	save_item(NAME(m_spi_clock_cycles));
	save_item(NAME(m_spi_clock_state));
	save_item(NAME(m_spi_mosi_dat));
	save_item(NAME(m_spi_miso_dat));
}

void chloe_state::machine_reset()
{
	spectrum_128_state::machine_reset();

	m_irq_raster_on_timer->reset();
	m_irq_raster_off_timer->reset();

	m_spi_clock->reset();
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;
	m_dma_hilo = 0;

	m_timex_mmu = 0;
	m_port_ff_data = 0;
	m_port_7ffd_data = 0;
	m_divmmc_paged = 1;
	m_divmmc_ctrl &= 0x40;
	m_ay_selected = 0;

	update_memory();
}

static const gfx_layout chloe_charlayout =
{
	8, 8,            // 8 x 8 characters
	256,             // 128 characters
	1,               // 1 bits per pixel
	{ 0 },           // no bitplanes
	{ STEP8(0, 1) }, // x offsets
	{ STEP8(0, 8) }, // y offsets
	8 * 8            // every char takes 8 bytes
};

static GFXDECODE_START(gfx_chloe)
	GFXDECODE_ENTRY("maincpu", 0x2a00, chloe_charlayout, 7, 1)
GFXDECODE_END

void chloe_state::video_start()
{
	spectrum_128_state::video_start();
	m_contention_pattern = {}; // Has no contention

	const u8 *ram = m_ram->pointer();
	m_ula->set_host_ram_ptr(ram);
}


void chloe_state::chloe(machine_config &config)
{
	spectrum_128(config);
	config.device_remove("exp");
	config.device_remove("palette");
	m_ram->set_default_size("512K").set_default_value(0xff);

	Z80(config.replace(), m_maincpu, 28_MHz_XTAL / 8);
	m_maincpu->set_m1_map(&chloe_state::map_fetch);
	m_maincpu->set_memory_map(&chloe_state::map_mem);
	m_maincpu->set_io_map(&chloe_state::map_io);
	m_maincpu->set_vblank_int("screen", FUNC(chloe_state::chloe_interrupt));
	m_maincpu->nomreq_cb().set_nop();

	ADDRESS_MAP_BANK(config, m_regs_map).set_map(&chloe_state::map_regs).set_options(ENDIANNESS_LITTLE, 8, 8, 0);

	/*
	???DMA(config, m_dma, 28_MHz_XTAL / 8);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->in_mreq_callback().set([this](offs_t offset) { return m_program.read_byte(offset); });
	m_dma->out_mreq_callback().set([this](offs_t offset, u8 data) { m_program.write_byte(offset, data); });
	m_dma->in_iorq_callback().set([this](offs_t offset) { return m_io.read_byte(offset); });
	m_dma->out_iorq_callback().set([this](offs_t offset, u8 data) { m_io.write_byte(offset, data); });
	*/

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set(FUNC(chloe_state::spi_miso_w));

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_chloe);
	m_screen->set_raw(25.175_MHz_XTAL, CYCLES_HORIZ, CYCLES_VERT, SCR_FULL); // VGA
	m_screen->set_screen_update(FUNC(chloe_state::screen_update));
	m_screen->set_no_palette();

	PALETTE(config, m_palette, FUNC(chloe_state::spectrum_palette), 256);
	SCREEN_ULA_PLUS(config, m_ula, 0).set_raster_offset(SCR_256x192.left(), SCR_256x192.top()).set_palette(m_palette->device().tag(), 0x000, 0x000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	config.device_remove("ay8912");
	AY8912(config, m_ay[0], 28_MHz_XTAL / 16)
		.add_route(0, "lspeaker", 0.50)
		.add_route(2, "lspeaker", 0.25)
		.add_route(2, "rspeaker", 0.25)
		.add_route(1, "rspeaker", 0.50);
	AY8912(config, m_ay[1], 28_MHz_XTAL / 16)
		.add_route(0, "lspeaker", 0.50)
		.add_route(2, "lspeaker", 0.25)
		.add_route(2, "rspeaker", 0.25)
		.add_route(1, "rspeaker", 0.50);

	DAC_8BIT_R2R(config, m_covox, 0)
		.add_route(ALL_OUTPUTS, "lspeaker", 0.75)
		.add_route(ALL_OUTPUTS, "rspeaker", 0.75);

	SOFTWARE_LIST(config, "cass_list_t").set_original("timex_cass");
}

ROM_START(chloe)
	ROM_REGION(0xc000, "maincpu", ROMREGION_ERASEFF)

	// SE/OS 1.0
	ROM_LOAD( "10_boot.rom", 0x0000, 0x4000, CRC(efbfe46e) SHA1(f5a86b56955661f72fa416e7e644de0b3afe6509))
	ROM_LOAD( "10_basic_42.rom", 0x4000, 0x4000, CRC(c6273eaa) SHA1(f09a26c50f5cfe454e4d56c920cdcc62bc4f90cb))
	ROM_LOAD( "10_dos_31.rom", 0x8000, 0x2000, CRC(67dfef09) SHA1(ba9616494071dfe65834d7db657e0d3bcce0b732))
ROM_END

} // Anonymous namespace

/*    YEAR   NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY               FULLNAME       FLAGS */
COMP( 1999,  chloe,   spec128, 0,      chloe,   chloe,  chloe_state,  empty_init,  "Chloe Corporation",  "Chloe 280SE", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_CONTROLS )
