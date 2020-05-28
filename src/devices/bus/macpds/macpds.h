// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macpds.h - Mac 68000 PDS implementation (SE, Portable)

  by R. Belmont

***************************************************************************/

#ifndef MAME_BUS_MACPDS_MACPDS_H
#define MAME_BUS_MACPDS_MACPDS_H

#pragma once


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
		set_macpds_slot(nbtag, tag);
	}

	macpds_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	void set_macpds_slot(const char *tag, const char *slottag) { m_macpds_tag = tag; m_macpds_slottag = slottag; }

protected:
	macpds_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// configuration
	const char *m_macpds_tag, *m_macpds_slottag;
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

	~macpds_device() { m_device_list.detach_all(); }
	// inline configuration
	void set_cputag(const char *tag) { m_cputag = tag; }

	void add_macpds_card(device_macpds_card_interface *card);
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, uint32_t mask=0xffffffff);
	void install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, uint32_t mask=0xffffffff);
	void install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data);
	void set_irq_line(int line, int state);

protected:
	macpds_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	cpu_device   *m_maincpu;

	simple_list<device_macpds_card_interface> m_device_list;
	const char *m_cputag;
};


// device type definition
DECLARE_DEVICE_TYPE(MACPDS, macpds_device)

// ======================> device_macpds_card_interface

// class representing interface-specific live macpds card
class device_macpds_card_interface : public device_interface
{
	friend class macpds_device;
	template <class ElememtType> friend class simple_list;
public:
	// construction/destruction
	virtual ~device_macpds_card_interface();

	device_macpds_card_interface *next() const { return m_next; }

	void set_macpds_device();

	// helper functions for card devices
	void install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data);
	void install_rom(device_t *dev, const char *romregion, uint32_t addr);

	// inline configuration
	void set_macpds_tag(const char *tag, const char *slottag) { m_macpds_tag = tag; m_macpds_slottag = slottag; }

protected:
	device_macpds_card_interface(const machine_config &mconfig, device_t &device);

	macpds_device  *m_macpds;
	const char *m_macpds_tag, *m_macpds_slottag;

private:
	device_macpds_card_interface *m_next;
};

#endif // MAME_BUS_MACPDS_MACPDS_H
