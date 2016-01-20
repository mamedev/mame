// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
#include "sound/msm5205.h"

class splash_state : public driver_device
{
public:
	splash_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_pixelram(*this, "pixelram"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_protdata(*this, "protdata"),
		m_bitmap_mode(*this, "bitmap_mode"),

		m_funystrp_val(0),
		m_funystrp_ff3cc7_val(0),
		m_funystrp_ff3cc8_val(0)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	optional_device<msm5205_device> m_msm1;
	optional_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_pixelram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_vregs;
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_protdata;
	optional_shared_ptr<UINT16> m_bitmap_mode;

	// driver init configuration
	int m_bitmap_type;
	int m_sprite_attr2_shift;

	tilemap_t *m_bg_tilemap[2];

	// splash specific
	int m_adpcm_data;

	//roldfrog specific
	int m_ret;
	int m_vblank_irq;
	int m_sound_irq;

	// funystrp specific
	UINT8 m_funystrp_val;
	UINT8 m_funystrp_ff3cc7_val;
	UINT8 m_funystrp_ff3cc8_val;
	int m_msm_data1;
	int m_msm_data2;
	int m_msm_toggle1;
	int m_msm_toggle2;
	int m_msm_source;
	int m_snd_interrupt_enable1;
	int m_snd_interrupt_enable2;

	// common
	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE16_MEMBER(coin_w);

	// splash specific
	DECLARE_WRITE_LINE_MEMBER(splash_msm5205_int);
	DECLARE_WRITE16_MEMBER(splash_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(splash_adpcm_data_w);

	// roldfrog specific
	DECLARE_WRITE16_MEMBER(roldf_sh_irqtrigger_w);
	DECLARE_READ16_MEMBER(roldfrog_bombs_r);
	DECLARE_WRITE8_MEMBER(roldfrog_vblank_ack_w);
	DECLARE_READ8_MEMBER(roldfrog_unk_r);
	DECLARE_WRITE_LINE_MEMBER(ym_irq);

	// funystrp specific
	DECLARE_READ16_MEMBER(spr_read);
	DECLARE_WRITE16_MEMBER(spr_write);
	DECLARE_READ8_MEMBER(int_source_r);
	DECLARE_WRITE8_MEMBER(msm1_data_w);
	DECLARE_WRITE8_MEMBER(msm1_interrupt_w);
	DECLARE_WRITE8_MEMBER(msm2_interrupt_w);
	DECLARE_WRITE8_MEMBER(msm2_data_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int1);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int2);
	DECLARE_WRITE16_MEMBER(funystrp_protection_w);
	DECLARE_READ16_MEMBER(funystrp_protection_r);
	DECLARE_WRITE16_MEMBER(funystrp_sh_irqtrigger_w);

	//roldfrog and funystrp specific
	DECLARE_WRITE8_MEMBER(sound_bank_w);

	DECLARE_DRIVER_INIT(splash10);
	DECLARE_DRIVER_INIT(roldfrog);
	DECLARE_DRIVER_INIT(splash);
	DECLARE_DRIVER_INIT(rebus);
	DECLARE_DRIVER_INIT(funystrp);
	virtual void video_start() override;
	DECLARE_MACHINE_START(splash);
	DECLARE_MACHINE_START(roldfrog);
	DECLARE_MACHINE_START(funystrp);
	DECLARE_MACHINE_RESET(splash);
	DECLARE_MACHINE_RESET(funystrp);

	TILE_GET_INFO_MEMBER(get_tile_info_tilemap0);
	TILE_GET_INFO_MEMBER(get_tile_info_tilemap1);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_funystrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void funystrp_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(roldfrog_interrupt);
	void roldfrog_update_irq(  );
};
