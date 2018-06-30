// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2ssc.h

    Apple II Super Serial Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2SSC_H
#define MAME_BUS_A2BUS_A2SSC_H

#include "a2bus.h"
#include "machine/mos6551.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ssc_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ssc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	required_ioport m_dsw1, m_dsw2;

	required_device<mos6551_device> m_acia;

private:
	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );

	uint8_t *m_rom;
	bool m_started;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SSC, a2bus_ssc_device)

#endif // MAME_BUS_A2BUS_A2SSC_H
