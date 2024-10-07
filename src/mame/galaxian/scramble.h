// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_GALAXIAN_SCRAMBLE_H
#define MAME_GALAXIAN_SCRAMBLE_H

#pragma once

#include "galaxold.h"

#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "sound/digitalk.h"
#include "sound/flt_rc.h"
#include "sound/tms5110.h"
#include "emupal.h"

class scramble_state : public galaxold_state
{
public:
	scramble_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaxold_state(mconfig, type, tag),
		m_konami_7474(*this, "konami_7474"),
		m_ppi8255_0(*this, "ppi8255_0"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_tmsprom(*this, "tmsprom"),
		m_soundram(*this, "soundram"),
		m_digitalker(*this, "digitalker"),
		m_soundlatch(*this, "soundlatch"),
		m_dial(*this, "DIAL"),
		m_cavelon_bank_object(*this, "cavelon_bank")
	{
	}

	optional_device<ttl7474_device> m_konami_7474;
	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	optional_device<tmsprom_device>  m_tmsprom;
	optional_shared_ptr<uint8_t> m_soundram;
	optional_device<digitalker_device> m_digitalker;
	optional_device<generic_latch_8_device> m_soundlatch;

	optional_ioport m_dial;

	memory_bank_creator m_cavelon_bank_object;

	ioport_value darkplnt_dial_r();
	uint8_t hncholms_prot_r();
	uint8_t scramble_soundram_r(offs_t offset);
	uint8_t mars_ppi8255_0_r(offs_t offset);
	uint8_t mars_ppi8255_1_r(offs_t offset);
	void scramble_soundram_w(offs_t offset, uint8_t data);
	uint8_t scramble_portB_r();
	uint8_t hustler_portB_r();
	void hotshock_sh_irqtrigger_w(uint8_t data);
	uint8_t hotshock_soundlatch_r();
	void mars_ppi8255_0_w(offs_t offset, uint8_t data);
	void mars_ppi8255_1_w(offs_t offset, uint8_t data);
	void ad2083_tms5110_ctrl_w(uint8_t data);

	// harem
	void harem_decrypt_bit_w(uint8_t data);
	void harem_decrypt_clk_w(uint8_t data);
	void harem_decrypt_rst_w(uint8_t data);
	uint8_t harem_digitalker_intr_r();
	void harem_digitalker_control_w(uint8_t data);

	void init_cavelon();
	void init_mariner();
	void init_scramble_ppi();
	void init_mars();
	void init_hotshock();
	void init_ad2083();
	void init_devilfsh();
	void init_mrkougar();
	void init_harem();
	void init_newsin7a();

	void init_scobra();
	void init_stratgyx();
	void init_tazmani2();
	void init_tazmaniet();
	void init_darkplnt();
	void init_rescue();
	void init_minefld();
	void init_hustler();
	void init_hustlerd();
	void init_billiard();
	DECLARE_MACHINE_RESET(scramble);
	DECLARE_MACHINE_RESET(explorer);
	void scramble_sh_7474_q_callback(int state);
	uint8_t mariner_protection_1_r();
	uint8_t mariner_protection_2_r();
	uint8_t triplep_pip_r();
	uint8_t triplep_pap_r();
	uint8_t cavelon_banksw_r(offs_t offset);
	void cavelon_banksw_w(offs_t offset, uint8_t data);
	uint8_t hunchbks_mirror_r(address_space &space, offs_t offset);
	void hunchbks_mirror_w(address_space &space, offs_t offset, uint8_t data);
	void scramble_sh_irqtrigger_w(uint8_t data);
	void mrkougar_sh_irqtrigger_w(uint8_t data);
	IRQ_CALLBACK_MEMBER( scramble_sh_irq_callback );

	void scramble(machine_config &config);
	void hncholms(machine_config &config);
	void cavelon(machine_config &config);
	void harem(machine_config &config);
	void ad2083(machine_config &config);
	void ad2083_audio(machine_config &config);
	void mrkougar(machine_config &config);
	void mars(machine_config &config);
	void hunchbks(machine_config &config);
	void hotshock(machine_config &config);
	void mariner(machine_config &config);
	void devilfsh(machine_config &config);
	void triplep(machine_config &config);
	void newsin7(machine_config &config);
	void mrkougb(machine_config &config);
	void ad2083_map(address_map &map) ATTR_COLD;
	void ad2083_sound_io_map(address_map &map) ATTR_COLD;
	void ad2083_sound_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void harem_map(address_map &map) ATTR_COLD;
	void harem_sound_io_map(address_map &map) ATTR_COLD;
	void harem_sound_map(address_map &map) ATTR_COLD;
	void hotshock_map(address_map &map) ATTR_COLD;
	void hotshock_sound_io_map(address_map &map) ATTR_COLD;
	void hunchbks_map(address_map &map) ATTR_COLD;
	void hunchbks_readport(address_map &map) ATTR_COLD;
	void mars_map(address_map &map) ATTR_COLD;
	void mrkougar_map(address_map &map) ATTR_COLD;
	void newsin7_map(address_map &map) ATTR_COLD;
	void scramble_map(address_map &map) ATTR_COLD;
	void scramble_sound_io_map(address_map &map) ATTR_COLD;
	void scramble_sound_map(address_map &map) ATTR_COLD;
	void triplep_io_map(address_map &map) ATTR_COLD;
	void triplep_map(address_map &map) ATTR_COLD;

private:
	void cavelon_banksw();
	inline int bit(int i,int n);
	void sh_init();

	uint8_t m_cavelon_bank = 0;

	// harem
	uint8_t m_harem_decrypt_mode = 0;
	uint8_t m_harem_decrypt_bit = 0;
	uint8_t m_harem_decrypt_clk = 0;
	uint8_t m_harem_decrypt_count = 0;
	std::unique_ptr<uint8_t[]> m_harem_decrypted_data;
	std::unique_ptr<uint8_t[]> m_harem_decrypted_opcodes;
};

#endif // MAME_GALAXIAN_SCRAMBLE_H
