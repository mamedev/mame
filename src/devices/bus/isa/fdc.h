// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/
#ifndef MAME_BUS_ISA_FDC_H
#define MAME_BUS_ISA_FDC_H

#pragma once

#include "isa.h"
#include "imagedev/floppy.h"
#include "machine/busmouse.h"
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
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// construction/destruction
	isa8_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void dack_line_w(int line, int state) override;
	virtual void eop_w(int state) override;

	required_device<pc_fdc_interface> m_fdc;
};

class isa8_fdc_xt_device : public isa8_fdc_device {
public:
	isa8_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class isa8_fdc_at_device : public isa8_fdc_device {
public:
	isa8_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class isa8_fdc_smc_device : public isa8_fdc_device {
public:
	isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class isa8_fdc_ps2_device : public isa8_fdc_device {
public:
	isa8_fdc_ps2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class isa8_fdc_superio_device : public isa8_fdc_device {
public:
	isa8_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class isa8_ec1841_0003_device : public isa8_fdc_device {
public:
	isa8_ec1841_0003_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	DECLARE_WRITE_LINE_MEMBER( aux_irq_w );

	required_device<bus_mouse_device> m_bus_mouse;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_FDC_XT,      isa8_fdc_xt_device)
DECLARE_DEVICE_TYPE(ISA8_FDC_AT,      isa8_fdc_at_device)
DECLARE_DEVICE_TYPE(ISA8_FDC_SMC,     isa8_fdc_smc_device)
DECLARE_DEVICE_TYPE(ISA8_FDC_PS2,     isa8_fdc_ps2_device)
DECLARE_DEVICE_TYPE(ISA8_FDC_SUPERIO, isa8_fdc_superio_device)
DECLARE_DEVICE_TYPE(ISA8_EC1841_0003, isa8_ec1841_0003_device)

#endif // MAME_BUS_ISA_FDC_H
