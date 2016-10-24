// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc4.h"

class bfm_sc5_state : public bfm_sc45_state
{
public:
	bfm_sc5_state(const machine_config &mconfig, device_type type, const char *tag)
		: bfm_sc45_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

protected:


public:
	required_device<m68000_base_device> m_maincpu;

	void init_sc5();
	uint8_t sc5_10202F0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sc5_10202F0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sc5_duart_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint8_t sc5_mux1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sc5_mux1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sc5_mux2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void bfm_sc5_duart_irq_handler(int state);
	void bfm_sc5_duart_txa(int state);
	uint8_t bfm_sc5_duart_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bfm_sc5_duart_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};
