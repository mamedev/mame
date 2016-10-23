// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#pragma once
#ifndef __K051733_H__
#define __K051733_H__

class k051733_device : public device_t
{
public:
	k051733_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k051733_device() {}

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	uint8_t    m_ram[0x20];
	uint8_t    m_rng;
};

extern const device_type K051733;

#define MCFG_K051733_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K051733, 0)

#endif
