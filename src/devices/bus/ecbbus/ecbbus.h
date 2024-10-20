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
//  TYPE DEFINITIONS
//**************************************************************************

class device_ecbbus_card_interface;
class ecbbus_device;


// ======================> ecbbus_slot_device

class ecbbus_slot_device : public device_t, public device_single_card_slot_interface<device_ecbbus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	ecbbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bustag, int num, U &&opts, char const *dflt)
		: ecbbus_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_ecbbus_slot(std::forward<T>(bustag), num);
	}

	ecbbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// inline configuration
	template <typename T> void set_ecbbus_slot(T &&tag, int num)
	{
		m_bus.set_tag(std::forward<T>(tag));
		m_bus_num = num;
	}

private:
	// configuration
	required_device<ecbbus_device> m_bus;
	int m_bus_num;
};


// device type definition
DECLARE_DEVICE_TYPE(ECBBUS_SLOT, ecbbus_slot_device)


// ======================> ecbbus_device

class ecbbus_device : public device_t
{
public:
	// construction/destruction
	ecbbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto nmi_wr_callback() { return m_write_nmi.bind(); }

	void add_card(device_ecbbus_card_interface &card, int pos);

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	void irq_w(int state) { m_write_irq(state); }
	void nmi_w(int state) { m_write_nmi(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

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
class device_ecbbus_card_interface : public device_interface
{
	friend class ecbbus_device;

public:
	// optional operation overrides
	virtual uint8_t ecbbus_mem_r(offs_t offset) { return 0; }
	virtual void ecbbus_mem_w(offs_t offset, uint8_t data) { }
	virtual uint8_t ecbbus_io_r(offs_t offset) { return 0; }
	virtual void ecbbus_io_w(offs_t offset, uint8_t data) { }

protected:
	// construction/destruction
	device_ecbbus_card_interface(const machine_config &mconfig, device_t &device);

	ecbbus_slot_device  *m_slot;
};


void ecbbus_cards(device_slot_interface &device);



#endif // MAME_BUS_ECBBUS_ECBBUS_H
