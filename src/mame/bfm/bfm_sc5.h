// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BFM_BFM_SC5_H
#define MAME_BFM_BFM_SC5_H

#pragma once

#include "bfm_sc4.h"
#include "cpu/m68000/mcf5206e.h"

class bfm_sc5_state : public bfm_sc45_state
{
public:
	bfm_sc5_state(const machine_config &mconfig, device_type type, const char *tag)
		: bfm_sc45_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void init_sc5();

	void bfm_sc5(machine_config &config);

protected:
	void sc5_map(address_map &map) ATTR_COLD;

	required_device<m68000_base_device> m_maincpu;

	uint8_t sc5_10202F0_r(offs_t offset);
	void sc5_10202F0_w(offs_t offset, uint8_t data);
	void sc5_duart_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t sc5_mux1_r(offs_t offset);
	void sc5_mux1_w(offs_t offset, uint8_t data);
	void sc5_mux2_w(offs_t offset, uint8_t data);

	void bfm_sc5_duart_irq_handler(int state);
	void bfm_sc5_duart_txa(int state);
	uint8_t bfm_sc5_duart_input_r();
	void bfm_sc5_duart_output_w(uint8_t data);
};

INPUT_PORTS_EXTERN( bfm_sc5 );

#endif // MAME_BFM_BFM_SC5_H
