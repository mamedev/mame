/*************************************************************************

    rokola hardware

*************************************************************************/

#include "devlegcy.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"


class snk6502_state : public driver_device
{
public:
	snk6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram2(*this, "videoram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_charram(*this, "charram"){ }

	UINT8 m_sasuke_counter;

	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_charram;

	int m_charbank;
	int m_backcolor;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	rgb_t m_palette[64];

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
};


/*----------- defined in audio/snk6502.c -----------*/

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

extern DECLARE_WRITE8_HANDLER( sasuke_sound_w );
extern DECLARE_WRITE8_HANDLER( satansat_sound_w );
extern DECLARE_WRITE8_HANDLER( vanguard_sound_w );
extern DECLARE_WRITE8_HANDLER( vanguard_speech_w );
extern DECLARE_WRITE8_HANDLER( fantasy_sound_w );
extern DECLARE_WRITE8_HANDLER( fantasy_speech_w );

class snk6502_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	snk6502_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~snk6502_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type SNK6502;


void snk6502_set_music_clock(running_machine &machine, double clock_time);
void snk6502_set_music_freq(running_machine &machine, int freq);
int snk6502_music0_playing(running_machine &machine);

DISCRETE_SOUND_EXTERN( fantasy );


/*----------- defined in video/snk6502.c -----------*/











