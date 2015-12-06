// license:???
// copyright-holders:Edgardo E. Contini Salvan
#include "machine/namcoio.h"
#include "sound/namco.h"

class toypop_state : public driver_device
{
public:
	enum
	{
		TIMER_NAMCOIO_RUN
	};

	toypop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_m68000_sharedram(*this, "m68k_shared"),
		m_bg_image(*this, "bg_image"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_namco15xx(*this, "namco"),
		m_namco58xx(*this, "58xx"),
		m_namco56xx_1(*this, "56xx_1"),
		m_namco56xx_2(*this, "56xx_2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_m68000_sharedram;
	required_shared_ptr<UINT16> m_bg_image;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<namco_15xx_device> m_namco15xx;
	required_device<namco58xx_device> m_namco58xx;
	required_device<namco56xx_device> m_namco56xx_1;
	required_device<namco56xx_device> m_namco56xx_2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	tilemap_t *m_bg_tilemap;

	int m_bitmapflip;
	int m_palettebank;
	int m_interrupt_enable_68k;
	UINT8 m_main_irq_mask;
	UINT8 m_sound_irq_mask;
	DECLARE_READ16_MEMBER(toypop_m68000_sharedram_r);
	DECLARE_WRITE16_MEMBER(toypop_m68000_sharedram_w);
	DECLARE_READ8_MEMBER(toypop_main_interrupt_enable_r);
	DECLARE_WRITE8_MEMBER(toypop_main_interrupt_enable_w);
	DECLARE_WRITE8_MEMBER(toypop_main_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_interrupt_enable_acknowledge_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_clear_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_assert_w);
	DECLARE_WRITE8_MEMBER(toypop_m68000_clear_w);
	DECLARE_WRITE8_MEMBER(toypop_m68000_assert_w);
	DECLARE_WRITE16_MEMBER(toypop_m68000_interrupt_enable_w);
	DECLARE_WRITE16_MEMBER(toypop_m68000_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_videoram_w);
	DECLARE_WRITE8_MEMBER(toypop_palettebank_w);
	DECLARE_WRITE16_MEMBER(toypop_flipscreen_w);
	DECLARE_READ16_MEMBER(toypop_merged_background_r);
	DECLARE_WRITE16_MEMBER(toypop_merged_background_w);
	DECLARE_READ8_MEMBER(dipA_l);
	DECLARE_READ8_MEMBER(dipA_h);
	DECLARE_READ8_MEMBER(dipB_l);
	DECLARE_READ8_MEMBER(dipB_h);
	DECLARE_WRITE8_MEMBER(out_coin0);
	DECLARE_WRITE8_MEMBER(out_coin1);
	DECLARE_WRITE8_MEMBER(flip);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(toypop);
	UINT32 screen_update_toypop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(toypop_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(toypop_sound_timer_irq);
	INTERRUPT_GEN_MEMBER(toypop_m68000_interrupt);
	TIMER_CALLBACK_MEMBER(namcoio_run);
	void draw_background(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *spriteram_base);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
