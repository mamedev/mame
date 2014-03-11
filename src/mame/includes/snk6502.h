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
		m_videoram2(*this, "videoram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_charram(*this, "charram"),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "snk6502"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 m_sasuke_counter;

	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_charram;

	required_device<cpu_device> m_maincpu;
	required_device<snk6502_sound_device> m_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_charbank;
	int m_backcolor;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	rgb_t m_palette_val[64];

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(snk6502_videoram_w);
	DECLARE_WRITE8_MEMBER(snk6502_videoram2_w);
	DECLARE_WRITE8_MEMBER(snk6502_colorram_w);
	DECLARE_WRITE8_MEMBER(snk6502_charram_w);
	DECLARE_WRITE8_MEMBER(snk6502_flipscreen_w);
	DECLARE_WRITE8_MEMBER(snk6502_scrollx_w);
	DECLARE_WRITE8_MEMBER(snk6502_scrolly_w);
	DECLARE_WRITE8_MEMBER(satansat_b002_w);
	DECLARE_WRITE8_MEMBER(satansat_backcolor_w);
	DECLARE_CUSTOM_INPUT_MEMBER(snk6502_music0_r);
	DECLARE_CUSTOM_INPUT_MEMBER(sasuke_count_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(satansat_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(satansat_get_fg_tile_info);
	DECLARE_MACHINE_RESET(sasuke);
	DECLARE_VIDEO_START(satansat);
	DECLARE_PALETTE_INIT(satansat);
	DECLARE_MACHINE_RESET(vanguard);
	DECLARE_VIDEO_START(snk6502);
	DECLARE_PALETTE_INIT(snk6502);
	DECLARE_MACHINE_RESET(satansat);
	DECLARE_MACHINE_RESET(pballoon);
	DECLARE_VIDEO_START(pballoon);
	UINT32 screen_update_snk6502(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(satansat_interrupt);
	INTERRUPT_GEN_MEMBER(snk6502_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(sasuke_update_counter);
	void sasuke_start_counter();
};


/*----------- defined in audio/snk6502.c -----------*/

#define CHANNELS    3

struct TONE
{
	int mute;
	int offset;
	int base;
	int mask;
	INT32   sample_rate;
	INT32   sample_step;
	INT32   sample_cur;
	INT16   form[16];
};

class snk6502_sound_device : public device_t,
									public device_sound_interface
{
public:
	snk6502_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~snk6502_sound_device() {}

	DECLARE_WRITE8_MEMBER( sasuke_sound_w );
	DECLARE_WRITE8_MEMBER( satansat_sound_w );
	DECLARE_WRITE8_MEMBER( vanguard_sound_w );
	DECLARE_WRITE8_MEMBER( vanguard_speech_w );
	DECLARE_WRITE8_MEMBER( fantasy_sound_w );
	DECLARE_WRITE8_MEMBER( fantasy_speech_w );

	void set_music_clock(double clock_time);
	void set_music_freq(int freq);
	int music0_playing();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state
	TONE m_tone_channels[CHANNELS];
	INT32 m_tone_clock_expire;
	INT32 m_tone_clock;
	sound_stream * m_tone_stream;

	samples_device *m_samples;
	UINT8 *m_ROM;
	int m_Sound0StopOnRollover;
	UINT8 m_LastPort1;

	int m_hd38880_cmd;
	UINT32 m_hd38880_addr;
	int m_hd38880_data_bytes;
	double m_hd38880_speed;

	inline void validate_tone_channel(int channel);
	void sasuke_build_waveform(int mask);
	void satansat_build_waveform(int mask);
	void build_waveform(int channel, int mask);
	void speech_w(UINT8 data, const UINT16 *table, int start);
};

extern const device_type SNK6502;

DISCRETE_SOUND_EXTERN( fantasy );
extern const samples_interface sasuke_samples_interface;
extern const samples_interface vanguard_samples_interface;
extern const samples_interface fantasy_samples_interface;
extern const sn76477_interface sasuke_sn76477_intf_1;
extern const sn76477_interface sasuke_sn76477_intf_2;
extern const sn76477_interface sasuke_sn76477_intf_3;
extern const sn76477_interface satansat_sn76477_intf;
extern const sn76477_interface vanguard_sn76477_intf_1;
extern const sn76477_interface vanguard_sn76477_intf_2;
extern const sn76477_interface fantasy_sn76477_intf;
