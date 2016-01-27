// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1802.h

    Hitachi MP-1802 floppy disk controller card for the MB-6890
    Hitachi MP-3550 floppy drive is attached

*********************************************************************/

#ifndef __BML3BUS_MP1802__
#define __BML3BUS_MP1802__

#include "emu.h"
#include "bml3bus.h"
#include "imagedev/flopdrv.h"
#include "machine/wd_fdc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_mp1802_device:
	public device_t,
	public device_bml3bus_card_interface
{
public:
	// construction/destruction
	bml3bus_mp1802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_READ8_MEMBER(bml3_mp1802_r);
	DECLARE_WRITE8_MEMBER(bml3_mp1802_w);
	DECLARE_WRITE_LINE_MEMBER(bml3_wd17xx_intrq_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<mb8866_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

	UINT8 *m_rom;
};

// device type definition
extern const device_type BML3BUS_MP1802;

#endif /* __BML3BUS_MP1802__ */
