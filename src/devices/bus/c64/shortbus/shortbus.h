// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    IDE64 ShortBus

     1  Ground          2  +5VDC
     3  /RESET          4  CSEL4
     5  R/W             6  /CSEL3
     7  PHI2            8  /CSEL2
     9  BA             10  /CSEL1
    11  DOT clock      12  /CSEL0
    13  /NMI           14  /IO2
    15  /IRQ           16  /IO1
    17  D7             18  A7
    19  D6             20  A6
    21  D5             22  A5
    23  D4             24  A4
    25  D3             26  A3
    27  D2             28  A2
    29  D1             30  A1
    31  D0             32  A0
    33  +5VDC          34  Ground

    CSEL0 $DE00-$DE0F   CSEL1 $DE10-$DE1F   CSEL2 $DE38-$DE47
    CSEL3 $DE48-$DE57   CSEL4 $DE58-$DE59 (active high)

    CSEL2 and CSEL3 are not available on IDE64 v4.2. JP2 on v4.1 swaps CSEL0 and CSEL1.

***************************************************************************/

#ifndef MAME_BUS_C64_SHORTBUS_SHORTBUS_H
#define MAME_BUS_C64_SHORTBUS_SHORTBUS_H

#pragma once


class device_ide64_shortbus_interface;

// ======================> ide64_shortbus_device

class ide64_shortbus_device : public device_t, public device_single_card_slot_interface<device_ide64_shortbus_interface>
{
public:
	enum
	{
		CSEL0 = 1 << 0,
		CSEL1 = 1 << 1,
		CSEL2 = 1 << 2,
		CSEL3 = 1 << 3,
		CSEL4 = 1 << 4
	};

	template <typename T>
	ide64_shortbus_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: ide64_shortbus_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	ide64_shortbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~ide64_shortbus_device();

	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	// called from host, io1 and io2 are active low and csel is the mask of asserted CSELn signals
	uint8_t cd_r(offs_t offset, uint8_t data, int io1, int io2, int csel);
	void cd_w(offs_t offset, uint8_t data, int io1, int io2, int csel);

	// called from card
	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_ide64_shortbus_interface *m_card;

	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
};

// ======================> device_ide64_shortbus_interface

class device_ide64_shortbus_interface : public device_interface
{
public:
	virtual ~device_ide64_shortbus_interface();

	virtual uint8_t shortbus_r(offs_t offset, uint8_t data, int io1, int io2, int csel);
	virtual void shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel);

protected:
	device_ide64_shortbus_interface(const machine_config &mconfig, device_t &device);

	ide64_shortbus_device *m_host;
};

// device type definition
DECLARE_DEVICE_TYPE(IDE64_SHORTBUS, ide64_shortbus_device)

#endif // MAME_BUS_C64_SHORTBUS_SHORTBUS_H
