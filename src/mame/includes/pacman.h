// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/namco.h"

/*************************************************************************

    Namco PuckMan

**************************************************************************/

class pacman_state : public driver_device
{
public:
	pacman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_namco_sound(*this, "namco"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_s2650_spriteram(*this, "s2650_spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_s2650games_tileram(*this, "s2650_tileram"),
		m_rocktrv2_prot_data(*this, "rocktrv2_prot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_patched_opcodes(*this, "patched_opcodes")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<namco_device> m_namco_sound;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram2;
	optional_shared_ptr<uint8_t> m_s2650_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_s2650games_tileram;
	optional_shared_ptr<uint8_t> m_rocktrv2_prot_data;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_patched_opcodes;

	uint8_t m_cannonb_bit_to_read;
	int m_mystery;
	uint8_t m_counter;
	int m_bigbucks_bank;
	uint8_t m_rocktrv2_question_bank;
	tilemap_t *m_bg_tilemap;
	uint8_t m_charbank;
	uint8_t m_spritebank;
	uint8_t m_palettebank;
	uint8_t m_colortablebank;
	uint8_t m_flipscreen;
	uint8_t m_bgpriority;
	int m_xoffsethack;
	uint8_t m_inv_spr;
	uint8_t m_irq_mask;

	void pacman_interrupt_vector_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void piranha_interrupt_vector_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nmouse_interrupt_vector_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pacman_leds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pacman_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pacman_coin_lockout_global_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void alibaba_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t alibaba_mystery_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t alibaba_mystery_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t maketrax_special_port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t maketrax_special_port3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t korosuke_special_port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t korosuke_special_port3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mschamp_kludge_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bigbucks_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bigbucks_question_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void porky_banking_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t drivfrcp_port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t _8bpm_port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t porky_port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t rocktrv2_prot1_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t rocktrv2_prot2_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t rocktrv2_prot3_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t rocktrv2_prot4_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rocktrv2_prot_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rocktrv2_question_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rocktrv2_question_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pacman_read_nop(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mspacman_disable_decode_r_0x0038(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mspacman_disable_decode_r_0x03b0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mspacman_disable_decode_r_0x1600(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mspacman_disable_decode_r_0x2120(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mspacman_disable_decode_r_0x3ff0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mspacman_disable_decode_r_0x8000(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mspacman_disable_decode_r_0x97f0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mspacman_disable_decode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mspacman_enable_decode_r_0x3ff8(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mspacman_enable_decode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mspacii_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t cannonbp_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pacman_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pacman_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pacman_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pengo_palettebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pengo_colortablebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pengo_gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void s2650games_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void s2650games_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void s2650games_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void s2650games_tilesbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jrpacman_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jrpacman_charbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jrpacman_spritebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jrpacman_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jrpacman_bgpriority_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void superabc_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_maketrax();
	void init_drivfrcp();
	void init_mspacmbe();
	void init_ponpoko();
	void init_eyes();
	void init_woodpek();
	void init_cannonbp();
	void init_jumpshot();
	void init_korosuke();
	void init_mspacii();
	void init_pacplus();
	void init_rocktrv2();
	void init_superabc();
	void init_8bpm();
	void init_porky();
	void init_mspacman();
	void init_mschamp();
	tilemap_memory_index pacman_scan_rows(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void pacman_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void s2650_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index jrpacman_scan_rows(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void jrpacman_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_pacman();
	void palette_init_pacman(palette_device &palette);
	void video_start_birdiy();
	void video_start_s2650games();
	void machine_reset_mschamp();
	void machine_reset_superabc();
	void video_start_pengo();
	void video_start_jrpacman();
	uint32_t screen_update_pacman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_s2650games(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void vblank_nmi(device_t &device);
	void s2650_interrupt(device_t &device);
	void init_save_state();
	void jrpacman_mark_tile_dirty( int offset );
	void maketrax_rom_decode();
	void korosuke_rom_decode();
	void eyes_decode(uint8_t *data);
	void mspacman_install_patches(uint8_t *ROM);

	// theglopb.c
	void theglobp_decrypt_rom_8();
	void theglobp_decrypt_rom_9();
	void theglobp_decrypt_rom_A();
	void theglobp_decrypt_rom_B();
	uint8_t theglobp_decrypt_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void machine_start_theglobp();
	void machine_reset_theglobp();

	// pacplus.c
	uint8_t pacplus_decrypt(int addr, uint8_t e);
	void pacplus_decode();

	// jumpshot.c
	uint8_t jumpshot_decrypt(int addr, uint8_t e);
	void jumpshot_decode();

	// acitya.c
	void acitya_decrypt_rom_8();
	void acitya_decrypt_rom_9();
	void acitya_decrypt_rom_A();
	void acitya_decrypt_rom_B();
	uint8_t acitya_decrypt_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void machine_start_acitya();
	void machine_reset_acitya();
};
