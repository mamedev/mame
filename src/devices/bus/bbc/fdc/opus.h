// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Floppy Disc Controllers

**********************************************************************/


#ifndef __BBC_OPUSFDC__
#define __BBC_OPUSFDC__

#include "emu.h"
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
	// construction/destruction
	bbc_opusfdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void motor_w(int state);
	uint8_t ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_dfs_rom;
	required_device<wd_fdc_t> m_fdc;
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
extern const device_type BBC_OPUS2791;
extern const device_type BBC_OPUS2793;
extern const device_type BBC_OPUS1770;

#endif /* __BBC_OPUSFDC_ */
