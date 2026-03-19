// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_ATLUS_CAVE_H
#define MAME_ATLUS_CAVE_H

#pragma once

/***************************************************************************

    Cave hardware

***************************************************************************/

#include "sprite013.h"

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "video/tmap038.h"

#include "emupal.h"
#include "screen.h"

class cave_state : public driver_device
{
public:
	cave_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki%u", 1)
		, m_int_timer(*this, "int_timer")
		, m_eeprom(*this, "eeprom")
		, m_hopper(*this, "hopper")
		, m_gfxdecode(*this, "gfxdecode.%u", 0U)
		, m_screen(*this, "screen.%u", 0U)
		, m_palette(*this, "palette.%u", 0U)
		, m_spritegen(*this, "spritegen.%u", 0U)
		, m_tilemap(*this, "tilemap.%u", 0U)
		, m_led_outputs(*this, "led%u", 0U)
		, m_spriteram(*this, "spriteram.%u", 0)
		, m_io_in0(*this, "IN0")
		, m_io_bet(*this, "BET")
		, m_spriteregion(*this, "sprites%u", 0)
		, m_tileregion(*this, "layer%u", 0)
		, m_okiregion(*this, "oki%u", 1)
	{ }

	int paccarn_bet4_r();
	int paccarn_bet8_r();

	void init_ddonpach() ATTR_COLD;
	void init_donpachi() ATTR_COLD;
	void init_esprade() ATTR_COLD;
	void init_feversos() ATTR_COLD;
	void init_guwange() ATTR_COLD;
	void init_korokoro() ATTR_COLD;
	void init_tjumpman() ATTR_COLD;
	void init_uopoko() ATTR_COLD;

