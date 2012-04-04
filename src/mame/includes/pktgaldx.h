/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/

class pktgaldx_state : public driver_device
{
public:
	pktgaldx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	UINT16 *  m_spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in decocomn.c)
	size_t    m_spriteram_size;

	UINT16*   m_pktgaldb_fgram;
	UINT16*   m_pktgaldb_sprites;

	/* devices */
	device_t *m_maincpu;
	device_t *m_deco_tilegen1;
	DECLARE_READ16_MEMBER(pckgaldx_unknown_r);
	DECLARE_READ16_MEMBER(pckgaldx_protection_r);
};



/*----------- defined in video/pktgaldx.c -----------*/

SCREEN_UPDATE_IND16( pktgaldx );
SCREEN_UPDATE_IND16( pktgaldb );
