// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Stephane Humbert
/***************************************************************************

  Toaplan Slap Fight hardware

***************************************************************************/

#include "cpu/z80/z80.h"
#include "video/bufsprite.h"


class slapfght_state : public driver_device
{
public:
	slapfght_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_fixvideoram(*this, "fixvideoram"),
		m_fixcolorram(*this, "fixcolorram")
	{ }

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_spriteram;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_fixvideoram;
	optional_shared_ptr<uint8_t> m_fixcolorram;

	/* This it the best way to allow game specific kludges until the system is fully understood */
	enum getstar_id
	{
		GETSTUNK = 0, /* unknown for inclusion of possible new sets */
		GETSTAR,
		GETSTARJ,
		GETSTARB1,    /* "good" bootleg with same behaviour as 'getstarj' */
		GETSTARB2     /* "lame" bootleg with lots of ingame bugs */
	} m_getstar_id;

	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_fix_tilemap;
	uint8_t m_palette_bank;
	uint8_t m_scrollx_lo;
	uint8_t m_scrollx_hi;
	uint8_t m_scrolly;
	bool m_main_irq_enabled;
	bool m_sound_nmi_enabled;

	bool m_mcu_sent;
	bool m_main_sent;
	uint8_t m_from_main;
	uint8_t m_from_mcu;
	uint8_t m_portA_in;
	uint8_t m_portA_out;
	uint8_t m_ddrA;
	uint8_t m_portB_in;
	uint8_t m_portB_out;
	uint8_t m_ddrB;
	uint8_t m_portC_in;
	uint8_t m_portC_out;
	uint8_t m_ddrC;

	int m_getstar_status;
	int m_getstar_sequence_index;
	int m_getstar_status_state;
	uint8_t m_getstar_cmd;
	uint8_t m_gs_a;
	uint8_t m_gs_d;
	uint8_t m_gs_e;
	uint8_t m_tigerhb_cmd;

	void sound_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prg_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vblank_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fixram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fixcol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollx_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrollx_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scrolly_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void tigerh_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tigerh_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tigerh_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tigerh_68705_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tigerh_68705_portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tigerh_68705_ddrA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tigerh_68705_portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tigerh_68705_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tigerh_68705_ddrB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tigerh_68705_portC_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tigerh_68705_portC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tigerh_68705_ddrC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t slapfight_68705_portA_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void slapfight_68705_portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void slapfight_68705_ddrA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t slapfight_68705_portB_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void slapfight_68705_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void slapfight_68705_ddrB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t slapfight_68705_portC_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void slapfight_68705_portC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void slapfight_68705_ddrC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t getstar_mcusim_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void getstar_mcusim_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t getstar_mcusim_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t getstarb1_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tigerhb1_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tigerhb1_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void machine_reset_getstar();

	void init_banks();
	void init_getstarj();
	void init_getstar();
	void init_getstarb1();
	void init_slapfigh();
	void init_getstarb2();

	void get_pf_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fix_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_perfrman();
	void video_start_slapfight();

	void draw_perfrman_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	void draw_slapfight_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(device_t &device);
	void sound_nmi(device_t &device);
};
