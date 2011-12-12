class sprcros2_state : public driver_device
{
public:
	sprcros2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_master(*this,"master"),
		m_slave(*this,"slave")
		{ }

	UINT8 m_s_port3;
	UINT8 m_port7;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;
	UINT8 *m_fgvideoram;
	UINT8 *m_bgvideoram;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;

	required_device<cpu_device> m_master;
	required_device<cpu_device> m_slave;
};


/*----------- defined in video/sprcros2.c -----------*/

WRITE8_HANDLER( sprcros2_fgvideoram_w );
WRITE8_HANDLER( sprcros2_bgvideoram_w );
WRITE8_HANDLER( sprcros2_bgscrollx_w );
WRITE8_HANDLER( sprcros2_bgscrolly_w );

PALETTE_INIT( sprcros2 );
VIDEO_START( sprcros2 );
SCREEN_UPDATE( sprcros2 );
