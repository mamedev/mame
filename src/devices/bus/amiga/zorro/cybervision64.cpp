// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Phase 5 CyberVision 64

    Zorro III RTG graphics card based on the S3 Trio64

    Hardware:
    - Trio64 GAAC2/86C764
    - 2 or 4 MB VRAM
    - Custom "Roxxler" chip for chunky-to-planar conversion
    - XTAL 14.31818 MHz (next to S3), 50 MHz (next to Roxxler)

    TODO:
    - The S3 core needs a lot of work. The following modes are ok on a
      recent version of P96 when accleration is disabled
      (NOBLITTER=Yes in the monitor tooltip):
      * 320x240 16-bit/32-bit (with DoubleScan disabled)
      * 640x480 16-bit/32-bit
      * 800x600 16-bit/32-bit
      * 1024x768 16-bit/32-bit
      * 1120x832 32-bit
      * 1152x900 32-bit
    - 2/4 MB config option
    - Unknown bits in the control register
    - Roxxler

***************************************************************************/

#include "emu.h"
#include "cybervision64.h"
#include "machine/autoconfig.h"
#include "video/pc_vga_s3.h"
#include "screen.h"

#define VERBOSE 1
#include "logmacro.h"


namespace bus::amiga::zorro {


class cybervision64_device : public device_t, public device_zorro3_card_interface, public amiga_autoconfig
{
public:
	cybervision64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void busrst_w(int state) override;
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	static constexpr uint32_t BOARD_SIZE = 0x4000000;

	void mmio_map(address_map &map) ATTR_COLD;

	offs_t translate_vram_address(offs_t address);
	uint32_t vga_mem_r(offs_t offset, uint32_t mem_mask);
	void vga_mem_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	void control_w(uint8_t data);
	void vsync_w(int state);
	void update_irq();

	required_device<s3trio64_vga_device> m_vga;
	required_device<ibm8514a_device> m_8514;
	required_device<screen_device> m_screen;

	offs_t m_base_address = 0;
	uint8_t m_control = 0;
	bool m_vsync_irq = false;
};

cybervision64_device::cybervision64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CYBERVISION64, tag, owner, clock),
	device_zorro3_card_interface(mconfig, *this),
	m_vga(*this, "vga"),
	m_8514(*this, "vga:8514a"),
	m_screen(*this, "screen")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void cybervision64_device::mmio_map(address_map &map)
{
	map(0x00040000, 0x00040003).w(FUNC(cybervision64_device::control_w)).umask32(0x00ff0000);
	map(0x01400000, 0x017fffff).rw(FUNC(cybervision64_device::vga_mem_r), FUNC(cybervision64_device::vga_mem_w));
	map(0x020003b0, 0x020003df).m(m_vga, FUNC(s3trio64_vga_device::io_map));
	map(0x020082e8, 0x020082eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_currenty_r), FUNC(ibm8514a_device::ibm8514_currenty_w));
	map(0x020086e8, 0x020086eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_currentx_r), FUNC(ibm8514a_device::ibm8514_currentx_w));
	map(0x02008ae8, 0x02008aeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_desty_r), FUNC(ibm8514a_device::ibm8514_desty_w));
	map(0x02008ee8, 0x02008eeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_destx_r), FUNC(ibm8514a_device::ibm8514_destx_w));
	map(0x020092e8, 0x020092eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_line_error_r), FUNC(ibm8514a_device::ibm8514_line_error_w));
	map(0x020096e8, 0x020096eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_width_r), FUNC(ibm8514a_device::ibm8514_width_w));
	map(0x02009ae8, 0x02009aeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_gpstatus_r), FUNC(ibm8514a_device::ibm8514_cmd_w));
	map(0x02009ee8, 0x02009eeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_ssv_r), FUNC(ibm8514a_device::ibm8514_ssv_w));
	map(0x0200a2e8, 0x0200a2eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_bgcolour_r), FUNC(ibm8514a_device::ibm8514_bgcolour_w));
	map(0x0200a6e8, 0x0200a6eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_fgcolour_r), FUNC(ibm8514a_device::ibm8514_fgcolour_w));
	map(0x0200aae8, 0x0200aaeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_write_mask_r), FUNC(ibm8514a_device::ibm8514_write_mask_w));
	map(0x0200aee8, 0x0200aeeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_read_mask_r), FUNC(ibm8514a_device::ibm8514_read_mask_w));
	map(0x0200b6e8, 0x0200b6eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_backmix_r), FUNC(ibm8514a_device::ibm8514_backmix_w));
	map(0x0200bae8, 0x0200baeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_foremix_r), FUNC(ibm8514a_device::ibm8514_foremix_w));
	map(0x0200bee8, 0x0200beeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_multifunc_r), FUNC(ibm8514a_device::ibm8514_multifunc_w));
	map(0x0200e2e8, 0x0200e2eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_pixel_xfer_r), FUNC(ibm8514a_device::ibm8514_pixel_xfer_w));
}


//**************************************************************************
//  ZORRO / AUTOCONFIG
//**************************************************************************

