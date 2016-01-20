// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria
/*************************************************************************

    Pole Position hardware

*************************************************************************/

#include "sound/filter.h"
#include "sound/namco.h"
#include "sound/tms5220.h"
#include "sound/discrete.h"


class polepos_state : public driver_device
{
public:
	polepos_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_sound(*this, "namco"),
		m_tms(*this, "tms"),
		m_sprite16_memory(*this, "sprite16_memory"),
		m_road16_memory(*this, "road16_memory"),
		m_alpha16_memory(*this, "alpha16_memory"),
		m_view16_memory(*this, "view16_memory"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	optional_device<namco_device> m_namco_sound;
	optional_device<tms5220_device> m_tms;
	UINT8 m_steer_last;
	UINT8 m_steer_delta;
	INT16 m_steer_accum;
	INT16 m_last_result;
	INT8 m_last_signed;
	UINT8 m_last_unsigned;
	int m_adc_input;
	int m_auto_start_mask;
	required_shared_ptr<UINT16> m_sprite16_memory;
	required_shared_ptr<UINT16> m_road16_memory;
	required_shared_ptr<UINT16> m_alpha16_memory;
	required_shared_ptr<UINT16> m_view16_memory;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT16 m_vertical_position_modifier[256];
	UINT16 m_road16_vscroll;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_chacl;
	UINT16 m_scroll;
	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	DECLARE_READ16_MEMBER(polepos2_ic25_r);
	DECLARE_READ8_MEMBER(polepos_adc_r);
	DECLARE_READ8_MEMBER(polepos_ready_r);
	DECLARE_WRITE8_MEMBER(polepos_latch_w);
	DECLARE_WRITE16_MEMBER(polepos_z8002_nvi_enable_w);
	DECLARE_READ16_MEMBER(polepos_sprite16_r);
	DECLARE_WRITE16_MEMBER(polepos_sprite16_w);
	DECLARE_READ8_MEMBER(polepos_sprite_r);
	DECLARE_WRITE8_MEMBER(polepos_sprite_w);
	DECLARE_READ16_MEMBER(polepos_road16_r);
	DECLARE_WRITE16_MEMBER(polepos_road16_w);
	DECLARE_READ8_MEMBER(polepos_road_r);
	DECLARE_WRITE8_MEMBER(polepos_road_w);
	DECLARE_WRITE16_MEMBER(polepos_road16_vscroll_w);
	DECLARE_READ16_MEMBER(polepos_view16_r);
	DECLARE_WRITE16_MEMBER(polepos_view16_w);
	DECLARE_READ8_MEMBER(polepos_view_r);
	DECLARE_WRITE8_MEMBER(polepos_view_w);
	DECLARE_WRITE16_MEMBER(polepos_view16_hscroll_w);
	DECLARE_WRITE8_MEMBER(polepos_chacl_w);
	DECLARE_READ16_MEMBER(polepos_alpha16_r);
	DECLARE_WRITE16_MEMBER(polepos_alpha16_w);
	DECLARE_READ8_MEMBER(polepos_alpha_r);
	DECLARE_WRITE8_MEMBER(polepos_alpha_w);
	DECLARE_CUSTOM_INPUT_MEMBER(high_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(low_port_r);
	DECLARE_CUSTOM_INPUT_MEMBER(auto_start_r);
	DECLARE_WRITE8_MEMBER(out_0);
	DECLARE_WRITE8_MEMBER(out_1);
	DECLARE_READ8_MEMBER(namco_52xx_rom_r);
	DECLARE_READ8_MEMBER(namco_52xx_si_r);
	DECLARE_READ8_MEMBER(namco_53xx_k_r);
	DECLARE_READ8_MEMBER(steering_changed_r);
	DECLARE_READ8_MEMBER(steering_delta_r);
	DECLARE_DRIVER_INIT(topracern);
	DECLARE_DRIVER_INIT(polepos2);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	DECLARE_MACHINE_RESET(polepos);
	DECLARE_VIDEO_START(polepos);
	DECLARE_PALETTE_INIT(polepos);
	UINT32 screen_update_polepos(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(polepos_scanline);
	void draw_road(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void zoom_sprite(bitmap_ind16 &bitmap,int big,UINT32 code,UINT32 color,int flipx,int sx,int sy,int sizex,int sizey);
};


/*----------- defined in audio/polepos.c -----------*/

class polepos_sound_device : public device_t,
								public device_sound_interface
{
public:
	polepos_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~polepos_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( polepos_engine_sound_lsb_w );
	DECLARE_WRITE8_MEMBER( polepos_engine_sound_msb_w );

private:
	UINT32 m_current_position;
	int m_sample_msb;
	int m_sample_lsb;
	int m_sample_enable;
	sound_stream *m_stream;
	filter2_context m_filter_engine[3];
};

extern const device_type POLEPOS;

DISCRETE_SOUND_EXTERN( polepos );
