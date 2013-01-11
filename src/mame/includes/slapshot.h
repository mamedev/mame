/*************************************************************************

    Slapshot / Operation Wolf 3

*************************************************************************/

struct slapshot_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class slapshot_state : public driver_device
{
public:
	slapshot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_color_ram(*this,"color_ram"),
		m_spriteram(*this,"spriteram"),
		m_spriteext(*this,"spriteext") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_color_ram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_spriteext;
	UINT16 *    m_spriteram_buffered;
	UINT16 *    m_spriteram_delayed;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	struct      slapshot_tempsprite *m_spritelist;
	INT32       m_sprites_disabled;
	INT32       m_sprites_active_area;
	INT32       m_sprites_master_scrollx;
	INT32       m_sprites_master_scrolly;
	int         m_sprites_flipscreen;
	int         m_prepare_sprites;
	int         m_dislayer[5];

	UINT16      m_spritebank[8];

	/* misc */
	INT32      m_banknum;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_tc0140syt;
	device_t *m_tc0480scp;
	device_t *m_tc0360pri;
	device_t *m_tc0640fio;
	DECLARE_READ16_MEMBER(color_ram_word_r);
	DECLARE_WRITE16_MEMBER(color_ram_word_w);
	DECLARE_READ16_MEMBER(slapshot_service_input_r);
	DECLARE_READ16_MEMBER(opwolf3_adc_r);
	DECLARE_WRITE16_MEMBER(opwolf3_adc_req_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(slapshot_msb_sound_w);
	DECLARE_READ16_MEMBER(slapshot_msb_sound_r);
	DECLARE_DRIVER_INIT(slapshot);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_slapshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_taito_no_buffer(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(slapshot_interrupt);
	TIMER_CALLBACK_MEMBER(slapshot_interrupt6);
};
