// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Gaelco 3D serial hardware

***************************************************************************/
#ifndef MAME_MACHINE_GAELCO3D_H
#define MAME_MACHINE_GAELCO3D_H

#pragma once


/***************************************************************************
    DEVICE INTERFACE TYPE
***************************************************************************/

/* ----- device interface ----- */

class gaelco_serial_device : public device_t
{
public:
	static constexpr unsigned EXT_STATUS_MASK = 0x03;

	gaelco_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_handler() { return m_irq_handler.bind(); }

	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE_LINE_MEMBER(rts_w);
	/* Set to 1 during transmit, 0 for receive */
	DECLARE_WRITE_LINE_MEMBER(tr_w);


	/* Big questions marks, related to serial i/o */

	/* Not used in surfplnt, but in radikalb
	 * Set at beginning of transfer sub, cleared at end
	 */
	DECLARE_WRITE_LINE_MEMBER(unknown_w);


	/* only used in radikalb, set at beginning of receive isr, cleared at end */
	DECLARE_WRITE_LINE_MEMBER(irq_enable);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

private:
	struct buf_t
	{
		volatile uint8_t data;
		volatile uint8_t stat;
		volatile int cnt;
		volatile int data_cnt;
	};

	struct shmem_t
	{
		buf_t               buf[2];
	};

	struct osd_shared_mem
	{
		char *fn;
		size_t size;
		void *ptr;
		int creator;
	};

	static osd_shared_mem *osd_sharedmem_alloc(const char *path, int create, size_t size);
	static void osd_sharedmem_free(osd_shared_mem *os_shmem);
	static void *osd_sharedmem_ptr(osd_shared_mem *os_shmem);

	static void buf_reset(buf_t *buf);

	// internal state
	devcb_write_line m_irq_handler;

	uint8_t m_status;
	int m_last_in_msg_cnt;
	int m_slack_cnt;

	emu_timer *m_sync_timer;

	buf_t *m_in_ptr;
	buf_t *m_out_ptr;
	osd_shared_mem *m_os_shmem;
	shmem_t *m_shmem;
	std::mutex m_mutex;

	TIMER_CALLBACK_MEMBER( set_status_cb );
	TIMER_CALLBACK_MEMBER( link_cb );
	void set_status(uint8_t mask, uint8_t set, int wait);
	void process_in();
	void sync_link();
};

DECLARE_DEVICE_TYPE(GAELCO_SERIAL, gaelco_serial_device)

#endif // MAME_MACHINE_GAELCO3D_H