	void cellage(machine_config &config) ATTR_COLD;
	void crusherm(machine_config &config) ATTR_COLD;
	void ddonpach(machine_config &config) ATTR_COLD;
	void dfeveron(machine_config &config) ATTR_COLD;
	void donpachi(machine_config &config) ATTR_COLD;
	void esprade(machine_config &config) ATTR_COLD;
	void gaia(machine_config &config) ATTR_COLD;
	void guwange(machine_config &config) ATTR_COLD;
	void jumbogod(machine_config &config) ATTR_COLD;
	void korokoro(machine_config &config) ATTR_COLD;
	void paccarn(machine_config &config) ATTR_COLD;
	void paceight(machine_config &config) ATTR_COLD;
	void pacslot(machine_config &config) ATTR_COLD;
	void tekkenbs(machine_config &config) ATTR_COLD;
	void tekkencw(machine_config &config) ATTR_COLD;
	void tjumpman(machine_config &config) ATTR_COLD;
	void uopoko(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device_array<okim6295_device, 2> m_oki;
	required_device<timer_device> m_int_timer;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<ticket_dispenser_device> m_hopper;
	optional_device_array<gfxdecode_device, 4> m_gfxdecode;
	optional_device_array<screen_device, 4> m_screen;
	optional_device_array<palette_device, 4> m_palette;
	optional_device_array<sprite013_device, 4> m_spritegen;
	optional_device_array<tilemap038_device, 4> m_tilemap;
	output_finder<9> m_led_outputs;

	// memory pointers
	optional_shared_ptr_array<u16, 4> m_spriteram;

	optional_ioport m_io_in0;
	optional_ioport m_io_bet;

	// memory regions
	optional_memory_region_array<4> m_spriteregion;
	optional_memory_region_array<4> m_tileregion;
	optional_memory_region_array<2> m_okiregion;

	s32       m_layers_offs_x = 0;
	s32       m_layers_offs_y = 0;
	s32       m_row_effect_offs_n = 0;
	s32       m_row_effect_offs_f = 0;
	u16       m_background_pen[4]{};

	int       m_kludge = 0;
	emu_timer *m_vblank_end_timer = nullptr;

	// misc
	int       m_time_vblank_irq = 0;
	u8        m_irq_level = 0U;
	u8        m_vblank_irq = 0U;
	u8        m_sound_irq = 0U;
	u8        m_unknown_irq = 0U;
	u8        m_agallet_vblank_irq = 0U;

	int       m_rasflag = 0;
	int       m_old_rasflag = 0;
	u16       m_leds[2]{};

	void add_base_config(machine_config &config, int layer) ATTR_COLD;
	void add_ymz(machine_config &config) ATTR_COLD;
	void namco_em_common(machine_config &config) ATTR_COLD;

	u16 irq_cause_r(offs_t offset);
	void gaia_coin_w(u8 data);
	u16 donpachi_videoregs_r(offs_t offset);
	void jumbogod_leds_w(u8 data);
	void korokoro_leds_w(offs_t offset, u16 data, u16 mem_mask);
	void tjumpman_leds_w(u8 data);
	void pacslot_leds_w(u8 data);
	void eeprom_w(u8 data);
	void guwange_eeprom_w(u8 data);
	void korokoro_eeprom_w(offs_t offset, u16 data, u16 mem_mask);
	void tjumpman_eeprom_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(vblank_end);
	TIMER_DEVICE_CALLBACK_MEMBER(vblank_start);
	TIMER_DEVICE_CALLBACK_MEMBER(vblank_start_left);
	TIMER_DEVICE_CALLBACK_MEMBER(vblank_start_right);

	void sound_irq_gen(int state);
	void update_irq_state();

	bool colpri_cb(u8 &dstpri, u32 &colpri);

	inline void tilemap_draw(int chip, screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u32 priority, u32 priority2, int layer);
	void init_cave() ATTR_COLD;
	void show_leds();

	void cellage_map(address_map &map) ATTR_COLD;
	void crusherm_map(address_map &map) ATTR_COLD;
	void ddonpach_map(address_map &map) ATTR_COLD;
	void dfeveron_map(address_map &map) ATTR_COLD;
	void donpachi_map(address_map &map) ATTR_COLD;
	void esprade_map(address_map &map) ATTR_COLD;
	void gaia_map(address_map &map) ATTR_COLD;
	void guwange_map(address_map &map) ATTR_COLD;
	void jumbogod_map(address_map &map) ATTR_COLD;
	void korokoro_map(address_map &map) ATTR_COLD;
	void paccarn_map(address_map &map) ATTR_COLD;
	void paceight_map(address_map &map) ATTR_COLD;
	void pacslot_map(address_map &map) ATTR_COLD;
	void tekkenbs_map(address_map &map) ATTR_COLD;
	void tekkencw_map(address_map &map) ATTR_COLD;
	void tjumpman_map(address_map &map) ATTR_COLD;
	void uopoko_map(address_map &map) ATTR_COLD;

	void nmk112_oki0_map(address_map &map) ATTR_COLD;
	void nmk112_oki1_map(address_map &map) ATTR_COLD;
};

// with sound Z80
class cave_z80_state : public cave_state
{
public:
	cave_z80_state(const machine_config &mconfig, device_type type, const char *tag)
		: cave_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_startup(*this, "startup")
		, m_z80region(*this, "audiocpu")
		, m_z80bank(*this, "z80bank")
		, m_okibank_lo(*this, "oki%u_banklo", 1)
		, m_okibank_hi(*this, "oki%u_bankhi", 1)
	{}

	void init_agallet() ATTR_COLD;
	void init_hotdogst() ATTR_COLD;
	void init_mazinger() ATTR_COLD;
	void init_metmqstr() ATTR_COLD;
	void init_pwrinst2() ATTR_COLD;
	void init_pwrinst2a() ATTR_COLD;
	void init_sailormn() ATTR_COLD;

