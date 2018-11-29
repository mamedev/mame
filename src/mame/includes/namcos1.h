// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_INCLUDES_NAMCOS1_H
#define MAME_INCLUDES_NAMCOS1_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "cpu/m6809/m6809.h"
#include "machine/c117.h"
#include "sound/dac.h"
#include "sound/namco.h"
#include "video/namco_c116.h"
#include "video/namco_c123tmap.h"
#include "machine/74157.h"

class namcos1_state : public driver_device
{
public:
	namcos1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_c116(*this, "c116"),
		m_c117(*this, "c117"),
		m_c123tmap(*this, "c123tmap"),
		m_dac(*this, "dac%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_triram(*this, "triram"),
		m_rom(*this, "user1"),
		m_soundbank(*this, "soundbank"),
		m_mcubank(*this, "mcubank"),
		m_io_dipsw(*this, "DIPSW"),
		m_dsw_sel(*this, "dsw_sel")
	{ }

	void ns1(machine_config &config);

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

private:
	required_device<mc6809e_device> m_maincpu;
	required_device<mc6809e_device> m_subcpu;
	required_device<mc6809e_device> m_audiocpu;
	required_device<hd63701_cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	required_device<namco_c117_device> m_c117;
	required_device<namco_c123tmap_device> m_c123tmap;
	required_device_array<dac_8bit_r2r_device, 2> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_triram;
	required_region_ptr<uint8_t> m_rom;

	required_memory_bank m_soundbank;
	required_memory_bank m_mcubank;

	required_ioport m_io_dipsw;
	required_device<ls157_device> m_dsw_sel;

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
	int m_copy_sprites;
	uint8_t m_drawmode_table[16];

	DECLARE_WRITE_LINE_MEMBER(subres_w);
	DECLARE_WRITE8_MEMBER(audiocpu_irq_ack_w);
	DECLARE_WRITE8_MEMBER(mcu_irq_ack_w);
	DECLARE_READ8_MEMBER(dsw_r);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(dac_gain_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(mcu_bankswitch_w);
	DECLARE_WRITE8_MEMBER(mcu_patch_w);
	DECLARE_READ8_MEMBER(quester_paddle_r);
	DECLARE_READ8_MEMBER(berabohm_buttons_r);
	DECLARE_READ8_MEMBER(faceoff_inputs_r);
	DECLARE_WRITE8_MEMBER(spriteram_w);
	DECLARE_WRITE8_MEMBER(_3dcs_w);
	DECLARE_READ8_MEMBER(no_key_r);
	DECLARE_WRITE8_MEMBER(no_key_w);
	DECLARE_READ8_MEMBER(key_type1_r);
	DECLARE_WRITE8_MEMBER(key_type1_w);
	DECLARE_READ8_MEMBER(key_type2_r);
	DECLARE_WRITE8_MEMBER(key_type2_w);
	DECLARE_READ8_MEMBER(key_type3_r);
	DECLARE_WRITE8_MEMBER(key_type3_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void driver_init() override;

	void TilemapCB(uint16_t code, int *tile, int *mask);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	void main_map(address_map &map);
	void mcu_map(address_map &map);
	void sound_map(address_map &map);
	void sub_map(address_map &map);
	void virtual_map(address_map &map);
};

#endif // MAME_INCLUDES_NAMCOS1_H
