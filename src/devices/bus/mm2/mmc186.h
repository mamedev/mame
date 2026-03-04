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
#include "machine/74259.h"
#include "machine/am9517a.h"
#include "machine/input_merger.h"
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

	virtual void bhlda_w(int state, int bcas) override;

private:
	void map(address_map &map) ATTR_COLD;

	void int_w(int state) { m_bus->ir4_w(state); }
	void hold_w(int state) { m_bus->hold3_w(state); }

	required_device<am9517a_device> m_dmac;
	required_device<nscsi_callback_device> m_sasi;
	required_device<upd765a_device> m_fdc;
	required_device<ls259_device> m_ctrl;
	required_device<input_merger_device> m_irqs;
	optional_device_array<floppy_image_device, 2> m_floppy;

	static void floppy_formats(format_registration &fr);

	uint8_t dmac_mem_r(offs_t offset) { return m_bus->memspace().read_byte(m_dma_hi << 16 | offset); }
	void dmac_mem_w(offs_t offset, uint8_t data) { m_bus->memspace().write_byte(m_dma_hi << 16 | offset, data); }
	void dma_hi_w(offs_t offset, uint8_t data) { m_dma_hi = data & 0x0f; }
	uint8_t sasi_status_r(offs_t offset);
	void sasi_cmd_w(offs_t offset, uint8_t data);
	uint8_t sasi_data_r(offs_t offset);
	void sasi_data_w(offs_t offset, uint8_t data);
	void sasi_bsy_w(int state);
	void sasi_req_w(int state);
	void sasi_io_w(int state);
	void eop_w(int state) { if (m_dma_ch == 1) m_fdc->tc_w(state); }
	void dack0_w(int state) { set_dma_channel(0, state); if (!state) m_dmac->dreq0_w(CLEAR_LINE); };
	void dack1_w(int state) { set_dma_channel(1, state); };
	void set_dma_channel(int channel, int state) { if (state) m_dma_ch = -1; else m_dma_ch = channel; }
	void sasi_select_w(int state);
	void update_sasi_irq() { m_irqs->in_w<0>(m_sasi_irq && m_sasi_ie); }
	void sasi_ie_w(int state) { m_sasi_ie = state; update_sasi_irq(); }
	void motor_on_w(int state) { m_floppy[0]->mon_w(!state); if (m_floppy[1]) m_floppy[1]->mon_w(!state); }

	int m_dma_ch;
	u8 m_dma_hi;

	bool m_sasi_select;
	u8 m_sasi_data;
	bool m_sasi_ie;
	bool m_sasi_irq;
	bool m_sasi_req;
};

#endif // MAME_BUS_MM2_MMC186_H
