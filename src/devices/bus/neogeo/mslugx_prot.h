// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


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
	mslugx_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	DECLARE_WRITE16_MEMBER( mslugx_protection_16_w );
	DECLARE_READ16_MEMBER( mslugx_protection_16_r );
	void mslugx_install_protection(cpu_device* maincpu);

	UINT16     m_mslugx_counter;
	UINT16     m_mslugx_command;

protected:
	virtual void device_start();
	virtual void device_reset();



private:


};

#endif
