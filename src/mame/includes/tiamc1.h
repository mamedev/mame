// license:BSD-3-Clause
// copyright-holders:Eugene Sandulenko

class tiamc1_state : public driver_device
{
public:
	tiamc1_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	std::unique_ptr<UINT8[]> m_videoram;
	UINT8 *m_tileram;
	UINT8 *m_charram;
	UINT8 *m_spriteram_x;
	UINT8 *m_spriteram_y;
	UINT8 *m_spriteram_a;
	UINT8 *m_spriteram_n;
	UINT8 m_layers_ctrl;
	UINT8 m_bg_vshift;
	UINT8 m_bg_hshift;
	tilemap_t *m_bg_tilemap1;
	tilemap_t *m_bg_tilemap2;
	std::unique_ptr<rgb_t[]> m_palette_ptr;
	DECLARE_WRITE8_MEMBER(tiamc1_control_w);
	DECLARE_WRITE8_MEMBER(tiamc1_videoram_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_x_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_y_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_a_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_n_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bg_vshift_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bg_hshift_w);
	DECLARE_WRITE8_MEMBER(tiamc1_palette_w);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(tiamc1);
	UINT32 screen_update_tiamc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/*----------- defined in audio/tiamc1.c -----------*/

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct timer8253chan
{
	timer8253chan() :
		count(0),
		cnval(0),
		bcdMode(0),
		cntMode(0),
		valMode(0),
		gate(0),
		output(0),
		loadCnt(0),
		enable(0)
	{}

	UINT16 count;
	UINT16 cnval;
	UINT8 bcdMode;
	UINT8 cntMode;
	UINT8 valMode;
	UINT8 gate;
	UINT8 output;
	UINT8 loadCnt;
	UINT8 enable;
};

struct timer8253struct
{
	struct timer8253chan channel[3];
};


// ======================> tiamc1_sound_device

class tiamc1_sound_device : public device_t,
							public device_sound_interface
{
public:
	tiamc1_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~tiamc1_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( tiamc1_timer0_w );
	DECLARE_WRITE8_MEMBER( tiamc1_timer1_w );
	DECLARE_WRITE8_MEMBER( tiamc1_timer1_gate_w );

private:
	void timer8253_reset(struct timer8253struct *t);
	void timer8253_tick(struct timer8253struct *t,int chn);
	void timer8253_wr(struct timer8253struct *t, int reg, UINT8 val);
	char timer8253_get_output(struct timer8253struct *t, int chn);
	void timer8253_set_gate(struct timer8253struct *t, int chn, UINT8 gate);

private:
	sound_stream *m_channel;
	int m_timer1_divider;

	timer8253struct m_timer0;
	timer8253struct m_timer1;
};

extern const device_type TIAMC1;
