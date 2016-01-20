// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
 * ncr5394/5396.h SCSI controller
 *
 */

#ifndef _NCR539x_H_
#define _NCR539x_H_

#include "legscsi.h"

//// 539x registers
//enum
//{
//};

// device stuff

#define MCFG_NCR539X_OUT_IRQ_CB(_devcb) \
	devcb = &ncr539x_device::set_out_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_NCR539X_OUT_DRQ_CB(_devcb) \
	devcb = &ncr539x_device::set_out_drq_callback(*device, DEVCB_##_devcb);

class ncr539x_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	ncr539x_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_irq_callback(device_t &device, _Object object) { return downcast<ncr539x_device &>(device).m_out_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_drq_callback(device_t &device, _Object object) { return downcast<ncr539x_device &>(device).m_out_drq_cb.set_callback(object); }

	// our API
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void dma_read_data(int bytes, UINT8 *pData);
	void dma_write_data(int bytes, UINT8 *pData);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void fifo_write(UINT8 data);
	void check_fifo_executable();
	void exec_fifo();
	void update_fifo_internal_state(int bytes);

	UINT32 m_xfer_count;
	UINT32 m_dma_size;
	UINT8 m_command;
	UINT8 m_last_id;
	UINT8 m_timeout;
	UINT8 m_sync_xfer_period;
	UINT8 m_sync_offset;
	UINT8 m_control1, m_control2, m_control3, m_control4;
	UINT8 m_clock_factor;
	UINT8 m_forced_test;
	UINT8 m_data_alignment;

	bool m_selected;
	bool m_chipid_available, m_chipid_lock;

	static const int m_fifo_size = 16;
	UINT8 m_fifo_ptr, m_fifo_read_ptr, m_fifo[m_fifo_size];

	//int m_xfer_remaining;   // amount in the FIFO when we're in data in phase

	// read-only registers
	UINT8 m_status, m_irq_status, m_internal_state, m_fifo_internal_state;

	static const int m_buffer_size = 2048;

	UINT8 m_buffer[m_buffer_size];
	int m_buffer_offset, m_buffer_remaining, m_total_data;

	emu_timer *m_operation_timer;

	devcb_write_line m_out_irq_cb;          /* IRQ line */
	devcb_write_line m_out_drq_cb;          /* DRQ line */
};

// device type definition
extern const device_type NCR539X;
#endif
