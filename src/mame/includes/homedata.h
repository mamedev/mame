// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/sn76496.h"
#include "sound/2203intf.h"

class homedata_state : public driver_device
{
public:
	homedata_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vreg(*this, "vreg"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_sn(*this, "snsnd")
	{
	}

	/* memory pointers */
	optional_shared_ptr<uint8_t> m_vreg;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t *m_bg_tilemap[2][4];
	int      m_visible_page;
	int      m_priority;
	uint8_t    m_reikaids_which;
	int      m_flipscreen;
	uint8_t      m_gfx_bank[2];   // pteacher only uses the first one
	uint8_t      m_blitter_bank;
	int      m_blitter_param_count;
	uint8_t      m_blitter_param[4];      /* buffers last 4 writes to 0x8006 */


	/* misc */
	int      m_vblank;
	int      m_sndbank;
	int      m_keyb;
	int      m_snd_command;
	int      m_upd7807_porta;
	int      m_upd7807_portc;
	int      m_to_cpu;
	int      m_from_cpu;

	/* device */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<ym2203_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // mrokumei
	optional_device<sn76489a_device> m_sn; // mrokumei and pteacher

	uint8_t m_prot_data;
	uint8_t mrokumei_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mrokumei_keyboard_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mrokumei_sound_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mrokumei_sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrokumei_sound_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t reikaids_upd7807_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void reikaids_upd7807_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reikaids_upd7807_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t reikaids_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t reikaids_snd_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void reikaids_snd_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pteacher_snd_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pteacher_snd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pteacher_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pteacher_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pteacher_upd7807_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pteacher_snd_answer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pteacher_upd7807_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pteacher_upd7807_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mirderby_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mirderby_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrokumei_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reikaids_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reikaids_gfx_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pteacher_gfx_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void homedata_blitter_param_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrokumei_blitter_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reikaids_blitter_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pteacher_blitter_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mrokumei_blitter_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reikaids_blitter_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pteacher_blitter_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_reikaids();
	void init_mjikaga();
	void init_jogakuen();
	void init_battlcry();
	void init_mirderby();
	void mrokumei_get_info0_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mrokumei_get_info1_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mrokumei_get_info0_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mrokumei_get_info1_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info0_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info1_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info0_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info1_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info0_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info1_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info0_3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void reikaids_get_info1_3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pteacher_get_info0_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pteacher_get_info1_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pteacher_get_info0_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void pteacher_get_info1_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void lemnangl_get_info0_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void lemnangl_get_info1_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void lemnangl_get_info0_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void lemnangl_get_info1_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mirderby_get_info0_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mirderby_get_info1_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mirderby_get_info0_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void mirderby_get_info1_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_homedata();
	void machine_reset_homedata();
	void video_start_mrokumei();
	void palette_init_mrokumei(palette_device &palette);
	void machine_start_reikaids();
	void machine_reset_reikaids();
	void video_start_reikaids();
	void palette_init_reikaids(palette_device &palette);
	void machine_start_pteacher();
	void machine_reset_pteacher();
	void video_start_pteacher();
	void palette_init_pteacher(palette_device &palette);
	void video_start_mirderby();
	void palette_init_mirderby(palette_device &palette);
	void video_start_lemnangl();
	uint32_t screen_update_mrokumei(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_reikaids(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pteacher(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mirderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_homedata(screen_device &screen, bool state);
	void homedata_irq(device_t &device);
	void upd7807_irq(device_t &device);
	void mrokumei_handleblit( address_space &space, int rom_base );
	void reikaids_handleblit( address_space &space, int rom_base );
	void pteacher_handleblit( address_space &space, int rom_base );
	inline void mrokumei_info0( tile_data &tileinfo, int tile_index, int page, int gfxbank );
	inline void mrokumei_info1( tile_data &tileinfo, int tile_index, int page, int gfxbank );
	inline void reikaids_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank );
	inline void pteacher_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxbank );
	inline void lemnangl_info( tile_data &tileinfo, int tile_index, int page, int layer, int gfxset, int gfxbank );
	inline void mirderby_info0( tile_data &tileinfo, int tile_index, int page, int gfxbank );
	inline void mirderby_info1( tile_data &tileinfo, int tile_index, int page, int gfxbank );
};
