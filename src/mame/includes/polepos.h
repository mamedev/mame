// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria
/*************************************************************************

    Pole Position hardware

*************************************************************************/

#include "sound/namco.h"
#include "sound/tms5220.h"
#include "sound/discrete.h"

struct filter2_context
{
	filter2_context() :
		x0(0.0),
		x1(0.0),
		x2(0.0),
		y0(0.0),
		y1(0.0),
		y2(0.0),
		a1(0.0),
		a2(0.0),
		b0(0.0),
		b1(0.0),
		b2(0.0)
	{}

	double x0, x1, x2;  /* x[k], x[k-1], x[k-2], current and previous 2 input values */
	double y0, y1, y2;  /* y[k], y[k-1], y[k-2], current and previous 2 output values */
	double a1, a2;      /* digital filter coefficients, denominator */
	double b0, b1, b2;  /* digital filter coefficients, numerator */
};


class polepos_state : public driver_device
{
public:
	polepos_state(const machine_config &mconfig, device_type type, const char *tag)
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
	uint8_t m_steer_last;
	uint8_t m_steer_delta;
	int16_t m_steer_accum;
	int16_t m_last_result;
	int8_t m_last_signed;
	uint8_t m_last_unsigned;
	int m_adc_input;
	int m_auto_start_mask;
	required_shared_ptr<uint16_t> m_sprite16_memory;
	required_shared_ptr<uint16_t> m_road16_memory;
	required_shared_ptr<uint16_t> m_alpha16_memory;
	required_shared_ptr<uint16_t> m_view16_memory;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	uint16_t m_vertical_position_modifier[256];
	uint16_t m_road16_vscroll;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int m_chacl;
	uint16_t m_scroll;
	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint16_t polepos2_ic25_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t polepos_adc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t polepos_ready_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void polepos_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void polepos_z8002_nvi_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t polepos_sprite16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void polepos_sprite16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t polepos_sprite_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void polepos_sprite_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t polepos_road16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void polepos_road16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t polepos_road_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void polepos_road_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void polepos_road16_vscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t polepos_view16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void polepos_view16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t polepos_view_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void polepos_view_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void polepos_view16_hscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void polepos_chacl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t polepos_alpha16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void polepos_alpha16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t polepos_alpha_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void polepos_alpha_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value high_port_r(ioport_field &field, void *param);
	ioport_value low_port_r(ioport_field &field, void *param);
	ioport_value auto_start_r(ioport_field &field, void *param);
	void out_0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void out_1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t namco_52xx_rom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t namco_52xx_si_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t namco_53xx_k_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t steering_changed_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t steering_delta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_topracern();
	void init_polepos2();
	void bg_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tx_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_polepos();
	void video_start_polepos();
	void palette_init_polepos(palette_device &palette);
	uint32_t screen_update_polepos(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void polepos_scanline(timer_device &timer, void *ptr, int32_t param);
	void draw_road(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void zoom_sprite(bitmap_ind16 &bitmap,int big,uint32_t code,uint32_t color,int flipx,int sx,int sy,int sizex,int sizey);
};


/*----------- defined in audio/polepos.c -----------*/

class polepos_sound_device : public device_t,
								public device_sound_interface
{
public:
	polepos_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~polepos_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void polepos_engine_sound_lsb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void polepos_engine_sound_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	uint32_t m_current_position;
	int m_sample_msb;
	int m_sample_lsb;
	int m_sample_enable;
	sound_stream *m_stream;
	filter2_context m_filter_engine[3];
};

extern const device_type POLEPOS;

DISCRETE_SOUND_EXTERN( polepos );
