#include "video/deco16ic.h"
#include "video/bufsprite.h"

class darkseal_state : public driver_device
{
public:
	darkseal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	      m_deco_tilegen1(*this, "tilegen1"),
		  m_deco_tilegen2(*this, "tilegen2"),
		  m_spriteram(*this, "spriteram") ,
		m_ram(*this, "ram"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf3_rowscroll(*this, "pf3_rowscroll"){ }

	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	//UINT16 *m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_pf3_rowscroll;
	//UINT16 *m_pf4_rowscroll;


	int m_flipscreen;
	DECLARE_WRITE16_MEMBER(darkseal_control_w);
	DECLARE_READ16_MEMBER(darkseal_control_r);
	DECLARE_WRITE16_MEMBER(darkseal_palette_24bit_rg_w);
	DECLARE_WRITE16_MEMBER(darkseal_palette_24bit_b_w);
};


/*----------- defined in video/darkseal.c -----------*/

VIDEO_START( darkseal );
SCREEN_UPDATE_IND16( darkseal );

