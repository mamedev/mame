// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Dan Boris
/*************************************************************************

    rokola hardware

*************************************************************************/

#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"

class snk6502_sound_device;

class snk6502_state : public driver_device
{
public:
	snk6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "snk6502"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_charram(*this, "charram") { }

	required_device<cpu_device> m_maincpu;
	required_device<snk6502_sound_device> m_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_charram;

	uint8_t m_sasuke_counter;
	int m_charbank;
	int m_backcolor;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	rgb_t m_palette_val[64];
	uint8_t m_irq_mask;

	// common
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void charram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void scrollx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void satansat_b002_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void satansat_backcolor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	ioport_value snk6502_music0_r(ioport_field &field, void *param);
	ioport_value sasuke_count_r(ioport_field &field, void *param);
	void coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void satansat_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void satansat_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	void machine_reset_sasuke();
	void video_start_satansat();
	void palette_init_satansat(palette_device &palette);
	void machine_reset_vanguard();
	void video_start_snk6502();
	void palette_init_snk6502(palette_device &palette);
	void machine_reset_satansat();
	void machine_reset_pballoon();
	void video_start_pballoon();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void satansat_interrupt(device_t &device);
	void snk6502_interrupt(device_t &device);
	void sasuke_update_counter(timer_device &timer, void *ptr, int32_t param);

	void sasuke_start_counter();
	void postload();
};


/*----------- defined in audio/snk6502.c -----------*/

#define CHANNELS    3

struct TONE
{
	int mute;
	int offset;
	int base;
	int mask;
	int32_t   sample_rate;
	int32_t   sample_step;
	int32_t   sample_cur;
	int16_t   form[16];
};

class snk6502_sound_device : public device_t,
									public device_sound_interface
{
public:
	snk6502_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~snk6502_sound_device() {}

	void sasuke_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void satansat_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vanguard_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vanguard_speech_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fantasy_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fantasy_speech_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void set_music_clock(double clock_time);
	void set_music_freq(int freq);
	int music0_playing();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	TONE m_tone_channels[CHANNELS];
	int32_t m_tone_clock_expire;
	int32_t m_tone_clock;
	sound_stream * m_tone_stream;

	optional_device<samples_device> m_samples;
	uint8_t *m_ROM;
	int m_Sound0StopOnRollover;
	uint8_t m_LastPort1;

	int m_hd38880_cmd;
	uint32_t m_hd38880_addr;
	int m_hd38880_data_bytes;
	double m_hd38880_speed;

	inline void validate_tone_channel(int channel);
	void sasuke_build_waveform(int mask);
	void satansat_build_waveform(int mask);
	void build_waveform(int channel, int mask);
	void speech_w(uint8_t data, const uint16_t *table, int start);
};

extern const device_type SNK6502;

DISCRETE_SOUND_EXTERN( fantasy );
extern const char *const sasuke_sample_names[];
extern const char *const vanguard_sample_names[];
extern const char *const fantasy_sample_names[];
