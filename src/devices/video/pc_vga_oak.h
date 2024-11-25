// license:BSD-3-Clause
// copyright-holders: Carl, Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_OAK_H
#define MAME_VIDEO_PC_VGA_OAK_H

#include "pc_vga.h"
#include "pc_xga.h"

#include "screen.h"

class oak_oti111_vga_device : public svga_device
{
public:
	oak_oti111_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 xga_read(offs_t offset);
	void xga_write(offs_t offset, u8 data);

	// $xxe0-$xxef in EXTIO
	void ramdac_mmio_map(address_map &map) ATTR_COLD;

	// $80 in MMIO space
	void multimedia_map(address_map &map) ATTR_COLD;

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint16_t offset() override;
	virtual void recompute_params() override;

	virtual void io_3bx_3dx_map(address_map &map) override ATTR_COLD;
private:
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_oak_space_config;

	required_device<xga_copro_device> m_xga;

	void oak_map(address_map &map) ATTR_COLD;

	u8 oak_index_r(offs_t offset);
	void oak_index_w(offs_t offset, u8 data);
	u8 oak_data_r(offs_t offset);
	void oak_data_w(offs_t offset, u8 data);

	u8 m_oak_idx = 0;

	u8 m_memory_size = 0;
	u8 m_i2c_data = 0;
	u8 m_scratchpad[8]{};
	bool m_oak_gfx_mode = false;
	bool m_oti_map_select = false;
	u8 m_oti_aperture_select = 0;
	u32 m_oti_aperture_mask = 0x3ffff;
	u8 m_pixel_mode = 0;
	bool m_color_swap = false;
	u8 m_bpp = 0;

	u16 m_cursor_x = 0, m_cursor_y = 0;
	u8 m_cursor_control = 0;
	u32 m_cursor_address_base = 0;
	u32 m_cursor_color[2]{};
};

DECLARE_DEVICE_TYPE(OTI111, oak_oti111_vga_device)

#endif // MAME_VIDEO_PC_VGA_OAK_H
