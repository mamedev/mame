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
	required_ioport m_dswx;

	required_device<mos6551_device> m_acia;

	required_region_ptr<uint8_t> m_rom;

private:
	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );
};

class apricorn_ssi_device : public a2bus_ssc_device
{
public:
	// construction/destruction
	apricorn_ssi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;

private:
	bool m_alt_bank;
};

// device type declarations
DECLARE_DEVICE_TYPE(A2BUS_SSC, a2bus_ssc_device)
DECLARE_DEVICE_TYPE(APRICORN_SSI, apricorn_ssi_device)

#endif // MAME_BUS_A2BUS_A2SSC_H
