// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Gaelco 3D serial hardware

***************************************************************************/
#ifndef MAME_GAELCO_GAELCO3D_M_H
#define MAME_GAELCO_GAELCO3D_M_H

#pragma once

#include <mutex>

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

	uint8_t status_r();
	void data_w(uint8_t data);
	uint8_t data_r();
	void rts_w(int state);
	/* Set to 1 during transmit, 0 for receive */
	void tr_w(int state);


	/* Big questions marks, related to serial i/o */

	/* Not used in surfplnt, but in radikalb
	 * Set at beginning of transfer sub, cleared at end
	 */
	void unknown_w(int state);


	/* only used in radikalb, set at beginning of receive isr, cleared at end */
	void irq_enable(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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
	emu_timer *m_status_set_timer;

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

#endif // MAME_GAELCO_GAELCO3D_M_H
