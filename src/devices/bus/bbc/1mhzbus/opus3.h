// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Challenger 3-in-1

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_OPUS3_H
#define MAME_BUS_BBC_1MHZBUS_OPUS3_H

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

	DECLARE_WRITE8_MEMBER(wd1770l_write);
	DECLARE_WRITE8_MEMBER(page_w);
	DECLARE_READ8_MEMBER(ramdisk_r);
	DECLARE_WRITE8_MEMBER(ramdisk_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);

	required_memory_region m_dfs_rom;
	required_device<ram_device> m_ramdisk;
	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	int m_fdc_ie;
	int m_fdc_drq;
	uint16_t m_ramdisk_page;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_OPUS3, bbc_opus3_device)


#endif // MAME_BUS_BBC_1MHZBUS_OPUS3_H
