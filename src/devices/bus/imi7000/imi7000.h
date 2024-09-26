// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    International Memories Incorporated IMI 7000 Series bus emulation

**********************************************************************

                    GND       1      2       GND
                  SPARE       3      4       SPARE
       -SEL UNIT ADDR 3       5      6       -SEL UNIT ADDR 2
              +R/W DATA       7      8       -R/W DATA
       -SEL UNIT ADDR 1       9      10      -SEL UNIT ADDR 0
             +SYS CLOCK      11      12      -SYS CLOCK
                    GND      13      14      GND
                -SECTOR      15      16      -INDEX
         -SEEK COMPLETE      17      18      -FAULT
            -CMD STROBE      19      20      -CMD R/W
          -CMD SELECT 0      21      22      -CMD SELECT 1
                  SPARE      23      24      SPARE
               -CMD ACK      25      26      SPARE
             -CMD BUS 6      27      28      -CMD BUS 7
             -CMD BUS 4      29      30      -CMD BUS 5
             -CMD BUS 2      31      32      -CMD BUS 3
             -CMD BUS 0      33      34      -CMD BUS 1

**********************************************************************/

#ifndef MAME_BUS_IMI7000_IMI7000_H
#define MAME_BUS_IMI7000_IMI7000_H

#pragma once

void imi7000_devices(device_slot_interface &device);

DECLARE_DEVICE_TYPE(IMI7000_BUS,  imi7000_bus_device)
DECLARE_DEVICE_TYPE(IMI7000_SLOT, imi7000_slot_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_imi7000_interface;


// ======================> imi7000_slot_device

class imi7000_slot_device : public device_t, public device_single_card_slot_interface<device_imi7000_interface>
{
public:
	// construction/destruction
	template <typename T>
	imi7000_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: imi7000_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	imi7000_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_imi7000_interface *m_card;
};


// ======================> imi7000_bus_device

class imi7000_bus_device : public device_t
{
public:
	// construction/destruction
	imi7000_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T, typename U, typename V, typename W>
	void set_slot_default_options(T &&def1, U &&def2, V &&def3, W &&def4)
	{
		m_units[0].lookup()->set_default_option(std::forward<T>(def1));
		m_units[1].lookup()->set_default_option(std::forward<U>(def2));
		m_units[2].lookup()->set_default_option(std::forward<V>(def3));
		m_units[3].lookup()->set_default_option(std::forward<W>(def4));
	}

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	required_device_array<imi7000_slot_device, 4> m_units;
};


// ======================> device_imi7000_interface

class device_imi7000_interface : public device_interface
{
	friend class imi7000_slot_device;

protected:
	// construction/destruction
	device_imi7000_interface(const machine_config &mconfig, device_t &device);

	imi7000_slot_device *m_slot;
};

#endif // MAME_BUS_IMI7000_IMI7000_H
