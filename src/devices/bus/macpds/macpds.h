// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macpds.h - Mac 68000 PDS implementation (SE, Portable)

  by R. Belmont

***************************************************************************/

#ifndef MAME_BUS_MACPDS_MACPDS_H
#define MAME_BUS_MACPDS_MACPDS_H

#pragma once

#include <functional>
#include <utility>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class macpds_device;

class macpds_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	macpds_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *nbtag, T &&opts, const char *dflt)
		: macpds_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_macpds_slot(nbtag);
	}

	macpds_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T>
	void set_macpds_slot(T &&tag) { m_macpds.set_tag(std::forward<T>(tag));}

protected:
	macpds_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<macpds_device> m_macpds;
};

// device type definition
DECLARE_DEVICE_TYPE(MACPDS_SLOT, macpds_slot_device)


class device_macpds_card_interface;

// ======================> macpds_device
class macpds_device : public device_t
{
public:
	// construction/destruction
	macpds_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *cputag)
		: macpds_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_cputag(cputag);
	}

	macpds_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~macpds_device();

	// inline configuration
	template <typename T>
	void set_cputag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }


	void add_macpds_card(device_macpds_card_interface &card);
	template<typename R, typename W> void install_device(offs_t start, offs_t end, R rhandler, W whandler, uint32_t mask=0xffffffff);
	void install_bank(offs_t start, offs_t end, uint8_t *data);
	void set_irq_line(int line, int state);

protected:
	macpds_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_device<cpu_device> m_maincpu;

	std::vector<std::reference_wrapper<device_macpds_card_interface> > m_device_list;
};


// device type definition
DECLARE_DEVICE_TYPE(MACPDS, macpds_device)

// ======================> device_macpds_card_interface

// class representing interface-specific live macpds card
class device_macpds_card_interface : public device_interface
{
	friend class macpds_device;
public:
	// construction/destruction
	virtual ~device_macpds_card_interface();

	void set_macpds_device();

	// helper functions for card devices
	void install_bank(offs_t start, offs_t end, uint8_t *data);
	void install_rom(device_t *dev, const char *romregion, uint32_t addr);

	// inline configuration
	void set_macpds_and_slot(macpds_device *macpds, macpds_slot_device *macpds_slot) { m_macpds = macpds; m_macpds_slot = macpds_slot; }

protected:
	device_macpds_card_interface(const machine_config &mconfig, device_t &device);

	macpds_device *m_macpds;
	macpds_slot_device *m_macpds_slot;
};

#endif // MAME_BUS_MACPDS_MACPDS_H
