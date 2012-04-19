/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/

class pktgaldx_state : public driver_device
{
public:
	pktgaldx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_spriteram(*this, "spriteram"),
		m_pktgaldb_fgram(*this, "pktgaldb_fgram"),
		m_pktgaldb_sprites(*this, "pktgaldb_spr"){ }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_pf1_rowscroll;
	optional_shared_ptr<UINT16> m_pf2_rowscroll;
	optional_shared_ptr<UINT16> m_spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in decocomn.c)

	optional_shared_ptr<UINT16> m_pktgaldb_fgram;
	optional_shared_ptr<UINT16> m_pktgaldb_sprites;

	/* devices */
	device_t *m_maincpu;
	device_t *m_deco_tilegen1;
	DECLARE_READ16_MEMBER(pckgaldx_unknown_r);
	DECLARE_READ16_MEMBER(pckgaldx_protection_r);
};



/*----------- defined in video/pktgaldx.c -----------*/

SCREEN_UPDATE_IND16( pktgaldx );
SCREEN_UPDATE_IND16( pktgaldb );
