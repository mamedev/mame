// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1805.h

    Hitachi MP-1805 floppy disk controller card for the MB-6890
    Floppy drive is attached

*********************************************************************/

#ifndef __BML3BUS_MP1805__
#define __BML3BUS_MP1805__

#include "emu.h"
#include "bml3bus.h"
#include "imagedev/flopdrv.h"
#include "machine/mc6843.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_mp1805_device:
	public device_t,
	public device_bml3bus_card_interface
{
public:
	// construction/destruction
	bml3bus_mp1805_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_READ8_MEMBER(bml3_mp1805_r);
	DECLARE_WRITE8_MEMBER(bml3_mp1805_w);

	DECLARE_WRITE_LINE_MEMBER( bml3_mc6843_intrq_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<mc6843_device> m_mc6843;

private:
	UINT8 *m_rom;
};

// device type definition
extern const device_type BML3BUS_MP1805;

#endif /* __BML3BUS_MP1805__ */
