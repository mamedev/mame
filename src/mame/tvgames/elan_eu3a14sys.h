// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_ELAN_EU3A14SYS_H
#define MAME_TVGAMES_ELAN_EU3A14SYS_H

#include "elan_eu3a05commonsys.h"

class elan_eu3a14sys_device : public elan_eu3a05commonsys_device, public device_memory_interface
{
public:
	elan_eu3a14sys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t dma_trigger_r();
	void dma_trigger_w(uint8_t data);

	uint8_t dma_param_r(offs_t offset);
	void dma_param_w(offs_t offset, uint8_t data);

	virtual void map(address_map &map) override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	const address_space_config      m_space_config;
	uint8_t m_dmaparams[9];
};



DECLARE_DEVICE_TYPE(ELAN_EU3A14_SYS, elan_eu3a14sys_device)

#endif // MAME_TVGAMES_ELAN_EU3A14SYS_H
