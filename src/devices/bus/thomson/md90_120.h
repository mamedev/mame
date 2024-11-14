// license:BSD-3-Clause
// copyright-holders:Antoine Mine

#ifndef MAME_BUS_THOMSON_MD90_120_H
#define MAME_BUS_THOMSON_MD90_120_H

#include "extension.h"
#include "machine/6850acia.h"

class md90_120_device : public device_t, public thomson_extension_interface
{
public:
	md90_120_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void modem_cb(int state);
	void modem_tx_w(int state);
	void write_acia_clock(int state);

	required_device<acia6850_device> m_acia;

	uint8_t m_modem_tx = 0;
};

// device type declaration
DECLARE_DEVICE_TYPE(MD90_120, md90_120_device)

#endif // MAME_BUS_THOMSON_MD90_120_H
