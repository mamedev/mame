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
	namconb1_state(const machine_config &mconfig, device_type type, const char *tag)
		: namcos2_shared_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_c116(*this, "c116"),
		m_eeprom(*this, "eeprom"),
		m_p1(*this, "P1"),
		m_p2(*this, "P2"),
		m_p3(*this, "P3"),
		m_p4(*this, "P4"),
		m_misc(*this, "MISC"),
		m_light0_x(*this, "LIGHT0_X"),
		m_light0_y(*this, "LIGHT0_Y"),
		m_light1_x(*this, "LIGHT1_X"),
		m_light1_y(*this, "LIGHT1_Y"),
		m_spritebank32(*this, "spritebank32"),
		m_tilebank32(*this, "tilebank32"),
		m_namconb_shareram(*this, "namconb_share") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	required_device<eeprom_parallel_28xx_device> m_eeprom;
	required_ioport m_p1;
	required_ioport m_p2;
	optional_ioport m_p3;
	optional_ioport m_p4;
	required_ioport m_misc;
	optional_ioport m_light0_x;
	optional_ioport m_light0_y;
	optional_ioport m_light1_x;
	optional_ioport m_light1_y;
	required_shared_ptr<uint32_t> m_spritebank32;
	optional_shared_ptr<uint32_t> m_tilebank32;
	required_shared_ptr<uint16_t> m_namconb_shareram;

	uint8_t m_vbl_irq_level;
	uint8_t m_pos_irq_level;
	uint8_t m_unk_irq_level;
	uint16_t m_count;
	uint8_t m_port6;
	uint32_t m_tilemap_tile_bank[4];

	uint32_t randgen_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void srand_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void namconb1_cpureg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void namconb2_cpureg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t namconb1_cpureg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t namconb2_cpureg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint32_t custom_key_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t gunbulet_gun_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t share_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void share_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void mcu_shared_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t port6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	void init_sws95();
	void init_machbrkr();
	void init_sws97();
	void init_sws96();
	void init_vshoot();
	void init_nebulray();
	void init_gunbulet();
	void init_gslgr94j();
	void init_outfxies();
	void init_gslgr94u();
	void machine_reset_namconb();
	void video_start_namconb1();
	void video_start_namconb2();
	void video_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bROZ);
	uint32_t screen_update_namconb1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_namconb2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void scantimer(timer_device &timer, void *ptr, int32_t param);
	void mcu_irq0_cb(timer_device &timer, void *ptr, int32_t param);
	void mcu_irq2_cb(timer_device &timer, void *ptr, int32_t param);
	void mcu_adc_cb(timer_device &timer, void *ptr, int32_t param);

	int NB1objcode2tile(int code);
	int NB2objcode2tile(int code);
};
