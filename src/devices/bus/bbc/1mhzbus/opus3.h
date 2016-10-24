// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Challenger 3-in-1

**********************************************************************/


#ifndef __BBC_OPUS3__
#define __BBC_OPUS3__

#include "emu.h"
#include "1mhzbus.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_opus3_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_opus3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void fdc_drq_w(int state);
	void wd1770l_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t ramdisk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ramdisk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_memory_region m_dfs_rom;
	required_device<ram_device> m_ramdisk;
	required_device<wd1770_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_fdc_ie;
	int m_fdc_drq;
	uint16_t m_ramdisk_page;
};


// device type definition
extern const device_type BBC_OPUS3;


#endif /* __BBC_OPUS3__ */
