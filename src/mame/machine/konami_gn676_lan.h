// license:BSD-3-Clause
// copyright-holders:Ville Linde

#ifndef MAME_MACHINE_KONAMI_GN676_LAN_H
#define MAME_MACHINE_KONAMI_GN676_LAN_H

#pragma once

class konami_gn676_lan_device :  public device_t
{
public:
	template <typename T>
	konami_gn676_lan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&work_ram_tag)
		: konami_gn676_lan_device(mconfig, tag, owner, clock)
	{
		m_work_ram.set_tag(std::forward<T>(work_ram_tag));
	}

	konami_gn676_lan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t lanc1_r(offs_t offset);
	void lanc1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t lanc2_r(offs_t offset, uint32_t mem_mask = ~0);
	void lanc2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override;

private:
	bool m_fpga_uploaded;
	int m_lanc2_ram_r;
	int m_lanc2_ram_w;
	uint8_t m_lanc2_reg[3];
	std::unique_ptr<uint8_t[]> m_lanc2_ram;

	required_shared_ptr<uint32_t> m_work_ram;
};

DECLARE_DEVICE_TYPE(KONAMI_GN676_LAN, konami_gn676_lan_device)


#endif // MAME_MACHINE_KONAMI_GN676_LAN_H
