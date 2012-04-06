class xyonix_state : public driver_device
{
public:
	xyonix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_vidram;
	tilemap_t *m_tilemap;

	int m_e0_data;
	int m_credits;
	int m_coins;
	int m_prev_coin;
	DECLARE_WRITE8_MEMBER(xyonix_irqack_w);
	DECLARE_READ8_MEMBER(xyonix_io_r);
	DECLARE_WRITE8_MEMBER(xyonix_io_w);
	DECLARE_WRITE8_MEMBER(xyonix_vidram_w);
};


/*----------- defined in video/xyonix.c -----------*/

PALETTE_INIT( xyonix );
VIDEO_START(xyonix);
SCREEN_UPDATE_IND16(xyonix);
