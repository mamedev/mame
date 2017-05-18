// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana QFS 8877A FDC

**********************************************************************/


#ifndef __BBC_CUMANA__
#define __BBC_CUMANA__

#include "fdc.h"
#include "machine/wd_fdc.h"
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_cumanafdc_device :
	public device_t,
	public device_bbc_fdc_interface

{
public:
	// construction/destruction
	bbc_cumanafdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_READ8_MEMBER(ctrl_r);
	DECLARE_WRITE8_MEMBER(ctrl_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	bool m_invert;

private:
	required_memory_region m_dfs_rom;
	required_device<mb8877_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_drive_control;
	int m_fdc_ie;
};

class bbc_cumana1_device : public bbc_cumanafdc_device
{
public:
	bbc_cumana1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class bbc_cumana2_device : public bbc_cumanafdc_device
{
public:
	bbc_cumana2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// device type definition
extern const device_type BBC_CUMANA1;
extern const device_type BBC_CUMANA2;


#endif /* __BBC_CUMANA__ */
