// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_VME_FCSCSI_H
#define MAME_BUS_VME_VME_FCSCSI_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/68230pit.h"
#include "machine/wd_fdc.h"
#include "machine/hd63450.h" // compatible with MC68450
#include "bus/vme/vme.h"

DECLARE_DEVICE_TYPE(VME_FCSCSI1, vme_fcscsi1_card_device)

class vme_fcscsi1_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_fcscsi1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fcscsi1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void update_irq_to_maincpu();

	uint8_t fdc_irq_state;
	uint8_t dmac_irq_state;

private:
	//dmac
	DECLARE_WRITE_LINE_MEMBER(dma_irq);
	uint8_t dma_iack();

	//fdc
	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_READ8_MEMBER(fdc_read_byte);
	DECLARE_WRITE8_MEMBER(fdc_write_byte);
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_READ16_MEMBER (bootvect_r);
	DECLARE_READ8_MEMBER (tcr_r);
	DECLARE_WRITE8_MEMBER (tcr_w);
	DECLARE_WRITE8_MEMBER (led_w);

	/* Dummy driver routines */
	DECLARE_READ8_MEMBER(not_implemented_r);
	DECLARE_WRITE8_MEMBER(not_implemented_w);

	DECLARE_READ8_MEMBER(scsi_r);
	DECLARE_WRITE8_MEMBER(scsi_w);

	void fcscsi1_mem(address_map &map);
	void cpu_space_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<wd1772_device> m_fdc;
	required_device<pit68230_device> m_pit;
	required_device<hd63450_device> m_dmac;

	uint8_t m_tcr;

	// Pointer to System ROMs needed by bootvect_r
	uint16_t  *m_sysrom;
};

#endif // MAME_BUS_VME_VME_FCSCSI_H
