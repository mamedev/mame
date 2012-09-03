#include "devlegcy.h"

class gomoku_state : public driver_device
{
public:
	gomoku_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bgram(*this, "bgram"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_bgram;
	int m_flipscreen;
	int m_bg_dispsw;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_bg_bitmap;
	DECLARE_READ8_MEMBER(input_port_r);
	DECLARE_WRITE8_MEMBER(gomoku_videoram_w);
	DECLARE_WRITE8_MEMBER(gomoku_colorram_w);
	DECLARE_WRITE8_MEMBER(gomoku_bgram_w);
	DECLARE_WRITE8_MEMBER(gomoku_flipscreen_w);
	DECLARE_WRITE8_MEMBER(gomoku_bg_dispsw_w);
};


/*----------- defined in audio/gomoku.c -----------*/

WRITE8_DEVICE_HANDLER( gomoku_sound1_w );
WRITE8_DEVICE_HANDLER( gomoku_sound2_w );

class gomoku_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	gomoku_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~gomoku_sound_device() { global_free(m_token); }

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

extern const device_type GOMOKU;



/*----------- defined in video/gomoku.c -----------*/

PALETTE_INIT( gomoku );
VIDEO_START( gomoku );
SCREEN_UPDATE_IND16( gomoku );

