// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    DEC VT100 Advanced Video Option (AVO) connector

***************************************************************************/

#ifndef MAME_DEC_VT100_AVO_H
#define MAME_DEC_VT100_AVO_H

#pragma once


class device_vt100_avo_interface;

class vt100_avo_slot_device : public device_t, public device_single_card_slot_interface<device_vt100_avo_interface>
{
public:
	template <typename T>
	vt100_avo_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: vt100_avo_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	vt100_avo_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// the terminal controller's memory space, where the AVO decodes addresses
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }

	bool present() const { return m_card != nullptr; }

	// video fetch of 2000-3FFF (offset from 2000): screen RAM 2C00-2FFF and
	// attribute RAM 3000-3FFF are on the AVO
	u8 video_r(offs_t offset);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	friend class device_vt100_avo_interface;

	required_address_space m_space;
	device_vt100_avo_interface *m_card;
};


class device_vt100_avo_interface : public device_interface
{
public:
	virtual u8 video_r(offs_t offset) = 0;

protected:
	device_vt100_avo_interface(const machine_config &mconfig, device_t &device);

	address_space &space() { return *m_slot->m_space; }

private:
	vt100_avo_slot_device *const m_slot;
};


DECLARE_DEVICE_TYPE(VT100_AVO_SLOT, vt100_avo_slot_device)
DECLARE_DEVICE_TYPE(VT100_AVO, device_vt100_avo_interface)
DECLARE_DEVICE_TYPE(VT100_AVO_AC, device_vt100_avo_interface)

void vt100_avo_options(device_slot_interface &device);

#endif // MAME_DEC_VT100_AVO_H
