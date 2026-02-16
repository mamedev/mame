// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 MMC186 emulation

*********************************************************************/

#ifndef MAME_BUS_MM2_MMC186_H
#define MAME_BUS_MM2_MMC186_H

#pragma once

#include "exp.h"
#include "bus/nscsi/devices.h"
#include "bus/scsi/s1410.h"
#include "bus/scsi/scsihd.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"
#include "machine/upd765.h"

DECLARE_DEVICE_TYPE(NOKIA_MMC186, mmc186_device)

class mmc186_device : public device_t, public device_mikromikko2_expansion_bus_card_interface
{
public:
	mmc186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	
protected:
    virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	
    virtual void device_start() override ATTR_COLD;
    virtual void device_reset() override ATTR_COLD;

private:
    void map(address_map &map) ATTR_COLD;

	void int_w(int state) { m_bus->ir4_w(state); }
    void hold_w(int state) { m_bus->hold3_w(state); }

    required_device<am9517a_device> m_dmac;
    required_device<nscsi_callback_device> m_sasi;
    required_device<upd765a_device> m_fdc;
	optional_device_array<floppy_image_device, 2> m_floppy;

	static void floppy_formats(format_registration &fr);

	uint8_t dmac_mem_r(offs_t offset);
	void dmac_mem_w(offs_t offset, uint8_t data);
	void dma_hi_w(offs_t offset, uint8_t data) { m_dma_hi = data & 0x0f; }
	uint8_t sasi_status_r(offs_t offset);
	void sasi_cmd_w(offs_t offset, uint8_t data);
	uint8_t sasi_data_r(offs_t offset);
	void sasi_data_w(offs_t offset, uint8_t data);
	void sasi_bsy_w(int state);
	void sasi_req_w(int state);
	void sasi_io_w(int state);
	void fdc_reset_w(offs_t offset, uint8_t data) { m_fdc->reset_w(!BIT(data, 0)); }
	void motor_on_w(offs_t offset, uint8_t data) { m_floppy[0]->mon_w(!BIT(data, 0)); }

	u8 m_dma_hi;
	u8 m_sasi_data;
};

#endif // MAME_BUS_MM2_MMC186_H
