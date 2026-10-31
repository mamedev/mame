// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB) plus Cleveland Codonics Imaginator I-100

  Not Currently Implemented
    - Support for Tektronix Emulation (missing ROM image)

****************************************************************************/

#include "emu.h"
#include "imaginator.h"

#include "tlbbase.h"

#include <algorithm>


namespace bus::heathzenith::h19 {

namespace {

/**
 * Heath TLB plus Cleveland Codonics Imaginator I-100
 */
class heath_imaginator_tlb_device : public heath_tlb_device
{
public:
	heath_imaginator_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;
	void crtc_hsync_w(int val);

	IRQ_CALLBACK_MEMBER(irq_ack_cb);

	void update_tap_6000h();
	void tap_6000h();
	void tap_8000h();

	void allow_tlb_intr();
	void allow_hsync_intr();

	void display_enable_w(offs_t reg, u8 val);
	void config_irq_w(offs_t reg, u8 val);
	void nop_w(offs_t reg, u8 val);
	virtual void set_irq_line() override;

	required_memory_bank       m_mem_bank;
	required_shared_ptr<u8>    m_p_graphic_ram;
	memory_passthrough_handler m_tap_6000h;

	u8   m_im2_val;

	bool m_alphanumeric_mode_active;
	bool m_graphics_mode_active;

	bool m_allow_tlb_interrupts;
	bool m_allow_imaginator_interrupts;

	bool m_hsync_irq_raised;
};

ROM_START( imaginator )
	// Program code
	ROM_REGION( 0x6000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("rev12")

	// Original terminal code
	ROM_LOAD( "2732_444-46_h19code.u437",                      0x0000, 0x1000, CRC(f4447da0) SHA1(fb4093d5b763be21a9580a0defebed664b1f7a7b))

	// Imaginator ROMs
	ROM_SYSTEM_BIOS(0, "rev12", "GPC Rev. 1.2 P/N 9000-0002")
	ROMX_LOAD( "2732_imaginator_gpc_rev_1.2_pn_9000_0002.u9a", 0x2000, 0x1000, CRC(507bb13f) SHA1(5b210f8d77e22fdf063f611eb5c29636cdb01250), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "unknown", "GPC Unknown revision")
	ROMX_LOAD( "2764_imaginator_gpc_unknown.u9a",              0x2000, 0x2000, CRC(8eff78ae) SHA1(d571cc5ec2e062fafca34fcc80f015ff82e84283), ROM_BIOS(1))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",                      0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Original keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_444-37_h19keyb.u445",                      0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END


/**
 * Cleveland Codonics Imaginator I-100 add-in board
 *
 *    Memory Range     Description
 *   -----------------------------------------------------------------
 *    0x0000-0x1fff    Program ROM (Heath TLB board)
 *    0x2000-0x3fff    Graphics Command Processor(GCP) ROM (I-100)
 *    0x4000-0x5fff    Scratchpad RAM (Heath TLB board)
 *    0x6000-0x7fff    Graphics Scratchpad  R/W RAM (I-100)
 *    0x8000-0xbfff    Graphics Display R/W RAM(I-100)
 *    0xc000-0xdfff    Optional Graphics Command Processor memory (I-100)
 *    0xe000-0xffff    Display Memory (Heath TLB board)
 *
 * New interrupt source
 *   - horizontal retrace interrupt
 */
heath_imaginator_tlb_device::heath_imaginator_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_tlb_device(mconfig, HEATH_IMAGINATOR, tag, owner, clock)
	, m_mem_bank(*this,      "membank")
	, m_p_graphic_ram(*this, "graphicram")
{
}

void heath_imaginator_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &heath_imaginator_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_imaginator_tlb_device::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(heath_imaginator_tlb_device::irq_ack_cb));

	m_crtc->out_hsync_callback().set(FUNC(heath_imaginator_tlb_device::crtc_hsync_w));
}

void heath_imaginator_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	save_item(NAME(m_im2_val));
	save_item(NAME(m_alphanumeric_mode_active));
	save_item(NAME(m_graphics_mode_active));
	save_item(NAME(m_allow_tlb_interrupts));
	save_item(NAME(m_allow_imaginator_interrupts));
	save_item(NAME(m_hsync_irq_raised));

	// set_irq_line() is called from the base device_reset(), before ours runs
	m_allow_tlb_interrupts        = true;
	m_allow_imaginator_interrupts = false;
	m_hsync_irq_raised            = false;

	m_mem_bank->configure_entries(0, 2, memregion("maincpu")->base(), 0x2000);

	m_maincpu->space(AS_PROGRAM).install_readwrite_tap(0x8000, 0xbfff, "irq_update",
			[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_8000h(); } },
			[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_8000h(); } });
}

void heath_imaginator_tlb_device::device_reset()
{
	heath_tlb_device::device_reset();

	m_mem_bank->set_entry(1);
	update_tap_6000h();

	m_alphanumeric_mode_active = true;
	m_graphics_mode_active     = false;

	m_hsync_irq_raised         = false;

	allow_tlb_intr();
}

