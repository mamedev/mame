#include "devlegcy.h"
#include "devcb.h"
#include "sound/discrete.h"


class phoenix_state : public driver_device
{
public:
	phoenix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram_pg[2];
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
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- video timing  -----------*/

#define MASTER_CLOCK			XTAL_11MHz

#define PIXEL_CLOCK				(MASTER_CLOCK/2)
#define CPU_CLOCK				(PIXEL_CLOCK)
#define HTOTAL					(512-160)
#define HBSTART					(256)
#define HBEND					(0)
#define VTOTAL					(256)
#define VBSTART					(208)
#define VBEND					(0)

/*----------- defined in audio/phoenix.c -----------*/

DISCRETE_SOUND_EXTERN( phoenix );

WRITE8_DEVICE_HANDLER( phoenix_sound_control_a_w );
WRITE8_DEVICE_HANDLER( phoenix_sound_control_b_w );

class phoenix_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	phoenix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~phoenix_sound_device() { global_free(m_token); }

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

extern const device_type PHOENIX;



/*----------- defined in video/phoenix.c -----------*/

PALETTE_INIT( phoenix );
PALETTE_INIT( survival );
PALETTE_INIT( pleiads );
VIDEO_START( phoenix );
SCREEN_UPDATE_IND16( phoenix );


READ8_DEVICE_HANDLER( survival_protection_r );

READ_LINE_DEVICE_HANDLER( survival_sid_callback );

