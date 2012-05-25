class xorworld_state : public driver_device
{
public:
	xorworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT16> m_videoram;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT16> m_spriteram;
	DECLARE_WRITE16_MEMBER(xorworld_irq2_ack_w);
	DECLARE_WRITE16_MEMBER(xorworld_irq6_ack_w);
	DECLARE_WRITE16_MEMBER(xorworld_videoram16_w);
	DECLARE_WRITE16_MEMBER(eeprom_chip_select_w);
	DECLARE_WRITE16_MEMBER(eeprom_serial_clock_w);
	DECLARE_WRITE16_MEMBER(eeprom_data_w);
};


/*----------- defined in video/xorworld.c -----------*/


PALETTE_INIT( xorworld );
VIDEO_START( xorworld );
SCREEN_UPDATE_IND16( xorworld );
