// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "sound/tms5220.h"

class zaccaria_state : public driver_device
{
public:
	zaccaria_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audio2(*this, "audio2"),
		m_pia0(*this, "pia0"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_tms(*this, "tms"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_attributesram(*this, "attributesram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_dsw_port(*this, "DSW") { }



	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audio2;
	required_device<pia6821_device> m_pia0;
	required_device<ay8910_device> m_ay1;
	required_device<ay8910_device> m_ay2;
	required_device<tms5220_device> m_tms;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_attributesram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram2;

	required_ioport_array<3> m_dsw_port;

	int m_dsw_sel;
	int m_active_8910;
	int m_port0a;
	int m_acs;
	int m_last_port0b;
	int m_toggle;
	tilemap_t *m_bg_tilemap;
	UINT8 m_nmi_mask;

	DECLARE_READ8_MEMBER(dsw_r);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(sound1_command_w);
	DECLARE_READ8_MEMBER(prot1_r);
	DECLARE_READ8_MEMBER(prot2_r);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(attributes_w);
	DECLARE_WRITE8_MEMBER(flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(flip_screen_y_w);
	DECLARE_CUSTOM_INPUT_MEMBER(acs_r);
	DECLARE_WRITE8_MEMBER(dsw_sel_w);
	DECLARE_WRITE8_MEMBER(ay8910_port0a_w);
	DECLARE_READ8_MEMBER(port0a_r);
	DECLARE_WRITE8_MEMBER(port0a_w);
	DECLARE_WRITE8_MEMBER(port0b_w);
	DECLARE_WRITE8_MEMBER(port1b_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(zaccaria);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cb1_toggle);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,UINT8 *spriteram,int color,int section);
};
