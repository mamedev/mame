#include "sound/okim6295.h"

class fuuki16_state : public driver_device
{
public:
	enum
	{
		TIMER_LEVEL_1_INTERRUPT,
		TIMER_VBLANK_INTERRUPT,
		TIMER_RASTER_INTERRUPT
	};

	fuuki16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_vregs(*this, "vregs"),
		m_unknown(*this, "unknown"),
		m_priority(*this, "priority"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr_array<UINT16,4> m_vram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_vregs;
	required_shared_ptr<UINT16> m_unknown;
	required_shared_ptr<UINT16> m_priority;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *m_tilemap[4];

	/* misc */
	emu_timer   *m_raster_interrupt_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE16_MEMBER(fuuki16_vregs_w);
	DECLARE_WRITE16_MEMBER(fuuki16_sound_command_w);
	DECLARE_WRITE8_MEMBER(fuuki16_sound_rombank_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_0_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_1_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_2_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_3_w);
	DECLARE_WRITE8_MEMBER(fuuki16_oki_banking_w);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_2);
	TILE_GET_INFO_MEMBER(get_tile_info_3);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_fuuki16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_);
	inline void fuuki16_vram_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _N_);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void fuuki16_draw_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int flag, int pri );
	DECLARE_WRITE_LINE_MEMBER(soundirq);
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
