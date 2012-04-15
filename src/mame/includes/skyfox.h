/*************************************************************************

    Skyfox

*************************************************************************/

class skyfox_state : public driver_device
{
public:
	skyfox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	UINT8      m_vreg[8];
	int        m_bg_pos;
	int        m_bg_ctrl;

	/* misc */
	int        m_palette_selected;

	/* devices */
	device_t *m_maincpu;
	DECLARE_READ8_MEMBER(skyfox_vregs_r);
	DECLARE_WRITE8_MEMBER(skyfox_vregs_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
};

/*----------- defined in video/skyfox.c -----------*/


PALETTE_INIT( skyfox );

SCREEN_UPDATE_IND16( skyfox );

