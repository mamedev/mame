// license:???
// copyright-holders:insideoutboy, David Haywood, Stephh
class flower_state : public driver_device
{
public:
	flower_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sn_nmi_enable(*this, "sn_nmi_enable"),
		m_spriteram(*this, "spriteram"),
		m_textram(*this, "textram"),
		m_bg0ram(*this, "bg0ram"),
		m_bg1ram(*this, "bg1ram"),
		m_bg0_scroll(*this, "bg0_scroll"),
		m_bg1_scroll(*this, "bg1_scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "subcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")   { }

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
	virtual void video_start() override;
	UINT32 screen_update_flower(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


// ======================> flower_sound_device


/* this structure defines the parameters for a channel */
struct flower_sound_channel
{
	UINT32 start;
	UINT32 pos;
	UINT16 freq;
	UINT8 volume;
	UINT8 voltab;
	UINT8 oneshot;
	UINT8 active;
	UINT8 effect;
	UINT32 ecount;

};

class flower_sound_device : public device_t,
									public device_sound_interface
{
public:
	flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~flower_sound_device() {}

	enum
	{
	TIMER_CLOCK_EFFECT
	};

	DECLARE_WRITE8_MEMBER( sound1_w );
	DECLARE_WRITE8_MEMBER( sound2_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void make_mixer_table(int voices, int gain);

	// internal state
	emu_timer *m_effect_timer;

	/* data about the sound system */
	flower_sound_channel m_channel_list[8];
	flower_sound_channel *m_last_channel;

	/* global sound parameters */
	const UINT8 *m_sample_rom;
	const UINT8 *m_volume_rom;
	sound_stream * m_stream;

	/* mixer tables and internal buffers */
	std::unique_ptr<INT16[]> m_mixer_table;
	INT16 *m_mixer_lookup;
	std::unique_ptr<short[]> m_mixer_buffer;

	UINT8 m_soundregs1[0x40];
	UINT8 m_soundregs2[0x40];

};

extern const device_type FLOWER;
