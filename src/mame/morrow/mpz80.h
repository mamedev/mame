// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_MORROW_MPZ80_H
#define MAME_MORROW_MPZ80_H

#pragma once

#include "bus/s100/s100.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"

#define Z80_TAG         "17a"
#define AM9512_TAG      "17d"
#define S100_TAG        "s100"

class mpz80_state : public driver_device
{
public:
	mpz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_ram(*this, RAM_TAG),
			m_s100(*this, S100_TAG),
			m_rom(*this, Z80_TAG),
			m_map_ram(*this, "map_ram", 0x200, ENDIANNESS_LITTLE),
			m_16c(*this, "16C"),
			m_nmi(1),
			m_pint(1),
			m_int_pend(0),
			m_pretrap(0),
			m_trap(0),
			m_trap_reset(0),
			m_trap_void(1),
			m_trap_halt(1),
			m_trap_int(1),
			m_trap_stop(1),
			m_trap_aux(1)
	{ }

	void mpz80(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<s100_bus_device> m_s100;
	required_memory_region m_rom;
	memory_share_creator<uint8_t> m_map_ram;
	required_ioport m_16c;

	inline offs_t get_address(offs_t offset);
	inline offs_t get_io_address(offs_t offset);

	inline void check_traps();
	inline void check_interrupt();

	uint8_t mmu_r(offs_t offset);
	void mmu_w(offs_t offset, uint8_t data);
	uint8_t mmu_io_r(offs_t offset);
	void mmu_io_w(offs_t offset, uint8_t data);
	uint8_t trap_addr_r();
	uint8_t keyboard_r();
	uint8_t switch_r();
	uint8_t status_r();
	void disp_seg_w(uint8_t data);
	void disp_col_w(uint8_t data);
	void task_w(uint8_t data);
	void mask_w(uint8_t data);
	void s100_pint_w(int state);
	void s100_nmi_w(int state);

	// memory state
	uint32_t m_addr = 0;
	uint8_t m_task = 0;
	uint8_t m_mask = 0;

	// interrupt state
	int m_nmi;
	int m_pint;
	int m_int_pend;

	// trap state
	uint8_t m_pretrap_addr;
	uint8_t m_trap_addr;
	uint8_t m_status;
	uint16_t m_trap_start;
	int m_pretrap;
	int m_trap;
	int m_trap_reset;
	int m_trap_void;
	int m_trap_halt;
	int m_trap_int;
	int m_trap_stop;
	int m_trap_aux;
	void init_mpz80();
	void mpz80_io(address_map &map) ATTR_COLD;
	void mpz80_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_MORROW_MPZ80_H
