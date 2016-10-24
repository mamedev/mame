// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 8271 and 1770 FDC

**********************************************************************/


#ifndef __BBC_ACORN__
#define __BBC_ACORN__

#include "emu.h"
#include "fdc.h"
#include "machine/i8271.h"
#include "machine/wd_fdc.h"
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_acorn8271_device :
	public device_t,
	public device_bbc_fdc_interface

{
public:
	// construction/destruction
	bbc_acorn8271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void fdc_intrq_w(int state);
	void motor_w(int state);
	void side_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_dfs_rom;
	required_device<i8271_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};

class bbc_acorn1770_device :
	public device_t,
	public device_bbc_fdc_interface

{
public:
	// construction/destruction
	bbc_acorn1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	uint8_t wd1770l_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wd1770l_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_dfs_rom;
	required_device<wd1770_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_drive_control;
};


// device type definition
extern const device_type BBC_ACORN8271;
extern const device_type BBC_ACORN1770;


#endif /* __BBC_ACORN__ */
