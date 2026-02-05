// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert

// Quick slot thingy to select the video vme board

#ifndef MAME_OMRON_LUNA_68K_VIDEO_H
#define MAME_OMRON_LUNA_68K_VIDEO_H

#pragma once

class device_luna_68k_video_interface;

class luna_68k_video_connector: public device_t, public device_single_card_slot_interface<device_luna_68k_video_interface>
{
public:
	template <typename T> luna_68k_video_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: luna_68k_video_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	luna_68k_video_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void install_vme_map(address_space_installer &space);

protected:
	virtual void device_start() override ATTR_COLD;
};

class device_luna_68k_video_interface: public device_interface
{
public:
	virtual ~device_luna_68k_video_interface();

	void install_vme_map(address_space_installer &space);

protected:
	luna_68k_video_connector *m_connector;

	device_luna_68k_video_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	virtual void vme_map(address_map &map) = 0;
};

DECLARE_DEVICE_TYPE(LUNA_68K_VIDEO_CONNECTOR, luna_68k_video_connector)

void luna_68k_video_intf(device_slot_interface &device);

#endif
