// license:BSD-3-Clause
// copyright-holders:Howie Cohen, Yochizo
// thanks-to:Richard Bush
#include "includes/seta.h"

class taitox_state : public seta_state
{
public:
	taitox_state(const machine_config &mconfig, device_type type, const char *tag)
		: seta_state(mconfig, type, tag) { }

	uint16_t superman_dsw_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t daisenpu_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void daisenpu_input_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void kyustrkr_input_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_kyustrkr();
	void machine_start_taitox();
	void machine_start_superman();

	// superman c-chip
	uint16_t m_current_bank;
	uint8_t m_cc_port;
	uint16_t cchip1_ctrl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t cchip1_ram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cchip1_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cchip1_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cchip1_ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
};
