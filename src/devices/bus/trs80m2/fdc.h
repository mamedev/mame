// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II floppy disk controller/printer interface emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_FDC_H
#define MAME_BUS_TRS80M2_FDC_H

#pragma once

#include "slot.h"

#include "bus/centronics/ctronics.h"
#include "imagedev/floppy.h"
#include "machine/output_latch.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"


class trs80m2_fdc_device : public device_t, public device_trs80m2_card_interface
{
public:
	trs80m2_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_trs80m2_card_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	void map(address_map &map) ATTR_COLD;

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	void drvslt_w(uint8_t data);
	void select_floppy();
	uint8_t pio_pa_r();
	void pio_pa_w(uint8_t data);

	required_device<fd1791_device> m_fdc;
	required_device<z80pio_device> m_pio;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device_array<floppy_connector, 4> m_floppies;
	floppy_image_device *m_floppy;

	uint8_t m_drvslt;
	uint8_t m_centronics_busy;
	uint8_t m_centronics_fault;
	uint8_t m_centronics_perror;
};


DECLARE_DEVICE_TYPE(TRS80M2_FDC, trs80m2_fdc_device)

#endif // MAME_BUS_TRS80M2_FDC_H
