// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Floppy Disc Controllers

**********************************************************************/

#ifndef MAME_BUS_BBC_FDC_OPUS_H
#define MAME_BUS_BBC_FDC_OPUS_H

#include "fdc.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/wd_fdc.h"
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_opus8272_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	// construction/destruction
	bbc_opus8272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};


class bbc_opusfdc_device :
	public device_t,
	public device_bbc_fdc_interface
{
public:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(motor_w);

protected:
	// construction/destruction
	bbc_opusfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	required_device<wd_fdc_device_base> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

private:
	int m_drive_control;
};

class bbc_opus2791_device : public bbc_opusfdc_device
{
public:
	bbc_opus2791_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class bbc_opus2793_device : public bbc_opusfdc_device
{
public:
	bbc_opus2793_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class bbc_opus1770_device : public bbc_opusfdc_device
{
public:
	bbc_opus1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_OPUS8272, bbc_opus8272_device)
DECLARE_DEVICE_TYPE(BBC_OPUS2791, bbc_opus2791_device)
DECLARE_DEVICE_TYPE(BBC_OPUS2793, bbc_opus2793_device)
DECLARE_DEVICE_TYPE(BBC_OPUS1770, bbc_opus1770_device)

#endif // MAME_BUS_BBC_FDC_OPUS_H
