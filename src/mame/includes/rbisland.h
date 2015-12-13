// license:BSD-3-Clause
// copyright-holders:Mike Coates
/*************************************************************************

    Rainbow Islands

*************************************************************************/

#include "video/pc080sn.h"
#include "video/pc090oj.h"

class rbisland_state : public driver_device
{
public:
	rbisland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	UINT16      m_sprite_ctrl;
	UINT16      m_sprites_flipscreen;

	/* misc */
	UINT8       m_jumping_latch;

	/* c-chip */
	UINT8       *m_CRAM[8];
	int         m_extra_version;
	UINT8       m_current_bank;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<pc080sn_device> m_pc080sn;
	optional_device<pc090oj_device> m_pc090oj;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER(jumping_sound_w);
	DECLARE_READ8_MEMBER(jumping_latch_r);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_ctrl_w);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_bank_w);
	DECLARE_WRITE16_MEMBER(rbisland_cchip_ram_w);
	DECLARE_READ16_MEMBER(rbisland_cchip_ctrl_r);
	DECLARE_READ16_MEMBER(rbisland_cchip_ram_r);
	DECLARE_WRITE16_MEMBER(rbisland_spritectrl_w);
	DECLARE_WRITE16_MEMBER(jumping_spritectrl_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_DRIVER_INIT(jumping);
	DECLARE_DRIVER_INIT(rbislande);
	DECLARE_DRIVER_INIT(rbisland);
	virtual void machine_start() override;
	DECLARE_VIDEO_START(jumping);
	UINT32 screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_jumping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(cchip_timer);
	void request_round_data(  );
	void request_world_data(  );
	void request_goalin_data(  );
	void rbisland_cchip_init( int version );
};
