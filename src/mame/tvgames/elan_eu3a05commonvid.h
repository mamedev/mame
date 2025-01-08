// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_ELAN_EU3A05COMMONVID_H
#define MAME_TVGAMES_ELAN_EU3A05COMMONVID_H

#include "emupal.h"

class elan_eu3a05commonvid_device : public device_t
{
public:
	elan_eu3a05commonvid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	elan_eu3a05commonvid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_palette(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	void set_entries(int entries) { m_palram.resize(entries*2); }

	uint8_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<palette_device> m_palette;
	std::vector<uint8_t> m_palram;
	void update_pen(int pen);
	double hue2rgb(double p, double q, double t);
};

DECLARE_DEVICE_TYPE(ELAN_EU3A05_COMMONVID, elan_eu3a05commonvid_device)

#endif // MAME_TVGAMES_ELAN_EU3A05COMMONVID_H
