// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#pragma once

#ifndef __SBP_PROT__
#define __SBP_PROT__

extern const device_type SBP_PROT;

#define MCFG_SBP_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SBP_PROT, 0)


class sbp_prot_device :  public device_t
{
public:
	// construction/destruction
	sbp_prot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);


	UINT8* m_mainrom;

	void sbp_install_protection(cpu_device* maincpu, UINT8* cpurom, UINT32 cpurom_size);
	DECLARE_WRITE16_MEMBER(sbp_lowerrom_w);
	DECLARE_READ16_MEMBER(sbp_lowerrom_r);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
