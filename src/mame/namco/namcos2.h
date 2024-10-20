// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***************************************************************************

  namcos2.h

  Common functions & declarations for the Namco System 2 driver

***************************************************************************/
#ifndef MAME_NAMCO_NAMCOS2_H
#define MAME_NAMCO_NAMCOS2_H

#pragma once

#include "namco_c139.h"
#include "namco_c148.h"
#include "machine/timer.h"
#include "sound/c140.h"
#include "namco_c45road.h"
#include "namco_c116.h"
#include "namco65.h"
#include "namco68.h"
#include "namco_c169roz.h"
#include "namco_c355spr.h"
#include "namco_c123tmap.h"
#include "namcos2_sprite.h"
#include "namcos2_roz.h"
#include "screen.h"

/*********************************************/
/* IF GAME SPECIFIC HACKS ARE REQUIRED THEN  */
/* USE THE m_gametype MEMBER TO FIND         */
/* OUT WHAT GAME IS RUNNING                  */
/*********************************************/

class namcos2_state : public driver_device
{
public:
	namcos2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gametype(0),
		m_update_to_line_before_posirq(false),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_audiocpu(*this, "audiocpu"),
		m_c65(*this, "c65mcu"),
		m_c68(*this, "c68mcu"),
		m_c140(*this, "c140"),
		m_c116(*this, "c116"),
		m_c123tmap(*this, "c123tmap"),
		m_master_intc(*this, "master_intc"),
		m_slave_intc(*this, "slave_intc"),
		m_sci(*this, "sci"),
		m_c169roz(*this, "c169roz"),
		m_c355spr(*this, "c355spr"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_audiobank(*this, "audiobank"),
		m_c140_region(*this, "c140"),
		m_dpram(*this, "dpram"),
		m_spriteram(*this, "spriteram"),
		m_c45_road(*this, "c45_road"),
		m_ns2sprite(*this, "s2sprite"),
		m_ns2roz(*this, "s2roz")
	{ }

	void configure_c116_standard(machine_config &config);
	void configure_c148_standard(machine_config &config);
	void configure_c65_standard(machine_config &config);
	void configure_c68_standard(machine_config &config);
	void configure_c123tmap_standard(machine_config &config);
	void configure_c169roz_standard(machine_config &config);
	void configure_c355spr_standard(machine_config &config);
	void configure_c45road_standard(machine_config &config);
	void configure_common_standard(machine_config &config);
	void configure_namcos2_sprite_standard(machine_config &config);
	void configure_namcos2_roz_standard(machine_config &config);
	void metlhawk(machine_config &config);
	void assaultp(machine_config &config);
	void sgunner2(machine_config &config);
	void base2(machine_config &config);
	void finallap_noio(machine_config &config);
	void base_fl(machine_config &config);
	void finallap(machine_config &config);
	void finallap_c68(machine_config &config);
	void finalap2(machine_config &config);
	void finalap3(machine_config &config);
	void suzuka8h(machine_config &config);
	void luckywld(machine_config &config);
	void base3(machine_config &config);
	void sgunner(machine_config &config);
	void base_noio(machine_config &config);
	void base(machine_config &config);
	void base_c68(machine_config &config);

	void init_cosmogng();
	void init_sgunner2();
	void init_kyukaidk();
	void init_bubbletr();
	void init_suzuk8h2();
	void init_burnforc();
	void init_gollygho();
	void init_rthun2j();
	void init_sws();
	void init_finehour();
	void init_finallap();
	void init_dirtfoxj();
	void init_sws92();
	void init_dsaber();
	void init_assault();
	void init_mirninja();
	void init_finalap2();
	void init_valkyrie();
	void init_fourtrax();
	void init_finalap3();
	void init_finalap3bl();
	void init_luckywld();
	void init_assaultj();
	void init_dsaberj();
	void init_suzuka8h();
	void init_phelios();
	void init_sws93();
	void init_metlhawk();
	void init_sws92g();
	void init_assaultp();
	void init_ordyne();
	void init_marvland();
	void init_rthun2();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

enum
	{
		/* Namco System 2 */
		NAMCOS2_ASSAULT = 0x1000,
		NAMCOS2_ASSAULT_JP,
		NAMCOS2_ASSAULT_PLUS,
		NAMCOS2_BUBBLE_TROUBLE,
		NAMCOS2_BURNING_FORCE,
		NAMCOS2_COSMO_GANG,
		NAMCOS2_COSMO_GANG_US,
		NAMCOS2_DIRT_FOX,
		NAMCOS2_DIRT_FOX_JP,
		NAMCOS2_DRAGON_SABER,
		NAMCOS2_FINAL_LAP,
		NAMCOS2_FINAL_LAP_2,
		NAMCOS2_FINAL_LAP_3,
		NAMCOS2_FINEST_HOUR,
		NAMCOS2_FOUR_TRAX,
		NAMCOS2_GOLLY_GHOST,
		NAMCOS2_LUCKY_AND_WILD,
		NAMCOS2_MARVEL_LAND,
		NAMCOS2_METAL_HAWK,
		NAMCOS2_MIRAI_NINJA,
		NAMCOS2_ORDYNE,
		NAMCOS2_PHELIOS,
		NAMCOS2_ROLLING_THUNDER_2,
		NAMCOS2_STEEL_GUNNER,
		NAMCOS2_STEEL_GUNNER_2,
		NAMCOS2_SUPER_WSTADIUM,
		NAMCOS2_SUPER_WSTADIUM_92,
		NAMCOS2_SUPER_WSTADIUM_92T,
		NAMCOS2_SUPER_WSTADIUM_93,
		NAMCOS2_SUZUKA_8_HOURS,
		NAMCOS2_SUZUKA_8_HOURS_2,
		NAMCOS2_VALKYRIE,
		NAMCOS2_KYUUKAI_DOUCHUUKI,
	};

	int m_gametype = 0;
	bool m_update_to_line_before_posirq = false;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_audiocpu;
	optional_device<namcoc65_device> m_c65;
	optional_device<namcoc68_device> m_c68;
	required_device<c140_device> m_c140;
	required_device<namco_c116_device> m_c116;
	required_device<namco_c123tmap_device> m_c123tmap;
	required_device<namco_c148_device> m_master_intc;
	required_device<namco_c148_device> m_slave_intc;
	required_device<namco_c139_device> m_sci;
	optional_device<namco_c169roz_device> m_c169roz;
	optional_device<namco_c355spr_device> m_c355spr;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_memory_bank m_audiobank;
	required_region_ptr<u16> m_c140_region;

	std::unique_ptr<uint8_t[]> m_eeprom;

	uint16_t dpram_word_r(offs_t offset);
	virtual void dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t dpram_byte_r(offs_t offset);
	void dpram_byte_w(offs_t offset, uint8_t data);

	void eeprom_w(offs_t offset, uint8_t data);
	uint8_t eeprom_r(offs_t offset);

	uint16_t c140_rom_r(offs_t offset);
	void sound_bankselect_w(uint8_t data);

	void sound_reset_w(uint8_t data);
	void system_reset_w(uint8_t data);
	void reset_all_subcpus(int state);

	void video_start_luckywld();
	void video_start_metlhawk();
	void video_start_sgunner();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_finallap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_luckywld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_metlhawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sgunner(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t c116_r(offs_t offset);

	uint16_t gfx_ctrl_r();
	void gfx_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void create_shadow_table();
	void apply_clip( rectangle &clip, const rectangle &cliprect );

	int get_pos_irq_scanline() { return (m_c116->get_reg(5) - 32) & 0xff; }
	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	required_shared_ptr<uint8_t> m_dpram; /* 2Kx8 */
	optional_shared_ptr<uint16_t> m_spriteram;
	uint16_t m_gfx_ctrl = 0;
	unsigned m_finallap_prot_count = 0;
	int m_sendval = 0;

	optional_device<namco_c45_road_device> m_c45_road;
	optional_device<namcos2_sprite_device> m_ns2sprite;
	optional_device<namcos2_roz_device> m_ns2roz;

	uint16_t namcos2_68k_key_r(offs_t offset);
	void namcos2_68k_key_w(offs_t offset, uint16_t data);
	uint16_t namcos2_finallap_prot_r(offs_t offset); // finalap2, finalap3
	uint16_t finalap3bl_prot_r(); // finalap3bl

	void TilemapCB(uint16_t code, int *tile, int *mask);
	void TilemapCB_finalap2(uint16_t code, int *tile, int *mask);
	void RozCB_luckywld(uint16_t code, int *tile, int *mask, int which);
	void RozCB_metlhawk(uint16_t code, int *tile, int *mask, int which);

	void c140_default_am(address_map &map) ATTR_COLD;
	void common_default_am(address_map &map) ATTR_COLD;
	void common_finallap_am(address_map &map) ATTR_COLD;
	void common_suzuka8h_am(address_map &map) ATTR_COLD;
	void common_suzuka8h_roz_am(address_map &map) ATTR_COLD;
	void common_luckywld_roz_am(address_map &map) ATTR_COLD;
	void common_metlhawk_am(address_map &map) ATTR_COLD;
	void common_sgunner_am(address_map &map) ATTR_COLD;
	void master_common_am(address_map &map) ATTR_COLD;
	void master_default_am(address_map &map) ATTR_COLD;
	void master_finallap_am(address_map &map) ATTR_COLD;
	void master_suzuka8h_am(address_map &map) ATTR_COLD;
	void master_luckywld_am(address_map &map) ATTR_COLD;
	void master_metlhawk_am(address_map &map) ATTR_COLD;
	void master_sgunner_am(address_map &map) ATTR_COLD;

	void namcos2_68k_default_cpu_board_am(address_map &map) ATTR_COLD;
	void slave_common_am(address_map &map) ATTR_COLD;
	void slave_default_am(address_map &map) ATTR_COLD;
	void slave_finallap_am(address_map &map) ATTR_COLD;
	void slave_suzuka8h_am(address_map &map) ATTR_COLD;
	void slave_luckywld_am(address_map &map) ATTR_COLD;
	void slave_metlhawk_am(address_map &map) ATTR_COLD;
	void slave_sgunner_am(address_map &map) ATTR_COLD;
	void sound_default_am(address_map &map) ATTR_COLD;
};

class gollygho_state : public namcos2_state
{
public:
	gollygho_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos2_state(mconfig, type, tag),
		m_out_digit(*this, "digit%u", 0U),
		m_out_diorama(*this, "diorama%u", 0U),
		m_out_gun_recoil(*this, "gun_recoil%u", 0U)
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

private:
	output_finder<8> m_out_digit;
	output_finder<6> m_out_diorama;
	output_finder<2> m_out_gun_recoil;
};


/**************************************************************/
/* Non-shared memory custom IO device - IRQ/Inputs/Outputs   */
/**************************************************************/

#define NAMCOS2_C148_0          0       /* 0x1c0000 */
#define NAMCOS2_C148_1          1       /* 0x1c2000 */
#define NAMCOS2_C148_2          2       /* 0x1c4000 */
#define NAMCOS2_C148_CPUIRQ     3       /* 0x1c6000 */
#define NAMCOS2_C148_EXIRQ      4       /* 0x1c8000 */
#define NAMCOS2_C148_POSIRQ     5       /* 0x1ca000 */
#define NAMCOS2_C148_SERIRQ     6       /* 0x1cc000 */
#define NAMCOS2_C148_VBLANKIRQ  7       /* 0x1ce000 */

#endif // MAME_NAMCO_NAMCOS2_H
