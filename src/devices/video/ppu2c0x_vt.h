// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    VT video emulation
	
	The VT video is based on the ppu2c0x but with enhanced capabilities such
	as 16 colour sprites.

******************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"

#define MCFG_PPU_VT03_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_VT03)

#define MCFG_PPU_VT03_READ_BG_CB(_devcb) \
	devcb = &ppu_vt03_device::set_read_bg_callback(*device, DEVCB_##_devcb);

#define MCFG_PPU_VT03_READ_SP_CB(_devcb) \
	devcb = &ppu_vt03_device::set_read_sp_callback(*device, DEVCB_##_devcb);

class ppu_vt03_device : public ppu2c0x_device {
public:
	ppu_vt03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_read_bg_callback(device_t &device, _Object object) { return downcast<ppu_vt03_device &>(device).m_read_bg.set_callback(object); }
	template<class _Object> static devcb_base &set_read_sp_callback(device_t &device, _Object object) { return downcast<ppu_vt03_device &>(device).m_read_sp.set_callback(object); }

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
	virtual DECLARE_READ8_MEMBER(palette_read) override;
	virtual DECLARE_WRITE8_MEMBER(palette_write) override;

	virtual void init_palette( palette_device &palette, int first_entry ) override;

	virtual void read_tile_plane_data(int address, int color) override;
	virtual void shift_tile_plane_data(uint8_t &pix) override;
	virtual void draw_tile_pixel(uint8_t pix, int color, uint16_t back_pen, uint16_t *&dest, const pen_t *color_table) override;

	virtual void read_sprite_plane_data(int address) override;
	virtual void make_sprite_pixel_data(uint8_t &pixel_data, int flipx) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, uint8_t pixel_data, bitmap_ind16& bitmap) override;
	virtual void read_extra_sprite_bits(int sprite_index) override;

	virtual void device_start() override;
	virtual void device_reset() override;

	void set_201x_reg(int reg, uint8_t data);
	uint8_t get_201x_reg(int reg);

	uint8_t get_va34();
	uint8_t get_m_read_bg4_bg3();
	uint8_t get_speva2_speva0();

private:
	devcb_read8 m_read_bg;
	devcb_read8 m_read_sp;

	std::unique_ptr<uint8_t[]> m_newpal;

	int m_read_bg4_bg3;
	int m_va34;

	uint8_t m_extplanebuf[2];
	uint8_t m_extra_sprite_bits;

	palette_device *m_palette;

	uint8_t m_201x_regs[0x20];

	void set_2010_reg(uint8_t data);

	void set_new_pen(int i);
};

DECLARE_DEVICE_TYPE(PPU_VT03,    ppu_vt03_device) 