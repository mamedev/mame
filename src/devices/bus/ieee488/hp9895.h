// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

	hp9895.h

	HP9895 floppy disk drive

*********************************************************************/

#pragma once

#ifndef _HP9895_H_
#define _HP9895_H_

#include "emu.h"
#include "ieee488.h"
#include "cpu/z80/z80.h"
#include "machine/phi.h"

class hp9895_device : public device_t,
					  public device_ieee488_interface
{
public:
	// construction/destruction
	hp9895_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	//virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_ieee488_interface overrides
	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

private:
	required_device<z80_device> m_cpu;
	required_device<phi_device> m_phi;
};

// device type definition
extern const device_type HP9895;

#endif /* _HP9895_H_ */
