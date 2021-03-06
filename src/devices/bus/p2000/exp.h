// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 Expansion Port emulation

**********************************************************************/

#ifndef MAME_BUS_P2000_EXP_H
#define MAME_BUS_P2000_EXP_H

#pragma once

//**************************************************************************
//  CONSTANTS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> p2000_expansion_slot_device

class device_p2000_expansion_slot_card_interface;

class p2000_expansion_slot_device : 
    public device_t,
	public device_single_card_slot_interface<device_p2000_expansion_slot_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	p2000_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: p2000_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	p2000_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~p2000_expansion_slot_device() { }

    // inline configuration
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io_space.set_tag(std::forward<T>(tag), spacenum); }
    address_space &io_space() const { return *m_io_space; }    

	auto irq() { return m_write_irq.bind(); }
    auto in_mode80() { return m_in_mode80.bind(); }

	// computer interface
    uint8_t dew_r();
    uint8_t vidon_r();
    uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
    
	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
    DECLARE_READ_LINE_MEMBER( mode80_r ) { return m_in_mode80(); }

protected:
	// device-level overrides
	virtual void device_start() override;

    // internal state
	required_address_space m_io_space;

	devcb_write_line   m_write_irq;
    devcb_read_line    m_in_mode80;

	device_p2000_expansion_slot_card_interface *m_card;
};


// ======================> device_p2000_expansion_slot_card_interface

class device_p2000_expansion_slot_card_interface : public device_interface
{
	friend class p2000_expansion_slot_device;

protected:
	// construction/destruction
	device_p2000_expansion_slot_card_interface(const machine_config &mconfig, device_t &device);

	// runtime
    virtual uint8_t dew_r() { return 0; };
    virtual uint8_t vidon_r() { return 1; };
    virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; };

    p2000_expansion_slot_device *m_slot;

	std::unique_ptr<uint8_t[]> m_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(P2000_EXPANSION_SLOT, p2000_expansion_slot_device)

void p2000_slot2_devices(device_slot_interface &device);
void p2000_ext1_devices(device_slot_interface &device);
void p2000_ext2_devices(device_slot_interface &device);

#endif // MAME_BUS_P2000_EXP_H
