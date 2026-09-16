// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_ELAN_EU3A05COMMONVID_H
#define MAME_MACHINE_ELAN_EU3A05COMMONVID_H

#include "emupal.h"

#include "elan_eu3a05commonsys.h"

class elan_eu3a05commonvid_device : public device_t
{
public:
	template <typename T> void set_palette(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	void set_entries(int entries) { m_palram.resize(entries*2); }

	uint8_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);

protected:
	elan_eu3a05commonvid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<palette_device> m_palette;

private:
	void update_pen(int pen);
	double hue2rgb(double p, double q, double t);

	std::vector<uint8_t> m_palram;
};

#endif // MAME_MACHINE_ELAN_EU3A05COMMONVID_H