	void agallet(machine_config &config) ATTR_COLD;
	void hotdogst(machine_config &config) ATTR_COLD;
	void mazinger(machine_config &config) ATTR_COLD;
	void metmqstr(machine_config &config) ATTR_COLD;
	void pwrinst2(machine_config &config) ATTR_COLD;
	void sailormn(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_16_device> m_soundlatch;
	optional_device<timer_device> m_startup;
	required_memory_region        m_z80region;
	required_memory_bank          m_z80bank;
	optional_memory_bank_array<2> m_okibank_lo;
	optional_memory_bank_array<2> m_okibank_hi;

	// sound related
	u8        m_soundbuf_wptr = 0;
	u8        m_soundbuf_rptr = 0;
	u8        m_soundbuf_data[32]{};
	bool      m_soundbuf_empty = false;
	//u8      m_sound_flag[2]{};

	// sailormn
	u8        m_sailormn_tilebank = 0;

	u8 soundflags_r();
	u16 soundflags_ack_r();
	void sound_cmd_w(u16 data);
	u8 soundlatch_lo_r();
	u8 soundlatch_hi_r();
	u16 soundlatch_ack_r();
	void soundlatch_ack_w(u8 data);

	template<int Chip> void pwrinst2_vctrl_w(offs_t offset, u16 data, u16 mem_mask);
	u16 sailormn_input0_r();
	template<int Mask> void z80_rombank_w(u8 data);
	template<unsigned Chip, int Mask> void oki_bank_w(u8 data);
	void sailormn_eeprom_w(u8 data);
	void hotdogst_eeprom_w(u8 data);
	void metmqstr_eeprom_w(u8 data);
	u16 pwrinst2_eeprom_r();
	void sailormn_get_banked_code(bool tiledim, u32 &color, u32 &pri, u32 &code);
	DECLARE_MACHINE_RESET(sailormn);
	TIMER_DEVICE_CALLBACK_MEMBER(sailormn_startup);
	void sailormn_tilebank_w(int bank);
	void sailormn_unpack_tiles(int chip);
	void init_z80_bank();
	void init_oki_bank(int chip);

	bool pwrinst2_colpri_cb(u8 &dstpri, u32 &colpri);

	void mazinger_decrypt_cb();
	void pwrinst2_decrypt_cb();
	void sailormn_decrypt_cb();

	void hotdogst_map(address_map &map) ATTR_COLD;
	void hotdogst_sound_map(address_map &map) ATTR_COLD;
	void hotdogst_sound_portmap(address_map &map) ATTR_COLD;
	void mazinger_map(address_map &map) ATTR_COLD;
	void mazinger_sound_map(address_map &map) ATTR_COLD;
	void mazinger_sound_portmap(address_map &map) ATTR_COLD;
	void metmqstr_map(address_map &map) ATTR_COLD;
	void metmqstr_sound_portmap(address_map &map) ATTR_COLD;
	template<unsigned Chip> void oki_map(address_map &map) ATTR_COLD;
	void pwrinst2_map(address_map &map) ATTR_COLD;
	void pwrinst2_sound_map(address_map &map) ATTR_COLD;
	void pwrinst2_sound_portmap(address_map &map) ATTR_COLD;
	void sailormn_map(address_map &map) ATTR_COLD;
	void sailormn_sound_map(address_map &map) ATTR_COLD;
	void sailormn_sound_portmap(address_map &map) ATTR_COLD;
};

// with 3 screens
class ppsatan_state : public cave_state
{
public:
	ppsatan_state(const machine_config &mconfig, device_type type, const char *tag)
		: cave_state(mconfig, type, tag)
		, m_int_timer_left(*this, "int_timer_left")
		, m_int_timer_right(*this, "int_timer_right")
		, m_touch_x(*this, "TOUCH%u_X", 1U)
		, m_touch_y(*this, "TOUCH%u_Y", 1U)
	{}

	void ppsatan(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<timer_device> m_int_timer_left;
	required_device<timer_device> m_int_timer_right;
	required_ioport_array<2> m_touch_x;
	required_ioport_array<2> m_touch_y;

	// ppsatan
	u16 m_ppsatan_io_mux = 0U;

	void ppsatan_eeprom_w(offs_t offset, u16 data, u16 mem_mask);
	void ppsatan_io_mux_w(offs_t offset, u16 data, u16 mem_mask);
	template<int Player> u16 ppsatan_touch_r();
	void ppsatan_out_w(offs_t offset, u16 data, u16 mem_mask);

	TIMER_DEVICE_CALLBACK_MEMBER(timer_lev2_cb);
	INTERRUPT_GEN_MEMBER(interrupt_ppsatan);
	u32 screen_update_ppsatan_core (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int chip);
	u32 screen_update_ppsatan_top  (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_ppsatan_left (screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_ppsatan_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ppsatan_map(address_map &map) ATTR_COLD;
};

#endif // MAME_ATLUS_CAVE_H
