// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/*
 * Robotron K7071 (ABS) text-only video
 *
 * Reference: http://www.tiffe.de/Robotron/MMS16/
 *
 * To do:
 * - fix screen corruption (DMA reprogramming timing?)
 * - loadable font - CRTC LA0 connected to NMI via gate
 */

#include "emu.h"
#include "robotron_k7071.h"

#define VERBOSE 0
#include "logmacro.h"

enum kgs_st : u8
{
	KGS_ST_OBF = 0x01,
	KGS_ST_IBF = 0x02,
	KGS_ST_INT = 0x04,
	KGS_ST_ERR = 0x80,
};

DEFINE_DEVICE_TYPE(ROBOTRON_K7071, robotron_k7071_device, "robotron_k7071", "Robotron K7071 ABS")

robotron_k7071_device::robotron_k7071_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ROBOTRON_K7071, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_dma(*this, "dma")
	, m_crtc(*this, "crtc")
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
	, m_ram(*this, "ram")
	, m_p_chargen(*this, "chargen")
{
}

ROM_START(robotron_k7071)
	ROM_REGION(0x1000, "eprom", ROMREGION_ERASEFF)
	ROM_LOAD("q209.bin", 0x0000, 0x0800, CRC(efc4aac0) SHA1(2cb4db3572e6ec7f3d3ae8e70d61b509b1fa3203))
	ROM_LOAD("q210.bin", 0x0800, 0x0800, CRC(d23e00ed) SHA1(eaca53ed784cf263323e9abba24a364b2aadac44))

	ROM_REGION(0x2000, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("q211.bin", 0x0000, 0x0800, CRC(b6241bc5) SHA1(8428e344df95451a34ad01322e245d2357712010))
ROM_END

const tiny_rom_entry *robotron_k7071_device::device_rom_region() const
{
	return ROM_NAME(robotron_k7071);
}

void robotron_k7071_device::device_start()
{
	save_item(NAME(m_kgs_datao));
	save_item(NAME(m_kgs_datai));
	save_item(NAME(m_kgs_ctrl));
	save_item(NAME(m_nmi_enable));

	m_cpu->space(AS_PROGRAM).specific(m_program);
	m_bus->space(AS_IO).install_readwrite_handler(0x200, 0x203, read16s_delegate(*this, FUNC(robotron_k7071_device::io_r)), write16s_delegate(*this, FUNC(robotron_k7071_device::io_w)));
}

void robotron_k7071_device::device_reset()
{
	m_kgs_ctrl = KGS_ST_IBF | KGS_ST_OBF;
	m_kgs_datao = m_kgs_datai = m_nmi_enable = 0;
}

void robotron_k7071_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, XTAL(16'000'000) / 6);
	m_cpu->set_addrmap(AS_PROGRAM, &robotron_k7071_device::cpu_mem);
	m_cpu->set_addrmap(AS_IO, &robotron_k7071_device::cpu_pio);

	I8257(config, m_dma, XTAL(16'000'000) / 6);
	m_dma->out_hrq_cb().set(FUNC(robotron_k7071_device::hrq_w));
	m_dma->in_memr_cb().set([this] (offs_t offset) { return m_program.read_byte(offset); });
	m_dma->out_iow_cb<0>().set(m_crtc, FUNC(i8275_device::dack_w));

	I8275(config, m_crtc, XTAL(16'000'000) / 8);
	m_crtc->set_screen(m_screen);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(robotron_k7071_device::display_pixels));
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq0_w));
	m_crtc->lc_wr_callback().set([this] (int data) { m_cpu->set_input_line(INPUT_LINE_IRQ0, data == 15 ? ASSERT_LINE : CLEAR_LINE); });

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(XTAL(16'000'000), 736, 0, 640, 432, 0, 416);
	m_screen->set_screen_update(m_crtc, FUNC(i8275_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);
}

void robotron_k7071_device::cpu_mem(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x0fff).rom().region("eprom", 0);
	map(0x1400, 0x1fff).ram().share("ram").mirror(0x8000);
}

void robotron_k7071_device::cpu_pio(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x00, 0x08).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x20, 0x21).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x30, 0x30).noprw(); // chargen ram access
	map(0x40, 0x40).nopw(); // line counter input for sync generator
	map(0x50, 0x50).lw8(NAME([this] (u8 data) { m_kgs_ctrl |= KGS_ST_INT; int_w(7, ASSERT_LINE); }));
	map(0x60, 0x60).lw8(NAME([this] (u8 data) { m_kgs_ctrl |= KGS_ST_ERR; }));
	map(0x70, 0x71).rw(FUNC(robotron_k7071_device::kgs_host_r), FUNC(robotron_k7071_device::kgs_host_w));
	map(0x80, 0x80).lw8(NAME([this] (u8 data) { m_nmi_enable = BIT(data, 7); }));
	map(0xdf, 0xdf).nopw(); // used by IRQ handler
}

uint16_t robotron_k7071_device::io_r(offs_t offset, uint16_t mem_mask)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_kgs_ctrl;
		break;

	case 1:
		data = m_kgs_datai;
		if (!machine().side_effects_disabled())
			m_kgs_ctrl &= ~KGS_ST_OBF;
		break;
	}

	return data;
}

void robotron_k7071_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0:
		m_kgs_ctrl &= ~(KGS_ST_ERR | KGS_ST_INT);
		int_w(7, CLEAR_LINE);
		break;

	case 1:
		if (m_kgs_ctrl & KGS_ST_IBF)
		{
			m_kgs_ctrl |= KGS_ST_ERR;
		}
		else
		{
			m_kgs_datao = data;
			m_kgs_ctrl |= KGS_ST_IBF;
		}
		break;
	}
}


uint8_t robotron_k7071_device::kgs_host_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_kgs_ctrl;
		break;

	case 1:
		data = m_kgs_datao;
		if (!machine().side_effects_disabled())
			m_kgs_ctrl &= ~(KGS_ST_ERR | KGS_ST_IBF);
		break;

	default:
		break;
	}

	return data;
}

void robotron_k7071_device::kgs_host_w(offs_t offset, uint8_t data)
{
	if (offset)
	{
		m_kgs_datai = data;
		m_kgs_ctrl |= KGS_ST_OBF;
	}
}

void robotron_k7071_device::hrq_w(int state)
{
	// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_cpu->set_input_line(Z80_INPUT_LINE_BUSREQ, state);
	m_cpu->set_input_line(INPUT_LINE_HALT, state); // do we need this?
	m_dma->hlda_w(state);
}

I8275_DRAW_CHARACTER_MEMBER(robotron_k7071_device::display_pixels)
{
	using namespace i8275_attributes;

	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u8 gfx = (BIT(attrcode, LTEN)) ? 0xff : 0;
	if (!BIT(attrcode, VSP))
		gfx = m_p_chargen[linecount | (charcode << 4)];

	if (BIT(attrcode, RVV))
		gfx ^= 0xff;

	for (u8 i = 0; i < 8; i++)
		bitmap.pix(y, x + i) = palette[BIT(gfx, 7-i) ? (BIT(attrcode, HLGT) ? 2 : 1) : 0];
}
