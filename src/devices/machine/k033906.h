// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 033906

***************************************************************************/

#pragma once

#ifndef __K033906_H__
#define __K033906_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K033906_VOODOO(_tag) \
	k033906_device::set_voodoo_tag(*device, _tag);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> k033906_device

class k033906_device :  public device_t
{
public:
	// construction/destruction
	k033906_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_voodoo_tag(device_t &device, const char *tag) { downcast<k033906_device &>(device).m_voodoo_tag = tag; }

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( set_reg );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override { }
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

private:

	UINT32 reg_r(int reg);
	void reg_w(int reg, UINT32 data);

	/* i/o lines */

	int          m_reg_set; // 1 = access reg / 0 = access ram

	const char   *m_voodoo_tag;
	device_t     *m_voodoo;

	UINT32       m_reg[256];
	UINT32       m_ram[32768];
};


// device type definition
extern const device_type K033906;

#endif  /* __K033906_H__ */
