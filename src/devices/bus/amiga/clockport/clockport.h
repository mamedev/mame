// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Amiga 1200 clock port

    Pins 19-40 of the P9B header:

    19  Ground          20  +5VDC
    21  /INT6           22  /SPARE_CS
    23  /RTC_CS         24  N/C
    25  /IORD           26  /IOWR
    27  A5              28  A4
    29  A3              30  A2
    31  D23             32  D22
    33  D21             34  D20
    35  D19             36  D18
    37  D17             38  D16
    39  Ground          40  /RESET

    /SPARE_CS decodes $D80000-$D8FFFF and /RTC_CS decodes $DC0000-$DCFFFF.
    A2-A5 select one of 16 registers on the D16-D23 byte lane.

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CLOCKPORT_CLOCKPORT_H
#define MAME_BUS_AMIGA_CLOCKPORT_CLOCKPORT_H

#pragma once


class device_amiga_clockport_interface;

// ======================> amiga_clockport_device

class amiga_clockport_device : public device_t, public device_single_card_slot_interface<device_amiga_clockport_interface>
{
public:
	template <typename T>
	amiga_clockport_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: amiga_clockport_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	amiga_clockport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~amiga_clockport_device();

	auto int6_handler() { return m_int6_handler.bind(); }

	// called from host
	uint8_t spare_r(offs_t offset);
	void spare_w(offs_t offset, uint8_t data);
	uint8_t rtc_r(offs_t offset);
	void rtc_w(offs_t offset, uint8_t data);
	bool has_card() const { return m_card != nullptr; }

	// called from card
	void int6_w(int state) { m_int6_handler(state); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	device_amiga_clockport_interface *m_card;

	devcb_write_line m_int6_handler;
};

// ======================> device_amiga_clockport_interface

class device_amiga_clockport_interface : public device_interface
{
public:
	virtual ~device_amiga_clockport_interface();

	virtual uint8_t spare_r(offs_t offset);
	virtual void spare_w(offs_t offset, uint8_t data);
	virtual uint8_t rtc_r(offs_t offset);
	virtual void rtc_w(offs_t offset, uint8_t data);

protected:
	device_amiga_clockport_interface(const machine_config &mconfig, device_t &device);

	amiga_clockport_device *m_host;
};

// device type definition
DECLARE_DEVICE_TYPE(AMIGA_CLOCKPORT, amiga_clockport_device)

#endif // MAME_BUS_AMIGA_CLOCKPORT_CLOCKPORT_H
