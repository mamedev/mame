// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Namco System NB-1 hardware

***************************************************************************/

#include "namcos2.h"
#include "machine/eeprompar.h"
#include "video/c116.h"

#define NAMCONB1_HTOTAL     (288)   /* wrong */
#define NAMCONB1_HBSTART    (288)
#define NAMCONB1_VTOTAL     (262)   /* needs to be checked */
#define NAMCONB1_VBSTART    (224)

#define NAMCONB1_TILEMASKREGION     "tilemask"
#define NAMCONB1_TILEGFXREGION      "tile"
#define NAMCONB1_SPRITEGFXREGION    "sprite"
#define NAMCONB1_ROTMASKREGION      "rotmask"
#define NAMCONB1_ROTGFXREGION       "rot"

#define NAMCONB1_TILEGFX        0
#define NAMCONB1_SPRITEGFX      1
#define NAMCONB1_ROTGFX         2

class namconb1_state : public namcos2_shared_state
{
public:
	namconb1_state(const machine_config &mconfig, device_type type, std::string tag)
		: namcos2_shared_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_c116(*this, "c116"),
		m_eeprom(*this, "eeprom"),
		m_spritebank32(*this, "spritebank32"),
		m_tilebank32(*this, "tilebank32"),
		m_namconb_shareram(*this, "namconb_share") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	required_device<eeprom_parallel_28xx_device> m_eeprom;
	required_shared_ptr<UINT32> m_spritebank32;
	optional_shared_ptr<UINT32> m_tilebank32;
	required_shared_ptr<UINT16> m_namconb_shareram;

	UINT8 m_vbl_irq_level;
	UINT8 m_pos_irq_level;
	UINT8 m_unk_irq_level;
	UINT16 m_count;
	UINT8 m_nbx_port6;
	UINT32 m_tilemap_tile_bank[4];

	DECLARE_READ32_MEMBER(randgen_r);
	DECLARE_WRITE32_MEMBER(srand_w);
	DECLARE_WRITE8_MEMBER(namconb1_cpureg_w);
	DECLARE_WRITE8_MEMBER(namconb2_cpureg_w);
	DECLARE_READ8_MEMBER(namconb1_cpureg_r);
	DECLARE_READ8_MEMBER(namconb2_cpureg_r);
	DECLARE_READ32_MEMBER(custom_key_r);
	DECLARE_READ32_MEMBER(gunbulet_gun_r);
	DECLARE_READ32_MEMBER(namconb_share_r);
	DECLARE_WRITE32_MEMBER(namconb_share_w);
	DECLARE_WRITE16_MEMBER(nbmcu_shared_w);
	DECLARE_READ8_MEMBER(port6_r);
	DECLARE_WRITE8_MEMBER(port6_w);
	DECLARE_READ8_MEMBER(port7_r);
	DECLARE_READ8_MEMBER(dac7_r);
	DECLARE_READ8_MEMBER(dac6_r);
	DECLARE_READ8_MEMBER(dac5_r);
	DECLARE_READ8_MEMBER(dac4_r);
	DECLARE_READ8_MEMBER(dac3_r);
	DECLARE_READ8_MEMBER(dac2_r);
	DECLARE_READ8_MEMBER(dac1_r);
	DECLARE_READ8_MEMBER(dac0_r);

	DECLARE_DRIVER_INIT(sws95);
	DECLARE_DRIVER_INIT(machbrkr);
	DECLARE_DRIVER_INIT(sws97);
	DECLARE_DRIVER_INIT(sws96);
	DECLARE_DRIVER_INIT(vshoot);
	DECLARE_DRIVER_INIT(nebulray);
	DECLARE_DRIVER_INIT(gunbulet);
	DECLARE_DRIVER_INIT(gslgr94j);
	DECLARE_DRIVER_INIT(outfxies);
	DECLARE_DRIVER_INIT(gslgr94u);
	DECLARE_MACHINE_RESET(namconb);
	DECLARE_VIDEO_START(namconb1);
	DECLARE_VIDEO_START(namconb2);
	UINT32 screen_update_namconb1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_namconb2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(namconb_scantimer);
};
