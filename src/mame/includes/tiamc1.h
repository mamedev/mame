// license:BSD-3-Clause
// copyright-holders:Eugene Sandulenko

#include "machine/pit8253.h"
#include "sound/speaker.h"

class tiamc1_state : public driver_device
{
public:
	tiamc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_speaker(*this, "speaker")
		{ }

	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t *m_tileram;
	uint8_t *m_charram;
	uint8_t *m_spriteram_x;
	uint8_t *m_spriteram_y;
	uint8_t *m_spriteram_a;
	uint8_t *m_spriteram_n;
	uint8_t *m_paletteram;
	uint8_t m_layers_ctrl;
	uint8_t m_bg_vshift;
	uint8_t m_bg_hshift;
	uint8_t m_bg_bplctrl;
	tilemap_t *m_bg_tilemap1;
	tilemap_t *m_bg_tilemap2;
	std::unique_ptr<rgb_t[]> m_palette_ptr;
	void tiamc1_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_sprite_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_sprite_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_sprite_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_sprite_n_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_bg_vshift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_bg_hshift_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_bg_bplctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kot_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kot_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pit8253_2_w(int state);

	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void video_start_kot();
	void palette_init_tiamc1(palette_device &palette);
	uint32_t screen_update_tiamc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
private:
	optional_device<speaker_sound_device> m_speaker;
	void update_bg_palette();
};


/*----------- defined in audio/tiamc1.c -----------*/

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct timer8253chan
{
	timer8253chan() :
		count(0),
		cnval(0),
		bcdMode(0),
		cntMode(0),
		valMode(0),
		gate(0),
		output(0),
		loadCnt(0),
		enable(0)
	{}

	uint16_t count;
	uint16_t cnval;
	uint8_t bcdMode;
	uint8_t cntMode;
	uint8_t valMode;
	uint8_t gate;
	uint8_t output;
	uint8_t loadCnt;
	uint8_t enable;
};

struct timer8253struct
{
	struct timer8253chan channel[3];
};


// ======================> tiamc1_sound_device

class tiamc1_sound_device : public device_t,
							public device_sound_interface
{
public:
	tiamc1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tiamc1_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void tiamc1_timer0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_timer1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tiamc1_timer1_gate_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	void timer8253_reset(struct timer8253struct *t);
	void timer8253_tick(struct timer8253struct *t,int chn);
	void timer8253_wr(struct timer8253struct *t, int reg, uint8_t val);
	char timer8253_get_output(struct timer8253struct *t, int chn);
	void timer8253_set_gate(struct timer8253struct *t, int chn, uint8_t gate);

private:
	sound_stream *m_channel;
	int m_timer1_divider;

	timer8253struct m_timer0;
	timer8253struct m_timer1;
};

extern const device_type TIAMC1;
