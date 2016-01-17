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
	k054000_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k054000_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE16_MEMBER( lsb_w );
	DECLARE_READ16_MEMBER( lsb_r );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT8    m_regs[0x20];
};

extern const device_type K054000;




#endif
