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
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_spriteram2;
	optional_shared_ptr<UINT8> m_s2650_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_s2650games_tileram;
	optional_shared_ptr<UINT8> m_rocktrv2_prot_data;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_patched_opcodes;

	UINT8 m_cannonb_bit_to_read;
	int m_mystery;
	UINT8 m_counter;
	int m_bigbucks_bank;
	UINT8 m_rocktrv2_question_bank;
	tilemap_t *m_bg_tilemap;
	UINT8 m_charbank;
	UINT8 m_spritebank;
	UINT8 m_palettebank;
	UINT8 m_colortablebank;
	UINT8 m_flipscreen;
	UINT8 m_bgpriority;
	int m_xoffsethack;
	UINT8 m_inv_spr;
	UINT8 m_irq_mask;

	DECLARE_WRITE8_MEMBER(pacman_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(piranha_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(nmouse_interrupt_vector_w);
	DECLARE_WRITE8_MEMBER(pacman_leds_w);
	DECLARE_WRITE8_MEMBER(pacman_coin_counter_w);
	DECLARE_WRITE8_MEMBER(pacman_coin_lockout_global_w);
	DECLARE_WRITE8_MEMBER(alibaba_sound_w);
	DECLARE_READ8_MEMBER(alibaba_mystery_1_r);
	DECLARE_READ8_MEMBER(alibaba_mystery_2_r);
	DECLARE_READ8_MEMBER(maketrax_special_port2_r);
	DECLARE_READ8_MEMBER(maketrax_special_port3_r);
	DECLARE_READ8_MEMBER(korosuke_special_port2_r);
	DECLARE_READ8_MEMBER(korosuke_special_port3_r);
	DECLARE_READ8_MEMBER(mschamp_kludge_r);
	DECLARE_WRITE8_MEMBER(bigbucks_bank_w);
	DECLARE_READ8_MEMBER(bigbucks_question_r);
	DECLARE_WRITE8_MEMBER(porky_banking_w);
	DECLARE_READ8_MEMBER(drivfrcp_port1_r);
	DECLARE_READ8_MEMBER(_8bpm_port1_r);
	DECLARE_READ8_MEMBER(porky_port1_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot1_data_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot2_data_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot3_data_r);
	DECLARE_READ8_MEMBER(rocktrv2_prot4_data_r);
	DECLARE_WRITE8_MEMBER(rocktrv2_prot_data_w);
	DECLARE_WRITE8_MEMBER(rocktrv2_question_bank_w);
	DECLARE_READ8_MEMBER(rocktrv2_question_r);
	DECLARE_READ8_MEMBER(pacman_read_nop);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x0038);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x03b0);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x1600);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x2120);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x3ff0);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x8000);
	DECLARE_READ8_MEMBER(mspacman_disable_decode_r_0x97f0);
	DECLARE_WRITE8_MEMBER(mspacman_disable_decode_w);
	DECLARE_READ8_MEMBER(mspacman_enable_decode_r_0x3ff8);
	DECLARE_WRITE8_MEMBER(mspacman_enable_decode_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_READ8_MEMBER(mspacii_protection_r);
	DECLARE_READ8_MEMBER(cannonbp_protection_r);
	DECLARE_WRITE8_MEMBER(pacman_videoram_w);
	DECLARE_WRITE8_MEMBER(pacman_colorram_w);
	DECLARE_WRITE8_MEMBER(pacman_flipscreen_w);
	DECLARE_WRITE8_MEMBER(pengo_palettebank_w);
	DECLARE_WRITE8_MEMBER(pengo_colortablebank_w);
	DECLARE_WRITE8_MEMBER(pengo_gfxbank_w);
	DECLARE_WRITE8_MEMBER(s2650games_videoram_w);
	DECLARE_WRITE8_MEMBER(s2650games_colorram_w);
	DECLARE_WRITE8_MEMBER(s2650games_scroll_w);
	DECLARE_WRITE8_MEMBER(s2650games_tilesbank_w);
	DECLARE_WRITE8_MEMBER(jrpacman_videoram_w);
	DECLARE_WRITE8_MEMBER(jrpacman_charbank_w);
	DECLARE_WRITE8_MEMBER(jrpacman_spritebank_w);
	DECLARE_WRITE8_MEMBER(jrpacman_scroll_w);
	DECLARE_WRITE8_MEMBER(jrpacman_bgpriority_w);
	DECLARE_WRITE8_MEMBER(superabc_bank_w);
	DECLARE_DRIVER_INIT(maketrax);
	DECLARE_DRIVER_INIT(drivfrcp);
	DECLARE_DRIVER_INIT(mspacmbe);
	DECLARE_DRIVER_INIT(ponpoko);
	DECLARE_DRIVER_INIT(eyes);
	DECLARE_DRIVER_INIT(woodpek);
	DECLARE_DRIVER_INIT(cannonbp);
	DECLARE_DRIVER_INIT(jumpshot);
	DECLARE_DRIVER_INIT(korosuke);
	DECLARE_DRIVER_INIT(mspacii);
	DECLARE_DRIVER_INIT(pacplus);
	DECLARE_DRIVER_INIT(rocktrv2);
	DECLARE_DRIVER_INIT(superabc);
	DECLARE_DRIVER_INIT(8bpm);
	DECLARE_DRIVER_INIT(porky);
	DECLARE_DRIVER_INIT(mspacman);
	DECLARE_DRIVER_INIT(mschamp);
	TILEMAP_MAPPER_MEMBER(pacman_scan_rows);
	TILE_GET_INFO_MEMBER(pacman_get_tile_info);
	TILE_GET_INFO_MEMBER(s2650_get_tile_info);
	TILEMAP_MAPPER_MEMBER(jrpacman_scan_rows);
	TILE_GET_INFO_MEMBER(jrpacman_get_tile_info);
	DECLARE_VIDEO_START(pacman);
	DECLARE_PALETTE_INIT(pacman);
	DECLARE_VIDEO_START(birdiy);
	DECLARE_VIDEO_START(s2650games);
	DECLARE_MACHINE_RESET(mschamp);
	DECLARE_MACHINE_RESET(superabc);
	DECLARE_VIDEO_START(pengo);
	DECLARE_VIDEO_START(jrpacman);
	UINT32 screen_update_pacman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_s2650games(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(vblank_nmi);
	INTERRUPT_GEN_MEMBER(s2650_interrupt);
	void init_save_state();
	void jrpacman_mark_tile_dirty( int offset );
	void maketrax_rom_decode();
	void korosuke_rom_decode();
	void eyes_decode(UINT8 *data);
	void mspacman_install_patches(UINT8 *ROM);

	// theglopb.c
	void theglobp_decrypt_rom_8();
	void theglobp_decrypt_rom_9();
	void theglobp_decrypt_rom_A();
	void theglobp_decrypt_rom_B();
	DECLARE_READ8_MEMBER(theglobp_decrypt_rom);
	DECLARE_MACHINE_START(theglobp);
	DECLARE_MACHINE_RESET(theglobp);

	// pacplus.c
	UINT8 pacplus_decrypt(int addr, UINT8 e);
	void pacplus_decode();

	// jumpshot.c
	UINT8 jumpshot_decrypt(int addr, UINT8 e);
	void jumpshot_decode();

	// acitya.c
	void acitya_decrypt_rom_8();
	void acitya_decrypt_rom_9();
	void acitya_decrypt_rom_A();
	void acitya_decrypt_rom_B();
	DECLARE_READ8_MEMBER(acitya_decrypt_rom);
	DECLARE_MACHINE_START(acitya);
	DECLARE_MACHINE_RESET(acitya);
};
