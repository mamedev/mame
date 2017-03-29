// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2eramworks3.c

    Applied Engineering RamWorks III

*********************************************************************/

#ifndef __A2EAUX_RAMWORKS3__
#define __A2EAUX_RAMWORKS3__

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
	a2eaux_ramworks3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	a2eaux_ramworks3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_auxram(uint16_t offset) override;
	virtual void write_auxram(uint16_t offset, uint8_t data) override;
	virtual uint8_t *get_vram_ptr() override;
	virtual uint8_t *get_auxbank_ptr() override;
	virtual bool allow_dhr() override { return true; }
	virtual void write_c07x(address_space &space, uint8_t offset, uint8_t data) override;

private:
	uint8_t m_ram[8*1024*1024];
	int m_bank;
};

// device type definition
extern const device_type A2EAUX_RAMWORKS3;

#endif  /* __A2EAUX_RAMWORKS3__ */
