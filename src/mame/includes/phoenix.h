// license:BSD-3-Clause
// copyright-holders:Richard Davies
#include "audio/pleiads.h"
#include "sound/discrete.h"
#include "sound/tms36xx.h"

class phoenix_state : public driver_device
{
public:
	phoenix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_pleiads_custom(*this, "pleiads_custom"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_device<cpu_device> m_maincpu;
	optional_device<pleiads_sound_device> m_pleiads_custom;
	required_device<gfxdecode_device> m_gfxdecode;
	std::unique_ptr<uint8_t[]> m_videoram_pg[2];
	uint8_t m_videoram_pg_index;
	uint8_t m_palette_bank;
	uint8_t m_cocktail_mode;
	uint8_t m_pleiads_protection_question;
	uint8_t m_survival_protection_value;
	int m_survival_sid_value;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	uint8_t m_survival_input_latches[2];
	uint8_t m_survival_input_readc;
	void phoenix_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void phoenix_videoreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pleiads_videoreg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void phoenix_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t survival_input_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	ioport_value player_input_r(ioport_field &field, void *param);
	ioport_value pleiads_protection_r(ioport_field &field, void *param);
	void init_condor();
	void init_vautourza();
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_phoenix();
	void video_start_phoenix();
	void palette_init_phoenix(palette_device &palette);
	void palette_init_survival(palette_device &palette);
	void palette_init_pleiads(palette_device &palette);
	uint32_t screen_update_phoenix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t survival_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	int survival_sid_callback();
};


/*----------- video timing  -----------*/

#define MASTER_CLOCK            XTAL_11MHz

#define PIXEL_CLOCK             (MASTER_CLOCK/2)
#define CPU_CLOCK               (PIXEL_CLOCK)
#define HTOTAL                  (512-160)
#define HBSTART                 (256)
#define HBEND                   (0)
#define VTOTAL                  (256)
#define VBSTART                 (208)
#define VBEND                   (0)

/*----------- defined in audio/phoenix.c -----------*/

struct c_state
{
	int32_t counter;
	int32_t level;
};

struct n_state
{
	int32_t counter;
	int32_t polyoffs;
	int32_t polybit;
	int32_t lowpass_counter;
	int32_t lowpass_polybit;
};

class phoenix_sound_device : public device_t,
									public device_sound_interface
{
public:
	phoenix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~phoenix_sound_device() {}

	void control_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void control_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:
	// internal state
	struct c_state      m_c24_state;
	struct c_state      m_c25_state;
	struct n_state      m_noise_state;
	uint8_t               m_sound_latch_a;
	sound_stream *      m_channel;
	std::unique_ptr<uint32_t[]>                m_poly18;
	discrete_device *m_discrete;
	tms36xx_device *m_tms;

	int update_c24(int samplerate);
	int update_c25(int samplerate);
	int noise(int samplerate);
};

extern const device_type PHOENIX;

DISCRETE_SOUND_EXTERN( phoenix );
