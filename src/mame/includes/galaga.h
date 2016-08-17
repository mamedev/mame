// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/discrete.h"
#include "sound/namco.h"
#include "sound/samples.h"

class galaga_state : public driver_device
{
public:
	galaga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_galaga_ram1(*this, "galaga_ram1"),
		m_galaga_ram2(*this, "galaga_ram2"),
		m_galaga_ram3(*this, "galaga_ram3"),
		m_galaga_starcontrol(*this, "starcontrol"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_sound(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	optional_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_galaga_ram1;
	optional_shared_ptr<UINT8> m_galaga_ram2;
	optional_shared_ptr<UINT8> m_galaga_ram3;
	optional_shared_ptr<UINT8> m_galaga_starcontrol;    // 6 addresses
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_device> m_namco_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	emu_timer *m_cpu3_interrupt_timer;
	UINT8 m_custom_mod;

	/* machine state */
	UINT32 m_stars_scrollx;
	UINT32 m_stars_scrolly;

	UINT32 m_galaga_gfxbank; // used by catsbee

	/* devices */

	/* bank support */

	/* shared */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_nmi_mask;
	DECLARE_READ8_MEMBER(bosco_dsw_r);
	DECLARE_WRITE8_MEMBER(galaga_flip_screen_w);
	DECLARE_WRITE8_MEMBER(bosco_latch_w);
	DECLARE_WRITE8_MEMBER(galaga_videoram_w);
	DECLARE_WRITE8_MEMBER(gatsbee_bank_w);
	DECLARE_WRITE8_MEMBER(out_0);
	DECLARE_WRITE8_MEMBER(out_1);
	DECLARE_READ8_MEMBER(namco_52xx_rom_r);
	DECLARE_READ8_MEMBER(namco_52xx_si_r);
	DECLARE_READ8_MEMBER(custom_mod_r);
	DECLARE_DRIVER_INIT(galaga);
	DECLARE_DRIVER_INIT(gatsbee);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_MACHINE_START(galaga);
	DECLARE_MACHINE_RESET(galaga);
	DECLARE_VIDEO_START(galaga);
	DECLARE_PALETTE_INIT(galaga);
	UINT32 screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_galaga(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub_vblank_irq);
	TIMER_CALLBACK_MEMBER(cpu3_interrupt_callback);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void bosco_latch_reset();
	struct star
	{
		UINT16 x,y;
		UINT8 col,set;
	};

	static struct star m_star_seed_tab[];
};

DISCRETE_SOUND_EXTERN( galaga );
DISCRETE_SOUND_EXTERN( bosco );
