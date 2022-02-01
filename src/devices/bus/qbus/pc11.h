// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DEC PC11 paper tape reader and punch controller (punch not implemented)

***************************************************************************/

#ifndef MAME_BUS_QBUS_PC11_H
#define MAME_BUS_QBUS_PC11_H

#pragma once

#include "qbus.h"

#include "imagedev/papertape.h"
#include "machine/pdp11.h"


#define PTRCSR_IMP      (CSR_ERR + CSR_BUSY + CSR_DONE + CSR_IE)
#define PTRCSR_WR       (CSR_IE)
#define PTPCSR_IMP      (CSR_ERR + CSR_DONE + CSR_IE)
#define PTPCSR_WR       (CSR_IE)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> pc11_device

class pc11_device : public paper_tape_reader_device,
					public device_qbus_card_interface
{
public:
	// construction/destruction
	pc11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual const char *image_interface() const noexcept override { return "pdp11_ptap"; }
	virtual const char *file_extensions() const noexcept override { return "bin,bim,lda"; }

	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	int m_rxvec;
	int m_txvec;

	device_image_interface *m_fd;

	line_state m_rxrdy;
	line_state m_txrdy;

	uint16_t m_rcsr;
	uint16_t m_rbuf;
	uint16_t m_tcsr;
	uint16_t m_tbuf;

	const char *pc11_regnames[4];
};


// device type definition
DECLARE_DEVICE_TYPE(DEC_PC11, pc11_device)

#endif
