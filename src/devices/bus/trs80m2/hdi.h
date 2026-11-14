// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II hard disk interface emulation

    8000143 host interface with 16K RAM, cabled to a WD1000-TB1
    controller (031095).

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_HDI_H
#define MAME_BUS_TRS80M2_HDI_H

#pragma once

#include "hdc.h"


class trs80m2_hdi_device : public trs80m2_wd1010_card_device_base
{
public:
	trs80m2_hdi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void map(address_map &map) override ATTR_COLD;
	virtual void update_drdy() override;
	virtual void intrq_w(int state) override;

private:
	uint8_t status_r();
	void hdi_control_w(uint8_t data);
	void wg_w(int state);

	bool write_protected(int drive) const;
	void update_intrq();

	required_device_array<harddisk_image_device, 4> m_hdd;

	memory_share_creator<uint8_t> m_ram;
	bool m_intrq;
	bool m_hdwpl;
	bool m_ram_installed;
};


DECLARE_DEVICE_TYPE(TRS80M2_HDI, trs80m2_hdi_device)

#endif // MAME_BUS_TRS80M2_HDI_H
