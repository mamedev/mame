#include "machine/nvram.h"
#include "sound/okiadpcm.h"

class mjkjidai_adpcm_device;

class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram3(*this, "spriteram3"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_mjk_adpcm(*this, "adpcm"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_spriteram1;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_spriteram3;
	required_shared_ptr<UINT8> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<mjkjidai_adpcm_device> m_mjk_adpcm;
	required_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_keyb;
	int m_nvram_init_count;
	int m_display_enable;
	tilemap_t *m_bg_tilemap;

	UINT8 m_nmi_mask;
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_select_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_videoram_w);
	DECLARE_WRITE8_MEMBER(mjkjidai_ctrl_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start();
	UINT32 screen_update_mjkjidai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};

class mjkjidai_adpcm_device : public device_t,
									public device_sound_interface
{
public:
	mjkjidai_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mjkjidai_adpcm_device() {}

	void mjkjidai_adpcm_play (int offset, int length);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	oki_adpcm_state m_adpcm;
	sound_stream *m_stream;
	UINT32 m_current;
	UINT32 m_end;
	UINT8 m_nibble;
	UINT8 m_playing;
	UINT8 *m_base;
};

extern const device_type MJKJIDAI;
