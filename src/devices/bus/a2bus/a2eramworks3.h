// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2eramworks3.c

    Applied Engineering RamWorks III

*********************************************************************/

#ifndef __A2EAUX_RAMWORKS3__
#define __A2EAUX_RAMWORKS3__

#include "emu.h"
#include "a2eauxslot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2eaux_ramworks3_device:
	public device_t,
	public device_a2eauxslot_card_interface
{
public:
	// construction/destruction
	a2eaux_ramworks3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2eaux_ramworks3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual UINT8 read_auxram(UINT16 offset) override;
	virtual void write_auxram(UINT16 offset, UINT8 data) override;
	virtual UINT8 *get_vram_ptr() override;
	virtual UINT8 *get_auxbank_ptr() override;
	virtual bool allow_dhr() override { return true; }
	virtual void write_c07x(address_space &space, UINT8 offset, UINT8 data) override;

private:
	UINT8 m_ram[8*1024*1024];
	int m_bank;
};

// device type definition
extern const device_type A2EAUX_RAMWORKS3;

#endif  /* __A2EAUX_RAMWORKS3__ */
