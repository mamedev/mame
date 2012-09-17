#include "devlegcy.h"

class flower_state : public driver_device
{
public:
	flower_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sn_nmi_enable(*this, "sn_nmi_enable"),
		m_spriteram(*this, "spriteram"),
		m_textram(*this, "textram"),
		m_bg0ram(*this, "bg0ram"),
		m_bg1ram(*this, "bg1ram"),
		m_bg0_scroll(*this, "bg0_scroll"),
		m_bg1_scroll(*this, "bg1_scroll"){ }

	required_shared_ptr<UINT8> m_sn_nmi_enable;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_textram;
	required_shared_ptr<UINT8> m_bg0ram;
	required_shared_ptr<UINT8> m_bg1ram;
	required_shared_ptr<UINT8> m_bg0_scroll;
	required_shared_ptr<UINT8> m_bg1_scroll;
	tilemap_t *m_bg0_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_text_tilemap;
	tilemap_t *m_text_right_tilemap;
	DECLARE_WRITE8_MEMBER(flower_maincpu_irq_ack);
	DECLARE_WRITE8_MEMBER(flower_subcpu_irq_ack);
	DECLARE_WRITE8_MEMBER(flower_soundcpu_irq_ack);
	DECLARE_WRITE8_MEMBER(flower_coin_counter_w);
	DECLARE_WRITE8_MEMBER(flower_coin_lockout_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(flower_textram_w);
	DECLARE_WRITE8_MEMBER(flower_bg0ram_w);
	DECLARE_WRITE8_MEMBER(flower_bg1ram_w);
	DECLARE_WRITE8_MEMBER(flower_flipscreen_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in audio/flower.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER( flower_sound1_w );
DECLARE_WRITE8_DEVICE_HANDLER( flower_sound2_w );

class flower_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~flower_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type FLOWER;



/*----------- defined in video/flower.c -----------*/


SCREEN_UPDATE_IND16( flower );


