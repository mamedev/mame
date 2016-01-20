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
	a2bus_lang_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	a2bus_lang_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;

private:
	void langcard_touch(offs_t offset);

	int last_offset;
};

// device type definition
extern const device_type A2BUS_LANG;

#endif  /* __A2BUS_LANG__ */
