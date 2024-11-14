// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Enterprise Sixty Four / One Two Eight Expansion Bus emulation

**********************************************************************

             LH SOUND IN    B1      A1      RH SOUND IN
                     _WR    B2      A2      _RFSH
                   _IORQ    B3      A3      _RD
                   +17V*    B4      A4      +17V*
                    _NMI    B5      A5      _MREQ
                      A9    B6      A6      A8
                     A11    B7      A7      A10
                     A13    B8      A8      A12
                     A15    B9      A9      A14
                      A1    B10     A10     A0
                      A3    B11     A11     A2
                      A5    B12     A12     A4
                      A7    B13     A13     A6
                      D1    B14     A14     D0
                      D3    B15     A15     D2
                      D5    B16     A16     D4
                      D7    B17     A17     D6
                    _INT    B18     A18     _RESET
                     GND    B19     A19     _WAIT
                     GND    B20     A20     _M1
                     GND    B21     A21     1M
                     GND    B22     A22     phi
                     GND    B23     A23     8M
                     EC1    B24     A24     EC0
                     EC3    B25     A25     EC2
                     A16    B26     A26     _EXTC
                     A18    B27     A27     A17
                     A20    B28     A28     A19
                     14M    B29     A29     A21
                   VSYNC    B30     A30     _LOCATE
                    _EXP    B31     A31     GND
                     GND    B32     A32     HSYNC
                     +9V    B33     A33     +9V

**********************************************************************/

#ifndef MAME_BUS_EP64_EXP_H
#define MAME_BUS_EP64_EXP_H

#pragma once


void ep64_expansion_bus_cards(device_slot_interface &device);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ep64_expansion_bus_slot_device

class device_ep64_expansion_bus_card_interface;

class ep64_expansion_bus_slot_device : public device_t,
										public device_single_card_slot_interface<device_ep64_expansion_bus_card_interface>
{
	friend class device_ep64_expansion_bus_card_interface;

public:
	// construction/destruction
	ep64_expansion_bus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, char const *dflt)
		: ep64_expansion_bus_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		ep64_expansion_bus_cards(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	ep64_expansion_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io_space.set_tag(std::forward<T>(tag), spacenum); }
	auto irq_wr() { return m_write_irq.bind(); }
	auto nmi_wr() { return m_write_nmi.bind(); }
	auto wait_wr() { return m_write_wait.bind(); }

	void irq_w(int state) { m_write_irq(state); }
	void nmi_w(int state) { m_write_nmi(state); }
	void wait_w(int state) { m_write_wait(state); }

	address_space &program() { return *m_program_space; }
	address_space &io() { return *m_io_space; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_write_irq;
	devcb_write_line m_write_nmi;
	devcb_write_line m_write_wait;

	required_address_space m_program_space;
	required_address_space m_io_space;

	device_ep64_expansion_bus_card_interface *m_card;
};


// ======================> device_ep64_expansion_bus_card_interface

class device_ep64_expansion_bus_card_interface : public device_interface
{
protected:
	// construction/destruction
	device_ep64_expansion_bus_card_interface(const machine_config &mconfig, device_t &device);

	ep64_expansion_bus_slot_device  *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(EP64_EXPANSION_BUS_SLOT, ep64_expansion_bus_slot_device)

#endif // MAME_BUS_EP64_EXP_H
