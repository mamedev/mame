// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_SEGAAI_EXP_H
#define MAME_BUS_SEGAAI_EXP_H

#pragma once


DECLARE_DEVICE_TYPE(SEGAAI_EXP_SLOT, segaai_exp_slot_device);


class device_segaai_exp_interface : public device_interface
{
public:
	device_segaai_exp_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_segaai_exp_interface();

	// 0x20000 - 0x3ffff
	virtual u8 read_lo(offs_t offset) { return 0xff; }
	virtual void write_lo(offs_t offset, u8 data) {}
	// 0x80000 - 0x9ffff
	virtual u8 read_hi(offs_t offset) { return 0xff; }
	virtual void write_hi(offs_t offset, u8 data) {}
	// I/O 0x20 - 0x3f
	virtual u8 read_io(offs_t offset) { return 0xff; }
	virtual void write_io(offs_t offset, u8 data) {}
};


class segaai_exp_slot_device : public device_t,
								public device_slot_interface
{
public:
	template <typename T>
	segaai_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: segaai_exp_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	segaai_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~segaai_exp_slot_device();

	virtual void device_start();

	virtual u8 read_lo(offs_t offset);
	virtual void write_lo(offs_t offset, u8 data);
	virtual u8 read_hi(offs_t offset);
	virtual void write_hi(offs_t offset, u8 data);
	virtual u8 read_io(offs_t offset);
	virtual void write_io(offs_t offset, u8 data);

protected:
	device_segaai_exp_interface*       m_exp;
};


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_SEGAAI_EXP_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, SEGAAI_EXP_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

// slot interfaces
void segaai_exp(device_slot_interface &device);

#endif
