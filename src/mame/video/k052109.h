#pragma once
#ifndef __K052109_H__
#define __K052109_H__


typedef void (*k052109_callback)(running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority);

struct k052109_interface
{
	const char         *m_gfx_memory_region;
	int                m_gfx_num;
	int                m_plane_order;
	int                m_deinterleave;
	k052109_callback   m_callback;
};

class k052109_device : public device_t,
										public k052109_interface
{
public:
	k052109_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k052109_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

	/*
	You don't have to decode the graphics: the vh_start() routines will do that
	for you, using the plane order passed.
	Of course the ROM data must be in the correct order. This is a way to ensure
	that the ROM test will pass.
	The konami_rom_deinterleave() function in konami_helper.h will do the reorganization for
	you in most cases (but see tmnt.c for additional bit rotations or byte
	permutations which may be required).

	The callback is passed:
	- layer number (0 = FIX, 1 = A, 2 = B)
	- bank (range 0-3, output of the pins CAB1 and CAB2)
	- code (range 00-FF, output of the pins VC3-VC10)
	NOTE: code is in the range 0000-FFFF for X-Men, which uses extra RAM
	- color (range 00-FF, output of the pins COL0-COL7)
	The callback must put:
	- in code the resulting tile number
	- in color the resulting color index
	- if necessary, put flags and/or priority for the TileMap code in the tile_info
	structure (e.g. TILE_FLIPX). Note that TILE_FLIPY is handled internally by the
	chip so it must not be set by the callback.
	*/

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	DECLARE_READ16_MEMBER( lsb_r );
	DECLARE_WRITE16_MEMBER( lsb_w );

	void set_rmrd_line(int state);
	int get_rmrd_line();
	void tilemap_update();
	int is_irq_enabled();
	void set_layer_offsets(int layer, int dx, int dy);
	void tilemap_mark_dirty(int tmap_num);
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, UINT32 flags, UINT8 priority);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;
	UINT8    *m_videoram_F;
	UINT8    *m_videoram_A;
	UINT8    *m_videoram_B;
	UINT8    *m_videoram2_F;
	UINT8    *m_videoram2_A;
	UINT8    *m_videoram2_B;
	UINT8    *m_colorram_F;
	UINT8    *m_colorram_A;
	UINT8    *m_colorram_B;

	tilemap_t  *m_tilemap[3];
	int      m_tileflip_enable;
	UINT8    m_charrombank[4];
	UINT8    m_charrombank_2[4];
	UINT8    m_has_extra_video_ram;
	INT32    m_rmrd_line;
	UINT8    m_irq_enabled;
	INT32    m_dx[3], m_dy[3];
	UINT8    m_romsubbank, m_scrollctrl;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	void get_tile_info( tile_data &tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram1, UINT8 *vram2 );
	void tileflip_reset();
};

extern const device_type K052109;

#define MCFG_K052109_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K052109, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K052109_GFXDECODE(_gfxtag) \
	k052109_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_K052109_PALETTE(_palette_tag) \
	k052109_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
