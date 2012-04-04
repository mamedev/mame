/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state : public driver_device
{
public:
	ambush_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_colorram;
	UINT8 *    m_scrollram;
	UINT8 *    m_colorbank;

	size_t     m_videoram_size;
	size_t     m_spriteram_size;
	DECLARE_WRITE8_MEMBER(ambush_coin_counter_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
};


/*----------- defined in video/ambush.c -----------*/

PALETTE_INIT( ambush );
SCREEN_UPDATE_IND16( ambush );
