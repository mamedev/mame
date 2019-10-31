// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

        ZX Spectrum Expansion Port emulation

**********************************************************************

    Pinout:

        A15   1  A14
        A13   2  A12
         D7   3  +5v
        ¬OE   4  NC
       SLOT   5  SLOT
         D0   6  0v
         D1   7  0v
         D2   8  ¬CK
         D6   9  A0
         D5  10  A1
         D3  11  A2
         D4  12  A3
       ¬INT  13  NC
       ¬NMI  14  0v
      ¬HALT  15  ¬OE
      ¬MREQ  16  NC
      ¬IORQ  17  NC
        ¬RD  18  NC
        ¬WR  19  ¬BUSRQ
         NC  20  ¬RESET
      ¬WAIT  21  A7
       +12v  22  A6
         NC  23  A5
        ¬M1  24  A4
      ¬RFSH  25  ¬ROMCS
         A8  26  ¬BUSACK
        A10  27  A9
         NC  28  A11

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_EXP_H
#define MAME_BUS_SPECTRUM_EXP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_expansion_slot_device

class device_spectrum_expansion_interface;

class spectrum_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_spectrum_expansion_interface>
{
public:
	// construction/destruction
	template <typename T>
	spectrum_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: spectrum_expansion_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	spectrum_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	void pre_opcode_fetch(offs_t offset);
	void post_opcode_fetch(offs_t offset);
	void pre_data_fetch(offs_t offset);
	void post_data_fetch(offs_t offset);

	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);
	DECLARE_READ_LINE_MEMBER( romcs );

	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_irq_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_nmi_handler(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	device_spectrum_expansion_interface *m_card;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
};


// ======================> device_spectrum_expansion_interface

class device_spectrum_expansion_interface : public device_interface
{
public:
	// reading and writing
	virtual void pre_opcode_fetch(offs_t offset) { };
	virtual void post_opcode_fetch(offs_t offset) { };
	virtual void pre_data_fetch(offs_t offset) { };
	virtual void post_data_fetch(offs_t offset) { };
	virtual uint8_t mreq_r(offs_t offset) { return 0xff; }
	virtual void mreq_w(offs_t offset, uint8_t data) { }
	virtual uint8_t iorq_r(offs_t offset) { return 0xff; }
	virtual void iorq_w(offs_t offset, uint8_t data) { }
	virtual DECLARE_READ_LINE_MEMBER(romcs) { return 0; }

protected:
	// construction/destruction
	device_spectrum_expansion_interface(const machine_config &mconfig, device_t &device);

	spectrum_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_EXPANSION_SLOT, spectrum_expansion_slot_device)

void spectrum_expansion_devices(device_slot_interface &device);
void spec128_expansion_devices(device_slot_interface &device);
void specpls3_expansion_devices(device_slot_interface &device);


#endif // MAME_BUS_SPECTRUM_EXP_H
