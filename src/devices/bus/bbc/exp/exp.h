// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Compact Expansion slot emulation

**********************************************************************

    Pin  Side A                   Side B

    1    SCREEN (0v)              SCREEN (0v)
    2    +5v                      +5v
    3    AT13                     A10
    4    NOT RST                  CD3
    5    AA15                     A11
    6    A8                       A9
    7    A13                      CD7
    8    A12                      CD6
    9    phi 2 OUT                CD5
    10   not connected            CD4
    11   not connected            LPTSTP
    12   B READ / NOT WRITE       BA7
    13   NOT NMI                  BA6
    14   NOT IRQ                  BA5
    15   NOT INFC                 BA4
    16   NOT INFD                 BA3
    17   AA14                     BA2
    18   NOT 8MHz                 BA1
    19   0v                       BA0
    20   PB7 (old user port)      CD0
    21   PB6 (old user port)      CD2
    22   PB5 (old user port)      CD1
    =========== POLARISATION SLOT ===========
    24   0v                       0v
    25   SCREEN (0v)              SCREEN (0v)

**********************************************************************/

#ifndef MAME_BUS_BBC_EXP_EXP_H
#define MAME_BUS_BBC_EXP_EXP_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_exp_slot_device

class device_bbc_exp_interface;

class bbc_exp_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_exp_interface>
{
public:
	// construction/destruction
	template <typename T>
	bbc_exp_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: bbc_exp_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	// callbacks for mertec device (also connects to joyport)
	auto cb1_handler() { return m_cb1_handler.bind(); }
	auto cb2_handler() { return m_cb2_handler.bind(); }

	uint8_t fred_r(offs_t offset);
	void fred_w(offs_t offset, uint8_t data);
	uint8_t jim_r(offs_t offset);
	void jim_w(offs_t offset, uint8_t data);
	uint8_t sheila_r(offs_t offset);
	void sheila_w(offs_t offset, uint8_t data);

	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

	// additional handlers for mertec device
	void cb1_w(int state) { m_cb1_handler(state); }
	void cb2_w(int state) { m_cb2_handler(state); }

	uint8_t pb_r();
	void pb_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_bbc_exp_interface *m_card;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;

	devcb_write_line m_cb1_handler;
	devcb_write_line m_cb2_handler;
};


// ======================> device_bbc_exp_interface

class device_bbc_exp_interface : public device_interface
{
public:
	virtual uint8_t fred_r(offs_t offset) { return 0xff; }
	virtual void fred_w(offs_t offset, uint8_t data) { }
	virtual uint8_t jim_r(offs_t offset) { return 0xff; }
	virtual void jim_w(offs_t offset, uint8_t data) { }
	virtual uint8_t sheila_r(offs_t offset) { return 0xfe; }
	virtual void sheila_w(offs_t offset, uint8_t data) { }

	virtual uint8_t pb_r() { return 0xff; }
	virtual void pb_w(uint8_t data) { }

protected:
	device_bbc_exp_interface(const machine_config &mconfig, device_t &device);

	bbc_exp_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_EXP_SLOT, bbc_exp_slot_device)

void bbc_exp_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_EXP_EXP_H
