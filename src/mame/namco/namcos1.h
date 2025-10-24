// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_NAMCO_NAMCOS1_H
#define MAME_NAMCO_NAMCOS1_H

#pragma once

#include "c117.h"
#include "namco_c116.h"
#include "namco_c123tmap.h"
#include "namcos1_sprite.h"

#include "cpu/m6800/m6801.h"
#include "cpu/m6809/m6809.h"
#include "machine/74157.h"
#include "sound/dac.h"
#include "sound/namco.h"

#include <utility>


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
		m_spritegen(*this, "spritegen"),
		m_c123tmap(*this, "c123tmap"),
		m_dac(*this, "dac%u", 0U),
		m_triram(*this, "triram"),
		m_rom(*this, "mainrom"),
		m_soundbank(*this, "soundbank"),
		m_mcubank(*this, "mcubank"),
		m_io_in(*this, "IN%u", 0U),
		m_io_control(*this, "CONTROL%u", 0U),
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
	void init_ws89();
	void init_dspirit();
	void init_pistoldm();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void driver_init();

	required_device<mc6809e_device> m_maincpu;
	required_device<mc6809e_device> m_subcpu;
	required_device<mc6809e_device> m_audiocpu;
	required_device<hd63701v0_cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	required_device<namco_c117_device> m_c117;
	required_device<namcos1_sprite_device> m_spritegen;
	required_device<namco_c123tmap_device> m_c123tmap;
	required_device_array<dac_8bit_r2r_device, 2> m_dac;

	required_shared_ptr<u8> m_triram;
	required_region_ptr<u8> m_rom;

	required_memory_bank m_soundbank;
	required_memory_bank m_mcubank;

	optional_ioport_array<4> m_io_in;
	optional_ioport_array<2> m_io_control;
	required_ioport m_io_dipsw;
	required_device<ls157_device> m_dsw_sel;

	int m_key_id = 0;
	int m_key_reg = 0;
	int m_key_rng = 0;
	int m_key_swap4_arg = 0;
	int m_key_swap4 = 0;
	int m_key_bottom4 = 0;
	int m_key_top4 = 0;
	u32 m_key_quotient = 0;
	u32 m_key_reminder = 0;
	u32 m_key_numerator_high_word = 0;
	u8 m_key[8]{};
	u8 m_mcu_patch_data = 0;
	s32 m_reset = 0;
	s32 m_input_count = 0;
	u8 m_strobe = 0;
	u8 m_strobe_count = 0;
	u8 m_stored_input[2]{};
	u8 m_drawmode_table[16]{};

	void subres_w(int state);
	void audiocpu_irq_ack_w(u8 data);
	void mcu_irq_ack_w(u8 data);
	u8 dsw_r(offs_t offset);
	void coin_w(offs_t offset, u8 data, u8 mem_mask);
	void dac_gain_w(u8 data);
	void sound_bankswitch_w(u8 data);
	void mcu_bankswitch_w(u8 data);
	void mcu_patch_w(u8 data);
	u8 berabohm_buttons_r(offs_t offset);
	u8 faceoff_inputs_r(offs_t offset);
	void _3dcs_w(offs_t offset, u8 data);
	u8 no_key_r(offs_t offset);
	void no_key_w(offs_t offset, u8 data);
	u8 key_type1_r(offs_t offset);
	void key_type1_w(offs_t offset, u8 data);
	u8 key_type2_r(offs_t offset);
	void key_type2_w(offs_t offset, u8 data);
	u8 key_type3_r(offs_t offset);
	void key_type3_w(offs_t offset, u8 data);

	void key_type_1_init(int key_id);
	void key_type_2_init(int key_id);
	void key_type_3_init(int key_id, int reg, int rng, int swap4_arg, int swap4, int bottom4, int top4);

	std::pair<bool, u8 const *> sprite_shadow_cb(u8 color);
	u32 sprite_pri_cb(u8 attr1, u8 attr2);
	u32 sprite_bank_cb(u32 code, u32 bank);
	void TilemapCB(u16 code, int &tile, int &mask);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void main_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void virtual_map(address_map &map) ATTR_COLD;
};

class quester_state : public namcos1_state
{
public:
	quester_state(const machine_config &mconfig, device_type type, const char *tag) :
		namcos1_state(mconfig, type, tag),
		m_io_paddle(*this, "PADDLE%u", 0U)
	{ }

	void quester(machine_config &config);

	void init_quester();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_ioport_array<2> m_io_paddle;

	u8 paddle_r(offs_t offset);

	void quester_mcu_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_NAMCOS1_H
