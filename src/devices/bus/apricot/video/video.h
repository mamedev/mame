// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot XEN Video Expansion Slot

    Large identical to the standard Apricot PC/Xi expansion slot.

***************************************************************************/

#ifndef MAME_BUS_APRICOT_VIDEO_VIDEO_H
#define MAME_BUS_APRICOT_VIDEO_VIDEO_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_apricot_video_interface;

// ======================> apricot_video_slot_device

class apricot_video_slot_device : public device_t, public device_single_card_slot_interface<device_apricot_video_interface>
{
public:
	// construction/destruction
	template <typename T>
	apricot_video_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, const char *dflt)
		: apricot_video_slot_device(mconfig, tag, owner, uint32_t(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	apricot_video_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~apricot_video_slot_device();

	// callbacks
	auto apvid_handler() { return m_apvid_handler.bind(); }

	// called from cart device
	void apvid_w(int state) { m_apvid_handler(state); }

	// called from host
	bool mem_r(offs_t offset, uint16_t &data, uint16_t mem_mask);
	bool mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	bool io_r(offs_t offset, uint16_t &data, uint16_t mem_mask);
	bool io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_apvid_handler;

	device_apricot_video_interface *m_card;
};

// ======================> device_apricot_video_interface

class device_apricot_video_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_apricot_video_interface();

	virtual bool mem_r(offs_t offset, uint16_t &data, uint16_t mem_mask) { return false; }
	virtual bool mem_w(offs_t offset, uint16_t data, uint16_t mem_mask) { return false; }
	virtual bool io_r(offs_t offset, uint16_t &data, uint16_t mem_mask) { return false; }
	virtual bool io_w(offs_t offset, uint16_t data, uint16_t mem_mask) { return false; }

protected:
	device_apricot_video_interface(const machine_config &mconfig, device_t &device);

	apricot_video_slot_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(APRICOT_VIDEO_SLOT, apricot_video_slot_device)

// include here so drivers don't need to
#include "cards.h"

#endif // MAME_BUS_APRICOT_VIDEO_VIDEO_H
