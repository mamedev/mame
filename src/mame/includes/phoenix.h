// license:???
// copyright-holders:Richard Davies
#include "audio/pleiads.h"
#include "sound/discrete.h"
#include "sound/tms36xx.h"

class phoenix_state : public driver_device
{
public:
	phoenix_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_pleiads_custom(*this, "pleiads_custom"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_device<cpu_device> m_maincpu;
	optional_device<pleiads_sound_device> m_pleiads_custom;
	required_device<gfxdecode_device> m_gfxdecode;
	std::unique_ptr<UINT8[]> m_videoram_pg[2];
	UINT8 m_videoram_pg_index;
	UINT8 m_palette_bank;
	UINT8 m_cocktail_mode;
	UINT8 m_pleiads_protection_question;
	UINT8 m_survival_protection_value;
	int m_survival_sid_value;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 m_survival_input_latches[2];
	UINT8 m_survival_input_readc;
	DECLARE_WRITE8_MEMBER(phoenix_videoram_w);
	DECLARE_WRITE8_MEMBER(phoenix_videoreg_w);
	DECLARE_WRITE8_MEMBER(pleiads_videoreg_w);
	DECLARE_WRITE8_MEMBER(phoenix_scroll_w);
	DECLARE_READ8_MEMBER(survival_input_port_0_r);
	DECLARE_CUSTOM_INPUT_MEMBER(player_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(pleiads_protection_r);
	DECLARE_DRIVER_INIT(condor);
	DECLARE_DRIVER_INIT(vautourza);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_MACHINE_RESET(phoenix);
	DECLARE_VIDEO_START(phoenix);
	DECLARE_PALETTE_INIT(phoenix);
	DECLARE_PALETTE_INIT(survival);
	DECLARE_PALETTE_INIT(pleiads);
	UINT32 screen_update_phoenix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(survival_protection_r);
	DECLARE_READ_LINE_MEMBER(survival_sid_callback);
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
	INT32 counter;
	INT32 level;
};

struct n_state
{
	INT32 counter;
	INT32 polyoffs;
	INT32 polybit;
	INT32 lowpass_counter;
	INT32 lowpass_polybit;
};

class phoenix_sound_device : public device_t,
									public device_sound_interface
{
public:
	phoenix_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~phoenix_sound_device() {}

	DECLARE_WRITE8_MEMBER( control_a_w );
	DECLARE_WRITE8_MEMBER( control_b_w );

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
	UINT8               m_sound_latch_a;
	sound_stream *      m_channel;
	std::unique_ptr<UINT32[]>                m_poly18;
	discrete_device *m_discrete;
	tms36xx_device *m_tms;

	int update_c24(int samplerate);
	int update_c25(int samplerate);
	int noise(int samplerate);
};

extern const device_type PHOENIX;

DISCRETE_SOUND_EXTERN( phoenix );
