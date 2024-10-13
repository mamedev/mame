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

	// device_image_interface implementation
	virtual const char *image_interface() const noexcept override { return "pdp11_ptap"; }
	virtual const char *file_extensions() const noexcept override { return "bin,bim,lda"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	TIMER_CALLBACK_MEMBER(read_tick);

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

	emu_timer *m_read_timer;

	const char *pc11_regnames[4];
};


// device type definition
DECLARE_DEVICE_TYPE(DEC_PC11, pc11_device)

#endif // MAME_BUS_QBUS_PC11_H
