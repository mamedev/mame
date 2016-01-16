// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Manuel Abadia
/*************************************************************************

    Combat School

*************************************************************************/
#include "sound/upd7759.h"
#include "sound/msm5205.h"
#include "video/k007121.h"

class combatsc_state : public driver_device
{
public:
	combatsc_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_k007121_1(*this, "k007121_1"),
		m_k007121_2(*this, "k007121_2"),
		m_maincpu(*this, "maincpu"),
		m_upd7759(*this, "upd"),
		m_msm5205(*this, "msm5205"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_scrollram;
	UINT8 *    m_io_ram;
	std::unique_ptr<UINT8[]>    m_spriteram[2];

	/* video-related */
	tilemap_t *m_bg_tilemap[2];
	tilemap_t *m_textlayer;
	UINT8 m_scrollram0[0x40];
	UINT8 m_scrollram1[0x40];
	int m_priority;

	int  m_vreg;
	int  m_bank_select; /* 0x00..0x1f */
	int  m_video_circuit; /* 0 or 1 */
	UINT8 *m_page[2];

	/* misc */
	UINT8 m_pos[4];
	UINT8 m_sign[4];
	int m_prot[2];
	int m_boost;
	emu_timer *m_interleave_timer;


	/* devices */
	required_device<cpu_device> m_audiocpu;
	optional_device<k007121_device> m_k007121_1;
	optional_device<k007121_device> m_k007121_2;
	required_device<cpu_device> m_maincpu;
	optional_device<upd7759_device> m_upd7759;
	optional_device<msm5205_device> m_msm5205;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(combatsc_vreg_w);
	DECLARE_WRITE8_MEMBER(combatscb_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(combatscb_io_r);
	DECLARE_WRITE8_MEMBER(combatscb_priority_w);
	DECLARE_WRITE8_MEMBER(combatsc_bankselect_w);
	DECLARE_WRITE8_MEMBER(combatscb_io_w);
	DECLARE_WRITE8_MEMBER(combatscb_bankselect_w);
	DECLARE_WRITE8_MEMBER(combatsc_coin_counter_w);
	DECLARE_READ8_MEMBER(trackball_r);
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(protection_clock_w);
	DECLARE_WRITE8_MEMBER(combatsc_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(combatsc_video_r);
	DECLARE_WRITE8_MEMBER(combatsc_video_w);
	DECLARE_WRITE8_MEMBER(combatsc_pf_control_w);
	DECLARE_READ8_MEMBER(combatsc_scrollram_r);
	DECLARE_WRITE8_MEMBER(combatsc_scrollram_w);
	DECLARE_READ8_MEMBER(combatsc_busy_r);
	DECLARE_WRITE8_MEMBER(combatsc_play_w);
	DECLARE_WRITE8_MEMBER(combatsc_voice_reset_w);
	DECLARE_WRITE8_MEMBER(combatsc_portA_w);
	DECLARE_WRITE8_MEMBER(combatscb_dac_w);
	DECLARE_DRIVER_INIT(combatsc);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_text_info);
	TILE_GET_INFO_MEMBER(get_tile_info0_bootleg);
	TILE_GET_INFO_MEMBER(get_tile_info1_bootleg);
	TILE_GET_INFO_MEMBER(get_text_info_bootleg);
	virtual void machine_reset() override;
	DECLARE_MACHINE_START(combatsc);
	DECLARE_VIDEO_START(combatsc);
	DECLARE_PALETTE_INIT(combatsc);
	DECLARE_MACHINE_START(combatscb);
	DECLARE_VIDEO_START(combatscb);
	DECLARE_PALETTE_INIT(combatscb);
	UINT32 screen_update_combatsc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_combatscb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *source, int circuit, bitmap_ind8 &priority_bitmap, UINT32 pri_mask );
	void bootleg_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *source, int circuit );
};
