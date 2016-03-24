// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "video/vsystem_spr2.h"

class welltris_state : public driver_device
{
public:
	welltris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"),
		m_charvideoram(*this, "charvideoram") { }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<vsystem_spr2_device> m_spr_old;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_pixelram;
	required_shared_ptr<UINT16> m_charvideoram;

	tilemap_t *m_char_tilemap;
	int m_pending_command;
	UINT8 m_gfxbank[2];
	UINT16 m_charpalettebank;
	UINT16 m_spritepalettebank;
	UINT16 m_pixelpalettebank;
	int m_scrollx;
	int m_scrolly;

	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE16_MEMBER(palette_bank_w);
	DECLARE_WRITE16_MEMBER(gfxbank_w);
	DECLARE_WRITE16_MEMBER(scrollreg_w);
	DECLARE_WRITE16_MEMBER(charvideoram_w);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);

	DECLARE_CUSTOM_INPUT_MEMBER(pending_sound_r);

	DECLARE_DRIVER_INIT(quiz18k);
	DECLARE_DRIVER_INIT(welltris);
	virtual void machine_start() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_tile_info);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void setbank(int num, int bank);
};
