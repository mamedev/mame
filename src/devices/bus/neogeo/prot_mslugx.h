// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli


#pragma once

#ifndef __MSLUGX_PROT__
#define __MSLUGX_PROT__

extern const device_type MSLUGX_PROT;

#define MCFG_MSLUGX_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MSLUGX_PROT, 0)


class mslugx_prot_device :  public device_t
{
public:
	// construction/destruction
	mslugx_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t     m_counter;
	uint16_t     m_command;
};

#endif
