// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3kanji.h

    Hitachi MP-9740 (?) kanji character ROM for the MB-689x

*********************************************************************/

#ifndef __BML3BUS_KANJI__
#define __BML3BUS_KANJI__

#include "emu.h"
#include "bml3bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_kanji_device:
	public device_t,
	public device_bml3bus_card_interface
{
public:
	// construction/destruction
	bml3bus_kanji_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	DECLARE_READ8_MEMBER(bml3_kanji_r);
	DECLARE_WRITE8_MEMBER(bml3_kanji_w);

protected:
	virtual void device_start();
	virtual void device_reset();

	UINT16 m_kanji_addr;

private:
	UINT8 *m_rom;
};

// device type definition
extern const device_type BML3BUS_KANJI;

#endif /* __BML3BUS_KANJI__ */
