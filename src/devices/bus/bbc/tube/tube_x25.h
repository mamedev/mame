// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Econet X25 Gateway

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_EconetX25.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_X25_H
#define MAME_BUS_BBC_TUBE_X25_H

#include "tube.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_x25_device

class bbc_tube_x25_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_x25_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::LAN; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	IRQ_CALLBACK_MEMBER( irq_callback );

	required_device_array<z80_device, 2> m_z80;
	required_device<tube_device> m_ula;
	required_memory_bank_array<2> m_bank;
	std::unique_ptr<uint8_t[]> m_ram;

	void primary_mem(address_map &map) ATTR_COLD;
	void primary_io(address_map &map) ATTR_COLD;
	void secondary_mem(address_map &map) ATTR_COLD;
	void secondary_io(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_X25, bbc_tube_x25_device)


#endif /* MAME_BUS_BBC_TUBE_X25_H */
