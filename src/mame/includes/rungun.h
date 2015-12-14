// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************

    Run and Gun / Slam Dunk

*************************************************************************/

#include "sound/k054539.h"
#include "machine/gen_latch.h"
#include "machine/k053252.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053936.h"
#include "video/konami_helper.h"
#include "screen.h"

class rungun_state : public driver_device
{
public:
	rungun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_k054539_1(*this, "k054539_1"),
		m_k054539_2(*this, "k054539_2"),
		m_k053936(*this, "k053936"),
		m_sprites(*this, "sprites"),
		m_video_timings(*this, "video_timings"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_palette2(*this, "palette2"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_sysreg(*this, "sysreg")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<k054539_device> m_k054539_1;
	required_device<k054539_device> m_k054539_2;
	required_device<k053936_device> m_k053936;
	required_device<k053246_055673_device> m_sprites;
	required_device<k053252_device> m_video_timings;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_palette2;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_sysreg;

	/* video-related */
	tilemap_t   *m_ttl_tilemap[2];
	tilemap_t   *m_936_tilemap[2];
	std::unique_ptr<uint16_t[]> m_psac2_vram;
	std::unique_ptr<uint16_t[]>    m_ttl_vram;
	std::unique_ptr<uint16_t[]>   m_pal_ram;
	uint8_t       m_current_display_bank;
	int         m_ttl_gfx_index;
	int         m_sprite_colorbase;

	uint8_t       *m_roz_rom;
	uint8_t       m_roz_rombase;

	/* sound */
	uint8_t       m_sound_ctrl;
	uint8_t       m_sound_status;
	uint8_t       m_sound_nmi_clk;

	bool        m_video_priority_mode;
	std::unique_ptr<uint16_t[]> m_banked_ram;
	bool        m_single_screen_mode;
	uint8_t       m_video_mux_bank;

	DECLARE_READ16_MEMBER(rng_sysregs_r);
	DECLARE_WRITE16_MEMBER(rng_sysregs_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_msb_r);
	DECLARE_WRITE8_MEMBER(sound_status_w);
	DECLARE_WRITE8_MEMBER(sound_ctrl_w);
	DECLARE_READ16_MEMBER(rng_ttl_ram_r);
	DECLARE_WRITE16_MEMBER(rng_ttl_ram_w);
	DECLARE_READ16_MEMBER(rng_psac2_videoram_r);
	DECLARE_WRITE16_MEMBER(rng_psac2_videoram_w);
	DECLARE_READ8_MEMBER(rng_53936_rom_r);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(get_rng_936_tile_info);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);
	DECLARE_READ16_MEMBER(palette_read);
	DECLARE_WRITE16_MEMBER(palette_write);


	virtual void machine_start() override;
	virtual void machine_reset() override;
	bitmap_ind16 m_rng_dual_demultiplex_left_temp;
	bitmap_ind16 m_rng_dual_demultiplex_right_temp;

	INTERRUPT_GEN_MEMBER(rng_interrupt);
};
