// license:BSD-3-Clause
// copyright-holders:David Haywood,AJR
/***************************************************************************

    Kawasaki LSI
    KL5C80A12 CPU (KL5C80A12CFP on hng64.c)

    Binary compatible with Z80, significantly faster opcode timings, operating at up to 10Mhz
    Timers / Counters, Parallel / Serial ports/ MMU, Interrupt Controller

    (is this different enough to need it's own core?)
    (todo: everything, some code currently lives in machine/hng64_net.c but not much)

***************************************************************************/

#ifndef MAME_CPU_Z80_KL5C80A12_H
#define MAME_CPU_Z80_KL5C80A12_H

#pragma once

#include "z80.h"
#include "machine/z80ctc.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class kl5c80a12_device : public z80_device
{
public:
	enum
	{
		KC82_B1 = Z80_WZ + 1, KC82_B2, KC82_B3, KC82_B4,
		KC82_A1, KC82_A2, KC82_A3
	};

	kl5c80a12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address) override;

	// z80_device overrides
	virtual u8 rm(u16 addr) override;
	virtual void wm(u16 addr, u8 value) override;
	virtual u8 rop() override;
	virtual u8 arg() override;
	virtual u16 arg16() override;

private:
	void internal_io(address_map &map);
	void mmu_remap_pages();
	u8 mmu_r(offs_t offset);
	void mmu_w(offs_t offset, u8 data);

	// address spaces
	address_space_config m_program_config;
	address_space_config m_opcodes_config;
	address_space_config m_io_config;

	// MMU registers
	u16 m_mmu_a[4];
	u8 m_mmu_b[5];
	u32 m_mmu_base[0x40];
};


// device type definition
DECLARE_DEVICE_TYPE(KL5C80A12, kl5c80a12_device)

#endif // MAME_CPU_Z80_KL5C80A12_H
