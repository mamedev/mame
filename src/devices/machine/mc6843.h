// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2007

  Motorola 6843 Floppy Disk Controller emulation.

**********************************************************************/

#ifndef MC6843_H
#define MC6843_H

#include "imagedev/flopdrv.h"

#define MCFG_MC6843_IRQ_CALLBACK(_write) \
	devcb = &mc6843_device::set_irq_wr_callback(*device, DEVCB_##_write);

class mc6843_device : public device_t
{
public:
	mc6843_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~mc6843_device() {}

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<mc6843_device &>(device).m_write_irq.set_callback(object); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_drive(int drive);
	void set_side(int side);
	void set_index_pulse(int index_pulse);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_CONT
	};

	devcb_write_line m_write_irq;

	/* registers */
	UINT8 m_CTAR;       /* current track */
	UINT8 m_CMR;        /* command */
	UINT8 m_ISR;        /* interrupt status */
	UINT8 m_SUR;        /* set-up */
	UINT8 m_STRA;       /* status */
	UINT8 m_STRB;       /* status */
	UINT8 m_SAR;        /* sector address */
	UINT8 m_GCR;        /* general count */
	UINT8 m_CCR;        /* CRC control */
	UINT8 m_LTAR;       /* logical address track (=track destination) */

	/* internal state */
	UINT8  m_drive;
	UINT8  m_side;
	UINT8  m_data[128];   /* sector buffer */
	UINT32 m_data_size;   /* size of data */
	UINT32 m_data_idx;    /* current read/write position in data */
	UINT32 m_data_id;     /* chrd_id for sector write */
	UINT8  m_index_pulse;

	/* trigger delayed actions (bottom halves) */
	emu_timer* m_timer_cont;

	legacy_floppy_image_device* floppy_image(UINT8 drive);
	legacy_floppy_image_device* floppy_image();
	void status_update();
	void cmd_end();
	void finish_STZ();
	void finish_SEK();
	int address_search(chrn_id* id);
	int address_search_read(chrn_id* id);
	void finish_RCR();
	void cont_SR();
	void cont_SW();

};

extern const device_type MC6843;

#endif
