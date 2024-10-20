// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2eramworks3.h

    Applied Engineering RamWorks III

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2ERAMWORKS3_H
#define MAME_BUS_A2BUS_A2ERAMWORKS3_H

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

protected:
	a2eaux_ramworks3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u8 read_auxram(uint16_t offset) override;
	virtual void write_auxram(uint16_t offset, u8 data) override;
	virtual u8 *get_vram_ptr() override;
	virtual u8 *get_auxbank_ptr() override;
	virtual u16 get_auxbank_mask() override;
	virtual bool allow_dhr() override { return true; }
	virtual void write_c07x(u8 offset, u8 data) override;

	int m_bank;

private:
	u8 m_ram[8*1024*1024];
};

class a2eaux_franklin384_device: public a2eaux_ramworks3_device
{
public:
	a2eaux_franklin384_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void write_c07x(u8 offset, u8 data) override;
};

class a2eaux_franklin512_device: public a2eaux_ramworks3_device
{
public:
	a2eaux_franklin512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void write_c07x(u8 offset, u8 data) override;
};

// device type definition
DECLARE_DEVICE_TYPE(A2EAUX_RAMWORKS3, a2eaux_ramworks3_device)
DECLARE_DEVICE_TYPE(A2EAUX_FRANKLIN384, a2eaux_franklin384_device)
DECLARE_DEVICE_TYPE(A2EAUX_FRANKLIN512, a2eaux_franklin512_device)

#endif // MAME_BUS_A2BUS_A2ERAMWORKS3_H
