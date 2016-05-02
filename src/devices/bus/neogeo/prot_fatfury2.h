// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#pragma once

#ifndef __FATFURY2_PROT__
#define __FATFURY2_PROT__

extern const device_type FATFURY2_PROT;

#define MCFG_FATFURY2_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, FATFURY2_PROT, 0)


class fatfury2_prot_device :  public device_t
{
public:
	// construction/destruction
	fatfury2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER( protection_r );
	DECLARE_WRITE16_MEMBER( protection_w );

	UINT32     m_prot_data;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