void heath_imaginator_tlb_device::mem_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x1fff).bankr(m_mem_bank);

	// Normal spot of the GCP ROM
	map(0x2000, 0x3fff).rom();

	// TLB Scratchpad
	map(0x4000, 0x40ff).mirror(0x1f00).ram();

	// GCP Scratchpad
	map(0x6000, 0x607f).mirror(0x1f80).ram();

	// Graphics RAM
	map(0x8000, 0xbfff).ram().share(m_p_graphic_ram);

	// optional Graphics Command Processor (GCP)
	// map(0xc000, 0xdfff);

	// Alphanumeric RAM
	map(0xe000, 0xe7ff).mirror(0x1800).ram().share(m_p_videoram);
}

void heath_imaginator_tlb_device::device_post_load()
{
	heath_tlb_device::device_post_load();

	// the bank is saved, but not whether the tap that switches it is installed
	update_tap_6000h();
}

// The GCP ROM is mapped at 0x0000 after reset until the first access at 0x6000-0x7fff
void heath_imaginator_tlb_device::update_tap_6000h()
{
	m_tap_6000h.remove();

	if (m_mem_bank->entry() == 1)
	{
		m_tap_6000h = m_maincpu->space(AS_PROGRAM).install_readwrite_tap(0x6000, 0x7fff, "mem_map_update",
				[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_6000h(); } },
				[this] (offs_t offset, u8 &data, u8 mem_mask) { if (!machine().side_effects_disabled()) { tap_6000h(); } });
	}
}

void heath_imaginator_tlb_device::tap_6000h()
{
	m_mem_bank->set_entry(0);
	m_tap_6000h.remove();
}

void heath_imaginator_tlb_device::tap_8000h()
{
	if (!m_allow_tlb_interrupts)
	{
		allow_tlb_intr();

		set_irq_line();
	}
}

void heath_imaginator_tlb_device::allow_tlb_intr()
{
	m_allow_tlb_interrupts        = true;
	m_allow_imaginator_interrupts = false;
	m_im2_val                     = 0x02;
}

void heath_imaginator_tlb_device::allow_hsync_intr()
{
	m_allow_tlb_interrupts        = false;
	m_allow_imaginator_interrupts = true;
	m_im2_val                     = 0x00;
}

/**
 * An input/output request (IORQ) is sent to the TERMINAL LOGIC board only when
 * both IORQ (pin 20 of U7) AND A4 are low.
 */
void heath_imaginator_tlb_device::io_map(address_map &map)
{
	heath_tlb_device::io_map(map);

	// interrupt routing
	map(0x10, 0x10).select(0x08).mirror(0x07).w(FUNC(heath_imaginator_tlb_device::config_irq_w));

	// display enabling (graphics or alphanumeric)
	map(0x30, 0x30).select(0xc0).mirror(0x0f).w(FUNC(heath_imaginator_tlb_device::display_enable_w));

	// Avoids writes to TLB when a4 is high on unused
	map(0x50, 0x50).select(0x80).mirror(0x0f).w(FUNC(heath_imaginator_tlb_device::nop_w));
}

IRQ_CALLBACK_MEMBER(heath_imaginator_tlb_device::irq_ack_cb)
{
	return m_im2_val;
}

void heath_imaginator_tlb_device::config_irq_w(offs_t reg, u8 val)
{
	if (BIT(reg, 3))
	{
		allow_tlb_intr();
	}
	else
	{
		allow_hsync_intr();
	}

	set_irq_line();
}

void heath_imaginator_tlb_device::display_enable_w(offs_t reg, u8 val)
{
	m_alphanumeric_mode_active = bool(BIT(reg, 7));
	m_graphics_mode_active     = bool(BIT(reg, 6));
}

void heath_imaginator_tlb_device::nop_w(offs_t, u8)
{
}

const tiny_rom_entry *heath_imaginator_tlb_device::device_rom_region() const
{
	return ROM_NAME(imaginator);
}

MC6845_UPDATE_ROW(heath_imaginator_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		for (int x = 0; x < x_count; x++)
		{
			u8 output = 0x00;

			if (m_alphanumeric_mode_active)
			{
				u8 inv = (x == cursor_x) ? 0xff : 0;
				u8 chr = m_p_videoram[(ma + x) & 0x7ff];

				if (chr & 0x80)
				{
					inv ^= 0xff;
					chr &= 0x7f;
				}

				output |= m_p_chargen[(chr << 4) | ra] ^ inv;
			}

			if (m_graphics_mode_active && (x > 7 ) && (x < 73))
			{
				output |= m_p_graphic_ram[((y * 64) + (x - 8)) & 0x3fff];
			}

			for (int b = 0; 8 > b; ++b)
			{
				*p++ = palette[BIT(output, b)];
			}
		}
	}
	else
	{
		std::fill_n(p, x_count * 8, palette[0]);
	}
}

void heath_imaginator_tlb_device::crtc_hsync_w(int val)
{
	m_hsync_irq_raised = bool(val);

	set_irq_line();
}

void heath_imaginator_tlb_device::set_irq_line()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0,
		(m_allow_imaginator_interrupts && m_hsync_irq_raised) ||
		(m_allow_tlb_interrupts && (m_keyboard_irq_raised || m_serial_irq_raised || m_break_key_irq_raised)) ?
		 ASSERT_LINE : CLEAR_LINE);
}

} // anonymous namespace

} // namespace bus::heathzenith::h19

DEFINE_DEVICE_TYPE_PRIVATE(HEATH_IMAGINATOR, device_heath_tlb_card_interface, bus::heathzenith::h19::heath_imaginator_tlb_device, "heath_imaginator_tlb", "Heath Terminal Logic Board plus Cleveland Codonics Imaginator I-100")
