// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
#include "sound/discrete.h"
#include "sound/samples.h"

class blockade_state : public driver_device
{
public:
	blockade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen") { }

	required_shared_ptr<uint8_t> m_videoram;
	required_device<discrete_device> m_discrete;
	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* input-related */
	uint8_t m_coin_latch;  /* Active Low */
	uint8_t m_just_been_reset;
	uint8_t blockade_input_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blockade_coin_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blockade_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blockade_env_on_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blockade_env_off_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_blockade(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void blockade_interrupt(device_t &device);
	void blockade_sound_freq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
};

/*----------- defined in audio/blockade.c -----------*/

extern const char *const blockade_sample_names[];
DISCRETE_SOUND_EXTERN( blockade );
