// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_KONAMI_KONPPC_JVSHOST_H
#define MAME_KONAMI_KONPPC_JVSHOST_H

#pragma once

#include "machine/jvshost.h"

class konppc_jvs_host_device : public jvs_host
{
public:
	// construction/destruction
	konppc_jvs_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto output_callback() { return output_cb.bind(); }

	void read();
	bool write(uint8_t data);

	int sense();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr int JVS_BUFFER_SIZE = 1024;

	devcb_write8 output_cb;

	std::unique_ptr<uint8_t[]> m_jvs_sdata;
	uint32_t m_jvs_sdata_ptr;
	bool m_jvs_is_escape_byte;
};

DECLARE_DEVICE_TYPE(KONPPC_JVS_HOST, konppc_jvs_host_device)

#endif // MAME_KONAMI_KONPPC_JVSHOST_H
