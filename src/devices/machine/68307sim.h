// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 SIM module */
#ifndef MAME_MACHINE_68307SIM_H
#define MAME_MACHINE_68307SIM_H

#pragma once

#include "68307.h"


class m68307_cpu_device::m68307_sim
{
public:
	uint16_t m_pacnt; // 8-bit
	uint16_t m_paddr; // 8-bit
	uint16_t m_padat; // 8-bit

	uint16_t m_pbcnt;
	uint16_t m_pbddr;
	uint16_t m_pbdat;

	uint16_t m_pivr; // 8-bit

	uint16_t m_br[4];
	uint16_t m_or[4];
	uint16_t m_picr;
	uint16_t m_licr1;
	uint16_t m_licr2;

	void write_pacnt(uint16_t data, uint16_t mem_mask);
	void write_paddr(uint16_t data, uint16_t mem_mask);
	uint16_t read_padat(m68307_cpu_device* m68k, address_space &space, uint16_t mem_mask);
	void write_padat(m68307_cpu_device* m68k, address_space &space, uint16_t data, uint16_t mem_mask);

	void write_pbcnt(uint16_t data, uint16_t mem_mask);
	void write_pbddr(uint16_t data, uint16_t mem_mask);
	uint16_t read_pbdat(m68307_cpu_device* m68k, address_space &space, uint16_t mem_mask);
	void write_pbdat(m68307_cpu_device* m68k, address_space &space, uint16_t data, uint16_t mem_mask);

	void write_licr1(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask);
	void write_licr2(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask);
	void write_picr(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask);
	void write_pivr(m68307_cpu_device* m68k, uint16_t data, uint16_t mem_mask);

	uint8_t get_ipl(const m68307_cpu_device *m68k) const;
	uint8_t get_int_type(const m68307_cpu_device* m68k, uint8_t pri) const;

	void reset();
};

#endif // MAME_MACHINE_68307SIM_H
