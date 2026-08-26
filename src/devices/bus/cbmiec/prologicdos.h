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
#include "machine/output_latch.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_prologic_dos_classic_device

class c1541_prologic_dos_classic_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_prologic_dos_classic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pia6821_device> m_pia;
	required_device<output_latch_device> m_cent_data_out;
	required_memory_region m_mmu_rom;

	uint8_t pia_r(offs_t offset);
	void pia_w(offs_t offset, uint8_t data);
	void pia_pa_w(uint8_t data);
	uint8_t pia_pb_r();
	void pia_pb_w(uint8_t data);
	uint8_t read();
	void write(uint8_t data);

	void c1541pdc_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_PROLOGIC_DOS_CLASSIC, c1541_prologic_dos_classic_device)


#endif // MAME_BUS_CBMIEC_PROLOGICDOS_H
