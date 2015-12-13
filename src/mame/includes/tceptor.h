// license:BSD-3-Clause
// copyright-holders:BUT
#include "sound/dac.h"
#include "sound/namco.h"
#include "video/c45.h"

class tceptor_state : public driver_device
{
public:
	tceptor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cus30(*this, "namco"),
		m_dac(*this, "dac"),
		m_tile_ram(*this, "tile_ram"),
		m_tile_attr(*this, "tile_attr"),
		m_bg_ram(*this, "bg_ram"),
		m_m68k_shared_ram(*this, "m68k_shared_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_c45_road(*this, "c45_road"),
		m_2dscreen(*this, "2dscreen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 m_m6809_irq_enable;
	UINT8 m_m68k_irq_enable;
	UINT8 m_mcu_irq_enable;
	required_device<cpu_device> m_maincpu;
	required_device<namco_cus30_device> m_cus30;
	required_device<dac_device> m_dac;
	required_shared_ptr<UINT8> m_tile_ram;
	required_shared_ptr<UINT8> m_tile_attr;
	required_shared_ptr<UINT8> m_bg_ram;
	required_shared_ptr<UINT8> m_m68k_shared_ram;
	required_shared_ptr<UINT16> m_sprite_ram;
	int m_sprite16;
	int m_sprite32;
	int m_bg;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;
	INT32 m_bg1_scroll_x;
	INT32 m_bg1_scroll_y;
	INT32 m_bg2_scroll_x;
	INT32 m_bg2_scroll_y;
	bitmap_ind16 m_temp_bitmap;
	UINT16 *m_sprite_ram_buffered;
	int m_is_mask_spr[1024/16];
	DECLARE_READ8_MEMBER(m68k_shared_r);
	DECLARE_WRITE8_MEMBER(m68k_shared_w);
	DECLARE_WRITE8_MEMBER(m6809_irq_enable_w);
	DECLARE_WRITE8_MEMBER(m6809_irq_disable_w);
	DECLARE_WRITE16_MEMBER(m68k_irq_enable_w);
	DECLARE_WRITE8_MEMBER(mcu_irq_enable_w);
	DECLARE_WRITE8_MEMBER(mcu_irq_disable_w);
	DECLARE_READ8_MEMBER(dsw0_r);
	DECLARE_READ8_MEMBER(dsw1_r);
	DECLARE_READ8_MEMBER(input0_r);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_READ8_MEMBER(readFF);
	DECLARE_WRITE8_MEMBER(tceptor_tile_ram_w);
	DECLARE_WRITE8_MEMBER(tceptor_tile_attr_w);
	DECLARE_WRITE8_MEMBER(tceptor_bg_ram_w);
	DECLARE_WRITE8_MEMBER(tceptor_bg_scroll_w);
	void tile_mark_dirty(int offset);
	DECLARE_WRITE8_MEMBER(voice_w);

	required_device<namco_c45_road_device> m_c45_road;
	required_device<screen_device> m_2dscreen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(tceptor);
	UINT32 screen_update_tceptor_2d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tceptor_3d_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tceptor_3d_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tceptor(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(m6809_vb_interrupt);
	INTERRUPT_GEN_MEMBER(m68k_vb_interrupt);
	INTERRUPT_GEN_MEMBER(mcu_vb_interrupt);
	inline int get_tile_addr(int tile_index);
	void decode_bg(const char * region);
	void decode_sprite(int gfx_index, const gfx_layout *layout, const void *data);
	void decode_sprite16(const char * region);
	void decode_sprite32(const char * region);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority);
	inline UINT8 fix_input0(UINT8 in1, UINT8 in2);
	inline UINT8 fix_input1(UINT8 in1, UINT8 in2);
};
