#include "devlegcy.h"

class warpwarp_state : public driver_device
{
public:
	warpwarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_geebee_videoram(*this, "geebee_videoram"),
		m_videoram(*this, "videoram"){ }

	optional_shared_ptr<UINT8> m_geebee_videoram;
	optional_shared_ptr<UINT8> m_videoram;
	int m_geebee_bgw;
	int m_ball_on;
	int m_ball_h;
	int m_ball_v;
	int m_ball_pen;
	int m_ball_sizex;
	int m_ball_sizey;
	int m_handle_joystick;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(geebee_in_r);
	DECLARE_WRITE8_MEMBER(geebee_out6_w);
	DECLARE_WRITE8_MEMBER(geebee_out7_w);
	DECLARE_READ8_MEMBER(warpwarp_sw_r);
	DECLARE_WRITE8_MEMBER(warpwarp_out0_w);
	DECLARE_WRITE8_MEMBER(warpwarp_out3_w);
	DECLARE_WRITE8_MEMBER(geebee_videoram_w);
	DECLARE_WRITE8_MEMBER(warpwarp_videoram_w);
	DECLARE_READ8_MEMBER(warpwarp_dsw1_r);
	DECLARE_READ8_MEMBER(warpwarp_vol_r);
	DECLARE_DRIVER_INIT(navarone);
	DECLARE_DRIVER_INIT(geebee);
	DECLARE_DRIVER_INIT(kaitein);
	DECLARE_DRIVER_INIT(warpwarp);
	DECLARE_DRIVER_INIT(sos);
	DECLARE_DRIVER_INIT(kaitei);
	DECLARE_DRIVER_INIT(bombbee);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(geebee_get_tile_info);
	TILE_GET_INFO_MEMBER(navarone_get_tile_info);
	TILE_GET_INFO_MEMBER(warpwarp_get_tile_info);
	DECLARE_VIDEO_START(geebee);
	DECLARE_PALETTE_INIT(geebee);
	DECLARE_VIDEO_START(warpwarp);
	DECLARE_PALETTE_INIT(warpwarp);
	DECLARE_VIDEO_START(navarone);
	DECLARE_PALETTE_INIT(navarone);
};


/*----------- defined in video/warpwarp.c -----------*/







SCREEN_UPDATE_IND16( geebee );


/*----------- defined in audio/geebee.c -----------*/

WRITE8_DEVICE_HANDLER( geebee_sound_w );

class geebee_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	geebee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~geebee_sound_device() { global_free(m_token); }

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

extern const device_type GEEBEE;



/*----------- defined in audio/warpwarp.c -----------*/

WRITE8_DEVICE_HANDLER( warpwarp_sound_w );
WRITE8_DEVICE_HANDLER( warpwarp_music1_w );
WRITE8_DEVICE_HANDLER( warpwarp_music2_w );

class warpwarp_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	warpwarp_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~warpwarp_sound_device() { global_free(m_token); }

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

extern const device_type WARPWARP;

