// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion 3-Link RS232 Serial Interface / Parallel Printer Interface

**********************************************************************/

#ifndef MAME_BUS_PSION_SIBO_3LINK_H
#define MAME_BUS_PSION_SIBO_3LINK_H

#include "slot.h"
#include "machine/output_latch.h"
#include "machine/psion_asic5.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_3link_serial_device

class psion_3link_serial_device : public device_t, public device_psion_sibo_interface
{
public:
	// construction/destruction
	psion_3link_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t data_r() override { return m_asic5->data_r(); }
	virtual void data_w(uint16_t data) override { m_asic5->data_w(data); }

private:
	required_region_ptr<uint8_t> m_rom;
	required_device<psion_asic5_device> m_asic5;
	required_device<rs232_port_device> m_rs232;

	uint32_t m_addr_latch;
};


// ======================> psion_3link_parallel_device

class psion_3link_parallel_device : public device_t, public device_psion_sibo_interface
{
public:
	// construction/destruction
	psion_3link_parallel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t data_r() override { return m_asic5->data_r(); }
	virtual void data_w(uint16_t data) override { m_asic5->data_w(data); }

private:
	required_device<psion_asic5_device> m_asic5;
	required_device<output_latch_device> m_cent_ctrl_out;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_3LINK_SERIAL, psion_3link_serial_device)
DECLARE_DEVICE_TYPE(PSION_3LINK_PARALLEL, psion_3link_parallel_device)


#endif // MAME_BUS_PSION_SIBO_3LINK_H
