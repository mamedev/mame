// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

	DEC PC11 paper tape reader and punch controller (punch not implemented)

***************************************************************************/

#pragma once

#ifndef __PC11__
#define __PC11__

#include "emu.h"

#include "qbus.h"

#include "includes/pdp11.h"
#include "softlist_dev.h"


#define PTRCSR_IMP      (CSR_ERR + CSR_BUSY + CSR_DONE + CSR_IE)
#define PTRCSR_WR       (CSR_IE)
#define PTPCSR_IMP      (CSR_ERR + CSR_DONE + CSR_IE)
#define PTPCSR_WR       (CSR_IE)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> pc11_device

class pc11_device : public device_t,
					public device_image_interface,
					public device_qbus_card_interface
{
public:
	// construction/destruction
	pc11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual iodevice_t image_type() const noexcept override { return IO_PUNCHTAPE; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "pdp11_ptap"; }
	virtual const char *file_extensions() const noexcept override { return "bin,bim,lda"; }

	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return image_software_list_loader::instance(); }

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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
