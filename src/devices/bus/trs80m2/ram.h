// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II 32K/64K memory board emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_RAM_H
#define MAME_BUS_TRS80M2_RAM_H

#pragma once

#include "slot.h"


class trs80m2_ram_device_base : public device_t, public device_trs80m2_card_interface
{
protected:
	trs80m2_ram_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int pages);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual int first_page() = 0;

	required_ioport m_config;

private:
	int const m_pages;
	memory_share_creator<uint8_t> m_ram;
	bool m_installed;
};


class trs80m2_ram32k_device : public trs80m2_ram_device_base
{
public:
	trs80m2_ram32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual int first_page() override;
};


class trs80m2_ram64k_device : public trs80m2_ram_device_base
{
public:
	trs80m2_ram64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual int first_page() override;
};


DECLARE_DEVICE_TYPE(TRS80M2_RAM32K, trs80m2_ram32k_device)
DECLARE_DEVICE_TYPE(TRS80M2_RAM64K, trs80m2_ram64k_device)

#endif // MAME_BUS_TRS80M2_RAM_H
