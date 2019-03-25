// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Kontron Europe Card Bus emulation

**********************************************************************

                              A  B  C
                +5 V    ---   *  1  *   --- +5V
                D5      ---   *  2  *   --- D0
                D6      ---   *  3  *   --- D7
                D3      ---   *  4  *   --- D2
                D4      ---   *  5  *   --- A0
                A2      ---   *  6  *   --- A3
                A4      ---   *  7  *   --- A1
                A5      ---   *  8  *   --- A8
                A6      ---   *  9  *   --- A7
                WAIT*   ---   * 10  *   --- D8
                BUSRQ*  ---   * 11  *   --- IEI
                BAI1    ---   * 12  *   --- D9
                +12 V   ---   * 13  *   --- D10
                D11     ---   * 14  *   --- D1
                -5 V    ---   * 15  *   --- -15 V
                phi2    ---   * 16  *   --- IEO
                BAO1    ---   * 17  *   --- A11
                A14     ---   * 18  *   --- A10
                +15 V   ---   * 19  *   --- D13
                M1*     ---   * 20  *   --- NMI*
                D14     ---   * 21  *   --- INT*
                D15     ---   * 22  *   --- WR*
                DPR*    ---   * 23  *   --- D12
                +5VBAT  ---   * 24  *   --- RD*
                phiN    ---   * 25  *   --- HALT*
                WRITE EN---   * 26  *   --- PWRRCL*
                IORQ*   ---   * 27  *   --- A12
                RFSH*   ---   * 28  *   --- A15
                A13     ---   * 29  *   --- PHI
                A9      ---   * 30  *   --- MREQ*
                BUSAK*  ---   * 31  *   --- RESET*
                GND     ---   * 32  *   --- GND

**********************************************************************/

#ifndef MAME_BUS_ECBBUS_ECBBUS_H
#define MAME_BUS_ECBBUS_ECBBUS_H

#pragma once



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define ECBBUS_TAG          "ecbbus"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ECBBUS_ADD() \
	MCFG_DEVICE_ADD(ECBBUS_TAG, ECBBUS, 0)
#define MCFG_ECBBUS_SLOT_ADD(_num, _tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, ECBBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	downcast<ecbbus_slot_device &>(*device).set_ecbbus_slot(ECBBUS_TAG, _num);


#define MCFG_ECBBUS_IRQ_CALLBACK(_write) \
	downcast<ecbbus_device &>(*device).set_irq_wr_callback(DEVCB_##_write);

#define MCFG_ECBBUS_NMI_CALLBACK(_write) \
	downcast<ecbbus_device &>(*device).set_nmi_wr_callback(DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ecbbus_slot_device

class ecbbus_device;

class ecbbus_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	ecbbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	void set_ecbbus_slot(const char *tag, int num) { m_bus_tag = tag; m_bus_num = num; }

private:
	// configuration
	const char *m_bus_tag;
	int m_bus_num;
	ecbbus_device  *m_bus;
};


// device type definition
DECLARE_DEVICE_TYPE(ECBBUS_SLOT, ecbbus_slot_device)


// ======================> ecbbus_interface

class device_ecbbus_card_interface;


// ======================> ecbbus_device

class ecbbus_device : public device_t
{
public:
	// construction/destruction
	ecbbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_irq_wr_callback(Object &&cb) { return m_write_irq.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_nmi_wr_callback(Object &&cb) { return m_write_nmi.set_callback(std::forward<Object>(cb)); }

	void add_card(device_ecbbus_card_interface *card, int pos);

	DECLARE_READ8_MEMBER( mem_r );
	DECLARE_WRITE8_MEMBER( mem_w );

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_write_nmi(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	static constexpr unsigned MAX_ECBBUS_SLOTS = 16;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_nmi;

	device_ecbbus_card_interface *m_ecbbus_device[MAX_ECBBUS_SLOTS];
};


// device type definition
DECLARE_DEVICE_TYPE(ECBBUS, ecbbus_device)


// ======================> device_ecbbus_card_interface

// class representing interface-specific live ecbbus card
class device_ecbbus_card_interface : public device_slot_card_interface
{
	friend class ecbbus_device;

public:
	// optional operation overrides
	virtual uint8_t ecbbus_mem_r(offs_t offset) { return 0; };
	virtual void ecbbus_mem_w(offs_t offset, uint8_t data) { };
	virtual uint8_t ecbbus_io_r(offs_t offset) { return 0; };
	virtual void ecbbus_io_w(offs_t offset, uint8_t data) { };

protected:
	// construction/destruction
	device_ecbbus_card_interface(const machine_config &mconfig, device_t &device);

	ecbbus_slot_device  *m_slot;
};


void ecbbus_cards(device_slot_interface &device);



#endif // MAME_BUS_ECBBUS_ECBBUS_H
