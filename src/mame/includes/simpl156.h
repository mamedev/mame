/*************************************************************************

    Simple 156 based board

*************************************************************************/

#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"

class simpl156_state : public driver_device
{
public:
	simpl156_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_deco_tilegen1(*this, "tilegen1"),
		  m_eeprom(*this, "eeprom"),
		  m_okimusic(*this, "okimusic") ,
		m_mainram(*this, "mainram"),
		m_systemram(*this, "systemram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<eeprom_device> m_eeprom;
	required_device<okim6295_device> m_okimusic;
	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_systemram;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;

	DECLARE_READ32_MEMBER(simpl156_inputs_read);
	DECLARE_READ32_MEMBER(simpl156_palette_r);
	DECLARE_WRITE32_MEMBER(simpl156_palette_w);
	DECLARE_READ32_MEMBER(simpl156_system_r);
	DECLARE_WRITE32_MEMBER(simpl156_eeprom_w);
	DECLARE_READ32_MEMBER(simpl156_spriteram_r);
	DECLARE_WRITE32_MEMBER(simpl156_spriteram_w);
	DECLARE_READ32_MEMBER(simpl156_mainram_r);
	DECLARE_WRITE32_MEMBER(simpl156_mainram_w);
	DECLARE_READ32_MEMBER(simpl156_pf1_rowscroll_r);
	DECLARE_WRITE32_MEMBER(simpl156_pf1_rowscroll_w);
	DECLARE_READ32_MEMBER(simpl156_pf2_rowscroll_r);
	DECLARE_WRITE32_MEMBER(simpl156_pf2_rowscroll_w);
	DECLARE_READ32_MEMBER(joemacr_speedup_r);
	DECLARE_READ32_MEMBER(chainrec_speedup_r);
	DECLARE_READ32_MEMBER(prtytime_speedup_r);
	DECLARE_READ32_MEMBER(charlien_speedup_r);
	DECLARE_READ32_MEMBER(osman_speedup_r);
	DECLARE_DRIVER_INIT(simpl156);
	DECLARE_DRIVER_INIT(joemacr);
	DECLARE_DRIVER_INIT(charlien);
	DECLARE_DRIVER_INIT(prtytime);
	DECLARE_DRIVER_INIT(osman);
	DECLARE_DRIVER_INIT(chainrec);
	virtual void video_start();
	UINT32 screen_update_simpl156(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
