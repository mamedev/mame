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
	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_cpu_tag(device_t &device, const char *tag) { downcast<k056230_device &>(device).m_cpu.set_tag(tag); }
	static void set_thunderh_hack(device_t &device, int thunderh) { downcast<k056230_device &>(device).m_is_thunderh = thunderh; }

	uint32_t lanc_ram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void lanc_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void network_irq_clear(void *ptr, int32_t param);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override { }
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

private:

	int m_is_thunderh;

	required_device<cpu_device> m_cpu;
	uint32_t m_ram[0x2000];
};


// device type definition
extern const device_type K056230;

#endif  /* __K056230_H__ */
