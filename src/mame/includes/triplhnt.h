// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Triple Hunt hardware

*************************************************************************/

#include "sound/discrete.h"
#include "sound/samples.h"


/* Discrete Sound Input Nodes */
#define TRIPLHNT_BEAR_ROAR_DATA NODE_01
#define TRIPLHNT_BEAR_EN        NODE_02
#define TRIPLHNT_SHOT_DATA      NODE_03
#define TRIPLHNT_SCREECH_EN     NODE_04
#define TRIPLHNT_LAMP_EN        NODE_05


class triplhnt_state : public driver_device
{
public:
	enum
	{
		TIMER_HIT
	};

	triplhnt_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_playfield_ram(*this, "playfield_ram"),
		m_vpos_ram(*this, "vpos_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_orga_ram(*this, "orga_ram"),
		m_code_ram(*this, "code_ram"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	UINT8 m_cmos[16];
	UINT8 m_da_latch;
	UINT8 m_misc_flags;
	UINT8 m_cmos_latch;
	UINT8 m_hit_code;
	required_shared_ptr<UINT8> m_playfield_ram;
	required_shared_ptr<UINT8> m_vpos_ram;
	required_shared_ptr<UINT8> m_hpos_ram;
	required_shared_ptr<UINT8> m_orga_ram;
	required_shared_ptr<UINT8> m_code_ram;
	int m_sprite_zoom;
	int m_sprite_bank;
	bitmap_ind16 m_helper;
	tilemap_t* m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(triplhnt_misc_w);
	DECLARE_READ8_MEMBER(triplhnt_cmos_r);
	DECLARE_READ8_MEMBER(triplhnt_input_port_4_r);
	DECLARE_READ8_MEMBER(triplhnt_misc_r);
	DECLARE_READ8_MEMBER(triplhnt_da_latch_r);
	DECLARE_DRIVER_INIT(triplhnt);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(triplhnt);
	UINT32 screen_update_triplhnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void triplhnt_set_collision(int code);
	void triplhnt_update_misc(address_space &space, int offset);
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in audio/triplhnt.c -----------*/
DISCRETE_SOUND_EXTERN( triplhnt );
extern const char *const triplhnt_sample_names[];
