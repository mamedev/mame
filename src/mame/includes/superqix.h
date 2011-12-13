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
	bitmap_t *m_fg_bitmap[2];
	int m_show_bitmap;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	UINT8 m_nmi_mask;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/superqix.c -----------*/

WRITE8_HANDLER( superqix_videoram_w );
WRITE8_HANDLER( superqix_bitmapram_w );
WRITE8_HANDLER( superqix_bitmapram2_w );
WRITE8_HANDLER( pbillian_0410_w );
WRITE8_HANDLER( superqix_0410_w );

VIDEO_START( pbillian );
SCREEN_UPDATE( pbillian );
VIDEO_START( superqix );
SCREEN_UPDATE( superqix );
