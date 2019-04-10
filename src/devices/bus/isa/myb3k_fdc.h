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
#include "formats/imd_dsk.h"

class isa8_myb3k_fdc471x_base
{
protected:
	enum {
		FDC_MSM_MODE = 0x40,
		FDC_DDEN = 0x20,
		//FDC_MOTOR_ON = 0x10, // According to service manual but not schematics and BIOS
		FDC_SIDE_SEL = 0x08,
		FDC_MOTOR_ON = 0x04, // According to schematics but "Motor Cont" according to service manual
		FDC_DRIVE_SEL = 0x03,
	};
};

class isa8_myb3k_fdc4710_device :
	public device_t,
	public device_isa8_card_interface,
	public isa8_myb3k_fdc471x_base
{
public:
	// construction/destruction
	isa8_myb3k_fdc4710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// construction/destruction
	isa8_myb3k_fdc4710_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	//  virtual void eop_w(int state) override;

	DECLARE_READ8_MEMBER(myb3k_inv_fdc_data_r);
	DECLARE_WRITE8_MEMBER(myb3k_inv_fdc_data_w);
	DECLARE_READ8_MEMBER(myb3k_fdc_status);
	DECLARE_WRITE8_MEMBER(myb3k_fdc_command);
	void map(address_map &map);

	required_device<mb8876_device> m_fdc;
	required_device<floppy_connector> m_fdd0;
	optional_device<floppy_connector> m_fdd1;

private:
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );
	DECLARE_FLOPPY_FORMATS( myb3k_floppy_formats );
};

class isa8_myb3k_fdc4711_device :
	public device_t,
	public device_isa8_card_interface,
	public isa8_myb3k_fdc471x_base
{
public:
	// construction/destruction
	isa8_myb3k_fdc4711_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// construction/destruction
	isa8_myb3k_fdc4711_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	// virtual void eop_w(int state) override;

	DECLARE_READ8_MEMBER(myb3k_inv_fdc_data_r);
	DECLARE_WRITE8_MEMBER(myb3k_inv_fdc_data_w);
	DECLARE_READ8_MEMBER(myb3k_fdc_status);
	DECLARE_WRITE8_MEMBER(myb3k_fdc_command);
	void map(address_map &map);

	required_device<fd1791_device> m_fdc;
	required_device<floppy_connector> m_fdd0;
	optional_device<floppy_connector> m_fdd1;
	optional_device<floppy_connector> m_fdd2;
	optional_device<floppy_connector> m_fdd3;

private:
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );
	DECLARE_FLOPPY_FORMATS( myb3k_floppy_formats );
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_MYB3K_FDC4710,      isa8_myb3k_fdc4710_device)
DECLARE_DEVICE_TYPE(ISA8_MYB3K_FDC4711,      isa8_myb3k_fdc4711_device)
//DECLARE_DEVICE_TYPE(ISA8_MYB3K_FDC4712,      isa8_myb3k_fdc4712_device)

#endif // MAME_BUS_ISA_MYB3K_FDC_H
