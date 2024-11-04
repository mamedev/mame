// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Expansion Slot

       A          B

     -12V   32   +12V
      +5V   31   +5V
     DB0    30   DB1
     DB2    29   DB3
     DB4    28   DB5
     DB6    27   DB7
     AB10   26   AB9
     AB11   25   AB12
    /AMWC   24   /MRDC
    /DMA2   23   DT/R
    /DMA1   22   /IORC
    /MWTC   21   /RES
    /IOWC   20   /AIOWC
      GND   19   GND
    /CLK5   18   DEN
    /IRDY   17   /MRDY
    /EXT1   16   /EXT2
    /INT3   15   /ALE
      AB6   14   /INT2
      AB8   13   AB7
      DB9   12   DB8
     DB11   11   DB10
     DB13   10   DB12
     DB15    9   DB14
      AB2    8   AB1
      AB4    7   AB3
      AB0    6   AB5
     AB14    5   AB13
     AB15    4   AB16
     AB17    3   AB18
     AB19    2   /BHE
      NMI    1   CLK15

***************************************************************************/

#ifndef MAME_BUS_APRICOT_EXPANSION_EXPANSION_H
#define MAME_BUS_APRICOT_EXPANSION_EXPANSION_H

#pragma once

#include <functional>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_apricot_expansion_card_interface;


// ======================> apricot_expansion_slot_device

class apricot_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_apricot_expansion_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	apricot_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: apricot_expansion_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	apricot_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	apricot_expansion_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(APRICOT_EXPANSION_SLOT, apricot_expansion_slot_device)


// ======================> apricot_expansion_bus_device

class apricot_expansion_bus_device : public device_t
{
public:
	// construction/destruction
	apricot_expansion_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~apricot_expansion_bus_device();

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_program_iop_space(T &&tag, int spacenum) { m_program_iop.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_io_iop_space(T &&tag, int spacenum) { m_io_iop.set_tag(std::forward<T>(tag), spacenum); }

	auto dma1() { return m_dma1_handler.bind(); }
	auto dma2() { return m_dma2_handler.bind(); }
	auto ext1() { return m_ext1_handler.bind(); }
	auto ext2() { return m_ext2_handler.bind(); }
	auto int2() { return m_int2_handler.bind(); }
	auto int3() { return m_int3_handler.bind(); }

	void add_card(device_apricot_expansion_card_interface *card);

	// from cards
	void dma1_w(int state);
	void dma2_w(int state);
	void ext1_w(int state);
	void ext2_w(int state);
	void int2_w(int state);
	void int3_w(int state);

	void install_ram(offs_t addrstart, offs_t addrend, void *baseptr);

	template <typename T>
	void install_io_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(class address_map &map), uint64_t unitmask = ~u64(0))
	{
		m_io->install_device(addrstart, addrend, device, map, unitmask);

		if (m_io_iop)
			m_io_iop->install_device(addrstart, addrend, device, map, unitmask);
	}

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	using card_vector = std::vector<std::reference_wrapper<device_apricot_expansion_card_interface> >;

	card_vector m_dev;

	// address spaces we have access to
	required_address_space m_program;
	required_address_space m_io;
	optional_address_space m_program_iop;
	optional_address_space m_io_iop;

	devcb_write_line m_dma1_handler;
	devcb_write_line m_dma2_handler;
	devcb_write_line m_ext1_handler;
	devcb_write_line m_ext2_handler;
	devcb_write_line m_int2_handler;
	devcb_write_line m_int3_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(APRICOT_EXPANSION_BUS, apricot_expansion_bus_device)


// ======================> device_apricot_expansion_card_interface

class device_apricot_expansion_card_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_apricot_expansion_card_interface();

	void set_bus_device(apricot_expansion_bus_device *bus);

protected:
	device_apricot_expansion_card_interface(const machine_config &mconfig, device_t &device);

	apricot_expansion_bus_device *m_bus;
};


// include here so drivers don't need to
#include "cards.h"


#endif // MAME_BUS_APRICOT_EXPANSION_EXPANSION_H
