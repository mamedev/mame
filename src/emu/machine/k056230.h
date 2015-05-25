// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 056230

***************************************************************************/

#pragma once

#ifndef __K056230_H__
#define __K056230_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K056230_CPU(_tag) \
	k056230_device::set_cpu_tag(*device, "^" _tag);

#define MCFG_K056230_HACK(_region) \
	k056230_device::set_thunderh_hack(*device, _region);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> k056230_device

class k056230_device :  public device_t
{
public:
	// construction/destruction
	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_cpu_tag(device_t &device, const char *tag) { downcast<k056230_device &>(device).m_cpu.set_tag(tag); }
	static void set_thunderh_hack(device_t &device, int thunderh) { downcast<k056230_device &>(device).m_is_thunderh = thunderh; }

	DECLARE_READ32_MEMBER(lanc_ram_r);
	DECLARE_WRITE32_MEMBER(lanc_ram_w);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	static TIMER_CALLBACK( network_irq_clear_callback );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset() { }
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }

private:

	void network_irq_clear();
	int m_is_thunderh;

	required_device<cpu_device> m_cpu;
	UINT32 m_ram[0x2000];
};


// device type definition
extern const device_type K056230;

#endif  /* __K056230_H__ */
