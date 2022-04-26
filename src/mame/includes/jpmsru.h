// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

    JPM Stepper Reel Unit

**********************************************************************/
#ifndef MAME_INCLUDES_JPMSRU_H
#define MAME_INCLUDES_JPMSRU_H

#pragma once

#include "machine/timer.h"
#include "machine/steppers.h"
#include "machine/genfruit.h"
#include "machine/nvram.h"
#include "machine/netlist.h"
#include "netlist/nl_setup.h"

class jpmsru_state : public genfruit_class
{
public:
	jpmsru_state(const machine_config &mconfig, device_type type, const char *tag)
		: genfruit_class(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_inputs(*this, "IN%u", 0U),
			m_reel(*this, "reel%u", 0U),
			m_lamp(*this, "lamp%u", 0U),
			m_digits(*this, "digit%u", 0U),
			m_audio_in1(*this, "nl_audio:in1"),
			m_audio_in2(*this, "nl_audio:in2"),
			m_audio_in3(*this, "nl_audio:in3"),
			m_audio_in4(*this, "nl_audio:in4"),
			m_audio_in5(*this, "nl_audio:in5"),
			m_audio_in6(*this, "nl_audio:in6"),
			m_nvram(*this, "nvram", 0x80, ENDIANNESS_BIG),
			m_dips(*this, "DIP%u", 0U)
	{ }

	void jpmsru_3k(machine_config &config);
	void jpmsru_3k_busext(machine_config &config);
	void jpmsru_4k(machine_config &config);
	void ewn(machine_config &config);
	void ewn2(machine_config &config);
	void ndu(machine_config &config);
	void dud(machine_config &config);
	void lan(machine_config &config);

	void init_jpmsru();
	
	template <unsigned N> DECLARE_READ_LINE_MEMBER(opto_r) { return m_opto[N]; }
protected:
	virtual void machine_start() override;

private:
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(opto_cb) { m_opto[N] = state; }

	uint8_t inputs_r(offs_t offset);
	void reel_w(offs_t offset, uint8_t data);
	void update_int();
	void audio_w(offs_t offset, uint8_t data);
	void int1_en_w(offs_t offset, uint8_t data);
	void int2_en_w(offs_t offset, uint8_t data);
	uint8_t busext_data_r(offs_t offset);
	void busext_data_w(offs_t offset, uint8_t data);
	void busext_bdir_w(offs_t offset, uint8_t data);
	void busext_mode_w(offs_t offset, uint8_t data);
	void busext_addr_w(offs_t offset, uint8_t data);
	uint8_t busext_dips_r(offs_t offset);
	void out_lamp_w(offs_t offset, uint8_t data);
	void out_lamp_ext_w(offs_t offset, uint8_t data);
	void out_disp_w(offs_t offset, uint8_t data);
	void out_payout_cash_w(offs_t offset, uint8_t data);
	void out_payout_token_w(offs_t offset, uint8_t data);
	template<unsigned meter> void out_meter_w(offs_t offset, uint8_t data);
	void out_coin_lockout_w(offs_t offset, uint8_t data);

	void jpmsru_3k_map(address_map &map);
	void jpmsru_4k_map(address_map &map);
	void jpmsru_io(address_map &map);
	void jpmsru_busext_io(address_map &map);
	void outputs_ewn(address_map &map);
	void outputs_ewn2(address_map &map);
	void outputs_ndu(address_map &map);
	void outputs_dud(address_map &map);
	void outputs_lan(address_map &map);
	
	bool m_int1;
	bool m_int2;
	bool m_int1_en;
	bool m_int2_en;
	int m_reelbits[4];
	bool m_opto[4];
	int m_disp_digit;
	bool m_disp_d1;
	bool m_disp_d2;
	bool m_busext_bdir;
	uint8_t m_busext_mode;
	uint8_t m_busext_addr;
	
	TIMER_DEVICE_CALLBACK_MEMBER(int1);
	TIMER_DEVICE_CALLBACK_MEMBER(int2);

	// devices
	required_device<cpu_device> m_maincpu;
	required_ioport_array<3> m_inputs;
	required_device_array<stepper_device, 4> m_reel;
	output_finder<56> m_lamp;
	output_finder<2> m_digits;
	required_device<netlist_mame_logic_input_device> m_audio_in1;
	required_device<netlist_mame_logic_input_device> m_audio_in2;
	required_device<netlist_mame_logic_input_device> m_audio_in3;
	required_device<netlist_mame_logic_input_device> m_audio_in4;
	required_device<netlist_mame_logic_input_device> m_audio_in5;
	required_device<netlist_mame_logic_input_device> m_audio_in6;
	
	memory_share_creator<uint8_t> m_nvram;
	optional_ioport_array<3> m_dips;
};

#endif // MAME_INCLUDES_JPMSRU_H