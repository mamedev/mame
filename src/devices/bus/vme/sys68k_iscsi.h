// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_SYS68K_ISCSI_H
#define MAME_BUS_VME_SYS68K_ISCSI_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/68230pit.h"
#include "machine/wd_fdc.h"
#include "machine/hd63450.h" // compatible with MC68450
#include "bus/vme/vme.h"

DECLARE_DEVICE_TYPE(VME_SYS68K_ISCSI1, vme_sys68k_iscsi1_card_device)

class vme_sys68k_iscsi1_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_sys68k_iscsi1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_iscsi1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void update_irq_to_maincpu();

	uint8_t fdc_irq_state;
	uint8_t dmac_irq_state;

private:
	//dmac
	void dma_irq(int state);
	uint8_t dma_iack();

	//fdc
	void fdc_irq(int state);
	uint8_t fdc_read_byte();
	void fdc_write_byte(uint8_t data);

	uint8_t tcr_r();
	void tcr_w(uint8_t data);
	void led_w(uint8_t data);

	/* Dummy driver routines */
	uint8_t not_implemented_r();
	void not_implemented_w(uint8_t data);

	uint8_t scsi_r(offs_t offset);
	void scsi_w(offs_t offset, uint8_t data);

	void fcscsi1_mem(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<wd1772_device> m_fdc;
	required_device<pit68230_device> m_pit;
	required_device<hd63450_device> m_dmac;

	required_memory_region m_eprom;
	required_shared_ptr<uint16_t> m_ram;
	memory_passthrough_handler m_boot_mph;

	uint8_t m_tcr;
};

#endif // MAME_BUS_VME_SYS68K_ISCSI_H
