// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Floppy Disc Controllers

**********************************************************************/

#ifndef MAME_BUS_BBC_FDC_OPUS_H
#define MAME_BUS_BBC_FDC_OPUS_H

#include "fdc.h"
#include "machine/wd_fdc.h"
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_opusfdc_device:
	public device_t,
	public device_bbc_fdc_interface

{
public:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_READ8_MEMBER(ctrl_r);
	DECLARE_WRITE8_MEMBER(ctrl_w);

protected:
	// construction/destruction
	bbc_opusfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_dfs_rom;
	required_device<wd_fdc_device_base> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_drive_control;
};

class bbc_opus2791_device : public bbc_opusfdc_device
{
public:
	bbc_opus2791_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class bbc_opus2793_device : public bbc_opusfdc_device
{
public:
	bbc_opus2793_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class bbc_opus1770_device : public bbc_opusfdc_device
{
public:
	bbc_opus1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_OPUS2791, bbc_opus2791_device)
DECLARE_DEVICE_TYPE(BBC_OPUS2793, bbc_opus2793_device)
DECLARE_DEVICE_TYPE(BBC_OPUS1770, bbc_opus1770_device)

#endif // MAME_BUS_BBC_FDC_OPUS_H
