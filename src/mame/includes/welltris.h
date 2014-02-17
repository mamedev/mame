#include "video/vsystem_spr2.h"

class welltris_state : public driver_device
{
public:
	welltris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"),
		m_charvideoram(*this, "charvideoram"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	int m_pending_command;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_pixelram;
	required_shared_ptr<UINT16> m_charvideoram;

	/* devices referenced above */
	required_device<vsystem_spr2_device> m_spr_old;

	tilemap_t *m_char_tilemap;
	UINT8 m_gfxbank[8];
	UINT16 m_charpalettebank;
	UINT16 m_spritepalettebank;
	UINT16 m_pixelpalettebank;
	int m_scrollx;
	int m_scrolly;
	DECLARE_WRITE8_MEMBER(welltris_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE16_MEMBER(welltris_palette_bank_w);
	DECLARE_WRITE16_MEMBER(welltris_gfxbank_w);
	DECLARE_WRITE16_MEMBER(welltris_scrollreg_w);
	DECLARE_WRITE16_MEMBER(welltris_charvideoram_w);
	void setbank(int num, int bank);
	DECLARE_CUSTOM_INPUT_MEMBER(pending_sound_r);
	DECLARE_DRIVER_INIT(quiz18k);
	DECLARE_DRIVER_INIT(welltris);
	TILE_GET_INFO_MEMBER(get_welltris_tile_info);
	virtual void video_start();
	UINT32 screen_update_welltris(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
