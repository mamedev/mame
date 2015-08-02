// license:BSD-3-Clause
// copyright-holders:David Graves, R. Belmont

#include "sound/okim6295.h"
#include "sound/msm5205.h"
#include "video/excellent_spr.h"

class gcpinbal_state : public driver_device
{
public:
	enum
	{
		TIMER_GCPINBAL_INTERRUPT1,
		TIMER_GCPINBAL_INTERRUPT3
	};

	gcpinbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_msm(*this, "msm"),
		m_tilemapram(*this, "tilemapram"),
		m_ioc_ram(*this, "ioc_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<msm6585_device> m_msm;

	/* memory pointers */
	required_shared_ptr<UINT16> m_tilemapram;
	required_shared_ptr<UINT16> m_ioc_ram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t     *m_tilemap[3];
	UINT16      m_scrollx[3];
	UINT16      m_scrolly[3];
	UINT16      m_bg0_gfxset;
	UINT16      m_bg1_gfxset;
#ifdef MAME_DEBUG
	UINT8       m_dislayer[4];
#endif

	/* sound-related */
	UINT32      m_msm_start;
	UINT32      m_msm_end;
	UINT32      m_msm_bank;
	UINT32      m_adpcm_start;
	UINT32      m_adpcm_end;
	UINT32      m_adpcm_idle;
	UINT8       m_adpcm_trigger;
	UINT8       m_adpcm_data;

	DECLARE_READ16_MEMBER(ioc_r);
	DECLARE_WRITE16_MEMBER(ioc_w);
	DECLARE_READ16_MEMBER(gcpinbal_tilemaps_word_r);
	DECLARE_WRITE16_MEMBER(gcpinbal_tilemaps_word_w);
	DECLARE_READ16_MEMBER(gcpinbal_ctrl_word_r);
	DECLARE_WRITE16_MEMBER(gcpinbal_ctrl_word_w);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_gcpinbal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gcpinbal_interrupt);
	void gcpinbal_core_vh_start(  );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs );
	DECLARE_WRITE_LINE_MEMBER(gcp_adpcm_int);
	required_device<excellent_spr_device> m_sprgen;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
