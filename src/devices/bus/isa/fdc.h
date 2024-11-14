// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Wilbert Pol
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
	void irq_w(int state);
	void drq_w(int state);
	static void floppy_formats(format_registration &fr);

protected:
	// construction/destruction
	isa8_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void dack_line_w(int line, int state) override;
	virtual void eop_w(int state) override;

	required_device<upd765_family_device> m_fdc;
};

class isa8_upd765_fdc_device : public isa8_fdc_device
{
protected:
	// construction/destruction
	isa8_upd765_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t dor_r();
	void dor_w(uint8_t data);
	uint8_t dir_r();
	void ccr_w(uint8_t data);

	void fdc_irq_w(int state);
	void fdc_drq_w(int state);

private:
	bool irq, drq, fdc_drq, fdc_irq;
	uint8_t dor;

	floppy_image_device *floppy[4];

	void check_irq();
	void check_drq();
};

class isa8_fdc_xt_device : public isa8_upd765_fdc_device {
public:
	isa8_fdc_xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	isa8_fdc_xt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;
	void dor_fifo_w(uint8_t data);
};

class isa8_fdc_at_device : public isa8_upd765_fdc_device {
public:
	isa8_fdc_at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;
};

class isa8_fdc_smc_device : public isa8_fdc_device {
public:
	isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

class isa8_fdc_ps2_device : public isa8_fdc_device {
public:
	isa8_fdc_ps2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

class isa8_fdc_superio_device : public isa8_fdc_device {
public:
	isa8_fdc_superio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};

class isa8_ec1841_0003_device : public isa8_fdc_xt_device {
public:
	isa8_ec1841_0003_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void aux_irq_w(int state);

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
