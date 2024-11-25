// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
#ifndef MAME_SEGA_STV_H
#define MAME_SEGA_STV_H

#pragma once

#include "saturn.h"

#include "segabill.h"

#include "rax.h"

#include "machine/eepromser.h"
#include "machine/ticket.h"

class stv_state : public saturn_state
{
public:
	stv_state(const machine_config &mconfig, device_type type, const char *tag) :
		saturn_state(mconfig, type, tag),
		m_cart1(*this, "stv_slot1"),
		m_cart2(*this, "stv_slot2"),
		m_cart3(*this, "stv_slot3"),
		m_cart4(*this, "stv_slot4"),
		m_rax(*this, "rax"),
		m_protbank(*this, "protbank"),
		m_eeprom(*this, "eeprom"),
		m_cryptdevice(*this, "315_5881"),
		m_5838crypt(*this, "315_5838"),
		m_hopper(*this, "hopper"),
		m_billboard(*this, "billboard"),
		m_ioga_ports(*this, "PORT%c", 'A'),
		m_ioga_counters(*this, "PORTG.%u", 0),
		m_ioga_mahjong{ { *this, "P1_KEY%u", 0 }, { *this, "P2_KEY%u", 0 } },
		m_pdr(*this, "PDR%u", 1),
		m_cc_digits(*this, "cc_digit%u", 0U)
	{
	}

	void stv_slot(machine_config &config);
	void stv_cartslot(machine_config &config);
	void stv(machine_config &config);
	void critcrsh(machine_config &config);
	void magzun(machine_config &config);
	void stvmp(machine_config &config);
	void hopper(machine_config &config);
	void batmanfr(machine_config &config);
	void shienryu(machine_config &config);
	void stv_5838(machine_config &config);
	void stv_5881(machine_config &config);
	void stvcd(machine_config &config);

	void init_astrass();
	void init_batmanfr();
	void init_finlarch();
	void init_decathlt();
	void init_decathlt_nokey();
	void init_sanjeon();
	void init_puyosun();
	void init_winterht();
	void init_gaxeduel();
	void init_rsgun();
	void init_groovef();
	void init_sandor();
	void init_cottonbm();
	void init_smleague();
	void init_nameclv3();
	void init_danchiq();
	void init_hanagumi();
	void init_cotton2();
	void init_seabass();
	void init_stv();
	void init_thunt();
	void init_sasissu();
	void init_dnmtdeka();
	void init_ffreveng();
	void init_fhboxers();
	void init_pblbeach();
	void init_sss();
	void init_diehard();
	void init_danchih();
	void init_shienryu();
	void init_elandore();
	void init_prikura();
	void init_maruchan();
	void init_colmns97();
	void init_grdforce();
	void init_suikoenb();
	void init_magzun();
	void init_shanhigw();
	void init_sokyugrt();
	void init_vfremix();
	void init_twcup98();
	void init_znpwfv();
	void init_othellos();
	void init_mausuke();
	void init_stv_us();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t stv_ioga_r(offs_t offset);
	void stv_ioga_w(offs_t offset, uint8_t data);
	uint8_t critcrsh_ioga_r(offs_t offset);
	void critcrsh_ioga_w(offs_t offset, uint8_t data);
	uint8_t magzun_ioga_r(offs_t offset);
	void magzun_ioga_w(offs_t offset, uint8_t data);
	uint8_t stvmp_ioga_r(offs_t offset);
	void stvmp_ioga_w(offs_t offset, uint8_t data);
	uint32_t magzun_hef_hack_r();
	uint32_t magzun_rx_hack_r();
	void hop_ioga_w(offs_t offset, uint8_t data);

	std::pair<std::error_condition, std::string> load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart1 ) { return load_cart(image, m_cart1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart2 ) { return load_cart(image, m_cart2); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart3 ) { return load_cart(image, m_cart3); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( stv_cart4 ) { return load_cart(image, m_cart4); }
	optional_device<generic_slot_device> m_cart1;
	optional_device<generic_slot_device> m_cart2;
	optional_device<generic_slot_device> m_cart3;
	optional_device<generic_slot_device> m_cart4;

	void install_stvbios_speedups( void );

	void batmanfr_sound_comms_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	optional_device<acclaim_rax_device> m_rax;

	uint8_t     m_port_sel,m_mux_data = 0;
	uint8_t     m_system_output = 0;
	uint8_t     m_ioga_mode = 0;
	uint8_t     m_ioga_portg = 0;
	uint16_t    m_ioga_count[4]{};
	uint16_t    m_serial_tx = 0;

	// protection specific variables and functions
	uint32_t m_abus_protenable = 0;
	uint32_t m_abus_protkey = 0;

	uint32_t decathlt_prot_r(offs_t offset, uint32_t mem_mask = ~0);
	void sega5838_map(address_map &map) ATTR_COLD;
	optional_memory_bank m_protbank;
	bool m_newprotection_element; // debug helper only, doesn't need saving
	int m_protbankval; // debug helper only, doesn't need saving
	void decathlt_prot_srcaddr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t m_a_bus[4]{};

	uint32_t common_prot_r(offs_t offset);
	void common_prot_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void install_common_protection();
	void stv_register_protection_savestates();

	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<sega_315_5881_crypt_device> m_cryptdevice;
	optional_device<sega_315_5838_comp_device> m_5838crypt;
	optional_device<ticket_dispenser_device> m_hopper;
	required_device<sega_billboard_device> m_billboard;
	optional_ioport_array<7> m_ioga_ports;
	required_ioport_array<4> m_ioga_counters;
	optional_ioport_array<5> m_ioga_mahjong[2];
	required_ioport_array<2> m_pdr;
	output_finder<2> m_cc_digits;
	uint16_t crypt_read_callback(uint32_t addr);

	uint8_t pdr1_input_r();
	uint8_t pdr2_input_r();
	void pdr1_output_w(uint8_t data);
	void pdr2_output_w(uint8_t data);
	void stv_select_game(int gameno);
	uint8_t     m_prev_gamebank_select = 0;

	void sound_mem(address_map &map) ATTR_COLD;
	void scsp_mem(address_map &map) ATTR_COLD;
	void stv_mem(address_map &map) ATTR_COLD;
	void critcrsh_mem(address_map &map) ATTR_COLD;
	void magzun_mem(address_map &map) ATTR_COLD;
	void stvmp_mem(address_map &map) ATTR_COLD;
	void hopper_mem(address_map &map) ATTR_COLD;
	void stvcd_mem(address_map &map) ATTR_COLD;
};

class stvpc_state : public stv_state
{
public:
	using stv_state::stv_state;
	static constexpr feature_type unemulated_features() { return feature::CAMERA | feature::PRINTER; }
};

//#define MASTER_CLOCK_352 57272720
//#define MASTER_CLOCK_320 53693174
#define CEF_1   m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1   m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0   m_vdp1_regs[0x010/2]&=~0x0001
#define STV_VDP1_TVMR ((m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)

extern gfx_decode_entry const gfx_stv[];

#endif // MAME_SEGA_STV_H
