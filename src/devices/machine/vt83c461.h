// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    vt83c461.h

    VIA VT83C461 (IDE Hard Drive controller).

***************************************************************************/

#ifndef MAME_MACHINE_VT83C461_H
#define MAME_MACHINE_VT83C461_H

#pragma once

#include "idectrl.h"

class vt83c461_device : public ide_controller_32_device
{
public:
	vt83c461_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> vt83c461_device &master(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::master(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> vt83c461_device &slave(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::slave(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> vt83c461_device &options(T &&opts, const char *master_default = nullptr, const char *slave_default = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::options(std::forward<T>(opts), master_default, slave_default, fixed);
		return *this;
	}
	template <typename T> vt83c461_device &set_slot_options(int index, T &&opts, const char *dflt, bool fixed)
	{
		abstract_ata_interface_device::set_slot_options(std::forward<T>(opts), dflt, fixed);
		return *this;
	}

	uint32_t config_r(offs_t offset);
	void config_w(offs_t offset, uint32_t data);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr unsigned  IDE_CONFIG_REGISTERS= 0x10;

	uint8_t           m_config_unknown;
	uint8_t           m_config_register[IDE_CONFIG_REGISTERS];
	uint8_t           m_config_register_num;
};

DECLARE_DEVICE_TYPE(VT83C461, vt83c461_device)

#endif // MAME_MACHINE_VT83C461_H
