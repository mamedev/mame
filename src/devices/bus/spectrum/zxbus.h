// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*******************************************************************************************

        ZXBUS device

                ┌──┬───┬──┐
                │ B│   │ A│
                │  │   │  │
    *5      A15 │ 1│   │ 1│ A14      *4
    *3      A13 │ 2│   │ 2│ A12      *2
    *13      D7 │ 3│   │ 3│ +5V  (guaranteed +5V only here ! )
     BLK (CSDS) │ 4│   │ 4│ DOS (DCDOS)
     -not-used- │ 5│   │ 5│ KAY = 14Mhz  / SCORPION = +12V
    *14      D0 │ 6│   │ 6│ GND
    *15      D1 │ 7│   │ 7│ GND
    *12      D2 │ 8│   │ 8│ CLK Z80 (SCORPION aka /RAS 3.5Mhz ONLY ! )
    *10      D6 │ 9│   │ 9│ A0      *30
    *9       D5 │10│   │10│ A1      *31
    *8       D3 │11│   │11│ A2
    *7       D4 │12│   │12│ A3
              - │13│   │13│ IOGE  (IORQCE)
    *17     NMI │14│   │14│ GND
              - │15│   │15│ RDR(RDROM)   (CSROMCE)
    *19    MREQ │16│   │16│ RS (BIT_4 OF #7FFD)
    *20    IORQ │17│   │17│ -not-used-
    *21      RD │18│   │18│ -not-used-
    *22      WR │19│   │19│ BUSRQ
     -not-used- │20│   │20│ RES     *26
    *24    WAIT │21│   │21│ A7      *37
     -not-used- │22│   │22│ A6      *36
     -not-used- │23│   │23│ A5      *35
    *27      M1 │24│   │24│ A4
          RFRSH │25│   │25│ CSR  (CSROM)
    *38      A8 │26│   │26│ BUSAK
    *40     A10 │27│   │27│ A9      *39
  KAY=+5V  n.u. │28│   │28│ A11     *1
  KAY=+12V n.u. │29│   │29│ n.u. KAY=+5V
            GND │30│   │30│ GND
                │  │   │  │
                │ B│   │A │
                └──┴───┴──┘
  '-' - not used
  '*' - Z80 CPU out

*******************************************************************************************/

#ifndef MAME_BUS_SPECTRUM_ZXBUS_H
#define MAME_BUS_SPECTRUM_ZXBUS_H

#pragma once

class zxbus_device;
class device_zxbus_card_interface;

class zxbus_slot_device : public device_t, public device_single_card_slot_interface<device_zxbus_card_interface>
{
public:
	zxbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T, typename U>
	zxbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, T &&zxbus_tag, U &&slot_options, const char *dflt)
		: zxbus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_zxbus_bus.set_tag(std::forward<T>(zxbus_tag));
	}

protected:
	zxbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

	required_device<zxbus_device> m_zxbus_bus;
};

DECLARE_DEVICE_TYPE(ZXBUS_SLOT, zxbus_slot_device)


class zxbus_device : public device_t
{
public:
	zxbus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }
	template<typename T> void install_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(class address_map &map), u64 unitmask = ~u64(0))
	{
		m_iospace->install_device(addrstart, addrend, device, map, unitmask);
	}

	void add_slot(zxbus_slot_device &slot);

protected:
	zxbus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_iospace;

	std::forward_list<zxbus_slot_device *> m_slot_list;
};

DECLARE_DEVICE_TYPE(ZXBUS, zxbus_device)


class device_zxbus_card_interface : public device_interface
{
	friend class zxbus_slot_device;

protected:
	device_zxbus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	void set_zxbusbus(zxbus_device &zxbus) { assert(!device().started()); m_zxbus = &zxbus; }

	zxbus_device *m_zxbus;
};


void zxbus_cards(device_slot_interface &device);


#endif // MAME_BUS_SPECTRUM_ZXBUS_H
