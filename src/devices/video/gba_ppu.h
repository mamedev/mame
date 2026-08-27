// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_ppu.h

    Nintendo Game Boy Advance / DS 2D graphics engine
	Formerly gba_lcd.cpp, now base class for GBA and NDS A & B 2D engines

***************************************************************************/
#ifndef MAME_VIDEO_GBA_PPU_H
#define MAME_VIDEO_GBA_PPU_H

#pragma once


class gba_ppu_device : public device_t
{
public:
	// the VRAM windows the engine fetches from
	enum : int
	{
		VRAM_BG = 0,
		VRAM_OBJ,
		VRAM_BG_EXTPAL,     // DS: four 8K slots of 16 palettes
		VRAM_OBJ_EXTPAL,    // DS: 8K, 16 palettes
		VRAM_REGION_COUNT
	};

	static constexpr int VRAM_PAGE_SHIFT = 14;      // the host supplies VRAM in 16K pages
	static constexpr int VRAM_MAX_PAGES = 32;       // engine A's 512K BG window
	static constexpr int MAX_WIDTH = 256;

	// system hookups
	void set_palette_ram(const uint32_t *pram) { m_pram = pram; }               // 1K: 256 BG colours then 256 OBJ colours
	void set_oam(const uint32_t *oam) { m_oam = oam; }                          // 1K
	void set_display_vram(const uint32_t *vram) { m_display_vram = vram; }      // DS engine A: banks A-D, for the VRAM display mode
	void set_vram_page(int region, int page, const uint32_t *data);
	void unmap_vram();

	// engine registers 0x000-0x06f as 32-bit words (DISPSTAT/VCOUNT at 0x004
	// belong to the LCD controller, so the host must handle that word itself)
	uint32_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// render one line as BGR555: 240 pixels for the GBA, 256 for the DS
	int width() const { return m_width; }
	void draw_scanline(int line, uint16_t *dest);

protected:
	gba_ppu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// per-variant configuration; each subclass sets the width and VRAM masks
	virtual void configure();

	// per-variant behaviour, consulted at most once per line or per BG (never in a pixel loop)
	virtual uint8_t bg_kind_for(int mode, int bg) const;
	virtual int display_mode(uint32_t dispcnt) const;
	virtual uint32_t char_base_ext(uint32_t dispcnt) const;
	virtual uint32_t map_base_ext(uint32_t dispcnt) const;
	virtual bool bg_extpal_enabled(uint32_t dispcnt) const;
	virtual bool obj_extpal_enabled(uint32_t dispcnt) const;
	virtual bool bitmap_uses_alpha() const;
	virtual bool obj_map_1d(uint32_t dispcnt) const;
	virtual uint32_t obj_tile_boundary(uint32_t dispcnt) const;
	virtual uint32_t obj_bmp_boundary(uint32_t dispcnt) const;
	virtual int obj_min_tile(uint32_t dispcnt) const;
	virtual bool obj_allow_bitmap() const;
	virtual bool bg0_is_3d(uint32_t dispcnt) const;
	virtual bool has_master_bright() const;

	// set by configure(); read on the hot path
	int m_width;
	uint32_t m_vram_mask[VRAM_REGION_COUNT];

private:
	uint32_t vram_read32(int region, uint32_t addr) const;
	uint16_t vram_read16(int region, uint32_t addr) const;
	uint8_t vram_read8(int region, uint32_t addr) const;
	uint16_t bg_pal(int index) const;
	uint16_t obj_pal(int index) const;
	uint16_t bg_extpal(int slot, int palette, int index) const;
	uint16_t obj_extpal(int palette, int index) const;
	uint16_t oam_r(int index) const;

	void reload_bg_ref(int bg, int xy);
	void render_line(int line, uint16_t *dest);
	void draw_text_bg(int bg, int line);
	void draw_affine_bg(int bg, int line, int kind);
	void apply_bg_mosaic(int bg);
	void draw_obj(int line);

	const uint32_t *m_vram_page[VRAM_REGION_COUNT][VRAM_MAX_PAGES];
	const uint32_t *m_pram;
	const uint32_t *m_oam;
	const uint32_t *m_display_vram;

	uint32_t m_regs[0x70 / 4];
	int32_t m_bg_ref[2][2];             // BG2/BG3 internal reference point counters [bg-2][x,y]
	int32_t m_bg_ref_mosaic[2][2];      // the same as latched at the top of the current mosaic block

	// per-line scratch: BGR555 with bit 15 set when opaque
	uint16_t m_bgline[4][MAX_WIDTH];
	uint16_t m_objline[MAX_WIDTH];
	uint8_t m_objprio[MAX_WIDTH];
	uint8_t m_objattr[MAX_WIDTH];
	uint8_t m_objwin[MAX_WIDTH];
};


// the DS main engine (A)
class gba_ppu_nds_a_device : public gba_ppu_device
{
public:
	gba_ppu_nds_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_gba_mode(bool gba_mode);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void configure() override;
	virtual uint8_t bg_kind_for(int mode, int bg) const override;
	virtual int display_mode(uint32_t dispcnt) const override;
	virtual uint32_t char_base_ext(uint32_t dispcnt) const override;
	virtual uint32_t map_base_ext(uint32_t dispcnt) const override;
	virtual bool bg_extpal_enabled(uint32_t dispcnt) const override;
	virtual bool obj_extpal_enabled(uint32_t dispcnt) const override;
	virtual bool bitmap_uses_alpha() const override;
	virtual bool obj_map_1d(uint32_t dispcnt) const override;
	virtual uint32_t obj_tile_boundary(uint32_t dispcnt) const override;
	virtual uint32_t obj_bmp_boundary(uint32_t dispcnt) const override;
	virtual int obj_min_tile(uint32_t dispcnt) const override;
	virtual bool obj_allow_bitmap() const override;
	virtual bool bg0_is_3d(uint32_t dispcnt) const override;
	virtual bool has_master_bright() const override;

private:
	bool m_gba_mode;
};


// the DS sub engine (B)
class gba_ppu_nds_b_device : public gba_ppu_device
{
public:
	gba_ppu_nds_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void configure() override;
	virtual uint8_t bg_kind_for(int mode, int bg) const override;
	virtual int display_mode(uint32_t dispcnt) const override;
	virtual bool bg_extpal_enabled(uint32_t dispcnt) const override;
	virtual bool obj_extpal_enabled(uint32_t dispcnt) const override;
	virtual bool bitmap_uses_alpha() const override;
	virtual bool obj_map_1d(uint32_t dispcnt) const override;
	virtual uint32_t obj_tile_boundary(uint32_t dispcnt) const override;
	virtual uint32_t obj_bmp_boundary(uint32_t dispcnt) const override;
	virtual int obj_min_tile(uint32_t dispcnt) const override;
	virtual bool obj_allow_bitmap() const override;
	virtual bool has_master_bright() const override;
};

DECLARE_DEVICE_TYPE(NDS_PPU_A, gba_ppu_nds_a_device)
DECLARE_DEVICE_TYPE(NDS_PPU_B, gba_ppu_nds_b_device)

#endif // MAME_VIDEO_GBA_PPU_H
