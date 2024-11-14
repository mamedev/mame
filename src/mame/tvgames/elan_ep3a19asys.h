// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_ELAN_EP3A19ASYS_H
#define MAME_TVGAMES_ELAN_EP3A19ASYS_H

#include "elan_eu3a05commonsys.h"

class elan_ep3a19asys_device : public elan_eu3a05commonsys_device, public device_memory_interface
{
public:
	elan_ep3a19asys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t elan_eu3a05_dmatrg_r();
	void elan_eu3a05_dmatrg_w(uint8_t data);

	uint8_t dma_param_r(offs_t offset);
	void dma_param_w(offs_t offset, uint8_t data);

	virtual void map(address_map &map) override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	void rombank_w(offs_t offset, uint8_t data);
	uint8_t rombank_r(offs_t offset);

private:
	const address_space_config      m_space_config;
	uint8_t m_dmaparams[7];
};

DECLARE_DEVICE_TYPE(ELAN_EP3A19A_SYS, elan_ep3a19asys_device)

#endif // MAME_TVGAMES_ELAN_EP3A19ASYS_H
