// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "machine/c117.h"
#include "sound/dac.h"
#include "sound/namco.h"
#include "video/c116.h"

class namcos1_state : public driver_device
{
public:
	namcos1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_c116(*this, "c116"),
		m_c117(*this, "c117"),
		m_dac0(*this, "dac0"),
		m_dac1(*this, "dac1"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_playfield_control(*this, "pfcontrol"),
		m_triram(*this, "triram"),
		m_rom(*this, "user1"),
		m_soundbank(*this, "soundbank"),
		m_mcubank(*this, "mcubank"),
		m_io_dipsw(*this, "DIPSW") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	required_device<namco_c117_device> m_c117;
	required_device<dac_8bit_r2r_device> m_dac0;
	required_device<dac_8bit_r2r_device> m_dac1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_playfield_control;
	required_shared_ptr<uint8_t> m_triram;
	required_region_ptr<uint8_t> m_rom;

	required_memory_bank m_soundbank;
	required_memory_bank m_mcubank;

	required_ioport m_io_dipsw;

	int m_key_id;
	int m_key_reg;
	int m_key_rng;
	int m_key_swap4_arg;
	int m_key_swap4;
	int m_key_bottom4;
	int m_key_top4;
	unsigned int m_key_quotient;
	unsigned int m_key_reminder;
	unsigned int m_key_numerator_high_word;
	uint8_t m_key[8];
	int m_mcu_patch_data;
	int m_reset;
	int m_input_count;
	int m_strobe;
	int m_strobe_count;
	int m_stored_input[2];
	tilemap_t *m_bg_tilemap[6];
	uint8_t *m_tilemap_maskdata;
	int m_copy_sprites;
	uint8_t m_drawmode_table[16];

	void subres_w(int state);
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dsw_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dac_gain_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_patch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t quester_paddle_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t berabohm_buttons_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t faceoff_inputs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void _3dcs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t no_key_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void no_key_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t key_type1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void key_type1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t key_type2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void key_type2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t key_type3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void key_type3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_pacmania();
	void init_ws();
	void init_wldcourt();
	void init_tankfrc4();
	void init_blazer();
	void init_dangseed();
	void init_splatter();
	void init_alice();
	void init_faceoff();
	void init_puzlclub();
	void init_bakutotu();
	void init_rompers();
	void init_ws90();
	void init_tankfrce();
	void init_soukobdx();
	void init_shadowld();
	void init_berabohm();
	void init_galaga88();
	void init_blastoff();
	void init_quester();
	void init_ws89();
	void init_dspirit();
	void init_pistoldm();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void driver_init();

	void bg_get_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bg_get_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bg_get_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void bg_get_info3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fg_get_info4(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void fg_get_info5(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);

private:
	inline void get_tile_info(tile_data &tileinfo,int tile_index,uint8_t *info_vram);
};
