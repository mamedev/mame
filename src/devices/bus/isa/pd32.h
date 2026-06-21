// license:BSD-3-Clause
// copyright-holders:Dave Rand
/*********************************************************************

    PD32 public-domain NS32016 coprocessor board (1986)

    Dave Rand and George Scolaro's open-design 32016 coprocessor:
    NS32016 + NS32081 FPU + NS32082 MMU + NS32202 ICU, 2MB DRAM and
    a 2x27256 boot EPROM, attached to the host through a one-byte
    74LS646 parallel latch with hardware wait-stating on both sides.
    Hosts the ZAIAZ port of AT&T UNIX System V Release 2, with the
    host PC serving console, disk and floppy I/O over a request/
    completion-block protocol.  Published in Micro Cornucopia #32
    (October-November 1986).

    Modelled from the released PAL equations (lhudec32/lhu646), the
    boot ROM source (rom.a32 v1.7) and the host driver sources.

*********************************************************************/

#ifndef MAME_BUS_ISA_PD32_H
#define MAME_BUS_ISA_PD32_H

#pragma once

#include "isa.h"

#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"

#include <deque>

class isa8_pd32_device : public device_t, public device_isa8_card_interface, public device_memory_interface
{
public:
	static constexpr flags_type emulation_flags() { return flags::SAVE_UNSUPPORTED; }

	isa8_pd32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	void cpu_map(address_map &map) ATTR_COLD;

	// host side (ports 170h-176h, even addresses)
	uint8_t host_r(offs_t offset);
	void host_w(offs_t offset, uint8_t data);

	// 32016 side: the top 2MB region (PARIO / INT86 / ICU by A8-A9)
	uint8_t top_r(offs_t offset);
	void top_w(offs_t offset, uint8_t data);

	void icu_map(address_map &map) ATTR_COLD;
	void cpu_iam_map(address_map &map) ATTR_COLD;
	void cpu_eim_map(address_map &map) ATTR_COLD;
	uint8_t icu_r(offs_t offset);
	void icu_w(offs_t offset, uint8_t data);
	void icu_int_w(int state);
	TIMER_CALLBACK_MEMBER(swap_jump);
	void set_swap(bool swap);
	void update_reset();
	virtual void remap(int space_id, offs_t start, offs_t end) override;
	void set_sreq(bool state);

	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;
	required_memory_region m_eprom;
	address_space_config m_icu_config;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_icu_space;

	memory_view m_lowmem;
	memory_view m_himem;

	memory_share_creator<uint8_t> m_ram;

	// the 74LS646 latch, modelled as queues (see implementation notes)
	std::deque<uint8_t> m_to_pd32;
	std::deque<uint8_t> m_to_host;

	uint8_t m_pdir;         // last ICU port-direction write (1 = input)
	uint8_t m_pdat;         // ICU port-data shadow (resets high)
	bool m_swap;            // EPROM at 0 (true at power-on; cleared via ICU G1)
	bool m_swap_staged;
	uint32_t m_swap_entry;
	bool m_rsti;            // latched host software reset
	bool m_sreq;            // PD32 requests host service
	uint8_t m_pair_even;
	bool m_pair_valid;
	bool m_wpair_valid;
};

DECLARE_DEVICE_TYPE(ISA8_PD32, isa8_pd32_device)

#endif // MAME_BUS_ISA_PD32_H
