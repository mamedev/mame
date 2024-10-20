// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ReCo6502

    http://www.zeridajh.org/hardware/reco6502/index.htm

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_RC6502_H
#define MAME_BUS_BBC_TUBE_RC6502_H

#include "tube.h"
#include "cpu/m6502/w65c02s.h"
#include "cpu/g65816/g65816.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_rc6502_device

class bbc_tube_rc6502_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_rc6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_tube_rc6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void add_common_devices(machine_config &config);

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

	uint8_t config_r();
	void register_w(uint8_t data);

	void tube_rc6502_bank(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_ioport m_config;

	void prst_w(int state);

private:
	void tube_rc6502_mem(address_map &map) ATTR_COLD;

	uint8_t m_default;
	uint8_t m_divider;
	uint8_t m_banking;
	uint8_t m_banknum;
};


class bbc_tube_rc65816_device : public bbc_tube_rc6502_device
{
public:
	bbc_tube_rc65816_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void tube_rc65816_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_RC6502, bbc_tube_rc6502_device)
DECLARE_DEVICE_TYPE(BBC_TUBE_RC65816, bbc_tube_rc65816_device)


#endif /* MAME_BUS_BBC_TUBE_RC6502_H */
