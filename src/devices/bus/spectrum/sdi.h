// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    SDI Interface

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_SDI_H
#define MAME_BUS_SPECTRUM_SDI_H

#include "exp.h"
#include "softlist.h"
#include "machine/i8255.h"
#include "bus/cbmiec/cbmiec.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_sdi_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_sdi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual bool romcs() override;

	required_memory_region m_rom;
	required_device<i8255_device> m_ppi;
	required_device<cbm_iec_device> m_iec;

	void ppic_w(uint8_t data);
	uint8_t ppic_r();

	bool m_romcs;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_SDI, spectrum_sdi_device)


#endif // MAME_BUS_SPECTRUM_SDI_H
