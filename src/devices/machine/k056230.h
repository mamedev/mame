// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 056230

***************************************************************************/

#ifndef MAME_MACHINE_K056230_H
#define MAME_MACHINE_K056230_H

#pragma once




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

class k056230_device : public device_t
{
public:
	// construction/destruction
	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_cpu_tag(device_t &device, const char *tag) { downcast<k056230_device &>(device).m_cpu.set_tag(tag); }
	static void set_thunderh_hack(device_t &device, int thunderh) { downcast<k056230_device &>(device).m_is_thunderh = thunderh; }

	DECLARE_READ32_MEMBER(lanc_ram_r);
	DECLARE_WRITE32_MEMBER(lanc_ram_w);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	TIMER_CALLBACK_MEMBER(network_irq_clear);

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
DECLARE_DEVICE_TYPE(K056230, k056230_device)

#endif // MAME_MACHINE_K056230_H
