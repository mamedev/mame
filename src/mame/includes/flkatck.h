/*************************************************************************

    Flak Attack / MX5000

*************************************************************************/

class flkatck_state : public driver_device
{
public:
	flkatck_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_k007121_ram(*this, "k007121_ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_k007121_ram;
//  UINT8 *    paletteram;  // this currently uses generic palette handling

	/* video-related */
	tilemap_t    *m_k007121_tilemap[2];
	int        m_flipscreen;

	/* misc */
	int        m_irq_enabled;
	int        m_multiply_reg[2];

	/* devices */
	cpu_device *m_audiocpu;
	device_t *m_k007121;
	DECLARE_WRITE8_MEMBER(flkatck_bankswitch_w);
	DECLARE_READ8_MEMBER(flkatck_ls138_r);
	DECLARE_WRITE8_MEMBER(flkatck_ls138_w);
	DECLARE_READ8_MEMBER(multiply_r);
	DECLARE_WRITE8_MEMBER(multiply_w);
	DECLARE_WRITE8_MEMBER(flkatck_k007121_w);
	DECLARE_WRITE8_MEMBER(flkatck_k007121_regs_w);
	TILE_GET_INFO_MEMBER(get_tile_info_A);
	TILE_GET_INFO_MEMBER(get_tile_info_B);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_flkatck(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


//static rectangle k007121_clip[2];


/*----------- defined in video/flkatck.c -----------*/




