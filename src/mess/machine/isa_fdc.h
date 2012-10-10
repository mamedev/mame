/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/
#pragma once

#ifndef ISA_FDC_H
#define ISA_FDC_H

#include "emu.h"
#include "machine/isa.h"
#include "machine/upd765.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_fdc_device

class isa8_fdc_device :
	public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	required_device<pc_fdc_interface> fdc;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 dack_r(int line);
	virtual void dack_w(int line, UINT8 data);
	virtual void eop_w(int state);

private:
	void irq_w(bool state);
	void drq_w(bool state);

};

class isa8_fdc_xt_device : public isa8_fdc_device {
public:
	isa8_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
};

class isa8_fdc_at_device : public isa8_fdc_device {
public:
	isa8_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
};

class isa8_fdc_smc_device : public isa8_fdc_device {
public:
	isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
};

class isa8_fdc_ps2_device : public isa8_fdc_device {
public:
	isa8_fdc_ps2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
};

class isa8_fdc_superio_device : public isa8_fdc_device {
public:
	isa8_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual machine_config_constructor device_mconfig_additions() const;
};

// device type definition
extern const device_type ISA8_FDC_XT;
extern const device_type ISA8_FDC_AT;
extern const device_type ISA8_FDC_SMC;
extern const device_type ISA8_FDC_PS2;
extern const device_type ISA8_FDC_SUPERIO;

#endif  /* ISA_FDC_H */
