/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state : public driver_device
{
public:
	ambush_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_scrollram(*this, "scrollram"),
		m_colorbank(*this, "colorbank"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_colorbank;

	DECLARE_WRITE8_MEMBER(ambush_coin_counter_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
};


/*----------- defined in video/ambush.c -----------*/

PALETTE_INIT( ambush );
SCREEN_UPDATE_IND16( ambush );
