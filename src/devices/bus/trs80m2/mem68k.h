// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 16/16B/6000 68000 memory board emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_MEM68K_H
#define MAME_BUS_TRS80M2_MEM68K_H

#pragma once

#include "slot.h"


class trs80m2_mem68k_device_base : public device_t, public device_trs80m2_card_interface
{
public:
	// device_trs80m2_card_interface implementation
	virtual void m68k_map(address_space &space) override;

protected:
	trs80m2_mem68k_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual offs_t base() = 0;

	required_ioport m_s1;
	offs_t const m_size;

private:
	memory_share_creator<uint16_t> m_ram;
};


class trs80m2_mem68k_device : public trs80m2_mem68k_device_base
{
protected:
	trs80m2_mem68k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual offs_t base() override;
};


class trs80m2_mem128k_device : public trs80m2_mem68k_device
{
public:
	trs80m2_mem128k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class trs80m2_mem256k_device : public trs80m2_mem68k_device
{
public:
	trs80m2_mem256k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class trs80m2_mem68k_t6000_device : public trs80m2_mem68k_device_base
{
protected:
	trs80m2_mem68k_t6000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual offs_t base() override;
};


class trs80m2_mem512k_device : public trs80m2_mem68k_t6000_device
{
public:
	trs80m2_mem512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class trs80m2_mem1m_device : public trs80m2_mem68k_t6000_device
{
public:
	trs80m2_mem1m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TRS80M2_MEM128K, trs80m2_mem128k_device)
DECLARE_DEVICE_TYPE(TRS80M2_MEM256K, trs80m2_mem256k_device)
DECLARE_DEVICE_TYPE(TRS80M2_MEM512K, trs80m2_mem512k_device)
DECLARE_DEVICE_TYPE(TRS80M2_MEM1M, trs80m2_mem1m_device)

#endif // MAME_BUS_TRS80M2_MEM68K_H
