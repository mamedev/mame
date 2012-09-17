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
	cpu_device *m_maincpu;
	device_t *m_deco_tilegen1;
	DECLARE_READ16_MEMBER(pckgaldx_unknown_r);
	DECLARE_READ16_MEMBER(pckgaldx_protection_r);
	DECLARE_WRITE16_MEMBER(pktgaldx_oki_bank_w);
	DECLARE_DRIVER_INIT(pktgaldx);
	virtual void machine_start();
	UINT32 screen_update_pktgaldx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pktgaldb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



/*----------- defined in video/pktgaldx.c -----------*/



