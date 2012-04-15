/*************************************************************************

    Epos games

**************************************************************************/

class epos_state : public driver_device
{
public:
	epos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	UINT8    m_palette;

	/* misc */
	int      m_counter;
	DECLARE_WRITE8_MEMBER(dealer_decrypt_rom);
	DECLARE_WRITE8_MEMBER(epos_port_1_w);
};


/*----------- defined in video/epos.c -----------*/

SCREEN_UPDATE_RGB32( epos );
