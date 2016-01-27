// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "machine/tmp68301.h"

class realbrk_state : public driver_device
{
public:
	realbrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tmp68301(*this, "tmp68301"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_vregs(*this, "vregs"),
		m_dsw_select(*this, "dsw_select"),
		m_backup_ram(*this, "backup_ram"),
		m_vram_0ras(*this, "vram_0ras"),
		m_vram_1ras(*this, "vram_1ras") { }

	required_device<cpu_device> m_maincpu;
	required_device<tmp68301_device> m_tmp68301;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_vram_2;
	required_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr<UINT16> m_dsw_select;
	optional_shared_ptr<UINT16> m_backup_ram;
	optional_shared_ptr<UINT16> m_vram_0ras;
	optional_shared_ptr<UINT16> m_vram_1ras;

	std::unique_ptr<bitmap_ind16> m_tmpbitmap0;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap1;
	int m_disable_video;
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;

	// common
	DECLARE_WRITE16_MEMBER(vram_0_w);
	DECLARE_WRITE16_MEMBER(vram_1_w);
	DECLARE_WRITE16_MEMBER(vram_2_w);
	DECLARE_WRITE16_MEMBER(vregs_w);

	// realbrk and/or dai2kaku
	DECLARE_READ16_MEMBER(realbrk_dsw_r);
	DECLARE_WRITE16_MEMBER(realbrk_flipscreen_w);
	DECLARE_WRITE16_MEMBER(dai2kaku_flipscreen_w);

	// pkgnsh and/or pkgnshdx
	DECLARE_READ16_MEMBER(pkgnsh_input_r);
	DECLARE_READ16_MEMBER(pkgnshdx_input_r);
	DECLARE_READ16_MEMBER(backup_ram_r);
	DECLARE_READ16_MEMBER(backup_ram_dx_r);
	DECLARE_WRITE16_MEMBER(backup_ram_w);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dai2kaku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void dai2kaku_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int layer);

	INTERRUPT_GEN_MEMBER(interrupt);
};
