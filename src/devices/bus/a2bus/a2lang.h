// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2lang.h

    Apple II Language Card

*********************************************************************/

#ifndef __A2BUS_LANG__
#define __A2BUS_LANG__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_lang_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_lang_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	a2bus_lang_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;

private:
	void langcard_touch(offs_t offset);

	int last_offset;
};

// device type definition
extern const device_type A2BUS_LANG;

#endif  /* __A2BUS_LANG__ */
