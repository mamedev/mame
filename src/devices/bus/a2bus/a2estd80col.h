// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2estd80col.c

    Apple IIe Standard 80 Column Card

*********************************************************************/

#ifndef __A2EAUX_STD80COL__
#define __A2EAUX_STD80COL__

#include "emu.h"
#include "a2eauxslot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2eaux_std80col_device:
	public device_t,
	public device_a2eauxslot_card_interface
{
public:
	// construction/destruction
	a2eaux_std80col_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2eaux_std80col_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual UINT8 read_auxram(UINT16 offset) override;
	virtual void write_auxram(UINT16 offset, UINT8 data) override;
	virtual UINT8 *get_vram_ptr() override;
	virtual UINT8 *get_auxbank_ptr() override;
	virtual bool allow_dhr() override { return false; }  // we don't allow DHR

private:
	UINT8 m_ram[2*1024];
};

// device type definition
extern const device_type A2EAUX_STD80COL;

#endif  /* __A2EAUX_STD80COL__ */
