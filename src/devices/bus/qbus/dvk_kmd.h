// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DVK KMD floppy controller (device driver MY.SYS)

***************************************************************************/

#ifndef MAME_BUS_QBUS_DVK_KMD_H
#define MAME_BUS_QBUS_DVK_KMD_H

#pragma once

#include "qbus.h"

#include "cpu/t11/t11.h"
#include "machine/1801vp128.h"
#include "machine/pdp11.h"
#include "machine/z80daisy.h"

#include "formats/bk0010_dsk.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> dvk_kmd_device

class dvk_kmd_device : public device_t,
					public device_qbus_card_interface,
					public device_z80daisy_interface
{
public:
	// construction/destruction
	dvk_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

	virtual void init_w() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

	void kmd_mem(address_map &map) ATTR_COLD;

	required_device<k1801vm1_device> m_maincpu;
	required_device<k1801vp128_device> m_fdc;

private:
	static constexpr uint16_t KMDCSR_GO      = 0001;
	static constexpr uint16_t KMDCSR_DONE    = 0040;
	static constexpr uint16_t KMDCSR_TR      = 0200;
	static constexpr uint16_t KMDCSR_RD      = CSR_ERR | KMDCSR_TR | CSR_IE | KMDCSR_DONE;
	static constexpr uint16_t KMDCSR_WR      = CSR_IE | 077437;
	static constexpr uint16_t KMDCSR_L_WR    = CSR_ERR | KMDCSR_TR | KMDCSR_DONE;

	bool m_installed;
	line_state m_rxrdy;

	uint16_t m_cr;
	uint16_t m_go;
	uint16_t m_dr;

	static void floppy_formats(format_registration &fr);

	uint16_t local_read(offs_t offset);
	void local_write(offs_t offset, uint16_t data);

	uint16_t dma_read(offs_t offset);
	void dma_write(offs_t offset, uint16_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(DVK_KMD, dvk_kmd_device)

#endif // MAME_BUS_QBUS_DVK_KMD_H
