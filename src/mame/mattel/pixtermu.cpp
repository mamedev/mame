// license:BSD-3-Clause
// copyright-holders:QUFB
/******************************************************************************

    WIP driver for Pixter Multi-Media.

    - The boot ROM is ignored, boot directly from NOR flash
    - The buttons are not implemented, only the touchscreen input
    - Audio and SSP DMA is not implemented, the SSP DMA interrupt is fired on Vblank to maintain progress
    - The soft buttons below the touchscreen are just drawn as rectangles

    References:

    - https://elinux.org/Pixter_Multimedia
    - https://www.nxp.com/docs/en/data-sheet/LH79524_525_N.pdf
    - https://www.nxp.com/docs/en/user-guide/LH79524-LH79525_UG_V1_3.pdf
    - https://lh79525.wordpress.com/

    Hardware
    --------

    Model H4651/J4287/J4288:

    - PCB Revision: PT1543A-BGA-4F2C 2005/04/30 Rev 4.2b
    - CPU: Sharp LH79524-NOE (ARM720T)
    - RAM: ICSI IC42S16100-7T (2MB each, 4MB total)
    - ROM: Chip-On-Board, selected by pin CS1

    JTAG
    ----

    To place the LH79524 into ARM ICE debug mode you will need to connect TEST1 via a 4.7K Ohm resistor to ground.

    Pinout:

    - D2: nTRST (Reset Input)
    - P4: TMS (Mode Select Input)
    - T3: TCK (Clock Input)
    - T1: TDI (Serial Data Input)
    - P3: TDO (Data Serial Output)
    - TEST1: ARM ICE debug mode

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/arm7/arm7.h"

#include "machine/lh79524_timer.h"
#include "machine/vic_pl192.h"

#include "softlist_dev.h"

#include "emupal.h"
#include "screen.h"

namespace {

class pixter_multimedia_state : public driver_device
{
public:
	pixter_multimedia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_touch(*this, { "TOUCHX", "TOUCHY", "TOUCH" })
		, m_cart(*this, "cartslot")
		, m_maincpu(*this, "maincpu")
		, m_ndcs0(*this, "ndcs0")
		, m_internal_sram(*this, "internal_sram")
		, m_timers(*this, "timer%u", 0U)
		, m_vic(*this, "vic")
		, m_clkrst(*this, "clkrst", 0x1000, ENDIANNESS_LITTLE)
		, m_bootctl(*this, "bootctl", 0x1000, ENDIANNESS_LITTLE)
		, m_lcdc(*this, "lcdc", 0x1000, ENDIANNESS_LITTLE)
		, m_adc(*this, "adc", 0x100, ENDIANNESS_LITTLE)
		, m_dma(*this, "dma", 0x100, ENDIANNESS_LITTLE)
		, m_remap_view(*this, "remap")
	{ }

	void pixter_multimedia(machine_config &config);

private:
	// Remap Control, mapped at 0xfffe2008, offset 0x0008/4
	static inline constexpr uint32_t CLKRST_REMAP = 2;
	// Power-up Boot Configuration, mapped at 0xfffe6000, offset 0x0000/4
	static inline constexpr uint32_t BOOTCTL_PBC = 0;
	// nCS1 Override, mapped at 0xfffe6004, offset 0x0004/4
	static inline constexpr uint32_t BOOTCTL_CS1OV = 1;
	// External Peripheral Mapping, mapped at 0xfffe6008, offset 0x0008/4
	static inline constexpr uint32_t BOOTCTL_EPM = 2;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void arm7_map(address_map &map) ATTR_COLD;
	void clkrst_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void bootctl_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void lcdc_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t lcdc_r(offs_t offset);

	uint32_t adc_r(offs_t offset);
	void adc_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t ssp_r(offs_t offset);
	void ssp_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	uint32_t gpioab_r(offs_t offset);
	uint32_t gpiogh_r(offs_t offset);
	uint32_t gpioij_r(offs_t offset);


	int adc_count;

	void apb_remap(uint32_t data);

	uint32_t screen_update_pixtermu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_ioport_array<3> m_touch;

	required_device<generic_slot_device> m_cart;
	required_device<arm7_cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_ndcs0;
	required_shared_ptr<uint32_t> m_internal_sram;
	required_device_array<lh79524_timer_device, 3> m_timers;
	required_device<vic_pl190_device> m_vic;

	memory_share_creator<uint32_t> m_clkrst;
	memory_share_creator<uint32_t> m_bootctl;
	memory_share_creator<uint32_t> m_lcdc;
	memory_share_creator<uint32_t> m_adc;
	memory_share_creator<uint32_t> m_dma;

	memory_view m_remap_view;
};

void pixter_multimedia_state::apb_remap(uint32_t data)
{
	// User's Guide - 1.6 Memory Interface Architecture
	if (data == 0) {
		m_remap_view.select((m_bootctl[BOOTCTL_PBC] & 0b0100) && (m_bootctl[BOOTCTL_CS1OV] & 0b1) ? 0 : 3);
	} else if (data < 3) {
		m_remap_view.select(data);
	} else {
		logerror("Unexpected remap %d\n", data);
	}
}

void pixter_multimedia_state::machine_start()
{
	if (m_cart->exists()) {
		memory_region *const cart_rom = m_cart->memregion("rom");
		device_generic_cart_interface::install_non_power_of_two<0>(
				cart_rom->bytes(),
				0x03ff'ffff,
				0,
				0x4800'0000,
				[this, cart_rom] (offs_t begin, offs_t end, offs_t mirror, offs_t src) {
					m_maincpu->space(AS_PROGRAM).install_rom(begin, end, mirror, cart_rom->base() + src);
				});
	}
}

void pixter_multimedia_state::machine_reset()
{
	m_bootctl[BOOTCTL_PBC] = 0b0000; // Boot from NOR Flash or SRAM, 16-bit data bus width, nBLEx LOW for reads
	m_bootctl[BOOTCTL_CS1OV] = 0b0; // nCS1 is routed for normal operation
	m_bootctl[BOOTCTL_EPM] = 0b1111; // All external devices are accessible following reset

	m_clkrst[CLKRST_REMAP] = 0b00; // Map nCS1

	adc_count = 0;

	apb_remap(m_clkrst[CLKRST_REMAP]);
}

DEVICE_IMAGE_LOAD_MEMBER(pixter_multimedia_state::cart_load)
{
	uint64_t length;
	memory_region *cart_rom = nullptr;
	if (m_cart->loaded_through_softlist()) {
		cart_rom = m_cart->memregion("rom");
		if (!cart_rom) {
			return std::make_pair(image_error::BADSOFTWARE, "Software list item has no 'rom' data area");
		}
		length = cart_rom->bytes();
	} else {
		length = m_cart->length();
	}

	if (!length) {
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must not be empty");
	}
	if (length & 3) {
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be a multiple of 4 bytes)");
	}

	if (!m_cart->loaded_through_softlist()) {
		cart_rom = machine().memory().region_alloc(m_cart->subtag("rom"), length, 4, ENDIANNESS_LITTLE);
		if (!cart_rom) {
			return std::make_pair(std::errc::not_enough_memory, std::string());
		}

		uint32_t *const base = reinterpret_cast<uint32_t *>(cart_rom->base());
		if (m_cart->fread(base, length) != length) {
			return std::make_pair(std::errc::io_error, "Error reading cartridge file");
		}

		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE) {
			for (uint64_t i = 0; (length / 4) > i; ++i)
				base[i] = swapendian_int32(base[i]);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

void pixter_multimedia_state::arm7_map(address_map &map)
{
	// Remap Bank
	map(0x0000'0000, 0x003f'ffff).view(m_remap_view);
	m_remap_view[0](0x0000'0000, 0x0000'1fff).rom().region("bootrom", 0);
	m_remap_view[1](0x0000'0000, 0x003f'ffff).ram().share("ndcs0");
	m_remap_view[2](0x0000'0000, 0x0000'3fff).ram().share("internal_sram");
	m_remap_view[3](0x0000'0000, 0x003f'ffff).rom().region("ncs1", 0);

	// External SRAM
	map(0x2000'0000, 0x203f'ffff).ram().share("ndcs0");
	// nCS0 (Unused NAND Flash?)
	// map(0x40000000, 0x403fffff)
	// nCS1 (Chip-On-Board ROM)
	map(0x4400'0000, 0x443f'ffff).mirror(0x03c0'0000).rom().region("ncs1", 0);
	// nCS2 (Cart ROM)
	// map(0x4800'0000, 0x483f'ffff)
	// nCS3 (Unused?)
	// map(0x4c00'0000, 0x4c3f'ffff)

	// Internal SRAM
	map(0x6000'0000, 0x6000'3fff).mirror(0x0fff'c000).ram().share("internal_sram");

	// Boot ROM
	map(0x8000'0000, 0x8000'1fff).rom().region("bootrom", 0);

	// APB Peripherals
	// ADC
	map(0xfffc'3000, 0xfffc'30ff).ram().share("adc").r(FUNC(pixter_multimedia_state::adc_r)).w(FUNC(pixter_multimedia_state::adc_w));
	// Timers
	map(0xfffc'4000, 0xfffc'402f).rw(m_timers[0], FUNC(lh79524_timer_device::read), FUNC(lh79524_timer_device::write));
	map(0xfffc'4030, 0xfffc'404f).rw(m_timers[1], FUNC(lh79524_timer_device::read), FUNC(lh79524_timer_device::write));
	map(0xfffc'4050, 0xfffc'406f).rw(m_timers[2], FUNC(lh79524_timer_device::read), FUNC(lh79524_timer_device::write));
	// SSP
	map(0xfffc'6000, 0xfffc'602f).r(FUNC(pixter_multimedia_state::ssp_r)).w(FUNC(pixter_multimedia_state::ssp_w));

	// GPIO I/J
	map(0xfffd'b000, 0xfffd'b00f).r(FUNC(pixter_multimedia_state::gpioij_r));
	// GPIO G/H
	map(0xfffd'c000, 0xfffd'c00f).r(FUNC(pixter_multimedia_state::gpiogh_r));
	// GPIO A/B
	map(0xfffd'f000, 0xfffd'f00f).r(FUNC(pixter_multimedia_state::gpioab_r));
	// DMA
	map(0xfffe'1000, 0xfffe'10ff).ram().share("dma").w(FUNC(pixter_multimedia_state::dma_w));

	// Reset Clock and Power Controller
	map(0xfffe'2000, 0xfffe'2fff).ram().share("clkrst").w(FUNC(pixter_multimedia_state::clkrst_w));
	// Boot Controller
	map(0xfffe'6000, 0xfffe'6fff).ram().share("bootctl").w(FUNC(pixter_multimedia_state::bootctl_w));


	// External Memory Control
	map(0xffff'1000, 0xffff'1fff).ram();
	// Color LCD Control
	map(0xffff'4000, 0xffff'4fff).ram().share("lcdc").w(FUNC(pixter_multimedia_state::lcdc_w)).r(FUNC(pixter_multimedia_state::lcdc_r));
	// USB Device
	map(0xffff'5000, 0xffff'5fff).ram();
	// Interrupt Vector Control
	map(0xffff'f000, 0xffff'ffff).m(m_vic, FUNC(vic_pl190_device::map)); // interrupt controller
}

void pixter_multimedia_state::clkrst_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == CLKRST_REMAP) {
		apb_remap(data);
	}

	COMBINE_DATA(&m_clkrst[offset]);
}

void pixter_multimedia_state::bootctl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bootctl[offset]);
}


void pixter_multimedia_state::lcdc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	offs_t addr = offset << 2;
	if (addr >= 0x200 && addr <= 0x3fc) {
		unsigned base = ((addr - 0x200) >> 2) * 2;
		for (int j = 0; j < 2; j++) {
			uint16_t ibgr1555 = data >> (16 * j);
			uint16_t i = ((ibgr1555 >> 15) & 0x1) << 2;
			uint16_t b = ((ibgr1555 >> 10) & 0x1F) << 3 | i;
			uint16_t g = ((ibgr1555 >> 5) & 0x1F) << 3 | i;
			uint16_t r = ((ibgr1555 >> 0) & 0x1F) << 3 | i;

			m_palette->set_pen_color(base + j, r, g, b);
		}
	}
	COMBINE_DATA(&m_lcdc[offset]);
}

uint32_t pixter_multimedia_state::lcdc_r(offs_t offset)
{
	return m_lcdc[offset];
}

uint32_t pixter_multimedia_state::ssp_r(offs_t offset)
{
	switch (offset << 2) {
		case 0x0C: // status
			return 1;
		default:
			return 0;
	}
}

void pixter_multimedia_state::ssp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
}

uint32_t pixter_multimedia_state::adc_r(offs_t offset)
{
	switch (offset << 2) {
		case 0x08: { // result
				if (adc_count > 0)
					--adc_count;
				unsigned index = ((m_adc[0x10 >> 2] & 0xF) -  adc_count);
				unsigned hc = m_adc[(0x24 >> 2) + index], lc = m_adc[(0x64 >> 2) + index];
				unsigned result = 0x3ff;
				if (hc == 0xFF80 && lc == 0x1080) { // touch or no touch
					result = m_touch[2]->read() ? 0 : 0x3ff;
				} else if (hc == 0xFFA0 && lc == 0x1080) {
					result = 0;
				} else if (hc == 0xFF91 && lc == 0x0015) { // Y position
					result = (m_touch[1]->read() * 1023) / 176;
				} else if (hc == 0xFF82 && lc == 0x00A2) { // X position
					result = 1023 - ((8 + m_touch[0]->read()) * 1023) / 176;
				}
				// At least one extra channel is used for battery
				return (result << 6) | (index & 0xF);
			}
		case 0x1C: // IRQ status
			return (m_touch[2]->read() ? 8 : 0) | 4;
		case 0x20: // FIFO status
			if (adc_count == 16)
				return 8;
			else if (adc_count == 0)
				return 4;
			else
				return 0;
		default:
			return m_adc[offset];
	}
}

void pixter_multimedia_state::adc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset << 2) {
		case 0x14:
			if(data & 0x4) { // start conversion
				adc_count = (m_adc[0x10 >> 2] & 0xF) + 1;
			};
			return;
		default:
			break;
	}
	COMBINE_DATA(&m_adc[offset]);
}

uint32_t pixter_multimedia_state::gpioab_r(offs_t offset)
{
	switch (offset << 2) {
		case 0x04: // port B data
			return 0xFF;
		default:
			return 0;
	}
}

uint32_t pixter_multimedia_state::gpiogh_r(offs_t offset)
{
	// Some of these GPIO are likely used for the buttons, which aren't implemented right now
	switch (offset << 2) {
		case 0x00: // port G data
			return 0x00;
		case 0x04: // port H data
			return 0x00;
		default:
			return 0;
	}
}


uint32_t pixter_multimedia_state::gpioij_r(offs_t offset)
{
	switch (offset << 2) {
		case 0x00: // port I data
			return 0xFF;
		default:
			return 0;
	}
}

void pixter_multimedia_state::dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset << 2) {
		case 0xF4: // interrupt clear
			if (data & 0x2) {
				m_dma[0xF8 >> 2] &= ~0x2;
			}
			break;
		default:
			COMBINE_DATA(&m_dma[offset]);
			break;
	}
	m_vic->irq_w<21>(((m_dma[0xF0 >> 2] & 0x2) & (m_dma[0xF8 >> 2] & 0x2)) ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t pixter_multimedia_state::screen_update_pixtermu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_lcdc[0x01C>>2], 1))
		return 0;
	const uint32_t base = (m_lcdc[0x010>>2] >> 2) & 0xfffff;

	for (int y = 0; y < 160; y++) {
		for (int x = 0; x < 160; x++) {
			int pixel = (y * 162 + x + 1);
			uint8_t ind = m_ndcs0[base + pixel / 4] >> ((pixel % 4) * 8);
			bitmap.pix(y, x) = ind;
		}
	}

	// HACK: draw the soft buttons where the touchscreen extends below the LCD as white squares
	// The label that is used in the physical hardware probably needs to be scanned
	for (int i = 0; i < 9; i++) {
		int x0 = (160 * i) / 9;
		int y0 = 160;
		for (int y = (y0 + 2); y <= (y0 + 14); y++) {
			for (int x = (x0 + 2); x <= (x0 + 14); x++) {
				bitmap.pix(y, x) = 0xFF;
			}
		}
	}

	return 0;
}

void pixter_multimedia_state::screen_vblank(int state)
{
	// Triggering this on vblank is definitely wrong, but it's the best place to set this right now...
	// The correct solution is to implement the SSP and DMA that drives the audio DAC. But without
	// this interrupt the system won't run.
	m_dma[0xF8 >> 2] |= 0x2;
	m_vic->irq_w<21>(((m_dma[0xF0 >> 2] & 0x2) & (m_dma[0xF8 >> 2] & 0x2)) ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( pixter_multimedia )

	PORT_START("TOUCHX")
	PORT_BIT(0x3ff, 80, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,159) PORT_SENSITIVITY(45) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT(0x3ff, 80, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,171) PORT_SENSITIVITY(45) PORT_KEYDELTA(13)

	PORT_START("TOUCH")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch")
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

INPUT_PORTS_END

void pixter_multimedia_state::pixter_multimedia(machine_config &config)
{
	// User's Guide - 1.3 Clock Strategy - AHB Fast CPU Clock (FCLK)
	ARM7(config, m_maincpu, 76'205'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pixter_multimedia_state::arm7_map);

	PL190_VIC(config, m_vic, 0);
	m_vic->out_irq_cb().set_inputline(m_maincpu, arm7_cpu_device::ARM7_IRQ_LINE);
	m_vic->out_fiq_cb().set_inputline(m_maincpu, arm7_cpu_device::ARM7_FIRQ_LINE);

	for (int i=0; i<3; i++) {
		LH79524_TIMER(config, m_timers[i], 76'205'000);
		m_timers[i]->set_timer_index(i);
	}

	m_timers[0]->irq_cb().set(m_vic, FUNC(vic_pl190_device::irq_w<4>));
	m_timers[1]->irq_cb().set(m_vic, FUNC(vic_pl190_device::irq_w<5>));
	m_timers[2]->irq_cb().set(m_vic, FUNC(vic_pl190_device::irq_w<6>));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pixter_cart");
	m_cart->set_endian(ENDIANNESS_LITTLE);
	m_cart->set_width(GENERIC_ROM32_WIDTH);
	m_cart->set_device_load(FUNC(pixter_multimedia_state::cart_load));
	m_cart->set_must_be_loaded(false);

	PALETTE(config, m_palette).set_format(palette_device::IRGB_1555, 256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_palette("palette");

	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	// The LCD is 160x160 but this is a hack to draw the softbuttons below it
	m_screen->set_size(160, 176);
	m_screen->set_visarea(0, 160-1, 0, 176-1);
	m_screen->set_screen_update(FUNC(pixter_multimedia_state::screen_update_pixtermu));

	m_screen->screen_vblank().set(FUNC(pixter_multimedia_state::screen_vblank));


	SOFTWARE_LIST(config, "cart_list").set_original("pixter_cart");
}

ROM_START( pixtermu )
	ROM_REGION32_LE( 0x400000, "ncs1", 0 )
	ROM_LOAD( "cs1.bin", 0x00000000, 0x400000, CRC(9d06745a) SHA1(c85ffd1777ffee4e99e5a208e3707a39b0dfc3aa) )

	ROM_REGION32_LE( 0x2000, "bootrom", 0 )
	ROM_LOAD( "lh79524.bootrom.bin", 0x00000000, 0x2000, CRC(5314a9e3) SHA1(23ed1914c7e7cc875cbb9f9b3d511a60a7324abd) )
ROM_END

} // anonymous namespace


//    year, name,     parent,  compat, machine,           input,             class,                   init,       company,  fullname,             flags
CONS( 2005, pixtermu, 0,       0,      pixter_multimedia, pixter_multimedia, pixter_multimedia_state, empty_init, "Mattel", "Pixter Multi-Media", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
