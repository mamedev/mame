// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    REX Data Technology ProLogic-DOS Classic

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_PROLOGICDOS_H
#define MAME_BUS_CBMIEC_PROLOGICDOS_H

#pragma once

#include "c1541.h"
#include "machine/6821pia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_prologic_dos_classic_device

class c1541_prologic_dos_classic_device :  public c1541_device_base, public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1541_prologic_dos_classic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_c64_floppy_parallel_interface implementation
	virtual void parallel_data_w(u8 data) override;
	virtual void parallel_strobe_w(int state) override;
	virtual void parallel_select_w(int state) override;

private:
	required_device<pia6821_device> m_pia;
	memory_share_creator<uint8_t> m_ram;
	memory_share_creator<uint8_t> m_mb_ram;
	required_region_ptr<u8> m_rom;
	required_region_ptr<u8> m_mmu;
	required_region_ptr<u8> m_kernal;
	required_ioport m_jp5;

	void c1541pdc_mem(address_map &map) ATTR_COLD;

	void pia_pa_w(uint8_t data);
	void pia_pb_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	void pia_cb2_w(int state);
	void via0_pa_w(uint8_t data);
	void via0_ca2_w(int state);
	uint8_t mmu_r(offs_t offset);
	void mmu_w(offs_t offset, uint8_t data);
	void mmu_a5_w(int state) { m_mmu_a5 = state; }
	TIMER_CALLBACK_MEMBER(parallel_data_sync);
	TIMER_CALLBACK_MEMBER(parallel_strobe_sync);

	offs_t mmu_address(offs_t offset) const { return 0x1e00 | (m_mmu_a8 << 8) | (m_dd_select << 7) | (m_pd_select << 6) | (m_mmu_a5 << 5) | (offset >> 11); }
	static offs_t mainboard_address(offs_t offset, u8 mmu) { return (BIT(mmu, 6) << 15) | (offset & 0x67ff) | (BIT(mmu, 5) << 12) | (BIT(mmu, 4) << 11); }

	u8 m_mmu_a5;
	u8 m_mmu_a8;
	u8 m_dd_select;
	u8 m_pd_select;
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_PROLOGIC_DOS_CLASSIC, c1541_prologic_dos_classic_device)


#endif // MAME_BUS_CBMIEC_PROLOGICDOS_H
