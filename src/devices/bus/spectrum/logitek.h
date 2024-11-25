// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Proceed 1 Interface
    (c) 1984 Logitek

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_LOGITEK_H
#define MAME_BUS_SPECTRUM_LOGITEK_H

#include "exp.h"
#include "softlist.h"
#include "machine/z80pio.h"
#include "bus/cbmiec/cbmiec.h"
#include "bus/centronics/ctronics.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_proceed_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_proceed_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	DECLARE_INPUT_CHANGED_MEMBER(nmi_button) { m_slot->nmi_w(newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void fetch(offs_t offset);

	virtual void pre_opcode_fetch(offs_t offset) override { fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { fetch(offset); }
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual bool romcs() override;

	required_memory_region m_rom;
	required_device<z80pio_device> m_z80pio;
	required_device<cbm_iec_device> m_iec;
	required_device<centronics_device> m_centronics;

	void pioa_w(uint8_t data);
	void piob_w(uint8_t data);
	uint8_t piob_r();

	bool m_romcs;
	int m_romen;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_PROCEED, spectrum_proceed_device)


#endif // MAME_BUS_SPECTRUM_LOGITEK_H
