#include "sound/discrete.h"

class wiz_state : public driver_device
{
public:
	wiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_attributesram2(*this, "attributesram2"),
		m_spriteram2(*this, "spriteram2"),
		m_videoram(*this, "videoram"),
		m_attributesram(*this, "attributesram"),
		m_spriteram(*this, "spriteram"),
		m_sprite_bank(*this, "sprite_bank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_dsc0;
	int m_dsc1;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_attributesram2;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_attributesram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_sprite_bank;

	INT32 m_flipx;
	INT32 m_flipy;
	INT32 m_bgpen;
	UINT8 m_char_bank[2];
	UINT8 m_palbank[2];
	int m_palette_bank;
	UINT8 m_main_nmi_mask;
	UINT8 m_sound_nmi_mask;

	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_READ8_MEMBER(wiz_protection_r);
	DECLARE_WRITE8_MEMBER(wiz_coin_counter_w);
	DECLARE_WRITE8_MEMBER(wiz_main_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(wiz_sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(wiz_palettebank_w);
	DECLARE_WRITE8_MEMBER(wiz_bgcolor_w);
	DECLARE_WRITE8_MEMBER(wiz_char_bank_select_w);
	DECLARE_WRITE8_MEMBER(wiz_flipx_w);
	DECLARE_WRITE8_MEMBER(wiz_flipy_w);
	DECLARE_DRIVER_INIT(wiz);
	DECLARE_DRIVER_INIT(scion);
	DECLARE_DRIVER_INIT(stinger);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(wiz);
	UINT32 screen_update_wiz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stinger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kungfut(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(wiz_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(wiz_sound_interrupt);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int bank, int colortype);
	void draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect, int colortype);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, UINT8* sprite_ram,int bank);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
