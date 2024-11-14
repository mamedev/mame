// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
 * Konami 573 Master Calendar
 *
 */
#ifndef MAME_KONAMI_K573_MCAL_H
#define MAME_KONAMI_K573_MCAL_H

#pragma once

#include "machine/jvsdev.h"
#include "machine/timer.h"

class k573mcal_device : public jvs_device
{
public:
	template <typename T>
	k573mcal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&jvs_host_tag)
		: k573mcal_device(mconfig, tag, owner, clock)
	{
		host.set_tag(std::forward<T>(jvs_host_tag));
	}

	k573mcal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// JVS device overrides
	virtual const char *device_id() override;
	virtual uint8_t command_format_version() override;
	virtual uint8_t jvs_standard_version() override;
	virtual uint8_t comm_method_version() override;
	virtual int handle_message(const uint8_t *send_buffer, uint32_t send_size, uint8_t *&recv_buffer) override;

private:
	required_ioport m_in1;

	uint8_t seconds;
	uint32_t mainId;
	uint32_t subId;
};

DECLARE_DEVICE_TYPE(KONAMI_573_MASTER_CALENDAR, k573mcal_device)

#endif // MAME_KONAMI_K573_MCAL_H
