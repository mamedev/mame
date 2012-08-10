/*************************************************************************

    Crude Buster

*************************************************************************/

class cbuster_state : public driver_device
{
public:
	cbuster_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ram(*this, "ram"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_pf3_rowscroll(*this, "pf3_rowscroll"),
		m_pf4_rowscroll(*this, "pf4_rowscroll"),
		m_spriteram16(*this, "spriteram16"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_pf3_rowscroll;
	required_shared_ptr<UINT16> m_pf4_rowscroll;
	required_shared_ptr<UINT16> m_spriteram16;
	UINT16    m_spriteram16_buffer[0x400];

	/* misc */
	UINT16    m_prot;
	int       m_pri;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_deco_tilegen1;
	device_t *m_deco_tilegen2;
	DECLARE_WRITE16_MEMBER(twocrude_control_w);
	DECLARE_READ16_MEMBER(twocrude_control_r);
	DECLARE_WRITE16_MEMBER(twocrude_palette_24bit_rg_w);
	DECLARE_WRITE16_MEMBER(twocrude_palette_24bit_b_w);
	DECLARE_DRIVER_INIT(twocrude);
};



/*----------- defined in video/cbuster.c -----------*/


VIDEO_START( twocrude );
SCREEN_UPDATE_RGB32( twocrude );