void cybervision64_device::busrst_w(int state)
{
	if (state)
		return;

	m_control = 0;
	m_vsync_irq = false;
	update_irq();

	if (m_base_address)
		zorro3_space().unmap_readwrite(m_base_address, m_base_address + BOARD_SIZE - 1);

	m_base_address = 0;
}

void cybervision64_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state)
		return;

	// setup autoconfig
	autoconfig_board_type(BOARD_TYPE_ZORRO3);
	autoconfig_board_subsize(BOARD_SUBSIZE_SAME);
	autoconfig_board_size(BOARD_SIZE_64M);

	autoconfig_product(34);
	autoconfig_manufacturer(8512);
	autoconfig_serial(0x00000000);

	autoconfig_link_into_memory(false);
	autoconfig_rom_vector_valid(false);
	autoconfig_multi_device(false);
	autoconfig_8meg_preferred(false); // z3 memory device
	autoconfig_can_shutup(true); // ?

	// zorro3 autoconfig handler
	zorro3_space().install_readwrite_handler(0xff000000, 0xff00ffff,
		read32_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read32)),
		write32_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write32)), 0xffffffff);
}

void cybervision64_device::autoconfig_base_address(offs_t address)
{
	LOG("Autoconfig address received: 0x%08x\n", uint32_t(address));

	// stop responding to default autoconfig
	zorro3_space().unmap_readwrite(0xff000000, 0xff00ffff);

	m_base_address = address;

	// install memory if we have a valid address
	if (m_base_address)
		zorro3_space().install_device(address, address + BOARD_SIZE - 1, *this, &cybervision64_device::mmio_map);

	// we're done
	cfgout_w(0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

offs_t cybervision64_device::translate_vram_address(offs_t address)
{
	// bit 6 reverses the byte lanes
	return BIT(m_control, 6) ? (address ^ 3) : address;
}

uint32_t cybervision64_device::vga_mem_r(offs_t offset, uint32_t mem_mask)
{
	const offs_t address = offset << 2;
	uint32_t data = 0xffffffff;

	if (ACCESSING_BITS_24_31)
		data = (data & 0x00ffffff) | m_vga->mem_linear_r(translate_vram_address(address + 0)) << 24;
	if (ACCESSING_BITS_16_23)
		data = (data & 0xff00ffff) | m_vga->mem_linear_r(translate_vram_address(address + 1)) << 16;
	if (ACCESSING_BITS_8_15)
		data = (data & 0xffff00ff) | m_vga->mem_linear_r(translate_vram_address(address + 2)) << 8;
	if (ACCESSING_BITS_0_7)
		data = (data & 0xffffff00) | m_vga->mem_linear_r(translate_vram_address(address + 3)) << 0;

	return data;
}

void cybervision64_device::vga_mem_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (0)
		logerror("vga_mem_w: %08x = %08x & %08x\n", offset, data, mem_mask);

	const offs_t address = offset << 2;

	if (ACCESSING_BITS_24_31)
		m_vga->mem_linear_w(translate_vram_address(address + 0), data >> 24);
	if (ACCESSING_BITS_16_23)
		m_vga->mem_linear_w(translate_vram_address(address + 1), data >> 16);
	if (ACCESSING_BITS_8_15)
		m_vga->mem_linear_w(translate_vram_address(address + 2), data >> 8);
	if (ACCESSING_BITS_0_7)
		m_vga->mem_linear_w(translate_vram_address(address + 3), data >> 0);
}


void cybervision64_device::control_w(uint8_t data)
{
	// 7-------  interrupt enable
	// -6------  32-bit swap
	// --5-----  another swap?
	// ---4----  video switch
	// ----3---  set to 1 on reset
	// -----2--  set to 1 on reset
	// ------1-  set to 0 on reset
	// -------0  set to 0 on reset

	LOG("control_w: %02x\n", data);

	m_control = data;
	update_irq();
}

void cybervision64_device::vsync_w(int state)
{
	m_vsync_irq = bool(state);
	update_irq();
}

void cybervision64_device::update_irq()
{
	int2_w(BIT(m_control, 7) && m_vsync_irq);
}

void cybervision64_device::device_start()
{
	// register for save states
	save_item(NAME(m_base_address));
	save_item(NAME(m_control));
	save_item(NAME(m_vsync_irq));
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void cybervision64_device::device_add_mconfig(machine_config &config)
{
	S3_TRIO64_VGA(config, m_vga);
	m_vga->set_screen(m_screen);
	m_vga->set_vram_size(0x400000);
	m_vga->vsync_cb().set(FUNC(cybervision64_device::vsync_w));

	SCREEN(config, m_screen);
	m_screen->set_raw(14.318181_MHz_XTAL * 2, 832, 0, 640, 520, 0, 480); // placeholder
	m_screen->set_screen_update(m_vga, FUNC(s3trio64_vga_device::screen_update));
}


} // bus::amiga::zorro


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(AMIGA_CYBERVISION64, device_zorro3_card_interface, bus::amiga::zorro::cybervision64_device, "amiga_cybervision64", "Phase 5 CyberVision 64")
