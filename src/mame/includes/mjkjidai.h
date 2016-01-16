// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/nvram.h"
#include "sound/msm5205.h"

class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_adpcmrom(*this, "adpcm"),
		m_videoram(*this, "videoram"),
		m_row(*this, "ROW") { }

	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_region_ptr<UINT8> m_adpcmrom;
	required_shared_ptr<UINT8> m_videoram;

	required_ioport_array<12> m_row;

	int m_adpcm_pos;
	int m_adpcm_end;
	int m_keyb;
	bool m_nmi_enable;
	bool m_display_enable;
	tilemap_t *m_bg_tilemap;

	DECLARE_CUSTOM_INPUT_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_select_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_videoram_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_ctrl_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_mjkjidai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
