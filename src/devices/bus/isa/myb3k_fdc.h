// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/
#ifndef MAME_BUS_ISA_MYB3K_FDC_H
#define MAME_BUS_ISA_MYB3K_FDC_H

#pragma once

#include "isa.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

class isa8_myb3k_fdc471x_device_base :
	public device_t,
	public device_isa8_card_interface
{
protected:
	isa8_myb3k_fdc471x_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override { }

	void map(address_map &map) ATTR_COLD;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	// virtual void eop_w(int state) override;

	virtual uint8_t myb3k_inv_fdc_data_r(offs_t offset);
	virtual void myb3k_inv_fdc_data_w(offs_t offset, uint8_t data);
	virtual uint8_t myb3k_fdc_status();
	virtual void myb3k_fdc_command(uint8_t data);

	void irq_w(int state);
	void drq_w(int state);

	required_device<wd_fdc_device_base> m_fdc;
	optional_device_array<floppy_connector, 4> m_floppy_connectors;

	offs_t io_base;
	int dma_channel;

	bool has_motor_control;

	enum FDC_COMMAND {
		FDC_DRIVE_SEL = 0x03,
		FDC_MOTOR_ON = 0x04, // According to schematics but "Motor Cont" according to service manual
		FDC_SIDE_SEL = 0x08,
		//FDC_MOTOR_ON = 0x10, // According to service manual but not schematics and BIOS
		FDC_DDEN = 0x20,
		FDC_MSM_MODE = 0x40,
	};

	enum FDC_STATUS {
		FDC_STATUS_DOUBLE_SIDED = 0x04,
	};
};

class isa8_myb3k_fdc4710_device : public isa8_myb3k_fdc471x_device_base
{
public:
	// construction/destruction
	isa8_myb3k_fdc4710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// construction/destruction
	isa8_myb3k_fdc4710_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class isa8_myb3k_fdc4711_device : public isa8_myb3k_fdc471x_device_base
{
public:
	// construction/destruction
	isa8_myb3k_fdc4711_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// construction/destruction
	isa8_myb3k_fdc4711_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class isa8_myb3k_fdc4712_device : public isa8_myb3k_fdc471x_device_base
{
public:
	// construction/destruction
	isa8_myb3k_fdc4712_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// construction/destruction
	isa8_myb3k_fdc4712_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void myb3k_fdc_command(uint8_t data) override;
	virtual uint8_t myb3k_fdc_status() override;

	uint8_t selected_drive;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_MYB3K_FDC4710,      isa8_myb3k_fdc4710_device)
DECLARE_DEVICE_TYPE(ISA8_MYB3K_FDC4711,      isa8_myb3k_fdc4711_device)
DECLARE_DEVICE_TYPE(ISA8_MYB3K_FDC4712,      isa8_myb3k_fdc4712_device)

#endif // MAME_BUS_ISA_MYB3K_FDC_H
