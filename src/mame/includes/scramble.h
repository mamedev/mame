// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_SCRAMBLE_H
#define MAME_INCLUDES_SCRAMBLE_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "includes/galaxold.h"
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
		m_dial(*this, "DIAL")
	{
	}

	optional_device<ttl7474_device> m_konami_7474;
	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	optional_device<tmsprom_device>  m_tmsprom;
	optional_shared_ptr<uint8_t> m_soundram;
	optional_device<digitalker_device> m_digitalker;
	required_device<generic_latch_8_device> m_soundlatch;

	optional_ioport m_dial;

	DECLARE_CUSTOM_INPUT_MEMBER(darkplnt_dial_r);
	template <int Mask> DECLARE_READ_LINE_MEMBER(ckongs_coinage_r);
	DECLARE_READ8_MEMBER(hncholms_prot_r);
	DECLARE_READ8_MEMBER(scramble_soundram_r);
	DECLARE_READ8_MEMBER(mars_ppi8255_0_r);
	DECLARE_READ8_MEMBER(mars_ppi8255_1_r);
	DECLARE_WRITE8_MEMBER(scramble_soundram_w);
	DECLARE_READ8_MEMBER(scramble_portB_r);
	DECLARE_READ8_MEMBER(hustler_portB_r);
	DECLARE_WRITE8_MEMBER(hotshock_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(hotshock_soundlatch_r);
	DECLARE_WRITE8_MEMBER(mars_ppi8255_0_w);
	DECLARE_WRITE8_MEMBER(mars_ppi8255_1_w);
	DECLARE_WRITE8_MEMBER(ad2083_tms5110_ctrl_w);

	// harem
	DECLARE_WRITE8_MEMBER(harem_decrypt_bit_w);
	DECLARE_WRITE8_MEMBER(harem_decrypt_clk_w);
	DECLARE_WRITE8_MEMBER(harem_decrypt_rst_w);
	DECLARE_READ8_MEMBER(harem_digitalker_intr_r);
	DECLARE_WRITE8_MEMBER(harem_digitalker_control_w);

	void init_cavelon();
	void init_mariner();
	void init_scramble_ppi();
	void init_mars();
	void init_mimonscr();
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
	void init_mimonkey();
	void init_mimonsco();
	void init_rescue();
	void init_minefld();
	void init_hustler();
	void init_hustlerd();
	void init_billiard();
	DECLARE_MACHINE_RESET(scramble);
	DECLARE_MACHINE_RESET(explorer);
	DECLARE_WRITE_LINE_MEMBER(scramble_sh_7474_q_callback);
	DECLARE_READ8_MEMBER( mariner_protection_1_r );
	DECLARE_READ8_MEMBER( mariner_protection_2_r );
	DECLARE_READ8_MEMBER( triplep_pip_r );
	DECLARE_READ8_MEMBER( triplep_pap_r );
	DECLARE_READ8_MEMBER( cavelon_banksw_r );
	DECLARE_WRITE8_MEMBER( cavelon_banksw_w );
	DECLARE_READ8_MEMBER( hunchbks_mirror_r );
	DECLARE_WRITE8_MEMBER( hunchbks_mirror_w );
	DECLARE_WRITE8_MEMBER( scramble_sh_irqtrigger_w );
	DECLARE_WRITE8_MEMBER( mrkougar_sh_irqtrigger_w );
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
	void mimonscr(machine_config &config);
	void ckongs(machine_config &config);
	void mrkougb(machine_config &config);
	void ad2083_map(address_map &map);
	void ad2083_sound_io_map(address_map &map);
	void ad2083_sound_map(address_map &map);
	void ckongs_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void harem_map(address_map &map);
	void harem_sound_io_map(address_map &map);
	void harem_sound_map(address_map &map);
	void hotshock_map(address_map &map);
	void hotshock_sound_io_map(address_map &map);
	void hunchbks_map(address_map &map);
	void hunchbks_readport(address_map &map);
	void mars_map(address_map &map);
	void mimonscr_map(address_map &map);
	void mrkougar_map(address_map &map);
	void newsin7_map(address_map &map);
	void scramble_map(address_map &map);
	void scramble_sound_io_map(address_map &map);
	void scramble_sound_map(address_map &map);
	void triplep_io_map(address_map &map);
	void triplep_map(address_map &map);

private:
	void cavelon_banksw();
	inline int bit(int i,int n);
	void sh_init();

	uint8_t m_cavelon_bank;

	// harem
	uint8_t m_harem_decrypt_mode;
	uint8_t m_harem_decrypt_bit;
	uint8_t m_harem_decrypt_clk;
	uint8_t m_harem_decrypt_count;
	std::unique_ptr<uint8_t[]> m_harem_decrypted_data;
	std::unique_ptr<uint8_t[]> m_harem_decrypted_opcodes;
};

#endif // MAME_INCLUDES_SCRAMBLE_H
