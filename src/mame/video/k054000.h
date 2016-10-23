// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once
#ifndef __K054000_H__
#define __K054000_H__

#define MCFG_K054000_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K054000, 0)

class k054000_device : public device_t
{
public:
	k054000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k054000_device() {}

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lsb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t lsb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t    m_regs[0x20];
};

extern const device_type K054000;




#endif
