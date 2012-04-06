class superqix_state : public driver_device
{
public:
	superqix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	INT16 *m_samplebuf;
	UINT8 m_port1;
	UINT8 m_port2;
	UINT8 m_port3;
	UINT8 m_port3_latch;
	UINT8 m_from_mcu;
	UINT8 m_from_z80;
	UINT8 m_portb;
	int m_from_mcu_pending;
	int m_from_z80_pending;
	int m_invert_coin_lockout;
	int m_oldpos[2];
	int m_sign[2];
	UINT8 m_portA_in;
	UINT8 m_portB_out;
	UINT8 m_portC;
	int m_curr_player;
	UINT8 *m_videoram;
	UINT8 *m_bitmapram;
	UINT8 *m_bitmapram2;
	int m_gfxbank;
	bitmap_ind16 *m_fg_bitmap[2];
	int m_show_bitmap;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	UINT8 m_nmi_mask;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(pbillian_sample_trigger_w);
	DECLARE_READ8_MEMBER(mcu_acknowledge_r);
	DECLARE_WRITE8_MEMBER(bootleg_mcu_p1_w);
	DECLARE_WRITE8_MEMBER(mcu_p3_w);
	DECLARE_READ8_MEMBER(bootleg_mcu_p3_r);
	DECLARE_READ8_MEMBER(sqixu_mcu_p0_r);
	DECLARE_WRITE8_MEMBER(sqixu_mcu_p2_w);
	DECLARE_READ8_MEMBER(sqixu_mcu_p3_r);
	DECLARE_READ8_MEMBER(nmi_ack_r);
	DECLARE_WRITE8_MEMBER(bootleg_flipscreen_w);
	DECLARE_READ8_MEMBER(hotsmash_68705_portA_r);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_portB_w);
	DECLARE_READ8_MEMBER(hotsmash_68705_portC_r);
	DECLARE_WRITE8_MEMBER(hotsmash_68705_portC_w);
	DECLARE_WRITE8_MEMBER(hotsmash_z80_mcu_w);
	DECLARE_READ8_MEMBER(hotsmash_from_mcu_r);
	DECLARE_WRITE8_MEMBER(pbillian_z80_mcu_w);
	DECLARE_READ8_MEMBER(pbillian_from_mcu_r);
	DECLARE_WRITE8_MEMBER(superqix_videoram_w);
	DECLARE_WRITE8_MEMBER(superqix_bitmapram_w);
	DECLARE_WRITE8_MEMBER(superqix_bitmapram2_w);
	DECLARE_WRITE8_MEMBER(pbillian_0410_w);
	DECLARE_WRITE8_MEMBER(superqix_0410_w);
};


/*----------- defined in video/superqix.c -----------*/


VIDEO_START( pbillian );
SCREEN_UPDATE_IND16( pbillian );
VIDEO_START( superqix );
SCREEN_UPDATE_IND16( superqix );
