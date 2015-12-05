// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#pragma once

#ifndef __KOG_PROT__
#define __KOG_PROT__

extern const device_type KOG_PROT;

#define MCFG_KOG_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, KOG_PROT, 0)


class kog_prot_device :  public device_t
{
public:
	// construction/destruction
	kog_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void kog_install_protection(cpu_device* maincpu);
	void kog_px_decrypt(UINT8* cpurom, UINT32 cpurom_size);
	READ16_MEMBER(read_jumper);

	required_ioport m_jumper;

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual ioport_constructor device_input_ports() const;
};

#endif
