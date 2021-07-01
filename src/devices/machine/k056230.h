// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 056230

***************************************************************************/

#ifndef MAME_MACHINE_K056230_H
#define MAME_MACHINE_K056230_H

#pragma once

class k056230_device : public device_t
{
public:
	// construction/destruction
	template <typename T>
	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: k056230_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
	}

	k056230_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t lanc_ram_r(offs_t offset, uint32_t mem_mask = ~0);
	void lanc_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

private:

	required_device<cpu_device> m_cpu;
	uint32_t m_ram[0x2000];
};


// device type definition
DECLARE_DEVICE_TYPE(K056230, k056230_device)

#endif // MAME_MACHINE_K056230_H
