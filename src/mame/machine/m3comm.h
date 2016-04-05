// license:BSD-3-Clause
// copyright-holders:MetalliC
#pragma once

#ifndef __M3COMM_H__
#define __M3COMM_H__

#include "machine/ram.h"
#include "cpu/m68000/m68000.h"

#define MCFG_M3COMM_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, M3COMM, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m3comm_device : public device_t
{
public:
	// construction/destruction
	m3comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_shared_ptr<UINT16> m68k_ram;
	required_device<m68000_device> m_commcpu;

	DECLARE_READ16_MEMBER(naomi_r);
	DECLARE_WRITE16_MEMBER(naomi_w);
	DECLARE_READ16_MEMBER(status0_r);
	DECLARE_WRITE16_MEMBER(status0_w);
	DECLARE_READ16_MEMBER(status1_r);
	DECLARE_WRITE16_MEMBER(status1_w);
	DECLARE_READ16_MEMBER(commbank_r);
	DECLARE_WRITE16_MEMBER(commbank_w);

protected:
	required_device<ram_device> m_ram;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

private:
	UINT16 m_control;
	UINT16 m_offset;
	UINT16 m_status0;
	UINT16 m_status1;
	UINT16 m_commbank;


	emu_file m_line_rx;    // rx line - can be either differential, simple serial or toslink
	emu_file m_line_tx;    // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];
};

// device type definition
extern const device_type M3COMM;

#endif  /* __M3COMM_H__ */
