// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, David Haywood

class gomoku_state : public driver_device
{
public:
	gomoku_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bgram(*this, "bgram"),
		m_inputs(*this, {"IN0", "IN1", "DSW", "UNUSED0", "UNUSED1", "UNUSED2", "UNUSED3", "UNUSED4"}),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_bgram;
	int m_flipscreen;
	int m_bg_dispsw;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_bg_bitmap;
	optional_ioport_array<8> m_inputs;
	uint8_t input_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gomoku_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gomoku_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gomoku_bgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gomoku_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gomoku_bg_dispsw_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_gomoku(palette_device &palette);
	uint32_t screen_update_gomoku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
};


/*----------- defined in audio/gomoku.c -----------*/

/* 4 voices max */
#define GOMOKU_MAX_VOICES 4

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct gomoku_sound_channel
{
	gomoku_sound_channel():
		channel(0),
		frequency(0),
		counter(0),
		volume(0),
		oneshotplaying(0) {}

	int channel;
	int frequency;
	int counter;
	int volume;
	int oneshotplaying;
};


// ======================> gomoku_sound_device

class gomoku_sound_device : public device_t,
							public device_sound_interface
{
public:
	gomoku_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~gomoku_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void sound1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	void make_mixer_table(int voices, int gain);

private:
	/* data about the sound system */
	gomoku_sound_channel m_channel_list[GOMOKU_MAX_VOICES];
	gomoku_sound_channel *m_last_channel;

	/* global sound parameters */
	const uint8_t *m_sound_rom;
	int m_num_voices;
	int m_sound_enable;
	sound_stream *m_stream;

	/* mixer tables and internal buffers */
	std::unique_ptr<int16_t[]> m_mixer_table;
	int16_t *m_mixer_lookup;
	std::unique_ptr<short[]> m_mixer_buffer;
	short *m_mixer_buffer_2;

	uint8_t m_soundregs1[0x20];
	uint8_t m_soundregs2[0x20];
};

extern const device_type GOMOKU;
